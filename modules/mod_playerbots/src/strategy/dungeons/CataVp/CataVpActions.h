/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAVPACTIONS_H
#define PLAYERBOTS_CATAVPACTIONS_H

#include "Action.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// Base of this dungeon's movement actions: the movement lock multiplier lets exactly these through.
class VpMoveAction : public MovementAction
{
public:
    VpMoveAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    // Go to (x, y) on the ground at the bot's height; forced priority so it overrides the usual combat movement.
    bool GoTo(float x, float y, MovementPriority priority = MovementPriority::MOVEMENT_FORCED);
};

// Grand Vizier Ertan - Cyclone Shield / Storm's Edge (Wowpedia Grand_Vizier_Ertan 2026-10-06; Wowhead spell
// 86309 "enemies outside the storm's circumference", 86292 cyclone touch 9.6-11.7k and -85% speed): stay inside
// the ring of cyclones (npc 46007); when it is called in, stack under the boss. Script: boss_grand_vizier_ertan.cpp
// :178-202 (Storm's Edge on every player farther than a cyclone), :241-253 (ring pulled in to 9 yd, #663 w64).
class VpErtanRingAction : public VpMoveAction
{
public:
    VpErtanRingAction(PlayerbotAI* botAI) : VpMoveAction(botAI, "vp ertan ring") {}
    bool Execute(Event event) override;
};

// Altairus - Chilling Breath 88308 (Wowpedia Altairus 2026-10-06; Wowhead: 2 s cast, 47.5-52.5k frost to every
// enemy in a 120 degree cone, SpellTargetRestrictions.dbc cone 120): while the boss casts it, a bot inside the
// cone (other than the target, whom the boss turns to) steps sideways out of it when that is reachable in time.
class VpAltairusBreathAction : public VpMoveAction
{
public:
    VpAltairusBreathAction(PlayerbotAI* botAI) : VpMoveAction(botAI, "vp altairus breath") {}
    bool Execute(Event event) override;
};

// Altairus (heroic only) - Twisting Winds (Wowpedia Altairus 2026-10-06; Wowhead 88313/88314: 11.7-12.3k nature
// and knock up within 7 yd of each twister, pulsing every 0.5 s): keep clear of the twisters (npc 47342). They
// only exist on heroic (boss_altairus.cpp:99, :207), so the normal-difficulty driver cannot exercise this edge.
class VpAltairusTwisterAction : public VpMoveAction
{
public:
    VpAltairusTwisterAction(PlayerbotAI* botAI) : VpMoveAction(botAI, "vp altairus twister") {}
    bool Execute(Event event) override;
};

// Altairus - Call of the Wind (Wowpedia Altairus 2026-10-06: dps and healers stay upwind of the boss; Wowhead
// 88286/88282 +-100% attack and cast speed, +-30% movement): a non-tank bot that carries Downwind of Altairus
// (88286) and stands off the air current's heading (npc 47305 orientation) walks round the boss to that side.
// The script grants the aura from the player's FACING (boss_altairus.cpp:129-205), see CataVpShared.h.
class VpAltairusDownwindAction : public VpMoveAction
{
public:
    VpAltairusDownwindAction(PlayerbotAI* botAI) : VpMoveAction(botAI, "vp altairus downwind") {}
    bool Execute(Event event) override;
};

// Asaad - Unstable Grounding Field / Supremacy of the Storm (Wowpedia Asaad 2026-10-06: "everyone must group up
// within the area ... an AoE which will wipe out anybody not under the boss"; Wowhead 86930 channel 6 s, 86911
// channel 20 s): during the field phase everyone stacks at the field and then directly under the boss. The script
// zeroes the storm's damage within 5 yd of him (boss_asaad.cpp:297).
class VpAsaadStackAction : public VpMoveAction
{
public:
    VpAsaadStackAction(PlayerbotAI* botAI) : VpMoveAction(botAI, "vp asaad stack") {}
    bool Execute(Event event) override;
};

// #692 (BEYOND AC) - follow the leader onto the Slipstream: walk to the platform's slipstream and click it there like a
// player (Unit::HandleSpellClick, the BG vehicle boarding primitive of VehicleActions.cpp EnterVehicle); the script seats
// the bot on its own carrier, flies it to the landing zone and sets it down there.
class VpSlipstreamAction : public VpMoveAction
{
public:
    VpSlipstreamAction(PlayerbotAI* botAI) : VpMoveAction(botAI, "vp slipstream") {}
    bool Execute(Event event) override;
};

#endif
