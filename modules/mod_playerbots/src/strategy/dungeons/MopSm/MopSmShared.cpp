/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" (Scarlet Monastery, epic #373, issue #658). See MopSmShared.h for sources.

#include "MopSmShared.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "DungeonStrategyUtils.h"
#include "Player.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <cfloat>
#include <list>

namespace MopScarletMonastery
{
namespace
{
// Same stop-a-cast test as the fork's MgT strategy (MgTShared.cpp:507): a silence-type interrupt, an explicit
// interrupt-cast effect, or a hard stun/silence aura.
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

// Dedicated interrupts first, hard control last (MgTShared.cpp:545-566, plus the MoP-only names: Monk
// "spear hand strike", Paladin "rebuke", Druid "skull bash", Hunter "counter shot").
char const* const INTERRUPT_SPELLS[] = {
    "kick", "pummel", "shield bash", "wind shear", "mind freeze", "counterspell", "spell lock",
    "spear hand strike", "rebuke", "skull bash", "counter shot", "silencing shot",
    "shockwave", "shadowfury", "hammer of justice", "kidney shot", "silence", "bash", "strangulate",
    "arcane torrent"
};

// #684: offensive magic dispels that remove Whitemane's Power Word: Shield (AC BTActions.cpp:1120-1128 casts "spellsteal"
// by name the same way). MoP: priest "dispel magic" is the offensive one (Purify is the friendly dispel).
char const* const SHIELD_DISPEL_SPELLS[] = { "spellsteal", "purge", "dispel magic", "tranquilizing shot" };
}

Creature* FindBoss(Player* bot, uint32 entry)
{
    return bot->FindNearestCreature(entry, BOSS_SCAN_RANGE, true);
}

Creature* FindNearestHazard(Player* bot, Hazard const& hazard, float& clearDistance)
{
    clearDistance = hazard.radius + hazard.margin;

    std::list<Creature*> carriers;
    bot->GetCreatureListWithEntryInGrid(carriers, hazard.entry, clearDistance + 10.0f);

    Creature* nearest = nullptr;
    float nearestDist = clearDistance;
    for (Creature* carrier : carriers)
    {
        if (!carrier || !carrier->IsAlive() || (hazard.aura && !carrier->HasAura(hazard.aura)))
            continue;

        float const dist = bot->GetExactDist2d(carrier);
        if (dist < nearestDist)
        {
            nearest = carrier;
            nearestDist = dist;
        }
    }

    return nearest;
}

uint8 InterruptPriority(uint32 spellId)
{
    switch (spellId)
    {
        case SPELL_MASS_RESURRECTION:  // raises every dead Scarlet in 100 yd - "Deadly" per Wowpedia
            return 1;
        case SPELL_SPIRIT_GALE:        // 122k + a 20 s damaging gale
            return 2;
        case SPELL_HEAL:               // Whitemane healing herself / Durand
            return 3;
        case SPELL_HOLY_SMITE:
            return 4;
        default:                       // incl. SPELL_SCARLET_RESURRECTION: never interrupted
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

Unit* FindInterruptTarget(Player* bot)
{
    Unit* best = nullptr;
    uint8 bestPrio = 0;
    for (uint32 entry : { NPC_HIGH_INQUISITOR_WHITEMANE, NPC_THALNOS_THE_SOULRENDER })
    {
        Creature* caster = FindBoss(bot, entry);
        if (!caster || !caster->IsNonMeleeSpellCasted(true))
            continue;

        uint8 const prio = CurrentCastPriority(caster);
        if (prio && (!bestPrio || prio < bestPrio))
        {
            best = caster;
            bestPrio = prio;
        }
    }

    return best;
}

std::string CastInterrupt(PlayerbotAI* botAI, Unit* target)
{
    if (!target->IsNonMeleeSpellCasted(true))
        return {};

    for (char const* spellName : INTERRUPT_SPELLS)
    {
        std::string const name = spellName;
        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
        if (!spellId || !StopsACast(spellId))
            continue;

        if (!botAI->CanCastSpell(name, target))
            continue;

        if (botAI->CastSpell(name, target))
            return name;
    }

    return {};
}

bool IsInterruptActionName(std::string const& name)
{
    for (char const* spellName : INTERRUPT_SPELLS)
    {
        // "kick", "kick on enemy healer", ... all start with the spell name.
        if (name.compare(0, std::char_traits<char>::length(spellName), spellName) == 0)
            return true;
    }

    return false;
}

bool WhitemaneIsResurrecting(Player* bot)
{
    Creature* whitemane = FindBoss(bot, NPC_HIGH_INQUISITOR_WHITEMANE);
    return whitemane && whitemane->FindCurrentSpellBySpellId(SPELL_SCARLET_RESURRECTION);
}

Unit* FindShieldedWhitemane(Player* bot)
{
    Creature* whitemane = FindBoss(bot, NPC_HIGH_INQUISITOR_WHITEMANE);
    if (!whitemane || !whitemane->IsInCombat() || !whitemane->HasAura(SPELL_WHITEMANE_SHIELD))
        return nullptr;

    return bot->IsValidAttackTarget(whitemane) ? whitemane : nullptr;
}

bool HasShieldDispel(PlayerbotAI* botAI)
{
    for (char const* spellName : SHIELD_DISPEL_SPELLS)
        if (botAI->GetAiObjectContext()->GetValue<uint32>("spell id", std::string(spellName))->Get())
            return true;

    return false;
}

std::string CastShieldDispel(PlayerbotAI* botAI, Unit* target)
{
    for (char const* spellName : SHIELD_DISPEL_SPELLS)
    {
        std::string const name = spellName;
        if (!botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get())
            continue;

        if (!botAI->CanCastSpell(name, target))
            continue;

        if (botAI->CastSpell(name, target))
            return name;
    }

    return {};
}

bool FirestormKickNear(Player* bot, Creature*& boss)
{
    boss = FindBoss(bot, NPC_BROTHER_KORLOFF);
    if (!boss || !boss->HasAura(SPELL_FIRESTORM_KICK))
        return false;

    return bot->GetExactDist2d(boss) <= FIRESTORM_REACT_RANGE;
}

Unit* FindFocusAdd(Player* bot)
{
    // Damage dealers only: the tank picks the adds up through threat, the healer heals.
    if (!PlayerBotSpec::IsDps(bot) || PlayerBotSpec::IsTank(bot))
        return nullptr;

    for (uint32 entry : { NPC_EMPOWERING_SPIRIT, NPC_EVICTED_SOUL, NPC_EMPOWERED_ZOMBIE })
    {
        std::list<Creature*> adds;
        bot->GetCreatureListWithEntryInGrid(adds, entry, ADD_SCAN_RANGE);

        Creature* best = nullptr;
        float bestDist = FLT_MAX;
        for (Creature* add : adds)
        {
            if (!add || !add->IsAlive() || !bot->IsValidAttackTarget(add))
                continue;

            float const dist = bot->GetExactDist2d(add);
            if (dist < bestDist)
            {
                best = add;
                bestDist = dist;
            }
        }

        if (best)
            return best;
    }

    return nullptr;
}
}
