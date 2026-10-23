/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAGBACTIONS_H
#define PLAYERBOTS_CATAGBACTIONS_H

#include "Action.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// Base of this dungeon's movement actions: the movement lock multiplier lets exactly these through.
class GbMoveAction : public MovementAction
{
public:
    GbMoveAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    // Go to (x, y) on the ground at the bot's height; forced priority so it overrides the usual combat movement.
    bool GoTo(float x, float y, MovementPriority priority = MovementPriority::MOVEMENT_FORCED);
};

// General Umbriss - Ground Siege 74634 (Wowpedia General_Umbriss 2026-10-06: "faces the location of a random
// player and unleashes a ground tremor towards it"; Wowhead: 2 s cast, 20 yd, 58.5-61.5k and a 4 s stun): while he
// casts it, a bot inside the cone from him to the Ground Siege stalker (npc 40030) steps out sideways. The cone is
// 90 degrees: the core default for a creature caster (Spell.cpp:1142), the spell has no ConeAngle.
class GbUmbrissGroundSiegeAction : public GbMoveAction
{
public:
    GbUmbrissGroundSiegeAction(PlayerbotAI* botAI) : GbMoveAction(botAI, "gb umbriss ground siege") {}
    bool Execute(Event event) override;
};

// General Umbriss - Blitz 74670 (Wowpedia General_Umbriss: "all players near the targeted player must also move";
// Wowhead 74670 3 s cast, 74675 97.5-102.5k and a knock up): while he casts it, a bot within ~9.5 yd of the Blitz
// stalker (npc 40040, on the target's spot) walks out of that circle.
class GbUmbrissBlitzAction : public GbMoveAction
{
public:
    GbUmbrissBlitzAction(PlayerbotAI* botAI) : GbMoveAction(botAI, "gb umbriss blitz") {}
    bool Execute(Event event) override;
};

// Forgemaster Throngus - shield phase (Wowpedia Forgemaster_Throngus 2026-10-06: Personal Phalanx "damage dealt by
// a player standing in front of the shield is reduced by 99%", heroic Flaming Shield is a 60 degree fire cone;
// Wowhead 74908): while he carries Personal Phalanx (74908), every bot but his victim stays out of his front cone.
class GbThrongusPhalanxAction : public GbMoveAction
{
public:
    GbThrongusPhalanxAction(PlayerbotAI* botAI) : GbMoveAction(botAI, "gb throngus phalanx") {}
    bool Execute(Event event) override;
};

// Forgemaster Throngus - Mighty Stomp -> Cave In (#688; Wowpedia Forgemaster_Throngus 2026-10-23: "pieces of stone begin to
// fall on top of a random player in a 5 yard radius area", 3.8-4.2k every second; "Tanks should move Throngus should these
// cave-ins appear on the melee"): every bot, the tank too, leaves the circle of a Cave In Stalker (npc 40228).
class GbThrongusCaveInAction : public GbMoveAction
{
public:
    GbThrongusCaveInAction(PlayerbotAI* botAI) : GbMoveAction(botAI, "gb throngus cave in") {}
    bool Execute(Event event) override;
};

// Forgemaster Throngus - heroic Lava Patch (#688; Wowpedia: "As Throngus moves with his fiery mace, he leaves patches of fire
// on the ground", 8.3-8.7k every second): leave the circle of a Fire Patch (npc 48711, 5 yd).
class GbThrongusFirePatchAction : public GbMoveAction
{
public:
    GbThrongusFirePatchAction(PlayerbotAI* botAI) : GbMoveAction(botAI, "gb throngus fire patch") {}
    bool Execute(Event event) override;
};

// Valiona - Valiona's Flame 75321 / heroic Devouring Flames 90950 (#688; Wowpedia Drahga_Shadowburner 2026-10-23: a 40 yd
// 40 degree cone "in the direction of a random player", 14.6-15.4k and a 2 s stun; heroic a 150 degree cone for 5 s, "everyone
// needs to move out of its way"): a bot inside the cone steps out sideways, or goes behind her when the cone is too wide.
class GbValionaBreathAction : public GbMoveAction
{
public:
    GbValionaBreathAction(PlayerbotAI* botAI) : GbMoveAction(botAI, "gb valiona breath") {}
    bool Execute(Event event) override;
};

// Drahga Shadowburner - Invocation of Flame 75222 (Wowpedia Drahga_Shadowburner 2026-10-06: "everyone moves away
// from the fire patch", it burns 4.4-4.6k within 4 yd): leave the stalker's circle (npc 40355, 6 s).
class GbDrahgaPortalAction : public GbMoveAction
{
public:
    GbDrahgaPortalAction(PlayerbotAI* botAI) : GbMoveAction(botAI, "gb drahga portal") {}
    bool Execute(Event event) override;
};

// Drahga Shadowburner - Invoked Flaming Spirit (Wowpedia: "the targeted person keeps away from the elemental
// until it is killed"; Wowhead 75238 Supernova 151-159k within 10 yd): the bot an Invoked Flaming Spirit (npc 40357)
// is chasing keeps running while it is within 14 yd. The spirit moves at 0.8x speed (boss_drahga_shadowburner.cpp:475).
class GbDrahgaKiteAction : public GbMoveAction
{
public:
    GbDrahgaKiteAction(PlayerbotAI* botAI) : GbMoveAction(botAI, "gb drahga kite") {}
    bool Execute(Event event) override;
};

// Drahga Shadowburner - Invoked Flaming Spirit (Wowpedia: "DPS focuses on the elemental after it spawns"): skull
// the spirit so "dps assist" (RtiTargetValue) switches every dps bot to it. The AC pattern for an add that must
// die first: FoSActions.cpp:24-52 (AttackCorruptedSoulFragmentAction), same helper EncounterHelpers::MarkTargetWithSkull.
class GbDrahgaSpiritAction : public Action
{
public:
    GbDrahgaSpiritAction(PlayerbotAI* botAI) : Action(botAI, "gb drahga spirit") {}
    bool Execute(Event event) override;
};

// Drahga Shadowburner / Valiona - Shredding Swipe -> Seeping Twilight (Wowpedia: "stay out of Shredding Swipe
// (a ground-based AoE zone)", a 10 yd pool for 20 s; Wowhead 75317 9.3-9.7k and -50% speed): leave the pool (npc
// 40365). The tank leaves too, which drags Valiona out of it.
class GbDrahgaSeepingTwilightAction : public GbMoveAction
{
public:
    GbDrahgaSeepingTwilightAction(PlayerbotAI* botAI) : GbMoveAction(botAI, "gb drahga seeping twilight") {}
    bool Execute(Event event) override;
};

// Erudax - Shadow Gale 75664 (Wowpedia Erudax 2026-10-06: "party members must move to the center of the swirl";
// Wowhead 75694 "moving to the eye of the Shadow Gale will prevent the damage"): walk to the eye (npc 40567). The
// script zeroes the damage within 5 yd of it (boss_erudax.cpp:483).
class GbErudaxGaleAction : public GbMoveAction
{
public:
    GbErudaxGaleAction(PlayerbotAI* botAI) : GbMoveAction(botAI, "gb erudax gale") {}
    bool Execute(Event event) override;
};

// Erudax - Binding Shadows 79466 (Wowpedia Erudax: "a projectile-based AoE attack that should be avoided"; Wowhead
// 1.5 s cast, drains 7.8-8.2k/s and roots within 8 yd for 6 s): the bot it is aimed at runs away from the group,
// everyone within 9.5 yd of that bot leaves the circle.
class GbErudaxShadowsAction : public GbMoveAction
{
public:
    GbErudaxShadowsAction(PlayerbotAI* botAI) : GbMoveAction(botAI, "gb erudax shadows") {}
    bool Execute(Event event) override;
};

// Erudax - Faceless Corruptor (Wowpedia Erudax: "It is imperative that the Corruptor adds die before hatching any
// eggs ... should be snared and killed immediately", Umbral Mending is interruptible): skull the Faceless
// Corruptor (npc 40600, heroic 48844) so every dps bot focuses it; the class interrupt triggers then use their
// kicks on its Umbral Mending because it is their target.
class GbErudaxCorruptorAction : public Action
{
public:
    GbErudaxCorruptorAction(PlayerbotAI* botAI) : Action(botAI, "gb erudax corruptor") {}
    bool Execute(Event event) override;
};

#endif
