/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Gurub (Cataclysm 5-man) multipliers, MoP-only lane L7 (#670). Pattern: AC Ai/Dungeon/HoS/HoSMultipliers.cpp and
// MgTMultipliers.cpp (the same "competing movement" and "target picker" action sets).

#include "CataZgMultipliers.h"
#include "Action.h"
#include "CataZgShared.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "ReachTargetActions.h"
#include "Timer.h"
#include <initializer_list>
#include <vector>

namespace Zg = CataZulGurub;

namespace
{
// MgTMultipliers.cpp MgTDampeningFieldMultiplier / MgTGravityLapseMultiplier: the actions that walk a bot toward the
// group, its target or its formation slot.
bool ZgIsCompetingMovement(Action* action)
{
    return dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action) ||
           dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<ReachTargetAction*>(action) ||
           dynamic_cast<CastReachTargetSpellAction*>(action);
}

bool ZgIsTargetPicker(Action* action)
{
    return dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action) ||
           dynamic_cast<TankAssistAction*>(action);
}
}

bool CataZgCachedMultiplier::Cached()
{
    if (!_valid || GetMSTimeDiffToNow(_stamp) >= 100)
    {
        _value = Compute();
        _stamp = getMSTime();
        _valid = true;
    }

    return _value;
}

bool CataZgHazardMultiplier::Compute()
{
    if (!bot->IsInCombat() || bot->GetMapId() != Zg::MAP_ZUL_GURUB)
        return false;

    // #670 w68: the tank walking Venoxis to the floor spot must not be pulled back up by "reach melee" / "tank face".
    float tx = 0.0f;
    float ty = 0.0f;
    float tz = 0.0f;
    if (Zg::GetVenoxisTankSpot(bot, tx, ty, tz))
        return true;

    for (Zg::HazardKind kind : { Zg::HazardKind::AcridPools, Zg::HazardKind::Bloodvenom,
                                 Zg::HazardKind::DevastatingSlam, Zg::HazardKind::WaveOfAgony,
                                 Zg::HazardKind::ZanzilFire, Zg::HazardKind::Deadzone, Zg::HazardKind::BreathOfHethiss,
                                 Zg::HazardKind::BerserkerPursuit, Zg::HazardKind::PoisonCloud,
                                 Zg::HazardKind::VenomousEffusion })
    {
        std::vector<Zg::Hazard> hazards;
        Zg::CollectHazards(bot, kind, hazards);
        if (!hazards.empty() &&
            Zg::MinSlack(hazards, bot->GetPositionX(), bot->GetPositionY()) < Zg::CLEAR_MARGIN)
            return true;
    }

    return false;
}

float CataZgHazardMultiplier::GetValue(Action* action)
{
    if (!ZgIsCompetingMovement(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}

bool CataZgLinkMultiplier::Compute()
{
    // The tank keeps holding Venoxis while linked (#670 w67, CataZgShared.cpp GetToxicPartner).
    return bot->GetMapId() == Zg::MAP_ZUL_GURUB && bot->IsInCombat() && bot->HasAura(Zg::SPELL_TOXIC_LINK_AURA) &&
           !PlayerBotSpec::IsTank(bot);
}

float CataZgLinkMultiplier::GetValue(Action* action)
{
    if (!ZgIsCompetingMovement(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}

bool CataZgWhispersInterruptMultiplier::Compute()
{
    return Zg::IsVenoxisWhispersFight(bot);
}

float CataZgWhispersInterruptMultiplier::GetValue(Action* action)
{
    if (!action || !Zg::IsClassInterruptAction(action->getName()))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}

bool CataZgVenoxisCloudMultiplier::Compute()
{
    return Zg::IsVenoxisInPoisonCloud(bot);
}

float CataZgVenoxisCloudMultiplier::GetValue(Action* action)
{
    // #670 w68: while Venoxis stands in his Poison Clouds (the pull on the altar, Bloodvenom) nobody walks in after him - reach,
    // "behind" and "tank face" moves wait until the tank has him on the floor (w68: rogue, tank and healer all died up there).
    if (!dynamic_cast<ReachTargetAction*>(action) && !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}

bool CataZgShadowsMultiplier::Compute()
{
    return bot->GetMapId() == Zg::MAP_ZUL_GURUB && bot->IsInCombat() && Zg::IsShadowsOfHakkarActive(bot);
}

float CataZgShadowsMultiplier::GetValue(Action* action)
{
    if (!ZgIsCompetingMovement(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}

bool CataZgChainMultiplier::Compute()
{
    return Zg::GetChainToStandAt(bot) != nullptr;
}

float CataZgChainMultiplier::GetValue(Action* action)
{
    if (!ZgIsCompetingMovement(action))
        return 1.0f;

    if (Cached())
        return 0.0f;

    // #670 w65/w67: once a chain was exposed (GetChainToStandAt = nullptr) the healer's default "follow" (relevance 1) walked it
    // back to the headless master on the leg stage, 60 yd north of the chains (w67 Rederios (-11796,-1670) -> (-11789,-1614) in
    // 10 s), out of heal range; the rogue died at 3% on chain 1431 and the w65 healer did the same. For the whole of phase 2 the
    // group stays at the chains: no follow and no formation move back toward the master.
    if ((dynamic_cast<FollowAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action)) && Zg::IsChainPhase(bot))
        return 0.0f;

    return 1.0f;
}

bool CataZgSpareSpiritMultiplier::Compute()
{
    return bot->GetMapId() == Zg::MAP_ZUL_GURUB && bot->IsInCombat() && !PlayerBotSpec::IsTank(bot) &&
           Zg::GetShieldedChain(bot) != nullptr;
}

float CataZgSpareSpiritMultiplier::GetValue(Action* action)
{
    if (!ZgIsTargetPicker(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}

bool CataZgFocusMultiplier::Compute()
{
    return Zg::GetFocusTarget(bot) != nullptr;
}

float CataZgFocusMultiplier::GetValue(Action* action)
{
    if (!ZgIsTargetPicker(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}

bool CataZgFormulaMultiplier::Compute()
{
    // #679: Toxic Torment 96328 is a Magic aura as well (SpellCategories Dispel 1); it has to last through the Graveyard Gas.
    return bot->GetMapId() == Zg::MAP_ZUL_GURUB && bot->IsInCombat() && Zg::GroupHasCauldronBuff(bot);
}

float CataZgFormulaMultiplier::GetValue(Action* action)
{
    // CastCureSpellAction: dispel / cleanse on self; CurePartyMemberAction: the same on a party member.
    if (!dynamic_cast<CastCureSpellAction*>(action) && !dynamic_cast<CurePartyMemberAction*>(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}
