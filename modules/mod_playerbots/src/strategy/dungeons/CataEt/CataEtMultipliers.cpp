/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// End Time bot strategy "cata-et" - multipliers (issue #671, epic #373).

#include "CataEtMultipliers.h"
#include "Action.h"
#include "AttackActions.h"
#include "CataEtActions.h"
#include "CataEtShared.h"
#include "Creature.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "Timer.h"
#include <vector>

namespace
{
constexpr uint32 CACHE_MS = 100;

// Spells that move the caster to / past its target or forward (class action names, Classes/*/*AiObjectContext.cpp).
bool IsCallGapCloser(std::string const& name)
{
    // w67 (#671): + Killing Spree, which teleports the rogue between enemies within 10 yd (other ghouls; back to Murozond).
    return name == "charge" || name == "intercept" || name == "heroic leap" || name == "blink" || name == "blink back" ||
           name == "shadowstep" || name == "roll" || name == "disengage" || name == "killing spree";
}
}

bool CataEtHazardMultiplier::NeedsMove()
{
    uint32 const now = getMSTime();
    if (_stamp && now - _stamp < CACHE_MS)
        return _cached;

    _stamp = now;
    _cached = EndTime::InHazard(bot, EndTime::HazardKind::BlightedArrows) ||
              EndTime::InHazard(bot, EndTime::HazardKind::Moonlance) ||
              EndTime::InHazard(bot, EndTime::HazardKind::EyeOfElune) ||
              EndTime::InHazard(bot, EndTime::HazardKind::DistortionBomb) ||
              EndTime::InBladePath(bot) || EndTime::NeedsFlarecoreAction(bot) || EndTime::NeedsLavaEscape(bot) ||
              EndTime::InInfiniteBreath(bot);
    return _cached;
}

float CataEtHazardMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<CataEtMovementAction*>(action))
        return 1.0f;

    return NeedsMove() ? 0.0f : 1.0f;
}

CataEtSylvanasCallMultiplier::CallState CataEtSylvanasCallMultiplier::GetState()
{
    uint32 const now = getMSTime();
    if (_stamp && now - _stamp < CACHE_MS)
        return _cached;

    _stamp = now;
    _cached = CallState::None;
    _designated = ObjectGuid::Empty;
    if (!EndTime::SylvanasCallActive(bot))
        return _cached;

    std::vector<Creature*> ghouls;
    EndTime::CollectRisenGhouls(bot, ghouls);
    if (ghouls.empty())
        return _cached;

    float angle = 0.0f;
    if (EndTime::GetSylvanasBreachAngle(EndTime::GetSylvanas(bot), ghouls, angle))
    {
        _cached = CallState::Breach;
        return _cached;
    }

    _cached = CallState::Kill;
    if (Creature* ghoul = EndTime::GetDesignatedGhoul(ghouls))
        _designated = ghoul->GetGUID();

    return _cached;
}

float CataEtSylvanasCallMultiplier::GetValue(Action* action)
{
    CallState const state = GetState();
    if (state == CallState::None)
        return 1.0f;

    if (dynamic_cast<CataEtMovementAction*>(action) || dynamic_cast<CataEtKillGhoulAction*>(action))
        return 1.0f;

    // w65 (#671): gap closers put the bot ON or PAST the ghoul it attacks (tank Charge + Heroic Leap: 32.8 yd out, 23 x 101221; mage
    // "enemy is close" Blink: 18.6 yd, past the ghoul, 3 x 57k, dead), and "set behind" / "tank face" walk round the ghoul to its
    // far side - all out of bounds (boss_echo_of_sylvanas.cpp:259-289). Plain reach-melee comes from the centre, the near side.
    if (IsCallGapCloser(action->getName()) || dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    Unit* target = action->GetTarget();
    if (state == CallState::Kill)
    {
        // w67 (#671): when the ghouls closed on the centre, the mage's and the healer's "flee" (enemy too close) walked them out
        // past the west ghoul (101221 54k + the ring's links: the mage died, Call 2 then failed with two DPS and Sacrifice killed
        // tank and rogue). Only reaching / attacking the designated ghoul may move a bot now; the strategy's own moves pass above.
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action) &&
            !dynamic_cast<ReachTargetAction*>(action))
            return 0.0f;

        // The Call is a damage race on ONE ghoul (w67 Call 1: it died at 4% hp 3 yd from Sylvanas); the mage's Blizzard /
        // Flamestrike / Cone of Cold spread over the neighbours. DPS AoE waits (AC HoL / MgT AoE multipliers; heals are
        // CastHealingSpellAction, also Aoe-typed, and stay).
        if (action->getThreatType() == Action::ActionThreatType::Aoe && !PlayerBotSpec::IsHeal(bot) &&
            !dynamic_cast<CastHealingSpellAction*>(action))
            return 0.0f;

        if (!target)
            return 1.0f;

        if (target->GetEntry() == EndTime::ET_NPC_ECHO_OF_SYLVANAS)
            return 0.0f;   // immune for the whole Call (100686)

        if (target->GetEntry() == EndTime::ET_NPC_RISEN_GHOUL && target->GetGUID() != _designated)
            return 0.0f;

        return 1.0f;
    }

    // Breach: stay on the spot until Sacrifice has gone off.
    if (target && target->GetEntry() == EndTime::ET_NPC_RISEN_GHOUL)
        return 0.0f;

    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 0.0f;

    return 1.0f;
}

float CataEtTyrandeEventMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<CataEtMovementAction*>(action) ||
        dynamic_cast<AttackAction*>(action))
        return 1.0f;

    uint32 const now = getMSTime();
    if (!_stamp || now - _stamp >= CACHE_MS)
    {
        _stamp = now;
        _cached = EndTime::TyrandeEventActive(bot);
    }

    return _cached ? 0.0f : 1.0f;
}

float CataEtHourglassMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<CataEtMovementAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) && !IsCallGapCloser(action->getName()))
        return 1.0f;

    uint32 const now = getMSTime();
    if (!_stamp || now - _stamp >= CACHE_MS)
    {
        _stamp = now;
        _cached = EndTime::NeedsHourglass(bot);
    }

    return _cached ? 0.0f : 1.0f;
}

float CataEtTyrandeKickMultiplier::GetValue(Action* action)
{
    if (!EndTime::IsInterruptActionName(action->getName()))
        return 1.0f;

    Creature* tyrande = EndTime::GetTyrande(bot);
    if (!tyrande || !tyrande->IsInCombat() || tyrande->HasAura(EndTime::ET_SPELL_IN_SHADOW))
        return 1.0f;

    return EndTime::TyrandeNeedsInterrupt(bot) ? 1.0f : 0.0f;
}

float CataEtTyrandeBandMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<ReachSpellAction*>(action) && !dynamic_cast<ReachPartyMemberToHealAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (!PlayerBotSpec::IsRanged(bot))
        return 1.0f;

    uint32 const now = getMSTime();
    if (!_stamp || now - _stamp >= CACHE_MS)
    {
        _stamp = now;
        _cached = EndTime::TyrandeFightActive(bot);
    }

    return _cached ? 0.0f : 1.0f;
}
