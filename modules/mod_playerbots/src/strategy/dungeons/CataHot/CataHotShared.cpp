/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight bot strategy "cata-hot" - helpers (issue #673, epic #373). See CataHotShared.h for the sources.

#include "CataHotShared.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "TemporarySummon.h"
#include "ThreatManager.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>

namespace HourOfTwilight
{
namespace
{
constexpr float GROUND_TIER_STEP   = 3.0f;
constexpr float GROUND_SEARCH_UP   = 5.0f;
constexpr float GROUND_SEARCH_DOWN = 12.0f;
constexpr float ESCAPE_LOS_HEIGHT  = 2.0f;
constexpr float ESCAPE_RADII[]     = { 4.0f, 7.0f, 10.0f, 13.0f };
constexpr int   ESCAPE_BEARINGS    = 16;

// The wave damages every player within 6 yd of itself once a second (npc_twilight_light_wave::GetPlayers,
// boss_archbishop_benedictus.cpp:549-565) while it walks a straight line to WaveDespawnPoint (:497, hour_of_twilight.h:34).
constexpr float WAVE_HIT_RADIUS   = 7.5f;
constexpr float WAVE_CLEAR_RADIUS = 9.5f;

// Righteous / Twilight Shear: "inflicts damage to enemies within 10 yards of the target per stack" (Wowpedia
// "Archbishop Benedictus (tactics)").
constexpr float SHEAR_RADIUS = 10.5f;

struct WaveLine
{
    float x;
    float y;
    float ux;
    float uy;
    float remaining;
};

void CollectWaveLines(Player* bot, std::vector<WaveLine>& out)
{
    std::vector<Creature*> waves;
    CollectCreatures(bot, { HOT_NPC_WAVE_OF_VIRTUE, HOT_NPC_WAVE_OF_TWILIGHT }, 150.0f, waves);
    for (Creature* wave : waves)
    {
        // The MoP world DB has two static Wave of Twilight rows (3479,244,-119) that never move: only a walking wave counts.
        if (!wave->isMoving())
            continue;

        float const dx = HOT_WAVE_DESPAWN_X - wave->GetPositionX();
        float const dy = HOT_WAVE_DESPAWN_Y - wave->GetPositionY();
        float const len = std::hypot(dx, dy);
        if (len < 2.0f)
            continue;

        out.push_back({ wave->GetPositionX(), wave->GetPositionY(), dx / len, dy / len, len });
    }
}

// Clearance of (x, y) from every wave path ahead of / around the wave: negative while inside WAVE_CLEAR_RADIUS of the path.
float WaveMargin(std::vector<WaveLine> const& lines, float x, float y)
{
    float margin = std::numeric_limits<float>::max();
    for (WaveLine const& line : lines)
    {
        float const dx = x - line.x;
        float const dy = y - line.y;
        float const along = dx * line.ux + dy * line.uy;
        if (along < -8.0f || along > line.remaining + 4.0f)
            continue;

        float const perp = std::fabs(-dx * line.uy + dy * line.ux);
        margin = std::min(margin, perp - WAVE_CLEAR_RADIUS);
    }

    return margin;
}

bool InsideWavePath(std::vector<WaveLine> const& lines, float x, float y)
{
    for (WaveLine const& line : lines)
    {
        float const dx = x - line.x;
        float const dy = y - line.y;
        float const along = dx * line.ux + dy * line.uy;
        if (along < -8.0f || along > line.remaining + 4.0f)
            continue;

        if (std::fabs(-dx * line.uy + dy * line.ux) < WAVE_HIT_RADIUS)
            return true;
    }

    return false;
}

float HazardMargin(std::vector<Hazard> const& hazards, float x, float y)
{
    float margin = std::numeric_limits<float>::max();
    for (Hazard const& hazard : hazards)
        margin = std::min(margin, std::hypot(x - hazard.x, y - hazard.y) - hazard.clear);

    return margin;
}

char const* const INTERRUPT_NAMES[] =
{
    "kick", "pummel", "wind shear", "mind freeze", "counterspell", "spell lock", "counter shot",
    "rebuke", "skull bash", "spear hand strike", "silence"
};
}

// ==== generic helpers (identical in CataEt / CataWoe / CataHot Shared) ======================================================

void CollectCreatures(Player* bot, std::initializer_list<uint32> entries, float radius, std::vector<Creature*>& out)
{
    // 5.4.8: WorldObject::GetCreatureListWithEntryInGridAppend takes one entry per call (Object.h:577).
    std::list<Creature*> found;
    for (uint32 entry : entries)
        bot->GetCreatureListWithEntryInGridAppend(found, entry, radius);

    for (Creature* creature : found)
    {
        if (creature && creature->IsAlive())
            out.push_back(creature);
    }
}

bool InsideAnyHazard(std::vector<Hazard> const& hazards, float x, float y)
{
    for (Hazard const& hazard : hazards)
    {
        float const dx = x - hazard.x;
        float const dy = y - hazard.y;
        if (dx * dx + dy * dy < hazard.stand * hazard.stand)
            return true;
    }

    return false;
}

// Nearest ring of candidate spots around the bot that has one with margin >= 0, on the bot's own ground tier and in
// line of sight (same checks as the MgT escape spots, MgTShared.cpp:370-390). The best margin of that ring wins.
bool FindClearSpot(Player* bot, std::function<float(float, float)> const& margin, float& outX, float& outY, float& outZ)
{
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const bz = bot->GetPositionZ();

    for (float radius : ESCAPE_RADII)
    {
        bool found = false;
        float best = 0.0f;
        for (int i = 0; i < ESCAPE_BEARINGS; ++i)
        {
            float const angle = (2.0f * static_cast<float>(M_PI) * i) / ESCAPE_BEARINGS;
            float const x = bx + std::cos(angle) * radius;
            float const y = by + std::sin(angle) * radius;
            float const m = margin(x, y);
            if (m < 0.0f)
                continue;

            float const z = bot->GetMapHeight(x, y, bz + GROUND_SEARCH_UP, true, GROUND_SEARCH_DOWN);
            if (!(std::fabs(z - bz) <= GROUND_TIER_STEP))
                continue;

            if (!bot->IsWithinLOS(x, y, z + ESCAPE_LOS_HEIGHT))
                continue;

            if (!found || m > best)
            {
                found = true;
                best = m;
                outX = x;
                outY = y;
                outZ = z;
            }
        }

        if (found)
            return true;
    }

    return false;
}

bool FindHazardEscape(Player* bot, std::vector<Hazard> const& hazards, float& outX, float& outY, float& outZ)
{
    if (hazards.empty() || !InsideAnyHazard(hazards, bot->GetPositionX(), bot->GetPositionY()))
        return false;

    return FindClearSpot(bot, [&hazards](float x, float y) { return HazardMargin(hazards, x, y); }, outX, outY, outZ);
}

// Dedicated interrupts only (no stuns); CanCastSpell is false for a spell the bot does not know, on cooldown or out of range.
std::string CastInterrupt(PlayerbotAI* botAI, Unit* target)
{
    if (!target || !target->IsNonMeleeSpellCasted(true))
        return {};

    for (char const* name : INTERRUPT_NAMES)
    {
        if (botAI->CanCastSpell(name, target) && botAI->CastSpell(name, target))
            return name;
    }

    return {};
}

void CollectBotMembers(Player* bot, std::vector<Player*>& out)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        if (bot->IsAlive())
            out.push_back(bot);
        return;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (member != bot && !GET_PLAYERBOT_AI(member))
            continue;

        out.push_back(member);
    }
}

Player* GetDesignatedBot(Player* bot)
{
    std::vector<Player*> members;
    CollectBotMembers(bot, members);

    Player* best = nullptr;
    for (Player* member : members)
    {
        if (!best)
        {
            best = member;
            continue;
        }

        bool const tank = PlayerBotSpec::IsTank(member);
        bool const bestTank = PlayerBotSpec::IsTank(best);
        if (tank != bestTank)
        {
            if (tank)
                best = member;
            continue;
        }

        if (member->GetGUID() < best->GetGUID())
            best = member;
    }

    return best;
}

// ==== hazards ===================================================================================================================

void CollectHazards(Player* bot, HazardKind kind, std::vector<Hazard>& out)
{
    std::vector<Creature*> found;
    switch (kind)
    {
        case HazardKind::SmokeBomb:
            // "Creates a cloud of thick smoke in an 8 yard radius around the caster ... Enemies are unable to target into or out
            // of the smoke cloud and will take 5000 Nature damage every 0.5 seconds" (Wowhead spell 103558); the tank must move
            // Asira out of it (Wowpedia "Asira Dawnslayer").
            for (Position const& pos : EncounterHelpers::GetDynamicObjectPositions(bot, 30.0f, HOT_SPELL_CHOKING_SMOKE_BOMB))
                out.push_back({ pos.GetPositionX(), pos.GetPositionY(), 8.0f, 10.5f });
            break;
        case HazardKind::LightOrb:
            // Three orbs fly from above Benedictus to a random player's spot; a landed orb carries the damage aura
            // (boss_archbishop_benedictus.cpp:457-470), "80000 damage within 6 yards" (Wowpedia). Orbs still hovering over the
            // boss (not moving, no aura yet) are no hazard.
            CollectCreatures(bot, { HOT_NPC_PURIFYING_LIGHT, HOT_NPC_TWILIGHT_BLAST }, 60.0f, found);
            for (Creature* creature : found)
            {
                if (creature->isMoving() || creature->HasAura(HOT_SPELL_PURIFIED_LIGHT) ||
                    creature->HasAura(HOT_SPELL_TWILIGHT_ORB))
                    out.push_back({ creature->GetPositionX(), creature->GetPositionY(), 7.0f, 9.5f });
            }
            break;
        case HazardKind::IcyBoulder:
            // #696: the glyph. A Frozen Servitor marks a player's spot with a World Trigger and lands Icy Boulder (102199, 2.5 yd,
            // ~20000 Frost) on it after a 3 s cast (boss_arcurion.cpp npc_arcurion_frozen_servitor; Wowhead 102198 / 102199). Only
            // markers a servitor summoned count; 4.5 / 6.5 yd covers the 2.5 yd radius plus the bot's own reach.
            CollectCreatures(bot, { HOT_NPC_GLYPH_MARKER }, 40.0f, found);
            for (Creature* creature : found)
            {
                TempSummon* summon = creature->ToTempSummon();
                if (summon && summon->GetSummonerGUID().GetEntry() == HOT_NPC_FROZEN_SERVITOR)
                    out.push_back({ creature->GetPositionX(), creature->GetPositionY(), 4.5f, 6.5f });
            }
            break;
    }
}

bool InHazard(Player* bot, HazardKind kind)
{
    std::vector<Hazard> hazards;
    CollectHazards(bot, kind, hazards);
    return InsideAnyHazard(hazards, bot->GetPositionX(), bot->GetPositionY());
}

// ==== Arcurion ==================================================================================================================

Creature* GetArcurion(Player* bot)
{
    return bot->FindNearestCreature(HOT_NPC_ARCURION, 150.0f, true);
}

Creature* GetIcyTomb(Player* bot)
{
    return bot->FindNearestCreature(HOT_NPC_ICY_TOMB, 90.0f, true);
}

// Icy Tomb (boss_arcurion.cpp:217-224): the tomb freezes Thrall, who otherwise keeps the Frozen Servitors in check; "all DPS
// should immediately switch to the ice block" (Wowpedia "Arcurion"). The tomb is REACT_PASSIVE and never attacks, so the generic
// target values do not pick it. Healers keep healing.
// #673 (w65): the tank stays on Arcurion too ("all DPS" switch, Wowpedia "Arcurion", MediaWiki API 2026-10-15). In w65 the tank
// went to the tomb and "tank assist" took it back every other tick (Amelena: hot kill icy tomb / tank alternating).
bool NeedsIcyTombKill(Player* bot)
{
    if (PlayerBotSpec::IsHeal(bot) || PlayerBotSpec::IsTank(bot))
        return false;

    Creature* arcurion = GetArcurion(bot);
    if (!arcurion || !arcurion->IsInCombat())
        return false;

    return GetIcyTomb(bot) != nullptr;
}

// The Frozen Servitors stand on the canyon rim 26-65 yd above the fight (boss_arcurion.cpp:62-70, z 92-132 against the arena
// at z ~66): melee cannot reach them and Thrall deals with them (Wowpedia "Arcurion").
void CollectServitors(Player* bot, std::vector<Creature*>& out)
{
    CollectCreatures(bot, { HOT_NPC_FROZEN_SERVITOR }, 200.0f, out);
}

// ==== Thrall's escorts ==========================================================================================================

namespace
{
constexpr float ESCORT_THRALL_RANGE   = 60.0f;   // Thrall this close to the bot (the driver keeps the master behind him)
constexpr float ESCORT_AMBUSHER_RANGE = 45.0f;   // an ambusher this close to the bot
constexpr float ESCORT_RIM_HEIGHT     = 10.0f;   // same split as the strategy's servitor exclusion (CataHotStrategy.cpp)
}

// #673 (w61-w64): the escort ambushes (Frozen Servitors / Crystalline Elemental at the 55389 / 55384 / 54560 stalkers,
// hour_of_twilight.cpp npc_frozen_servitor_stalker; Twilight's Hammer waves that SetInCombatWith Thrall 54972,
// npc_twilights_hammer EVENT_COMBAT) fight THRALL, so they never reach the bot's "attackers" value (AttackersValue.cpp:99-116
// only takes creatures that have a group member on their threat list) and the bots stood by while Thrall soloed them:
// escort-1 took 547-626 s (w61-w64) for a 170 yd walk. Wowpedia "Arcurion" (MediaWiki API, 2026-10-15): "Another ambush. Watch
// your back!" ... "Intro after killing trash: Take a moment to catch your breath before we move on" - the party kills the
// ambush. AC has no escort dungeon edge of this kind (its HoS / CoS strategies have none); this is a MoP-only dungeon edge in
// AC's dungeon-strategy pattern. The nearest live unit that has Thrall on its threat list (or is his victim), on the bot's
// ground tier, not a boss and not the Icy Tomb.
Unit* GetEscortAmbusher(Player* bot)
{
    if (!bot || bot->GetMapId() != HOT_MAP_HOUR_OF_TWILIGHT)
        return nullptr;

    std::vector<Creature*> thralls;
    CollectCreatures(bot, { HOT_NPC_THRALL_1, HOT_NPC_THRALL_2, HOT_NPC_THRALL_3, HOT_NPC_THRALL_4 }, ESCORT_THRALL_RANGE,
                     thralls);

    Unit* best = nullptr;
    float bestDist = ESCORT_AMBUSHER_RANGE;
    auto consider = [&](Unit* unit)
    {
        if (!unit || unit == best || !unit->IsAlive() || !unit->IsInWorld() || unit->GetMapId() != bot->GetMapId())
            return;

        if (Creature* creature = unit->ToCreature())
        {
            uint32 const entry = creature->GetEntry();
            if (entry == HOT_NPC_ARCURION || entry == HOT_NPC_ASIRA || entry == HOT_NPC_BENEDICTUS || entry == HOT_NPC_ICY_TOMB)
                return;
        }

        if (unit->GetPositionZ() - bot->GetPositionZ() > ESCORT_RIM_HEIGHT)
            return;

        float const dist = bot->GetDistance(unit);
        if (dist >= bestDist)
            return;

        if (!bot->IsValidAttackTarget(unit) || !bot->IsWithinLOSInMap(unit))
            return;

        best = unit;
        bestDist = dist;
    };

    for (Creature* thrall : thralls)
    {
        if (!thrall->IsInCombat())
            continue;

        consider(thrall->GetVictim());
        for (auto const& [guid, ref] : thrall->GetThreatManager().GetThreatenedByMeList())
            consider(ref ? ref->GetOwner() : nullptr);
    }

    return best;
}

// Only while the bot has nothing alive to fight: a bot already on a target keeps it (the generic target values pick the
// ambushers up as attackers once a group member is on their threat list). Healers keep healing.
bool NeedsEscortAssist(Player* bot)
{
    if (!bot || bot->GetMapId() != HOT_MAP_HOUR_OF_TWILIGHT || !bot->IsAlive() || PlayerBotSpec::IsHeal(bot))
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;

    Unit* current = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (current && current->IsAlive() && bot->IsValidAttackTarget(current))
        return false;

    return GetEscortAmbusher(bot) != nullptr;
}

// ==== Asira Dawnslayer ==========================================================================================================

namespace
{
constexpr float KNIFE_COVER_WIDTH  = 0.8f;   // the knife takes the first player within 1 yd of the line (IsInBetween, 1.0f)
constexpr float KNIFE_COVER_BEHIND = 8.0f;   // stand this far behind the tank on Asira's line

Player* GetKnifeCoverTank(Player* bot)
{
    std::vector<Player*> members;
    CollectBotMembers(bot, members);
    for (Player* member : members)
        if (member != bot && PlayerBotSpec::IsTank(member) && !member->HasAura(HOT_SPELL_MARK_OF_SILENCE) &&
            bot->GetExactDist2d(member) < 60.0f)
            return member;

    return nullptr;
}
}

// #696 (#673): Asira marks the casters (Mark of Silence 102726, spell_asira_dawnslayer_mark_of_silence) and throws a knife at a
// marked player who casts (CheckCast); the knife hits the first player within 1 yd of the line and silences only a marked one
// (spell_asira_dawnslayer_throw_knife). Wowpedia "Asira Dawnslayer" (MediaWiki API, 2026-10-22): "If you are the target of Mark
// of Silence, position yourself so that a non-marked player (preferably the tank) is between you and Asira". A marked bot with a
// tank that does not cover it moves onto the line behind the tank.
bool NeedsKnifeCover(Player* bot)
{
    if (!bot || !bot->IsAlive() || !bot->HasAura(HOT_SPELL_MARK_OF_SILENCE) || PlayerBotSpec::IsTank(bot))
        return false;

    Creature* asira = bot->FindNearestCreature(HOT_NPC_ASIRA, 60.0f, true);
    if (!asira || !asira->IsInCombat())
        return false;

    Player* tank = GetKnifeCoverTank(bot);
    if (!tank || tank->IsInBetween(asira, bot, KNIFE_COVER_WIDTH))
        return false;

    // Only with a reachable spot: the hazard multiplier holds every other move while this is true.
    float x = 0.0f, y = 0.0f, z = 0.0f;
    return GetKnifeCoverSpot(bot, x, y, z);
}

bool GetKnifeCoverSpot(Player* bot, float& outX, float& outY, float& outZ)
{
    Creature* asira = bot->FindNearestCreature(HOT_NPC_ASIRA, 60.0f, true);
    Player* tank = asira ? GetKnifeCoverTank(bot) : nullptr;
    if (!tank)
        return false;

    float const dx = tank->GetPositionX() - asira->GetPositionX();
    float const dy = tank->GetPositionY() - asira->GetPositionY();
    float const len = std::hypot(dx, dy);
    if (len < 0.5f)
        return false;

    float const x = tank->GetPositionX() + dx / len * KNIFE_COVER_BEHIND;
    float const y = tank->GetPositionY() + dy / len * KNIFE_COVER_BEHIND;
    float const z = bot->GetMapHeight(x, y, tank->GetPositionZ() + GROUND_SEARCH_UP, true, GROUND_SEARCH_DOWN);
    if (!(std::fabs(z - tank->GetPositionZ()) <= GROUND_TIER_STEP) || !bot->IsWithinLOS(x, y, z + ESCAPE_LOS_HEIGHT))
        return false;

    outX = x;
    outY = y;
    outZ = z;
    return true;
}

// ==== Archbishop Benedictus =====================================================================================================

namespace
{
constexpr float SHELL_STAND = 2.5f;   // Water Shell pulse 103744 reaches 4 yd around Thrall (SpellEffect.dbc radius index 26)
}

// #696: phase 1 only - a Wave of Virtue walks and Thrall 54971 can shell (not held by Engulfing Twilight in phase 2).
Creature* GetShellThrall(Player* bot)
{
    if (!bot || bot->GetMapId() != HOT_MAP_HOUR_OF_TWILIGHT)
        return nullptr;

    Creature* benedictus = bot->FindNearestCreature(HOT_NPC_BENEDICTUS, 100.0f, true);
    if (!benedictus || !benedictus->IsInCombat())
        return nullptr;

    Creature* thrall = bot->FindNearestCreature(HOT_NPC_THRALL_5, 80.0f, true);
    if (!thrall || thrall->HasUnitState(UNIT_STATE_STUNNED))
        return nullptr;

    std::vector<Creature*> waves;
    CollectCreatures(bot, { HOT_NPC_WAVE_OF_VIRTUE }, 150.0f, waves);
    for (Creature* wave : waves)
        if (wave->isMoving())
            return thrall;

    return nullptr;
}

// Wowpedia "Archbishop Benedictus (tactics)" (MediaWiki API, 2026-10-22): "When Benedictus summons the Wave of Virtue, all party
// members should move to the Water Shell Thrall creates regardless of where the wave is traveling".
bool NeedsWaterShell(Player* bot)
{
    if (!bot || !bot->IsAlive())
        return false;

    Creature* thrall = GetShellThrall(bot);
    return thrall && bot->GetExactDist2d(thrall) > SHELL_STAND;
}

bool InWavePath(Player* bot)
{
    // #696: the shell buff (103744), not Thrall's own channel (103688), is what the wave exempts; while Thrall can shell, the
    // bot goes to him instead of stepping aside ("hot water shell").
    if (bot->HasAura(HOT_SPELL_WATER_SHELL_BUFF) || GetShellThrall(bot))
        return false;

    std::vector<WaveLine> lines;
    CollectWaveLines(bot, lines);
    return !lines.empty() && InsideWavePath(lines, bot->GetPositionX(), bot->GetPositionY());
}

bool GetWaveEscape(Player* bot, float& outX, float& outY, float& outZ)
{
    std::vector<WaveLine> lines;
    CollectWaveLines(bot, lines);
    if (lines.empty() || !InsideWavePath(lines, bot->GetPositionX(), bot->GetPositionY()))
        return false;

    return FindClearSpot(bot, [&lines](float x, float y) { return WaveMargin(lines, x, y); }, outX, outY, outZ);
}

Player* GetNearestShearVictimNeighbour(Player* bot)
{
    std::vector<Player*> members;
    CollectBotMembers(bot, members);

    Player* closest = nullptr;
    float closestDist = SHEAR_RADIUS;
    for (Player* member : members)
    {
        if (member == bot)
            continue;

        float const dist = bot->GetExactDist2d(member);
        if (dist < closestDist)
        {
            closest = member;
            closestDist = dist;
        }
    }

    return closest;
}

bool NeedsShearSpread(Player* bot)
{
    if (!bot->HasAura(HOT_SPELL_RIGHTEOUS_SHEAR) && !bot->HasAura(HOT_SPELL_TWILIGHT_SHEAR))
        return false;

    return GetNearestShearVictimNeighbour(bot) != nullptr;
}
}
