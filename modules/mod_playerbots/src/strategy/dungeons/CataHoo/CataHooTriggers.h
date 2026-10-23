/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAHOOTRIGGERS_H
#define PLAYERBOTS_CATAHOOTRIGGERS_H

#include "Trigger.h"

// Temple Guardian Anhuur - Shield of Light (Wowpedia Temple_Guardian_Anhuur 2026-10-06: at 66%/33% he is immune
// until "two players jump down into the pit and hit a switch on the left and right"; Wowhead 74938): while he
// carries the shield, the two duty bots (first two non-tank bots, damage dealers first) go to the Beacon of Light
// gameobjects (203133 / 203136, pit floor) and use them. From the platform they jump down (JumpTo, as the BG tactics
// do for ledges). The script flags a used beacon GO_FLAG_INTERACT_COND (boss_temple_guardian_anhuur.cpp:376).
class HooAnhuurBeaconsTrigger : public Trigger
{
public:
    HooAnhuurBeaconsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo anhuur beacons") {}
    bool IsActive() override;
};

// Temple Guardian Anhuur - the way back: once the shield is down, a bot still in the pit (12 yd or more below
// him) jumps back up to the boss, so the duty bots rejoin the fight and can interrupt the Reverberating Hymn.
class HooAnhuurPitReturnTrigger : public Trigger
{
public:
    HooAnhuurPitReturnTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo anhuur pit return") {}
    bool IsActive() override;
};

// Temple Guardian Anhuur - Burning/Searing Light (Wowpedia: "everyone must watch for Searing Light and move if it
// targets you", 4 yd for 10 s, 9.7k/s; SpellRadius 26 on 75117): leave the circle of a Searing Light (npc 40283).
class HooAnhuurSearingLightTrigger : public Trigger
{
public:
    HooAnhuurSearingLightTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo anhuur searing light") {}
    bool IsActive() override;
};

// Temple Guardian Anhuur - Divine Reckoning 75592 (Wowpedia: "healer and ranged need to spread out to avoid
// Divine Reckoning hitting more than one target"; the 5.4.8 pulse 75591 is 15 yd): a bot (not a tank) that carries
// the debuff walks away from the rest of the group until nobody is within 16 yd of it.
class HooAnhuurReckoningTrigger : public Trigger
{
public:
    HooAnhuurReckoningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo anhuur reckoning") {}
    bool IsActive() override;
};

// Earthrager Ptah - Tumultuous Earthstorm / Quicksand 75546 (Wowpedia Earthrager_Ptah 2026-10-06: "watch for
// Quicksand and run out if it appears under them"; Wowhead 75547 19.5-20.5k per 2 s and -50% speed): #691: the
// script now summons the pools (npc 40503) near the players while he is submerged; a bot inside a pool (or near the boss
// while he carries the old 75546 pulse) walks out.
class HooPtahQuicksandTrigger : public Trigger
{
public:
    HooPtahQuicksandTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo ptah quicksand") {}
    bool IsActive() override;
};

// Anraphet - Alpha Beams (Wowpedia Anraphet 2026-10-06: "everyone must not stand in the void zones ... stop
// whatever you are doing and move"; Wowhead 76184 14.6-15.4k every 0.5 s within 5 yd): leave every Alpha Beam
// (npc 41144). The fork leaves the zone up after the channel in every difficulty (boss_anraphet.cpp:353).
class HooAnraphetAlphaBeamTrigger : public Trigger
{
public:
    HooAnraphetAlphaBeamTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo anraphet alpha beam") {}
    bool IsActive() override;
};

// Isiset - Supernova 74136 (Wowpedia Isiset 2026-10-06: "everyone must face away from her at cast completion, or
// be blinded for 4 seconds"; Wowhead 3 s cast): while she casts it every non-tank bot turns its back to her. The
// script hits whoever has her in a 2.5 rad front arc (boss_isiset.cpp:575-581). HooSupernovaMultiplier keeps the
// bot from attacking or casting at her until the cast is over (that would turn it back).
class HooIsisetSupernovaTrigger : public Trigger
{
public:
    HooIsisetSupernovaTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo isiset supernova") {}
    bool IsActive() override;
};

// Isiset - Mirror Images (Wowpedia Isiset: at 66% and 33% she splits; "Astral Rain is the most dangerous, and should
// be killed first ... it is more advantageous to kill Celestial Call at this point as Veil of Sky is easily
// dispelled"): skull the image to kill - Astral Rain (npc 39720), then Celestial Call (39721), then Veil of Sky (39722).
class HooIsisetImageTrigger : public Trigger
{
public:
    HooIsisetImageTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo isiset image") {}
    bool IsActive() override;
};

// Ammunae - Seedling Pods (Wowpedia Ammunae 2026-10-06: "kill the Seedling Pods when they spawn or they will stack a
// self healing and haste buff"; Wowhead 75657 Energize): skull the pod (npc 40550 / 40716) so dps assist burns it.
class HooAmmunaePodTrigger : public Trigger
{
public:
    HooAmmunaePodTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo ammunae pod") {}
    bool IsActive() override;
};

// Ammunae - Spore / Noxious Spores (Wowpedia: "kill them and avoid the Spore Cloud they drop", 6 yd for 30 s,
// 8.3-9.7k per 1.5 s; Wowhead 75702): leave a popped Spore (npc 40585 with the cloud aura 75701).
class HooAmmunaeSporeCloudTrigger : public Trigger
{
public:
    HooAmmunaeSporeCloudTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo ammunae spore cloud") {}
    bool IsActive() override;
};

// Setesh - Void Seeker / Chaos Portal (Wowpedia Setesh 2026-10-06: "damage dealers must interrupt Anti-Magic Prison
// and kill the Void Seekers whenever they emerge", heroic "must close the Chaos Portals as fast as possible"): skull
// the Void Seeker (npc 41371), else an attackable Chaos Portal (41055), else Setesh (#691); the class interrupt triggers then fight
// Anti-Magic Prison because the seeker is their target.
class HooSeteshPriorityTrigger : public Trigger
{
public:
    HooSeteshPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo setesh priority") {}
    bool IsActive() override;
};

// Rajh - Solar Winds (Wowpedia Rajh 2026-10-06: "players need only keep their distance from it to avoid damage";
// Wowhead 74108 9.6-11.2k and a knock back within 15 yd, every 1.5 s): keep 16.5 yd from every Solar Winds vortex
// (npcs 39634/39635/47922 that carry the pulse aura 74107).
class HooRajhSolarWindsTrigger : public Trigger
{
public:
    HooRajhSolarWindsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo rajh solar winds") {}
    bool IsActive() override;
};

// #691 Setesh - Reign of Chaos (Wowpedia Setesh 2026-10-23: "small 2 yard radius bubbles of void energy every 0.5 second at
// the location of players and random spots in the room ... 6000 unresistable damage every second ... lasts 5 seconds"): step
// out of a bubble (npc 41168), to a spot clear of every bubble and Chaos Blast.
class HooSeteshReignOfChaosTrigger : public Trigger
{
public:
    HooSeteshReignOfChaosTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo setesh reign of chaos") {}
    bool IsActive() override;
};

// #691 Setesh - Chaos Blast (Wowpedia: thrown at a player at least 15 yd from him, "After 5 seconds the Chaos Blast lands ...
// leaves behind a ... void zone"): leave the Chaos Blast (npc 41041) before and after it lands.
class HooSeteshChaosBlastTrigger : public Trigger
{
public:
    HooSeteshChaosBlastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo setesh chaos blast") {}
    bool IsActive() override;
};

// #691 Rajh - Summon Sun Orb (Wowpedia Rajh 2026-10-23: the orb "falls, and when it lands ... any player within 13 yards"):
// leave the spot under a Sun Orb (npc 40835) before it lands.
class HooRajhSunOrbTrigger : public Trigger
{
public:
    HooRajhSunOrbTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo rajh sun orb") {}
    bool IsActive() override;
};

// #691 Rajh - Inferno Leap (Wowpedia: "prepares to leap to the location of a random player ... players within 10 yards of
// the chosen location have their movement speed increased by 50%"): leave the landing marker (npc 47040) before he lands.
class HooRajhInfernoLeapTrigger : public Trigger
{
public:
    HooRajhInfernoLeapTrigger(PlayerbotAI* botAI) : Trigger(botAI, "hoo rajh inferno leap") {}
    bool IsActive() override;
};

#endif
