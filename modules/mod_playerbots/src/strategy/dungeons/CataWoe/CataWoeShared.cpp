/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity bot strategy "cata-woe" - helpers (issue #672, epic #373). See CataWoeShared.h for the sources.

#include "CataWoeShared.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "GameObject.h"
#include "Group.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>

namespace WellOfEternity
{
namespace
{
constexpr float GROUND_TIER_STEP   = 3.0f;
constexpr float GROUND_SEARCH_UP   = 5.0f;
constexpr float GROUND_SEARCH_DOWN = 12.0f;
constexpr float ESCAPE_LOS_HEIGHT  = 2.0f;
constexpr float ESCAPE_RADII[]     = { 4.0f, 7.0f, 10.0f, 13.0f };
constexpr int   ESCAPE_BEARINGS    = 16;

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
        case HazardKind::FelFlames:
            // Fel Flames: 33250 Fire after 3 s, then 11875 every second to anyone in the area for 30 s (Wowhead spell 108141,
            // pulse spell 108217); the stalker (57329) is the area's centre.
            CollectCreatures(bot, { WOE_NPC_FEL_FLAMES }, 40.0f, found);
            for (Creature* creature : found)
                out.push_back({ creature->GetPositionX(), creature->GetPositionY(), 6.0f, 8.5f });
            break;
        case HazardKind::EyeOfPerotharn:
            // The hunt-phase eyes (45 s) follow random players at 0.8 run speed and report a player within 1 yd
            // (boss_perotharn.cpp:366-376, :441-510); a found player is stunned 8 s and Peroth'arn leaps onto him.
            CollectCreatures(bot, { WOE_NPC_EYE_OF_PEROTHARN_1, WOE_NPC_EYE_OF_PEROTHARN_2 }, 45.0f, found);
            for (Creature* creature : found)
                out.push_back({ creature->GetPositionX(), creature->GetPositionY(), 8.0f, 12.0f });
            break;
        case HazardKind::FelFirestorm:
            // "rapidly creating 3 yard radius Fel Flames around the area for 12 sec. Fel Flames inflicts 19500 to 20500 Fire damage
            // every second to any player standing within them" (Wowpedia "Mannoroth and Varo'then"); 55502 carries aura 103892.
            CollectCreatures(bot, { WOE_NPC_FEL_FLAMES_FIRESTORM }, 40.0f, found);
            for (Creature* creature : found)
                out.push_back({ creature->GetPositionX(), creature->GetPositionY(), 4.5f, 6.5f });
            break;
        case HazardKind::ArcaneBomb:
            // The Hammer of Divinity ground marker (54865) is where the bomb lands: 69-80k Arcane within 4.5 yd and a 50% slow
            // (Wowhead spell 102455; marker + falling hammer, boss_queen_azshara.cpp:385-395, :416-440).
            CollectCreatures(bot, { WOE_NPC_HAMMER_OF_DIVINITY_2 }, 50.0f, found);
            for (Creature* creature : found)
                out.push_back({ creature->GetPositionX(), creature->GetPositionY(), 6.0f, 8.5f });
            break;
    }
}

bool InHazard(Player* bot, HazardKind kind)
{
    std::vector<Hazard> hazards;
    CollectHazards(bot, kind, hazards);
    return InsideAnyHazard(hazards, bot->GetPositionX(), bot->GetPositionY());
}

// ==== bosses ====================================================================================================================

Creature* GetPerotharn(Player* bot)
{
    return bot->FindNearestCreature(WOE_NPC_PEROTHARN, 200.0f, true);
}

Creature* GetAzshara(Player* bot)
{
    return bot->FindNearestCreature(WOE_NPC_AZSHARA, 200.0f, true);
}

Creature* GetMannoroth(Player* bot)
{
    return bot->FindNearestCreature(WOE_NPC_MANNOROTH, 200.0f, true);
}

Creature* GetVarothen(Player* bot)
{
    return bot->FindNearestCreature(WOE_NPC_VAROTHEN, 200.0f, true);
}

// Total Obedience: "Eventually, Queen Azshara is overcome with boredom. She will attempt to turn the entire party into puppets.
// This ability must be interrupted." (Wowpedia "Queen Azshara (tactics)"). The cast is 10 s (Wowhead spell 103241); the script
// interrupts it on ANY interrupt effect (SpellHit, boss_queen_azshara.cpp:187-195) and casts it every 10-20 s after the magi
// engage (:238-241).
Unit* GetObedienceTarget(Player* bot)
{
    Creature* azshara = GetAzshara(bot);
    if (!azshara || !azshara->HasUnitState(UNIT_STATE_CASTING) ||
        !azshara->FindCurrentSpellBySpellId(WOE_SPELL_TOTAL_OBEDIENCE))
        return nullptr;

    return azshara;
}

bool ObedienceNeedsInterrupt(Player* bot)
{
    return GetObedienceTarget(bot) != nullptr;
}

Creature* GetNearestDebilitator(Player* bot)
{
    std::vector<Creature*> debilitators;
    CollectCreatures(bot, { WOE_NPC_DOOMGUARD_DEBILITATOR }, 120.0f, debilitators);

    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (Creature* creature : debilitators)
    {
        float const dist = bot->GetExactDist2d(creature);
        if (!best || dist < bestDist)
        {
            best = creature;
            bestDist = dist;
        }
    }

    return best;
}

// The two Dreadlord Debilitators are REACT_PASSIVE (boss_mannoroth.cpp:618) so the generic target values never pick them, yet
// Tyrande only recovers - and Mannoroth only takes damage below 88% - once both are dead (npc_well_of_eternity_tyrande,
// well_of_eternity.cpp:3373-3387; Mannoroth DamageTaken, boss_mannoroth.cpp:267-274). Everyone but the healers hunts them.
Creature* GetHandOfTheQueen(Player* bot)
{
    return bot->FindNearestCreature(WOE_NPC_HAND_OF_THE_QUEEN, 80.0f, true);
}

bool IsPuppet(Unit* unit)
{
    return unit && unit->HasAura(WOE_SPELL_SERVANT_OF_THE_QUEEN);
}

// Everyone but the healers and the puppet itself; the Hand is REACT_PASSIVE, so the generic target values never pick it.
bool NeedsHandKill(Player* bot)
{
    if (PlayerBotSpec::IsHeal(bot) || IsPuppet(bot) || bot->IsCharmed())
        return false;

    return GetHandOfTheQueen(bot) != nullptr;
}

GameObject* GetUsablePortalFocus(Player* bot)
{
    for (uint32 entry : { WOE_GO_PORTAL_ENERGY_FOCUS_1, WOE_GO_PORTAL_ENERGY_FOCUS_2, WOE_GO_PORTAL_ENERGY_FOCUS_3 })
    {
        GameObject* go = bot->FindNearestGameObject(entry, 30.0f);
        if (!go || !go->isSpawned() || go->getLootState() != GO_READY ||
            go->HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_LOCKED) || go->HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE))
            continue;
        return go;
    }

    return nullptr;
}

// One bot clicks (tank first, GetDesignatedBot), the way one player does in retail.
bool NeedsPortalFocusUse(Player* bot)
{
    if (!bot->IsAlive() || GetDesignatedBot(bot) != bot)
        return false;

    return GetUsablePortalFocus(bot) != nullptr;
}

bool NeedsDebilitatorKill(Player* bot)
{
    if (PlayerBotSpec::IsHeal(bot))
        return false;

    Creature* mannoroth = GetMannoroth(bot);
    if (!mannoroth || !mannoroth->IsInCombat())
        return false;

    return GetNearestDebilitator(bot) != nullptr;
}
}
