/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#include "CataBcShared.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <string>
#include <vector>

namespace CataBc
{
namespace
{
// Interruptible casts of this dungeon, most urgent first (0 = not listed).
uint8 InterruptPriority(uint32 spellId)
{
    switch (spellId)
    {
        case SPELL_DARK_COMMAND:  // Corla: fear + 30000 shadow damage on a random player, 2 s cast, "must be interrupted"
            return 1;
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

float SegmentPointDist(float ax, float ay, float bx, float by, float px, float py, float& cx, float& cy)
{
    float const dx = bx - ax;
    float const dy = by - ay;
    float const len2 = dx * dx + dy * dy;
    float t = len2 < 1e-6f ? 0.0f : ((px - ax) * dx + (py - ay) * dy) / len2;
    t = std::max(0.0f, std::min(1.0f, t));
    cx = ax + t * dx;
    cy = ay + t * dy;
    return std::hypot(px - cx, py - cy);
}
}  // namespace

Creature* FindCreature(Player* bot, uint32 entry, float range)
{
    return bot->FindNearestCreature(entry, range, true);
}

bool IsCasting(Unit* unit, uint32 spellId)
{
    return unit && unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(spellId);
}

// Rom'ogg ---------------------------------------------------------------------------------------------------------
Creature* GetRomogg(Player* bot)
{
    return FindCreature(bot, NPC_ROMOGG, 40.0f);
}

Creature* GetChains(Player* bot)
{
    return FindCreature(bot, NPC_CHAINS_OF_WOE, 40.0f);
}

bool IsChained(Player* bot)
{
    return bot->HasAura(SPELL_CHAINS_OF_WOE_ROOT);
}

// Rom'ogg is winding up The Skullcracker (8 s cast, 237k physical to everything within 12 yd) and the bot is free to
// move and still inside the blast: run out. A chained bot cannot move; it breaks the chains instead.
bool SkullcrackerDanger(Player* bot)
{
    if (!bot->IsInCombat())
        return false;

    Creature* boss = GetRomogg(bot);
    if (!IsCasting(boss, SPELL_THE_SKULLCRACKER))
        return false;

    if (IsChained(bot))
        return false;

    return bot->GetExactDist2d(boss) < SKULLCRACKER_SAFE_RADIUS;
}

// Corla ----------------------------------------------------------------------------------------------------------
// The bot is carrying a dangerous Evolution stack count AND is still on (or next to) a beam. Out of the beam the
// stacks simply run out (15 s duration), so the evade multiplier must not freeze the bot for that whole time.
bool EvolutionDanger(Player* bot)
{
    Aura* evolution = bot->GetAura(SPELL_EVOLUTION);
    if (!evolution || evolution->GetStackAmount() < EVOLUTION_LEAVE_STACKS)
        return false;

    float px = 0.0f;
    float py = 0.0f;
    return GetNearestBeamPoint(bot, px, py) && bot->GetExactDist2d(px, py) < BEAM_CLEAR_RADIUS;
}

bool GetNearestBeamPoint(Player* bot, float& px, float& py)
{
    std::list<Creature*> zealots;
    std::list<Creature*> triggers;
    bot->GetCreatureListWithEntryInGrid(zealots, NPC_TWILIGHT_ZEALOT_CORLA, 100.0f);
    bot->GetCreatureListWithEntryInGrid(triggers, NPC_NETHER_ESSENCE_TRIGGER, 100.0f);

    bool found = false;
    float best = 0.0f;
    for (Creature* zealot : zealots)
    {
        if (!zealot->IsAlive())
            continue;

        for (Creature* trigger : triggers)
        {
            if (!trigger->IsAlive())
                continue;

            float cx = 0.0f;
            float cy = 0.0f;
            float const dist = SegmentPointDist(trigger->GetPositionX(), trigger->GetPositionY(),
                                                zealot->GetPositionX(), zealot->GetPositionY(),
                                                bot->GetPositionX(), bot->GetPositionY(), cx, cy);
            if (!found || dist < best)
            {
                found = true;
                best = dist;
                px = cx;
                py = cy;
            }
        }
    }

    return found;
}

// Karsh -----------------------------------------------------------------------------------------------------------
Creature* GetKarsh(Player* bot)
{
    return FindCreature(bot, NPC_KARSH, 60.0f);
}

uint32 KarshSuperheatStacks(Unit* karsh)
{
    Aura* heat = karsh ? karsh->GetAura(SPELL_SUPERHEATED_QUICKSILVER_ARMOR) : nullptr;
    return heat ? heat->GetStackAmount() : 0;
}

void GetKarshPost(Unit* karsh, float& tx, float& ty)
{
    float const angle = std::atan2(KARSH_PILLAR_Y - karsh->GetPositionY(), KARSH_PILLAR_X - karsh->GetPositionX());
    tx = KARSH_PILLAR_X + std::cos(angle) * KARSH_FAR_SIDE;
    ty = KARSH_PILLAR_Y + std::sin(angle) * KARSH_FAR_SIDE;
}

// Beauty ----------------------------------------------------------------------------------------------------------
// Lava Drool (Lucky / Buster, SmartAI) is a persistent area aura: a DynamicObject of spell 76628, 5 yd, 20 s.
bool GetLavaDroolDanger(Player* bot, float& cx, float& cy)
{
    if (!bot->IsInCombat())
        return false;

    bool found = false;
    float best = 0.0f;
    for (Position const& pos : EncounterHelpers::GetDynamicObjectPositions(bot, LAVA_DROOL_SCAN, SPELL_LAVA_DROOL))
    {
        float const dist = bot->GetExactDist2d(pos);
        if (dist >= LAVA_DROOL_SAFE_RADIUS - 1.0f)
            continue;

        if (!found || dist < best)
        {
            found = true;
            best = dist;
            cx = pos.GetPositionX();
            cy = pos.GetPositionY();
        }
    }

    return found;
}

// Obsidius --------------------------------------------------------------------------------------------------------
// The Shadows of Obsidius carry "absorb all incoming damage" (aura 76164): hitting one is wasted damage and, via the
// "Twitchy" script, hands it to whoever hit it. Bots leave them alone while Obsidius is around.
GuidVector FindShadowGuids(Player* bot)
{
    GuidVector guids;
    if (!FindCreature(bot, NPC_OBSIDIUS, 80.0f))
        return guids;

    std::list<Creature*> shadows;
    bot->GetCreatureListWithEntryInGrid(shadows, NPC_SHADOW_OF_OBSIDIUS, 80.0f);
    for (Creature* shadow : shadows)
    {
        if (shadow->IsAlive())
            guids.push_back(shadow->GetGUID());
    }

    return guids;
}

// Interrupts ------------------------------------------------------------------------------------------------------
Unit* GetInterruptTarget(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return nullptr;

    Unit* best = nullptr;
    uint8 bestPrio = 0;
    float bestDist = 0.0f;
    GuidVector const& attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
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

    if (SkullcrackerDanger(bot))
        return true;

    if (bot->HasAura(SPELL_QUAKE))
        return true;

    float cx = 0.0f;
    float cy = 0.0f;
    if (GetLavaDroolDanger(bot, cx, cy))
        return true;

    return EvolutionDanger(bot);
}
}  // namespace CataBc
