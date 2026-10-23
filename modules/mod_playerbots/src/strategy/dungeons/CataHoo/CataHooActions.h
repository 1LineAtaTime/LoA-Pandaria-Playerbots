/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAHOOACTIONS_H
#define PLAYERBOTS_CATAHOOACTIONS_H

#include "Action.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// Base of this dungeon's movement actions: the movement lock multiplier lets exactly these through.
class HooMoveAction : public MovementAction
{
public:
    HooMoveAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    // Go to (x, y) on the ground at the bot's height; forced priority so it overrides the usual combat movement.
    bool GoTo(float x, float y, MovementPriority priority = MovementPriority::MOVEMENT_FORCED);
};

// Temple Guardian Anhuur - Shield of Light (Wowpedia Temple_Guardian_Anhuur 2026-10-06: at 66%/33% he is immune
// until "two players jump down into the pit and hit a switch on the left and right"; Wowhead 74938): while he
// carries the shield, the two duty bots (first two non-tank bots, damage dealers first) go to the Beacon of Light
// gameobjects (203133 / 203136, pit floor) and use them. From the platform they jump down (JumpTo, as the BG tactics
// do for ledges). The script flags a used beacon GO_FLAG_INTERACT_COND (boss_temple_guardian_anhuur.cpp:376).
class HooAnhuurBeaconsAction : public HooMoveAction
{
public:
    HooAnhuurBeaconsAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo anhuur beacons") {}
    bool Execute(Event event) override;
};

// Temple Guardian Anhuur - the way back: once the shield is down, a bot still in the pit (12 yd or more below
// him) jumps back up to the boss, so the duty bots rejoin the fight and can interrupt the Reverberating Hymn.
class HooAnhuurPitReturnAction : public HooMoveAction
{
public:
    HooAnhuurPitReturnAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo anhuur pit return") {}
    bool Execute(Event event) override;
};

// Temple Guardian Anhuur - Burning/Searing Light (Wowpedia: "everyone must watch for Searing Light and move if it
// targets you", 4 yd for 10 s, 9.7k/s; SpellRadius 26 on 75117): leave the circle of a Searing Light (npc 40283).
class HooAnhuurSearingLightAction : public HooMoveAction
{
public:
    HooAnhuurSearingLightAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo anhuur searing light") {}
    bool Execute(Event event) override;
};

// Temple Guardian Anhuur - Divine Reckoning 75592 (Wowpedia: "healer and ranged need to spread out to avoid
// Divine Reckoning hitting more than one target"; the 5.4.8 pulse 75591 is 15 yd): a bot (not a tank) that carries
// the debuff walks away from the rest of the group until nobody is within 16 yd of it.
class HooAnhuurReckoningAction : public HooMoveAction
{
public:
    HooAnhuurReckoningAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo anhuur reckoning") {}
    bool Execute(Event event) override;
};

// Earthrager Ptah - Tumultuous Earthstorm / Quicksand 75546 (Wowpedia Earthrager_Ptah 2026-10-06: "watch for
// Quicksand and run out if it appears under them"; Wowhead 75547 19.5-20.5k per 2 s and -50% speed): #691: the
// script now summons the pools (npc 40503) near the players while he is submerged; a bot inside a pool (or near the boss
// while he carries the old 75546 pulse) walks out.
class HooPtahQuicksandAction : public HooMoveAction
{
public:
    HooPtahQuicksandAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo ptah quicksand") {}
    bool Execute(Event event) override;
};

// Anraphet - Alpha Beams (Wowpedia Anraphet 2026-10-06: "everyone must not stand in the void zones ... stop
// whatever you are doing and move"; Wowhead 76184 14.6-15.4k every 0.5 s within 5 yd): leave every Alpha Beam
// (npc 41144). The fork leaves the zone up after the channel in every difficulty (boss_anraphet.cpp:353).
class HooAnraphetAlphaBeamAction : public HooMoveAction
{
public:
    HooAnraphetAlphaBeamAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo anraphet alpha beam") {}
    bool Execute(Event event) override;
};

// Isiset - Supernova 74136 (Wowpedia Isiset 2026-10-06: "everyone must face away from her at cast completion, or
// be blinded for 4 seconds"; Wowhead 3 s cast): while she casts it every non-tank bot turns its back to her. The
// script hits whoever has her in a 2.5 rad front arc (boss_isiset.cpp:575-581). HooSupernovaMultiplier keeps the
// bot from attacking or casting at her until the cast is over (that would turn it back).
class HooIsisetSupernovaAction : public Action
{
public:
    HooIsisetSupernovaAction(PlayerbotAI* botAI) : Action(botAI, "hoo isiset supernova") {}
    bool Execute(Event event) override;
};

// Isiset - Mirror Images (Wowpedia Isiset: at 66% and 33% she splits; "Astral Rain is the most dangerous, and should
// be killed first ... it is more advantageous to kill Celestial Call at this point as Veil of Sky is easily
// dispelled"): skull the image to kill - Astral Rain (npc 39720), then Celestial Call (39721), then Veil of Sky (39722).
class HooIsisetImageAction : public Action
{
public:
    HooIsisetImageAction(PlayerbotAI* botAI) : Action(botAI, "hoo isiset image") {}
    bool Execute(Event event) override;
};

// Ammunae - Seedling Pods (Wowpedia Ammunae 2026-10-06: "kill the Seedling Pods when they spawn or they will stack a
// self healing and haste buff"; Wowhead 75657 Energize): skull the pod (npc 40550 / 40716) so dps assist burns it.
class HooAmmunaePodAction : public Action
{
public:
    HooAmmunaePodAction(PlayerbotAI* botAI) : Action(botAI, "hoo ammunae pod") {}
    bool Execute(Event event) override;
};

// Ammunae - Spore / Noxious Spores (Wowpedia: "kill them and avoid the Spore Cloud they drop", 6 yd for 30 s,
// 8.3-9.7k per 1.5 s; Wowhead 75702): leave a popped Spore (npc 40585 with the cloud aura 75701).
class HooAmmunaeSporeCloudAction : public HooMoveAction
{
public:
    HooAmmunaeSporeCloudAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo ammunae spore cloud") {}
    bool Execute(Event event) override;
};

// Setesh - Void Seeker / Chaos Portal (Wowpedia Setesh 2026-10-06: "damage dealers must interrupt Anti-Magic Prison
// and kill the Void Seekers whenever they emerge", heroic "must close the Chaos Portals as fast as possible"): skull
// the Void Seeker (npc 41371), else an attackable Chaos Portal (41055), else Setesh (#691); the class interrupt triggers then fight
// Anti-Magic Prison because the seeker is their target.
class HooSeteshPriorityAction : public Action
{
public:
    HooSeteshPriorityAction(PlayerbotAI* botAI) : Action(botAI, "hoo setesh priority") {}
    bool Execute(Event event) override;
};

// Rajh - Solar Winds (Wowpedia Rajh 2026-10-06: "players need only keep their distance from it to avoid damage";
// Wowhead 74108 9.6-11.2k and a knock back within 15 yd, every 1.5 s): keep 16.5 yd from every Solar Winds vortex
// (npcs 39634/39635/47922 that carry the pulse aura 74107).
class HooRajhSolarWindsAction : public HooMoveAction
{
public:
    HooRajhSolarWindsAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo rajh solar winds") {}
    bool Execute(Event event) override;
};

// #691 Setesh - step out of a Reign of Chaos bubble (see HooSeteshReignOfChaosTrigger).
class HooSeteshReignOfChaosAction : public HooMoveAction
{
public:
    HooSeteshReignOfChaosAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo setesh reign of chaos") {}
    bool Execute(Event event) override;
};

// #691 Setesh - leave a Chaos Blast and its void zone (see HooSeteshChaosBlastTrigger).
class HooSeteshChaosBlastAction : public HooMoveAction
{
public:
    HooSeteshChaosBlastAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo setesh chaos blast") {}
    bool Execute(Event event) override;
};

// #691 Rajh - leave the landing spot of a Sun Orb (see HooRajhSunOrbTrigger).
class HooRajhSunOrbAction : public HooMoveAction
{
public:
    HooRajhSunOrbAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo rajh sun orb") {}
    bool Execute(Event event) override;
};

// #691 Rajh - leave the Inferno Leap landing spot (see HooRajhInfernoLeapTrigger).
class HooRajhInfernoLeapAction : public HooMoveAction
{
public:
    HooRajhInfernoLeapAction(PlayerbotAI* botAI) : HooMoveAction(botAI, "hoo rajh inferno leap") {}
    bool Execute(Event event) override;
};

#endif
