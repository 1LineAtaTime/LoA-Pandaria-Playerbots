/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (epic #373, issue #661). See CataTottShared.h for sources.

#include "CataTottShared.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "Group.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <list>
#include <string>

namespace CataTott
{
namespace
{
struct HazardSpec
{
    uint32 entries[2];  // 0 = unused
    float radius;
};

HazardSpec const& GetSpec(HazardId id)
{
    // Order = enum HazardId. Radii are Wowpedia's (read 2026-10-06) with a little extra where the fork's script
    // leaves the exact size to the spell data.
    static HazardSpec const specs[] = {
        { { NPC_GEYSER, 0 }, 5.0f },                     // Summon Geyser: erupts within 5 yd
        { { NPC_WATERSPOUT, NPC_WATERSPOUT_H }, 2.0f },  // vortex touches at 2 yd (npc_lady_nazjar_waterspoutAI)
        { { NPC_DARK_FISSURE, 0 }, 7.0f },               // Dark Fissure 6 yd (grows on heroic)
        { { NPC_MIND_FOG, 0 }, 10.0f },                  // Mind Fog 10 yd (SpellEffect.dbc 76230: 10 yd)
        { { NPC_EARTH_SHARDS, 0 }, 5.0f },               // Earth Shards 5 yd
        { { NPC_BLIGHT_OF_OZUMAT, 0 }, 10.0f },          // Blight of Ozumat pool 10 yd
    };

    return specs[static_cast<uint8>(id)];
}

// Interruptible casts of this dungeon, most urgent first (0 = not listed).
uint8 InterruptPriority(uint32 spellId)
{
    switch (spellId)
    {
        case SPELL_SHOCK_BLAST:       // Naz'jar: 70k nature on the tank, 2 s cast (the fork's SpellHit interrupts it too)
            return 1;
        case SPELL_BRAIN_SPIKE:       // Vicious Mindlasher: 40 yd shadow wave + mana loss
            return 2;
        case SPELL_CHAIN_LIGHTNING:   // Tempest Witch: hits three players
            return 3;
        case SPELL_LAVA_BOLT:         // Erunak: 56k fire + knockback on a random player
            return 4;
        case SPELL_SHADOW_BOLT:       // Vicious Mindlasher
            return 5;
        case SPELL_VEIL_OF_SHADOW:    // Vicious Mindlasher, heroic: -50% healing on a random player
            return 6;
        default:
            return 0;
    }
}

uint8 CurrentCastPriority(Unit* caster)
{
    uint8 best = 0;
    for (CurrentSpellTypes slot : { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL })
    {
        Spell* spell = caster->GetCurrentSpell(slot);
        if (!spell || !spell->m_spellInfo)
            continue;

        uint8 const prio = InterruptPriority(spell->m_spellInfo->Id);
        if (prio && (!best || prio < best))
            best = prio;
    }

    return best;
}

// Same test as fork strategy/dungeons/MgT/MgTShared.cpp StopsACast: a spell that actually stops a cast.
bool StopsACast(uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
        return false;

    if ((info->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT) &&
        info->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
        return true;

    for (uint8 i = EFFECT_0; i <= EFFECT_2; ++i)
    {
        if (info->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST)
            return true;

        if (info->Effects[i].Effect != SPELL_EFFECT_APPLY_AURA)
            continue;

        if (info->Effects[i].ApplyAuraName == SPELL_AURA_MOD_SILENCE ||
            info->Effects[i].ApplyAuraName == SPELL_AURA_MOD_STUN)
            return true;
    }

    return false;
}

// 5.4.8 interrupt spell names as the fork's class strategies use them (strategy/Classes/*Triggers.h).
// A name the bot does not know resolves to spell id 0 and is skipped.
char const* const INTERRUPT_SPELLS[] = {
    "kick", "pummel", "wind shear", "mind freeze", "counterspell", "spell lock", "spear hand strike",
    "skull bash", "rebuke", "silencing shot", "counter shot", "silence", "strangulate",
};
}  // namespace

Creature* FindCreature(Player* bot, uint32 entry, float range)
{
    return bot->FindNearestCreature(entry, range, true);
}

float HazardSafeRadius(HazardId id)
{
    return GetSpec(id).radius + HAZARD_SAFE_EXTRA;
}

Creature* GetHazardCreature(Player* bot, HazardId id)
{
    if (!bot->IsInCombat())
        return nullptr;

    HazardSpec const& spec = GetSpec(id);
    float const inside = spec.radius + HAZARD_MARGIN;

    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (uint32 const entry : spec.entries)
    {
        if (!entry)
            continue;

        std::list<Creature*> found;
        bot->GetCreatureListWithEntryInGrid(found, entry, inside);
        for (Creature* creature : found)
        {
            if (!creature->IsAlive())
                continue;

            float const dist = bot->GetExactDist2d(creature);
            if (dist >= inside)
                continue;

            if (!best || dist < bestDist)
            {
                best = creature;
                bestDist = dist;
            }
        }
    }

    return best;
}

bool InAnyHazard(Player* bot)
{
    for (uint8 i = 0; i < static_cast<uint8>(HazardId::COUNT); ++i)
    {
        if (GetHazardCreature(bot, static_cast<HazardId>(i)))
            return true;
    }

    return false;
}

bool HasLightningSurge(Player* bot)
{
    return bot->HasAura(SPELL_LIGHTNING_SURGE);
}

// Same walk as fork strategy/dungeons/Mech/MechActions.cpp PartyCentroid
Player* GetNearestGroupMember(Player* bot, float maxDist)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Player* best = nullptr;
    float bestDist = maxDist;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        float const dist = bot->GetExactDist2d(member);
        if (dist < bestDist)
        {
            best = member;
            bestDist = dist;
        }
    }

    return best;
}

bool IsFocusDps(Player* bot)
{
    return PlayerBotSpec::IsDps(bot) && !PlayerBotSpec::IsTank(bot) && !PlayerBotSpec::IsHeal(bot);
}

Creature* GetEngagedNeptulon(Player* bot)
{
    Creature* neptulon = FindCreature(bot, NPC_NEPTULON, SCAN_RANGE);
    if (!neptulon || !(bot->IsInCombat() || neptulon->IsInCombat()))
        return nullptr;

    return neptulon;
}

namespace
{
// Stage 1-2 adds on Neptulon (boss_ozumat.cpp:347-385: every add is summoned with AttackStart(Neptulon), so none of
// them is in the group's "attackers" until somebody hits it). Mindlashers first (three kills end stage 1, and they
// cast), then the Behemoth, the Blight Beasts, the murloc packs; any other unit on Neptulon last.
uint8 NeptulonAddRank(Unit* unit)
{
    switch (unit->GetEntry())
    {
        case NPC_VICIOUS_MINDLASHER:
            return 1;
        case NPC_UNYIELDING_BEHEMOTH:
            return 2;
        case NPC_BLIGHT_BEAST:
            return 3;
        case NPC_DEEP_MURLOC_INVADER:
            return 4;
        default:
            return 5;
    }
}

Unit* GetNeptulonAdd(Player* bot)
{
    Creature* neptulon = GetEngagedNeptulon(bot);
    if (!neptulon)
        return nullptr;

    Unit* best = nullptr;
    uint8 bestRank = 0;
    float bestDist = 0.0f;
    for (Unit* attacker : neptulon->getAttackers())
    {
        if (!attacker || !attacker->IsAlive() || !bot->IsValidAttackTarget(attacker))
            continue;

        uint8 const rank = NeptulonAddRank(attacker);
        float const dist = bot->GetExactDist2d(attacker);
        if (!best || rank < bestRank || (rank == bestRank && dist < bestDist))
        {
            best = attacker;
            bestRank = rank;
            bestDist = dist;
        }
    }

    return best;
}
}  // namespace

Unit* GetFocusTarget(Player* bot, FocusId id)
{
    switch (id)
    {
        case FocusId::NAZJAR_ADDS:
        {
            if (!bot->IsInCombat())
                return nullptr;

            // Only while Naz'jar channels Waterspout: she deflects spells and ranged hits, the adds are what dies
            Creature* nazjar = FindCreature(bot, NPC_LADY_NAZJAR, SCAN_RANGE);
            if (!nazjar || !nazjar->HasAura(SPELL_WATERSPOUT))
                return nullptr;

            Creature* witch = FindCreature(bot, NPC_TEMPEST_WITCH, SCAN_RANGE);
            return witch ? witch : FindCreature(bot, NPC_HONOR_GUARD, SCAN_RANGE);
        }
        case FocusId::SAPPERS:
            // passive, channelling on Neptulon (boss_ozumat.cpp:361-365): the group may be out of combat
            if (!GetEngagedNeptulon(bot))
                return nullptr;

            return FindCreature(bot, NPC_FACELESS_SAPPER, SCAN_RANGE);
        case FocusId::NEPTULON_ADDS:
            return GetNeptulonAdd(bot);
        case FocusId::OZUMAT:
        {
            // ACTION_OZUMAT_START (boss_ozumat.cpp:516-523) clears his NON_ATTACKABLE / NOT_SELECTABLE / IMMUNE flags
            if (!GetEngagedNeptulon(bot))
                return nullptr;

            Creature* ozumat = FindCreature(bot, NPC_OZUMAT, SCAN_RANGE);
            return (ozumat && bot->IsValidAttackTarget(ozumat)) ? ozumat : nullptr;
        }
        default:
            return nullptr;
    }
}

Unit* GetAnyFocusTarget(Player* bot)
{
    for (uint8 i = 0; i < static_cast<uint8>(FocusId::COUNT); ++i)
    {
        if (Unit* target = GetFocusTarget(bot, static_cast<FocusId>(i)))
            return target;
    }

    return nullptr;
}

bool GhurshaAbsorbing(Player* bot)
{
    Creature* ghursha = FindCreature(bot, NPC_GHURSHA, SCAN_RANGE);
    return ghursha && ghursha->HasAura(SPELL_ABSORB_MAGIC);
}

Unit* GetInterruptTarget(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return nullptr;

    Unit* best = nullptr;
    uint8 bestPrio = 0;
    float bestDist = 0.0f;
    GuidVector const attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* caster = botAI->GetUnit(guid);
        if (!caster)
            continue;

        float const dist = bot->GetExactDist2d(caster);
        if (dist > INTERRUPT_SCAN)
            continue;

        uint8 const prio = CurrentCastPriority(caster);
        if (!prio)
            continue;

        if (!best || prio < bestPrio || (prio == bestPrio && dist < bestDist))
        {
            best = caster;
            bestPrio = prio;
            bestDist = dist;
        }
    }

    return best;
}

bool CastInterrupt(PlayerbotAI* botAI, Unit* target)
{
    if (!target || !target->IsNonMeleeSpellCasted(true))
        return false;

    for (char const* spellName : INTERRUPT_SPELLS)
    {
        std::string const name = spellName;

        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
        if (!spellId || !StopsACast(spellId))
            continue;

        if (!botAI->CanCastSpell(name, target))
            continue;

        if (botAI->CastSpell(name, target))
            return true;
    }

    return false;
}

bool InEvadeDanger(Player* bot)
{
    if (!bot->IsInCombat())
        return false;

    return HasLightningSurge(bot) || InAnyHazard(bot);
}
}  // namespace CataTott
