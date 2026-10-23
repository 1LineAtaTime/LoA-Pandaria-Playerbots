/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// End Time bot strategy "cata-et" - helpers (issue #671, epic #373). See CataEtShared.h for the sources.

#include "CataEtShared.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "EncounterHelpers.h"
#include "GameObject.h"
#include "Group.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>

namespace EndTime
{
namespace
{
constexpr float GROUND_TIER_STEP   = 3.0f;
constexpr float GROUND_SEARCH_UP   = 5.0f;
constexpr float GROUND_SEARCH_DOWN = 12.0f;
constexpr float ESCAPE_LOS_HEIGHT  = 2.0f;
constexpr float ESCAPE_RADII[]     = { 4.0f, 7.0f, 10.0f, 13.0f };
constexpr int   ESCAPE_BEARINGS    = 16;

// Frost Blades fly 75 yd straight along their facing (boss_echo_of_jaina.cpp:175-183 GetClosePoint(..., 75.0f)) and
// stun everything within 4 yd of them (spell_echo_of_jaina_frost_blades DistanceCheck, :413-424).
constexpr float BLADE_HIT_RADIUS   = 5.5f;
constexpr float BLADE_CLEAR_RADIUS = 7.0f;
constexpr float BLADE_PATH_LENGTH  = 80.0f;

// Infinite Breath: "20 yd range, 1.3 sec cast, all enemies in front of the caster" (Wowhead spell 102569).
constexpr float BREATH_RANGE = 26.0f;
constexpr float BREATH_ARC   = 1.45f;   // ~83 degrees, a little wider than the spell cone

constexpr float FRAGMENT_SEARCH = 220.0f;

// Moonlance path ahead of each lance (CollectHazards): ~4-6 s of warning at 4.2-6.3 yd/s.
constexpr float MOONLANCE_PATH_AHEAD = 24.0f;
constexpr float MOONLANCE_PATH_STEP  = 3.0f;

struct BladeLine
{
    float x;
    float y;
    float ux;
    float uy;
};

void CollectBladeLines(Player* bot, std::vector<BladeLine>& out)
{
    std::vector<Creature*> blades;
    CollectCreatures(bot, { ET_NPC_FROST_BLADE }, 100.0f, blades);
    for (Creature* blade : blades)
    {
        float const o = blade->GetOrientation();
        out.push_back({ blade->GetPositionX(), blade->GetPositionY(), std::cos(o), std::sin(o) });
    }
}

// Clearance of (x, y) from every blade path: negative while it is inside a path (smaller than BLADE_CLEAR_RADIUS).
float BladeMargin(std::vector<BladeLine> const& lines, float x, float y)
{
    float margin = std::numeric_limits<float>::max();
    for (BladeLine const& line : lines)
    {
        float const dx = x - line.x;
        float const dy = y - line.y;
        float const along = dx * line.ux + dy * line.uy;
        if (along < -4.0f || along > BLADE_PATH_LENGTH)
            continue;

        float const perp = std::fabs(-dx * line.uy + dy * line.ux);
        margin = std::min(margin, perp - BLADE_CLEAR_RADIUS);
    }

    return margin;
}

bool InsideBladePath(std::vector<BladeLine> const& lines, float x, float y)
{
    for (BladeLine const& line : lines)
    {
        float const dx = x - line.x;
        float const dy = y - line.y;
        float const along = dx * line.ux + dy * line.uy;
        if (along < -4.0f || along > BLADE_PATH_LENGTH)
            continue;

        if (std::fabs(-dx * line.uy + dy * line.ux) < BLADE_HIT_RADIUS)
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

// The class strategies' interrupt actions are named after the spell, plus the "<spell> on enemy healer" variants
// (Classes/*/*AiObjectContext.cpp creators).
bool IsInterruptActionName(std::string const& name)
{
    for (char const* spell : INTERRUPT_NAMES)
    {
        std::string const base(spell);
        if (name == base || name == base + " on enemy healer")
            return true;
    }

    return false;
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
        case HazardKind::BlightedArrows:
            // 3 yd blight circles (Wowpedia "Echo of Sylvanas"), the arrows knock back whoever is still inside.
            CollectCreatures(bot, { ET_NPC_BLIGHTED_ARROW }, 30.0f, found);
            for (Creature* creature : found)
                out.push_back({ creature->GetPositionX(), creature->GetPositionY(), 5.0f, 7.5f });
            break;
        case HazardKind::Moonlance:
            // "50000 Arcane damage to all players within 2 yards and stuns them for 5 sec" (Wowpedia "Echo of Tyrande").
            // w65 (#671): a lance moves straight along its facing at run speed x0.6 / x0.9 (4.2 / 6.3 yd/s,
            // boss_echo_of_tyrande.cpp:493; 15 yd, then three lances 27 yd at -45/0/+45 degrees, :512-530). A circle around the lance
            // alone gave under a second of warning (et leave moonlance 246 FAILED / 24 OK, mage + healer stunned together and the mage
            // killed), so the hazard is the lance's path ahead: one circle every 3 yd up to MOONLANCE_PATH_AHEAD.
            CollectCreatures(bot, { ET_NPC_MOONLANCE_1, ET_NPC_MOONLANCE_2_1, ET_NPC_MOONLANCE_2_2, ET_NPC_MOONLANCE_2_3 },
                             60.0f, found);
            for (Creature* creature : found)
            {
                float const o = creature->GetOrientation();
                for (float ahead = 0.0f; ahead <= MOONLANCE_PATH_AHEAD; ahead += MOONLANCE_PATH_STEP)
                    out.push_back({ creature->GetPositionX() + std::cos(o) * ahead, creature->GetPositionY() + std::sin(o) * ahead,
                                    4.5f, 7.5f });
            }
            break;
        case HazardKind::EyeOfElune:
            // Piercing Gaze every second within 6 yd of each eye, 5 sec silence (Wowhead spell 102183).
            CollectCreatures(bot, { ET_NPC_EYE_OF_ELUNE_1_E, ET_NPC_EYE_OF_ELUNE_1_W, ET_NPC_EYE_OF_ELUNE_2_E,
                                    ET_NPC_EYE_OF_ELUNE_2_W, ET_NPC_EYE_OF_ELUNE_3_E, ET_NPC_EYE_OF_ELUNE_3_W },
                             45.0f, found);
            for (Creature* creature : found)
                out.push_back({ creature->GetPositionX(), creature->GetPositionY(), 7.0f, 9.5f });
            break;
        case HazardKind::DistortionBomb:
            // 8 yd area, 25000 Arcane per second (Wowpedia "Murozond (tactics)"; spell 101984 is a PERSISTENT_AREA_AURA).
            for (Position const& pos : EncounterHelpers::GetDynamicObjectPositions(bot, 40.0f, ET_SPELL_DISTORTION_BOMB_DMG))
                out.push_back({ pos.GetPositionX(), pos.GetPositionY(), 8.0f, 10.5f });
            break;
    }
}

bool InHazard(Player* bot, HazardKind kind)
{
    std::vector<Hazard> hazards;
    CollectHazards(bot, kind, hazards);
    return InsideAnyHazard(hazards, bot->GetPositionX(), bot->GetPositionY());
}

// ==== Echo of Sylvanas ==========================================================================================================
// Calling of the Highborne (boss_echo_of_sylvanas.cpp:308-341): Sylvanas teleports to the centre (spell_target_position 101398 =
// centerPos 3839.42,911.25), goes passive and immune (100686), Death Grips everyone to her (:324-326) and summons eight Risen
// Ghouls 32.8 yd around her, linked i -> i+1 (SpawnAround :216-250). BossAI::JustSummoned (ScriptedCreature.cpp:779-784)
// zone-engages every ghoul, so all eight walk to the centre at once (:569-580, walk speed x0.5) and the first to arrive casts
// Sacrifice (101348, 277k Shadow, radius 11 yd; DoAction ACTION_GHOUL :189-200) about 31 s after the Call. While a ghoul walks, its links deal Wracking Pain 101257 (83k every 0.5 s) to anyone between it and
// its neighbour, and anyone beyond a LIVE ghoul of their sector gets 101221 (50k per second, IsPlayerOutOfBounds :252-280, checked every 1 s from :333-341).
// A dead ghoul removes both links (JustDied :511-524) and its sector check (GetCreature fails), so the only safe place is
// the dead ghoul's sector, outside 11 yd: "Players should move to intercept and kill the ghoul ... Once the ghoul has died, part
// of the Wracking Pain is dispelled. Players should move through this breach and away from Sylvanas so that they are not killed
// by Sacrifice" (Wowpedia "Echo of Sylvanas", MediaWiki API, fetched 2026-10-14). Seeping Shadows (103182, +10% damage taken
// per stack, 0.2 stacks per missing hp%, :723-734) makes the kill ~1.6x faster than its 1.08M hp (77490 x Health_mod 14).
// w61: no ghoul was killed; three bots died together on the old 18 yd spot ~17 s after the Call yell (19:38:43) as the ring
// passed over them, the healer at 19:39:14 ("Cry havoc!").

Creature* GetSylvanas(Player* bot)
{
    return bot->FindNearestCreature(ET_NPC_ECHO_OF_SYLVANAS, 150.0f, true);
}

bool SylvanasCallActive(Player* bot)
{
    Creature* sylvanas = GetSylvanas(bot);
    return sylvanas && sylvanas->HasAura(ET_SPELL_CALL_OF_THE_HIGHBORNE);
}

void CollectRisenGhouls(Player* bot, std::vector<Creature*>& out)
{
    CollectCreatures(bot, { ET_NPC_RISEN_GHOUL }, 120.0f, out);
}

Creature* GetDesignatedGhoul(std::vector<Creature*> const& ghouls)
{
    Creature* best = nullptr;
    for (Creature* ghoul : ghouls)
    {
        if (!best || ghoul->GetGUID() < best->GetGUID())
            best = ghoul;
    }

    return best;
}

// The ghouls walk straight to the centre, so each keeps its spawn bearing from Sylvanas; a killed one leaves a 90-degree gap.
bool GetSylvanasBreachAngle(Creature* sylvanas, std::vector<Creature*> const& ghouls, float& outAngle)
{
    if (!sylvanas || ghouls.empty())
        return false;

    std::vector<float> angles;
    for (Creature* ghoul : ghouls)
        angles.push_back(sylvanas->GetAngle(ghoul));

    std::sort(angles.begin(), angles.end());

    float const fullCircle = 2.0f * static_cast<float>(M_PI);
    float bestGap = 0.0f;
    float bestStart = 0.0f;
    for (size_t i = 0; i < angles.size(); ++i)
    {
        float const next = i + 1 < angles.size() ? angles[i + 1] : angles[0] + fullCircle;
        float const gap = next - angles[i];
        if (gap > bestGap)
        {
            bestGap = gap;
            bestStart = angles[i];
        }
    }

    if (bestGap < ET_SYLVANAS_BREACH_MIN_GAP)
        return false;

    outAngle = std::fmod(bestStart + bestGap / 2.0f, fullCircle);
    return true;
}

bool GetSylvanasSafeSpot(Player* bot, float& outX, float& outY, float& outZ)
{
    Creature* sylvanas = GetSylvanas(bot);
    if (!sylvanas)
        return false;

    std::vector<Creature*> ghouls;
    CollectRisenGhouls(bot, ghouls);
    float angle = 0.0f;
    if (!GetSylvanasBreachAngle(sylvanas, ghouls, angle))
        return false;

    outX = sylvanas->GetPositionX() + std::cos(angle) * ET_SYLVANAS_SAFE_DIST;
    outY = sylvanas->GetPositionY() + std::sin(angle) * ET_SYLVANAS_SAFE_DIST;

    float const bz = bot->GetPositionZ();
    float const z = bot->GetMapHeight(outX, outY, bz + GROUND_SEARCH_UP, true, GROUND_SEARCH_DOWN);
    outZ = std::fabs(z - bz) <= GROUND_TIER_STEP ? z : sylvanas->GetPositionZ();
    return true;
}

bool NeedsSylvanasSafeSpot(Player* bot)
{
    if (!SylvanasCallActive(bot))
        return false;

    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!GetSylvanasSafeSpot(bot, x, y, z))
        return false;

    return bot->GetExactDist2d(x, y) > 2.5f;
}

// w65 (#671): the ghoul kill worked (thp 98 -> 2, breach and safe spot reached) but the bots that killed it stood OUTSIDE it:
// the tank Charged / Heroic Leaped onto it (leg log Elopa cast: charge, then 23 x 101221 at 22-43k while it chased the walking
// ghoul from behind, d=32.8 -> 15.0 yd), the rogue stood still at 23.3 yd while the ghoul walked past her (101221 x4, dead), and
// in the second Call the mage's "enemy is close" Blink put it at 18.6 yd beyond the ghoul (3 x 57k, dead). IsPlayerOutOfBounds
// (boss_echo_of_sylvanas.cpp:259-289) hurts a player farther from Sylvanas than the live ghoul of his 45-degree sector, so the
// bots stay ET_SYLVANAS_INSIDE_STAND inside it: a bot attacking the ghoul on the ghoul's bearing (melee reach 5, :514-515),
// any other bot on its own bearing.
bool GetSylvanasInsideSpot(Player* bot, float& outX, float& outY, float& outZ)
{
    if (!SylvanasCallActive(bot))
        return false;

    Creature* sylvanas = GetSylvanas(bot);
    std::vector<Creature*> ghouls;
    CollectRisenGhouls(bot, ghouls);
    if (!sylvanas || ghouls.empty())
        return false;

    float const botDist = sylvanas->GetExactDist2d(bot);
    float const botAngle = sylvanas->GetAngle(bot);
    Creature* anchor = nullptr;
    float anchorDist = 0.0f;
    for (Creature* ghoul : ghouls)
    {
        float const ghoulAngle = sylvanas->GetAngle(ghoul);
        if (std::fabs(Position::NormalizePitch(ghoulAngle - botAngle)) > ET_SYLVANAS_SECTOR_HALF)
            continue;

        float const ghoulDist = sylvanas->GetExactDist2d(ghoul);
        if (!anchor || ghoulDist < anchorDist)
        {
            anchor = ghoul;
            anchorDist = ghoulDist;
        }
    }

    if (!anchor || botDist < anchorDist - ET_SYLVANAS_INSIDE_LIMIT)
        return false;

    float const angle = bot->GetVictim() == anchor ? sylvanas->GetAngle(anchor) : botAngle;
    float const dist = std::max(0.0f, anchorDist - ET_SYLVANAS_INSIDE_STAND);
    outX = sylvanas->GetPositionX() + std::cos(angle) * dist;
    outY = sylvanas->GetPositionY() + std::sin(angle) * dist;

    float const bz = bot->GetPositionZ();
    float const z = bot->GetMapHeight(outX, outY, bz + GROUND_SEARCH_UP, true, GROUND_SEARCH_DOWN);
    outZ = std::fabs(z - bz) <= GROUND_TIER_STEP ? z : sylvanas->GetPositionZ();
    return true;
}

bool NeedsSylvanasInside(Player* bot)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    return GetSylvanasInsideSpot(bot, x, y, z);
}

// Every bot but the healers attacks the designated ghoul until a breach is open (ranged from the centre: the ring is 32.8 yd).
bool NeedsGhoulKill(Player* bot)
{
    if (PlayerBotSpec::IsHeal(bot) || !SylvanasCallActive(bot))
        return false;

    Creature* sylvanas = GetSylvanas(bot);
    std::vector<Creature*> ghouls;
    CollectRisenGhouls(bot, ghouls);
    float angle = 0.0f;
    if (ghouls.empty() || GetSylvanasBreachAngle(sylvanas, ghouls, angle))
        return false;

    Creature* ghoul = GetDesignatedGhoul(ghouls);
    return ghoul && bot->GetVictim() != ghoul;
}

// "Make sure ranged players stand 10 yards apart" (Wowpedia "Echo of Sylvanas"): Black Arrow splashes 10 yd, Unholy Shot
// hits 3 players within 10 yd. Same shape as AC PoS TyrannusAction::RangedSpread (PoSActions.cpp:292-312).
bool NeedsRangedSpread(Player* bot)
{
    if (!PlayerBotSpec::IsRanged(bot) || !bot->IsInCombat())
        return false;

    // w65 (#671): also at Echo of Tyrande - mage and healer stood on the same spot 32 yd from her and one Moonlance (2 yd, 5 s stun,
    // Wowpedia "Echo of Tyrande") stunned both; the mage died first, the stunned healer could not heal through Stardust.
    Creature* sylvanas = GetSylvanas(bot);
    bool const sylvanasFight =
        sylvanas && sylvanas->IsInCombat() && !sylvanas->HasAura(ET_SPELL_CALL_OF_THE_HIGHBORNE);
    Creature* tyrande = sylvanasFight ? nullptr : GetTyrande(bot);
    bool const tyrandeFight = tyrande && tyrande->IsInCombat() && !tyrande->HasAura(ET_SPELL_IN_SHADOW);
    if (!sylvanasFight && !tyrandeFight)
        return false;

    std::vector<Player*> members;
    CollectBotMembers(bot, members);
    for (Player* member : members)
    {
        if (member != bot && bot->GetExactDist2d(member) < 8.0f)
            return true;
    }

    return false;
}

// ==== Echo of Tyrande ===========================================================================================================

Creature* GetTyrande(Player* bot)
{
    return bot->FindNearestCreature(ET_NPC_ECHO_OF_TYRANDE, 300.0f, true);
}

// Moonbolt (1.5 s cast, 40k on the tank, every 3 s) and Stardust (3 s cast, 50k to EVERY player, every 7-8 s) are Tyrande's
// casts (boss_echo_of_tyrande.cpp:430-453). w64 (#671): only Stardust is kicked - Wowpedia "Echo of Tyrande" (MediaWiki API,
// 2026-10-14) marks Stardust alone as interruptable; a kick spent on a Moonbolt can leave the 10-24 s interrupts on cooldown for
// the group-wide Stardust (w61/w63 pulls: 551/304 FAILED interrupt rows; w62/w63 wiped at 69%/76% on tank and healer deaths).
Unit* GetTyrandeInterruptTarget(Player* bot)
{
    Creature* tyrande = GetTyrande(bot);
    if (!tyrande || !tyrande->HasUnitState(UNIT_STATE_CASTING))
        return nullptr;

    if (tyrande->FindCurrentSpellBySpellId(ET_SPELL_STARDUST))
        return tyrande;

    return nullptr;
}

bool TyrandeNeedsInterrupt(Player* bot)
{
    return GetTyrandeInterruptTarget(bot) != nullptr;
}

Creature* GetMoonlightPool(Player* bot)
{
    return bot->FindNearestCreature(ET_NPC_POOL_OF_MOONLIGHT, 300.0f, true);
}

// The five-pool event (boss_echo_of_tyrande.cpp:327-361, area trigger :752-785): Tyrande keeps the 90% damage reduction
// aura "In Shadow" (101841) on herself and on her adds until the event is DONE; adds die only inside a Pool of Moonlight
// (Wowpedia "Echo of Tyrande", Trash section). Pools last 40 s and shrink every 2 s.
// w62 (#671): the pool's Moonlit aura (101946) triggers 101842 every 0.5 s, which REMOVES In Shadow (SpellEffect 164) within
// 5 yd of the pool (SpellRadius 8) - so the bots hold the pool centre (2 yd) and the melee adds chasing them reach that 5 yd.
// w61 stood up to 4 yd out and chased adds across the shrine: 8-9 shielded adds were still up when the event ended.
bool NeedsMoonlightPool(Player* bot)
{
    Creature* tyrande = GetTyrande(bot);
    if (!tyrande || !tyrande->HasAura(ET_SPELL_IN_SHADOW))
        return false;

    Creature* pool = GetMoonlightPool(bot);
    return pool && bot->GetExactDist2d(pool) > 2.0f;
}

// Tyrande carries In Shadow from her constructor (:163) until EVENT_STOP_EVENT_1 (:413-429), so "In Shadow" alone is also
// true before the area trigger; a pool or a live event add means the event runs.
bool TyrandeEventActive(Player* bot)
{
    Creature* tyrande = GetTyrande(bot);
    if (!tyrande || !tyrande->HasAura(ET_SPELL_IN_SHADOW))
        return false;

    if (GetMoonlightPool(bot))
        return true;

    std::vector<Creature*> adds;
    CollectCreatures(bot, { ET_NPC_TIME_TWISTED_NIGHTSABER_1, ET_NPC_TIME_TWISTED_NIGHTSABER_2, ET_NPC_TIME_TWISTED_NIGHTSABER_3,
                            ET_NPC_TIME_TWISTED_SENTINEL, ET_NPC_TIME_TWISTED_HUNTRESS },
                     80.0f, adds);
    return !adds.empty();
}

bool TyrandeFightActive(Player* bot)
{
    Creature* tyrande = GetTyrande(bot);
    return tyrande && tyrande->IsInCombat() && !tyrande->HasAura(ET_SPELL_IN_SHADOW);
}

namespace
{
// The unit a ranged bot must stay within ET_TYRANDE_HEAL_REACH of: the healer heals Tyrande's victim, everyone else is healed
// by the group's healer (w68: the mage sat at 16% for 38 s, 54-57 yd from the walking healer, until Stardust killed it).
Unit* GetTyrandeBandAnchor(Player* bot, Creature* tyrande)
{
    Unit* anchor = nullptr;
    if (PlayerBotSpec::IsHeal(bot))
        anchor = tyrande->GetVictim();
    else
    {
        std::vector<Player*> members;
        CollectBotMembers(bot, members);
        for (Player* member : members)
        {
            if (member != bot && PlayerBotSpec::IsHeal(member))
            {
                anchor = member;
                break;
            }
        }
    }

    if (!anchor || anchor == bot || !anchor->IsAlive())
        return nullptr;

    return anchor;
}

// >= 0 where (x, y) is out of every Eye orbit's reach: inside ET_TYRANDE_INNER_SAFE or at ET_TYRANDE_RANGED_MIN and beyond.
float TyrandeOrbitMargin(Creature* tyrande, float x, float y)
{
    float const d = tyrande->GetExactDist2d(x, y);
    return std::max(ET_TYRANDE_INNER_SAFE - d, d - ET_TYRANDE_RANGED_MIN);
}
}

// w67 (#671): the mage (17-23 yd, Moonlance stun then 100 -> 19 -> dead) and the healer (24-26 yd, 55 -> 21 -> Spirit of
// Redemption) died inside the Eye orbits; the tank followed without heals. The circle round an eye (CollectHazards) gives
// under a second of warning at 7.7 yd/s, so ranged bots stand outside the outer orbit instead (ET_TYRANDE_RANGED_*).
bool GetTyrandeRangedSpot(Player* bot, float& outX, float& outY, float& outZ)
{
    if (!PlayerBotSpec::IsRanged(bot) || !bot->IsInCombat())
        return false;

    Creature* tyrande = GetTyrande(bot);
    if (!tyrande || !tyrande->IsInCombat() || tyrande->HasAura(ET_SPELL_IN_SHADOW))
        return false;

    // w68 (#671): the old spot sat on the TANK's bearing. The tank stands 1-3 yd from Tyrande, so that bearing swung -19 -> 176 ->
    // 139 -> 96 -> 68 degrees in the first 60 s and the healer re-walked the chord every time (band 74 OK / 279 FAILED = still
    // walking, moving in ~90% of its probes, ~10 heals cast in 60 s, 4 of them Purify): the tank died to Moonbolt at 61 s, then
    // the healer's chord to the new victim crossed 21 yd (Piercing Gaze 48.8k + 41.6k + 34.2k, Spirit of Redemption).
    // Now every ranged bot keeps its OWN bearing: a bot off the band moves radially, a bot out of its anchor's reach steps along
    // the band toward the anchor's bearing. With the tank within 3 yd of Tyrande, any band spot within ~105 degrees of the tank's
    // bearing is in reach, so the healer rarely moves at all.
    Unit* anchor = GetTyrandeBandAnchor(bot, tyrande);
    float const dist = tyrande->GetExactDist2d(bot);
    bool const inBand = dist >= ET_TYRANDE_RANGED_MIN && dist <= ET_TYRANDE_RANGED_MAX;
    bool const inReach = !anchor || bot->GetExactDist2d(anchor) <= ET_TYRANDE_HEAL_REACH;
    if (inBand && inReach)
        return false;

    float const base = tyrande->GetAngle(bot);
    float heading = base;
    if (!inReach)
    {
        float const diff = Position::NormalizePitch(tyrande->GetAngle(anchor) - base);
        heading = base + std::max(-ET_TYRANDE_ARC_STEP, std::min(ET_TYRANDE_ARC_STEP, diff));
    }

    // Nothing to gain from a step this short (e.g. an anchor on the bot's own bearing that is still walking in, or a bot a few
    // tenths inside ET_TYRANDE_RANGED_MIN): no sideways correction either.
    if (bot->GetExactDist2d(tyrande->GetPositionX() + std::cos(heading) * ET_TYRANDE_RANGED_STAND,
                            tyrande->GetPositionY() + std::sin(heading) * ET_TYRANDE_RANGED_STAND) < 1.5f)
        return false;

    // In the band only small corrections (a wide one is a chord through the orbits); from inside / outside the move is radial.
    static float const NEAR_OFFSETS[] = { 0.0f, 0.13f, -0.13f, 0.26f, -0.26f };
    static float const WIDE_OFFSETS[] = { 0.0f, 0.13f, -0.13f, 0.26f, -0.26f, 0.52f, -0.52f, 0.79f, -0.79f, 1.05f, -1.05f };
    float const tz = tyrande->GetPositionZ();
    auto tryOffset = [&](float offset) -> bool
    {
        float const angle = heading + offset;
        float const x = tyrande->GetPositionX() + std::cos(angle) * ET_TYRANDE_RANGED_STAND;
        float const y = tyrande->GetPositionY() + std::sin(angle) * ET_TYRANDE_RANGED_STAND;
        float const z = bot->GetMapHeight(x, y, std::max(bot->GetPositionZ(), tz) + 10.0f, true, 30.0f);
        if (!(std::fabs(z - tz) <= 8.0f))
            return false;

        if (!tyrande->IsWithinLOS(x, y, z + ESCAPE_LOS_HEIGHT))
            return false;

        outX = x;
        outY = y;
        outZ = z;
        return true;
    };

    if (inBand)
    {
        for (float offset : NEAR_OFFSETS)
            if (tryOffset(offset))
                return true;
    }
    else
    {
        for (float offset : WIDE_OFFSETS)
            if (tryOffset(offset))
                return true;
    }

    return false;
}

bool FindTyrandeHazardEscape(Player* bot, std::vector<Hazard> const& hazards, float& outX, float& outY, float& outZ)
{
    if (hazards.empty() || !InsideAnyHazard(hazards, bot->GetPositionX(), bot->GetPositionY()))
        return false;

    // w68 (#671): a lance / eye escape is a 4-13 yd step (FindClearSpot) - from the band or from melee it can land in the orbits.
    Creature* tyrande = TyrandeFightActive(bot) ? GetTyrande(bot) : nullptr;
    if (tyrande &&
        FindClearSpot(bot,
                      [&hazards, tyrande](float x, float y)
                      { return std::min(HazardMargin(hazards, x, y), TyrandeOrbitMargin(tyrande, x, y)); },
                      outX, outY, outZ))
        return true;

    return FindHazardEscape(bot, hazards, outX, outY, outZ);
}

bool NeedsTyrandeRangedBand(Player* bot)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    return GetTyrandeRangedSpot(bot, x, y, z);
}

// ==== Echo of Jaina =============================================================================================================

Creature* GetJaina(Player* bot)
{
    return bot->FindNearestCreature(ET_NPC_ECHO_OF_JAINA, 300.0f, true);
}

Unit* GetJainaInterruptTarget(Player* bot)
{
    Creature* jaina = GetJaina(bot);
    if (!jaina || !jaina->HasUnitState(UNIT_STATE_CASTING) || !jaina->FindCurrentSpellBySpellId(ET_SPELL_FROSTBOLT_VOLLEY))
        return nullptr;

    return jaina;
}

bool JainaNeedsInterrupt(Player* bot)
{
    return GetJainaInterruptTarget(bot) != nullptr;
}

// A used fragment gets GO_FLAG_IN_USE and moves to phase 256 (go_echo_of_jaina_jaina_staff_fragment,
// boss_echo_of_jaina.cpp:373-386), so skip those.
// w62 (#671): every bot picks the fragment nearest to the DESIGNATED bot (tank first), so the group walks to one fragment at a
// time. w61 every bot walked to its own nearest fragment: the four split across the shrine (Delamy alone at fragment
// 3170.8,487.3 inside an east pack), pulled the Time-Twisted packs one by one and only 12 of the 16 fragments were used.
GameObject* FindJainaFragment(Player* bot)
{
    Player* leader = GetDesignatedBot(bot);
    if (!leader)
        leader = bot;

    std::list<GameObject*> fragments;
    bot->GetGameObjectListWithEntryInGrid(fragments, ET_GO_JAINA_FRAGMENT, FRAGMENT_SEARCH);

    GameObject* best = nullptr;
    float bestDist = 0.0f;
    for (GameObject* go : fragments)
    {
        if (!go || go->HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE) || !bot->InSamePhase(go))
            continue;

        float const dist = leader->GetExactDist2d(go);
        if (!best || dist < bestDist)
        {
            best = go;
            bestDist = dist;
        }
    }

    return best;
}

// Jaina stays passive and invisible until 16 fragments are used (Reset/DoAction, boss_echo_of_jaina.cpp:108-150); the
// bots have to collect them or the echo can never be fought.
bool NeedsFragments(Player* bot)
{
    if (bot->IsInCombat())
        return false;

    // The whole group regroups after a pull before it walks on (no member still fighting).
    std::vector<Player*> members;
    CollectBotMembers(bot, members);
    for (Player* member : members)
    {
        if (member->IsInCombat())
            return false;
    }

    Creature* jaina = GetJaina(bot);
    if (!jaina || !jaina->HasReactState(REACT_PASSIVE))
        return false;

    return FindJainaFragment(bot) != nullptr;
}

bool InBladePath(Player* bot)
{
    std::vector<BladeLine> lines;
    CollectBladeLines(bot, lines);
    return !lines.empty() && InsideBladePath(lines, bot->GetPositionX(), bot->GetPositionY());
}

bool GetBladeEscape(Player* bot, float& outX, float& outY, float& outZ)
{
    std::vector<BladeLine> lines;
    CollectBladeLines(bot, lines);
    if (lines.empty() || !InsideBladePath(lines, bot->GetPositionX(), bot->GetPositionY()))
        return false;

    return FindClearSpot(bot, [&lines](float x, float y) { return BladeMargin(lines, x, y); }, outX, outY, outZ);
}

Creature* GetFlarecore(Player* bot)
{
    return bot->FindNearestCreature(ET_NPC_FLARECORE, 70.0f, true);
}

// "Whoever is closest to the Flarecore should run to it" (Wowpedia "Echo of Jaina"): an ember not touched within 10 s
// explodes for ~94k on everyone. The script detonates it when a player is within 3 yd (boss_echo_of_jaina.cpp:338-343).
// Tanks only when nobody else is available. Every bot evaluates the same rule, so they agree on one stomper.
Player* GetFlarecoreStomper(Player* bot, Creature* ember)
{
    std::vector<Player*> members;
    CollectBotMembers(bot, members);

    Player* best = nullptr;
    bool bestTank = false;
    float bestDist = 0.0f;
    for (Player* member : members)
    {
        bool const tank = PlayerBotSpec::IsTank(member);
        float const dist = member->GetExactDist2d(ember);
        bool better = !best;
        if (!better)
        {
            if (tank != bestTank)
                better = !tank;
            else if (std::fabs(dist - bestDist) > 0.5f)
                better = dist < bestDist;
            else
                better = member->GetGUID() < best->GetGUID();
        }

        if (better)
        {
            best = member;
            bestTank = tank;
            bestDist = dist;
        }
    }

    return best;
}

bool FlarecoreActive(Player* bot)
{
    return GetFlarecore(bot) != nullptr;
}

bool NeedsFlarecoreAction(Player* bot)
{
    Creature* ember = GetFlarecore(bot);
    if (!ember)
        return false;

    return GetFlarecoreStomper(bot, ember) == bot || bot->GetExactDist2d(ember) < 8.5f;
}

// ==== Echo of Baine =============================================================================================================

Creature* GetBaine(Player* bot)
{
    return bot->FindNearestCreature(ET_NPC_ECHO_OF_BAINE, 200.0f, true);
}

bool BaineFightActive(Player* bot)
{
    Creature* baine = GetBaine(bot);
    return baine && baine->IsInCombat();
}

// Pulverize destroys the platform nearest to the landing spot (Wowhead spell 101627: "destroying the nearby platform").
GameObject* FindIntactPlatform(Player* bot)
{
    GameObject* best = nullptr;
    float bestDist = 0.0f;
    for (uint32 entry : { ET_GO_PLATFORM_1, ET_GO_PLATFORM_2, ET_GO_PLATFORM_3, ET_GO_PLATFORM_4 })
    {
        GameObject* go = bot->FindNearestGameObject(entry, 80.0f);
        if (!go || go->GetDestructibleState() == GO_DESTRUCTIBLE_DESTROYED)
            continue;

        float const dist = bot->GetExactDist2d(go);
        if (!best || dist < bestDist)
        {
            best = go;
            bestDist = dist;
        }
    }

    return best;
}

// The Obsidian Dragonshrine floor is lava: the script itself treats IsInWater() as "in the lava" (Molten Axe,
// boss_echo_of_baine.cpp:167-168) and stacking lava damage is what kills the group (Wowpedia "Echo of Baine").
bool NeedsLavaEscape(Player* bot)
{
    return bot->IsInWater() && BaineFightActive(bot) && FindIntactPlatform(bot) != nullptr;
}

Creature* GetBaineTotem(Player* bot)
{
    return bot->FindNearestCreature(ET_NPC_BAINES_TOTEM, 80.0f, true);
}

Player* GetTotemClicker(Player* bot, Creature* totem)
{
    std::vector<Player*> members;
    CollectBotMembers(bot, members);

    Player* best = nullptr;
    bool bestTank = false;
    float bestDist = 0.0f;
    for (Player* member : members)
    {
        if (member->IsInWater())
            continue;

        bool const tank = PlayerBotSpec::IsTank(member);
        float const dist = member->GetExactDist2d(totem);
        bool better = !best;
        if (!better)
        {
            if (tank != bestTank)
                better = !tank;
            else
                better = dist < bestDist;
        }

        if (better)
        {
            best = member;
            bestTank = tank;
            bestDist = dist;
        }
    }

    return best;
}

// Throw Totem: Baine's totem (54434) can be clicked (npc_spellclick_spells 54434 -> 107837) and thrown back, "inflicts 5%
// Physical damage, stunning him for 6 sec and increasing damage taken by 50%" (Wowhead spell 101602). One bot clicks, the
// bot holding the aura throws (101603 -> missile 101602; the cast removes 107837, so one throw per pickup - #842).
bool NeedsTotemThrow(Player* bot)
{
    if (bot->IsInWater() || !BaineFightActive(bot))
        return false;

    if (bot->HasAura(ET_SPELL_THROW_TOTEM_AURA))
        return true;

    Creature* totem = GetBaineTotem(bot);
    return totem && GetTotemClicker(bot, totem) == bot;
}

// ==== Murozond ==================================================================================================================

Creature* GetMurozond(Player* bot)
{
    return bot->FindNearestCreature(ET_NPC_MUROZOND, 200.0f, true);
}

bool InInfiniteBreath(Player* bot)
{
    Creature* boss = GetMurozond(bot);
    if (!boss || boss->GetVictim() == bot)
        return false;

    if (!boss->HasUnitState(UNIT_STATE_CASTING) || !boss->FindCurrentSpellBySpellId(ET_SPELL_INFINITE_BREATH))
        return false;

    if (boss->GetExactDist2d(bot) > BREATH_RANGE)
        return false;

    return boss->HasInArc(BREATH_ARC, bot);
}

// Stand beside the boss (90 degrees off its facing), which is outside the breath cone from any bot position.
bool GetBreathEscape(Player* bot, float& outX, float& outY, float& outZ)
{
    Creature* boss = GetMurozond(bot);
    if (!boss)
        return false;

    float const o = boss->GetOrientation();
    float const fx = std::cos(o);
    float const fy = std::sin(o);
    float const cross = fx * (bot->GetPositionY() - boss->GetPositionY()) - fy * (bot->GetPositionX() - boss->GetPositionX());
    float const nearSide = cross >= 0.0f ? 1.0f : -1.0f;

    float const bz = bot->GetPositionZ();
    for (float side : { nearSide, -nearSide })
    {
        for (float lateral : { 9.0f, 12.0f })
        {
            float const x = boss->GetPositionX() - fy * side * lateral;
            float const y = boss->GetPositionY() + fx * side * lateral;
            float const z = bot->GetMapHeight(x, y, bz + GROUND_SEARCH_UP, true, GROUND_SEARCH_DOWN);
            if (!(std::fabs(z - bz) <= GROUND_TIER_STEP))
                continue;

            if (!bot->IsWithinLOS(x, y, z + ESCAPE_LOS_HEIGHT))
                continue;

            outX = x;
            outY = y;
            outZ = z;
            return true;
        }
    }

    return false;
}

// "The player assigned to activate the hourglass should wait until the tank or healer dies, the distortion areas make it hard
// for DPS to stay in range, or ..." (Wowpedia "Murozond (tactics)"). The script allows five uses (hourglass = 5,
// boss_murozond.cpp:138) and GetData(TYPE_HOURGLASS = 3) returns what is left (:157-162).
bool NeedsHourglass(Player* bot)
{
    Creature* boss = GetMurozond(bot);
    if (!boss || !boss->IsInCombat() || !boss->AI() || boss->AI()->GetData(3) == 0)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    uint32 dead = 0;
    bool keyDead = false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member->GetMapId() != bot->GetMapId() || !GET_PLAYERBOT_AI(member) || member->IsAlive())
            continue;

        ++dead;
        if (PlayerBotSpec::IsTank(member) || PlayerBotSpec::IsHeal(member))
            keyDead = true;
    }

    size_t const bombs = EncounterHelpers::GetDynamicObjectPositions(bot, 60.0f, ET_SPELL_DISTORTION_BOMB_DMG).size();
    if (!keyDead && dead < 2 && bombs < 5)
        return false;

    GameObject* go = bot->FindNearestGameObject(ET_GO_HOURGLASS, 250.0f);
    if (!go)
        return false;

    // The bot nearest to the hourglass that is not the tank or a healer presses it (ties by guid).
    std::vector<Player*> members;
    CollectBotMembers(bot, members);
    Player* user = nullptr;
    bool userKey = false;
    float userDist = 0.0f;
    for (Player* member : members)
    {
        bool const key = PlayerBotSpec::IsTank(member) || PlayerBotSpec::IsHeal(member);
        float const dist = member->GetExactDist2d(go);
        bool better = !user;
        if (!better)
        {
            if (key != userKey)
                better = !key;
            else if (std::fabs(dist - userDist) > 0.5f)
                better = dist < userDist;
            else
                better = member->GetGUID() < user->GetGUID();
        }

        if (better)
        {
            user = member;
            userKey = key;
            userDist = dist;
        }
    }

    return user == bot;
}
}
