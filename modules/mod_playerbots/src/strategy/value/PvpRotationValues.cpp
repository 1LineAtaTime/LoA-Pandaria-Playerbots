/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #855 (epic #854, rotation lane 0): see PvpRotationValues.h.
 */

#include "PvpRotationValues.h"

#include "Battleground.h"
#include "BattlegroundDG.h"
#include "BattlegroundEY.h"
#include "BattlegroundTOK.h"
#include "BattlegroundWS.h"
#include "Map.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellInfo.h"

namespace
{
    // MoP "valid enemies around" for the interrupt rows: the "random enemy" rows have no range condition of their
    // own, the spell's range decides (5-40 yd). The contract fixes the scan at 30 yd; CastSpellOnEnemyCasterAction
    // then checks the interrupt's own range.
    constexpr float ENEMY_CASTER_RANGE = 30.0f;

    // Mechanics that count as crowd control for "is cc" (contract: stunned, feared, polymorphed / incapacitated,
    // rooted or silenced). Snare, daze, disarm, bleed etc. are not CC.
    constexpr uint32 CC_MECHANIC_MASK =
        (1 << MECHANIC_CHARM) | (1 << MECHANIC_DISORIENTED) | (1 << MECHANIC_FEAR) | (1 << MECHANIC_ROOT) |
        (1 << MECHANIC_SILENCE) | (1 << MECHANIC_SLEEP) | (1 << MECHANIC_STUN) | (1 << MECHANIC_FREEZE) |
        (1 << MECHANIC_KNOCKOUT) | (1 << MECHANIC_POLYMORPH) | (1 << MECHANIC_BANISH) | (1 << MECHANIC_SHACKLE) |
        (1 << MECHANIC_TURN) | (1 << MECHANIC_HORROR) | (1 << MECHANIC_SAPPED);

    bool IsSpellInterruptible(Spell const* spell, CurrentSpellTypes type)
    {
        if (!spell)
            return false;

        SpellInfo const* info = spell->GetSpellInfo();
        if (!info)
            return false;

        // Same test as Spell::EffectInterruptCast (SpellEffects.cpp): only a spell that is being cast (or prepared with
        // a cast time), has the silence prevention type and the interrupt flag of its slot can be stopped.
        if (!(spell->getState() == SPELL_STATE_CASTING ||
              (spell->getState() == SPELL_STATE_PREPARING && spell->GetCastTime() > 0)))
            return false;

        if (!(info->PreventionType & SPELL_PREVENTION_TYPE_SILENCE))
            return false;

        if (type == CURRENT_GENERIC_SPELL)
            return (info->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT) != 0;

        return (info->ChannelInterruptFlags[0] & CHANNEL_INTERRUPT_FLAG_INTERRUPT) != 0;
    }

    bool IsCastingPositive(Unit* unit)
    {
        for (CurrentSpellTypes type : { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL })
            if (Spell* spell = unit->GetCurrentSpell(type))
                if (spell->GetSpellInfo() && spell->GetSpellInfo()->IsPositive())
                    return true;

        return false;
    }
}

bool EnemyCasterTargetValue::IsCastingInterruptible(Unit* unit)
{
    if (!unit)
        return false;

    return IsSpellInterruptible(unit->GetCurrentSpell(CURRENT_GENERIC_SPELL), CURRENT_GENERIC_SPELL) ||
           IsSpellInterruptible(unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL), CURRENT_CHANNELED_SPELL);
}

// MoP rows "<class> <interrupt> victim" (target type 1) + "<class> <interrupt> random enemy"
// (target type 2, condition 17 "casting any spell (interruptible)"), priority group 10: the victim first, then every valid
// enemy around. The contract orders players before healers before the rest, nearest first inside a rank.
Unit* EnemyCasterTargetValue::Calculate()
{
    if (!bot->IsInWorld() || !bot->IsAlive())
        return nullptr;

    std::vector<Unit*> candidates;
    if (Unit* victim = AI_VALUE(Unit*, "current target"))
        candidates.push_back(victim);

    GuidVector const attackers = AI_VALUE(GuidVector, "attackers");
    for (ObjectGuid const guid : attackers)
        if (Unit* unit = botAI->GetUnit(guid))
            candidates.push_back(unit);

    Unit* best = nullptr;
    int32 bestRank = -1;
    float bestDist = 0.0f;
    for (Unit* unit : candidates)
    {
        if (!unit->IsInWorld() || !unit->IsAlive() || unit->GetMapId() != bot->GetMapId())
            continue;

        if (!IsCastingInterruptible(unit))
            continue;

        if (!bot->IsWithinDistInMap(unit, ENEMY_CASTER_RANGE))
            continue;

        if (!bot->IsValidAttackTarget(unit) || !bot->IsWithinLOSInMap(unit))
            continue;

        Player* player = unit->ToPlayer();
        bool const healer = IsCastingPositive(unit) || (player && PlayerBotSpec::IsHeal(player, true));
        int32 const rank = (player ? 2 : 0) + (healer ? 1 : 0);
        float const dist = bot->GetDistance(unit);
        if (rank > bestRank || (rank == bestRank && dist < bestDist))
        {
            best = unit;
            bestRank = rank;
            bestDist = dist;
        }
    }

    return best;
}

bool IsCcValue::IsCrowdControlled(Unit* unit)
{
    if (!unit)
        return false;

    // MoP: aura types 5 confuse, 6 charm, 7 fear, 12 stun, 407 fear 2. The unit states cover the
    // same CC applied by the core directly (stun / fleeing / confused).
    if (unit->HasUnitState(UNIT_STATE_STUNNED | UNIT_STATE_FLEEING | UNIT_STATE_CONFUSED))
        return true;

    if (unit->HasAuraType(SPELL_AURA_MOD_STUN) || unit->HasAuraType(SPELL_AURA_MOD_FEAR) ||
        unit->HasAuraType(SPELL_AURA_MOD_FEAR_2) || unit->HasAuraType(SPELL_AURA_MOD_CONFUSE) ||
        unit->HasAuraType(SPELL_AURA_MOD_CHARM) || unit->HasAuraType(SPELL_AURA_MOD_ROOT) ||
        unit->HasAuraType(SPELL_AURA_MOD_SILENCE) || unit->HasAuraType(SPELL_AURA_MOD_PACIFY_SILENCE))
        return true;

    return unit->HasAuraWithMechanic(CC_MECHANIC_MASK);
}

bool IsCcValue::Calculate()
{
    return IsCrowdControlled(AI_VALUE(Unit*, qualifier.empty() ? "current target" : qualifier));
}

bool CarryingObjectiveValue::IsCarrying(Unit* unit)
{
    Player* player = unit ? unit->ToPlayer() : nullptr;
    if (!player || !player->InBattleground())
        return false;

    // Holder auras (BattlegroundWS.h / BattlegroundTP.h 23333 + 23335, BattlegroundEY.h 34976, BattlegroundTOK.h orbs,
    // BattlegroundDG.h carts) - the same cues BGTactics::isCarryingObjective / isCarryingTOK / isCarryingDG read.
    if (player->HasAura(BG_WS_SPELL_WARSONG_FLAG) || player->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
        player->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL) || player->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_1) ||
        player->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_2) || player->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_3) ||
        player->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_4) || player->HasAura(BG_DG_ALLIANCE_MINE_CART) ||
        player->HasAura(BG_DG_HORDE_MINE_CART))
        return true;

    // The core's picker table (WSG / TP / DG per team, EotS with -1) for the frame between the pick-up and the aura.
    Battleground* bg = player->GetBattleground();
    if (!bg)
        return false;

    ObjectGuid const guid = player->GetGUID();
    return bg->GetFlagPickerGUID(-1) == guid || bg->GetFlagPickerGUID(int32(TEAM_ALLIANCE)) == guid ||
           bg->GetFlagPickerGUID(int32(TEAM_HORDE)) == guid;
}

bool CarryingObjectiveValue::Calculate()
{
    return IsCarrying(qualifier.empty() ? static_cast<Unit*>(bot) : AI_VALUE(Unit*, qualifier));
}

// #868: a running duel counts as PvP too (name kept for every lane-0 caller). MoP gates none of its PvP rows on the map
// except three (rogue Stealth in a battleground, Blizzard "not in
// arena"), so interrupts on a caster, gap closers, CC guards and low-health defensives fire against a duel opponent there;
// The fork's lanes gated those
// rows on this value to keep PvE AC-like; a duel is PvP, so they apply to the duel opponent. startTime != 0: the fight has
// begun (the countdown is not hostile yet).
bool InPvpInstanceValue::Calculate()
{
    Map* map = bot->IsInWorld() ? bot->GetMap() : nullptr;
    if (map && map->IsBattlegroundOrArena())
        return true;
    return bot->duel && bot->duel->opponent && bot->duel->startTime != 0;
}
