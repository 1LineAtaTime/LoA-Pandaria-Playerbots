/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple strategy "mop-snt" (epic #373, issue #656) - actions.

#ifndef PLAYERBOTS_MOPSNTACTIONS_H
#define PLAYERBOTS_MOPSNTACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "MopSntShared.h"
#include "MovementActions.h"

// Walks to the point the "mop snt hazard" value computed: away from Thousand Blades, out of the Dashing Strike /
// Blade Rush line, away from an armed bomb, out of a Caustic Pitch.
class MopSntEscapeAction : public MovementAction
{
public:
    MopSntEscapeAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop snt escape") {}
    bool Execute(Event event) override;
    // #656 w50: false while the bot is still walking to the escape point it was already sent to (logged USELESS, not FAILED)
    bool isUseful() override;

private:
    bool EscapeInFlight(MopSnt::Hazard const& hazard);
};

// Casts the bot's interrupt on the caster FindInterruptTarget() found; #781: out of range, it runs to him (a
// MovementAction for that reason - the hazard multiplier exempts it).
class MopSntInterruptAction : public MovementAction
{
public:
    MopSntInterruptAction(PlayerbotAI* botAI, std::string const name, MopSnt::InterruptGroup group)
        : MovementAction(botAI, name), _group(group)
    {
    }

    bool Execute(Event event) override;
    // #656 w50: false while the bot is already running to him on its chase move
    bool isUseful() override;

private:
    MopSnt::InterruptGroup const _group;
};

// Attacks the Sap Globule named by the "mop snt focus target" value.
class MopSntFocusAction : public AttackAction
{
public:
    MopSntFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "mop snt focus target") {}
    bool Execute(Event event) override;
};

// Pa'valak's Bulwark (119476, absorbs 9000000) is broken with the Siege Explosives the sappers drop: walk to an
// unarmed bomb, spellclick it (npc_spellclick_spells 61452 -> 119388, "Throw the bomb!") and throw it (119393) at
// Pa'valak. Spellclick as TOC/TOCActions.cpp:218-262 (Unit::HandleSpellClick).
class MopSntSiegeBombAction : public MovementAction
{
public:
    MopSntSiegeBombAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop snt siege bomb") {}
    bool Execute(Event event) override;

private:
    uint32 _lastThrow = 0;
    // #781: one throw per pick-up - the carry aura 119388 re-applies the 119386 override every second
    Aura const* _usedCarry = nullptr;
    time_t _usedCarryApplied = 0;
};

// #781: walk into Jin'bak's Sap Puddle (its inner half) to shrink it before Detonate.
class MopSntSoakPuddleAction : public MovementAction
{
public:
    MopSntSoakPuddleAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop snt soak puddle") {}
    bool Execute(Event event) override;
};

// #687 Quick-Dry Resin: "Jump to prevent the resin from hardening!". A real short hop (MotionMaster::MoveJump through
// MovementAction::JumpTo, as the BG tactics and CataHoo use it), about once a second. Toward the escape point when the
// bot has a hazard to leave (a Caustic Pitch), else alternately 2 yd ahead and 2 yd back so it stays in place.
// #656: walk the bridge to Ner'onok after he landed at its other end; #656 w50: already while he flies there
// (MopSnt::NeronokRegroupPoint).
class MopSntNeronokApproachAction : public MovementAction
{
public:
    MopSntNeronokApproachAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop snt neronok approach") {}
    bool Execute(Event event) override;
};

// #656 w54 Vo'jak: a DPS bot clicks a ready Mantid Tar Keg and throws it (123039, ground-targeted) at the waves / Vo'jak, away
// from the group (MopSnt::FindTarTarget). Same click-then-throw shape as MopSntSiegeBombAction.
class MopSntTarKegAction : public MovementAction
{
public:
    MopSntTarKegAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop snt tar keg") {}
    bool Execute(Event event) override;

private:
    uint32 _lastThrow = 0;
};

class MopSntResinHopAction : public MovementAction
{
public:
    MopSntResinHopAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop snt resin") {}
    bool Execute(Event event) override;

private:
    uint32 _lastHop = 0;
    bool _back = false;
};

#endif
