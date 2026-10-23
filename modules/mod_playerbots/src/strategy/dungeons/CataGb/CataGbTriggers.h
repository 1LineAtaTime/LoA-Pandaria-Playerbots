/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAGBTRIGGERS_H
#define PLAYERBOTS_CATAGBTRIGGERS_H

#include "Trigger.h"

// General Umbriss - Ground Siege 74634 (Wowpedia General_Umbriss 2026-10-06: "faces the location of a random
// player and unleashes a ground tremor towards it"; Wowhead: 2 s cast, 20 yd, 58.5-61.5k and a 4 s stun): while he
// casts it, a bot inside the cone from him to the Ground Siege stalker (npc 40030) steps out sideways. The cone is
// 90 degrees: the core default for a creature caster (Spell.cpp:1142), the spell has no ConeAngle.
class GbUmbrissGroundSiegeTrigger : public Trigger
{
public:
    GbUmbrissGroundSiegeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb umbriss ground siege") {}
    bool IsActive() override;
};

// General Umbriss - Blitz 74670 (Wowpedia General_Umbriss: "all players near the targeted player must also move";
// Wowhead 74670 3 s cast, 74675 97.5-102.5k and a knock up): while he casts it, a bot within ~9.5 yd of the Blitz
// stalker (npc 40040, on the target's spot) walks out of that circle.
class GbUmbrissBlitzTrigger : public Trigger
{
public:
    GbUmbrissBlitzTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb umbriss blitz") {}
    bool IsActive() override;
};

// Forgemaster Throngus - shield phase (Wowpedia Forgemaster_Throngus 2026-10-06: Personal Phalanx "damage dealt by
// a player standing in front of the shield is reduced by 99%", heroic Flaming Shield is a 60 degree fire cone;
// Wowhead 74908): while he carries Personal Phalanx (74908), every bot but his victim stays out of his front cone.
class GbThrongusPhalanxTrigger : public Trigger
{
public:
    GbThrongusPhalanxTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb throngus phalanx") {}
    bool IsActive() override;
};

// Forgemaster Throngus - Cave In (#688): the bot stands within 6.5 yd of a Cave In Stalker (npc 40228) while he is engaged.
class GbThrongusCaveInTrigger : public Trigger
{
public:
    GbThrongusCaveInTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb throngus cave in") {}
    bool IsActive() override;
};

// Forgemaster Throngus - heroic Lava Patch (#688): the bot stands within 6.5 yd of a Fire Patch (npc 48711).
class GbThrongusFirePatchTrigger : public Trigger
{
public:
    GbThrongusFirePatchTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb throngus fire patch") {}
    bool IsActive() override;
};

// Valiona - Valiona's Flame / heroic Devouring Flames (#688): the bot stands in the breath cone.
class GbValionaBreathTrigger : public Trigger
{
public:
    GbValionaBreathTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb valiona breath") {}
    bool IsActive() override;
};

// Drahga Shadowburner - Invocation of Flame 75222 (Wowpedia Drahga_Shadowburner 2026-10-06: "everyone moves away
// from the fire patch", it burns 4.4-4.6k within 4 yd): leave the stalker's circle (npc 40355, 6 s).
class GbDrahgaPortalTrigger : public Trigger
{
public:
    GbDrahgaPortalTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb drahga portal") {}
    bool IsActive() override;
};

// Drahga Shadowburner - Invoked Flaming Spirit (Wowpedia: "the targeted person keeps away from the elemental
// until it is killed"; Wowhead 75238 Supernova 151-159k within 10 yd): the bot an Invoked Flaming Spirit (npc 40357)
// is chasing keeps running while it is within 14 yd. The spirit moves at 0.8x speed (boss_drahga_shadowburner.cpp:475).
class GbDrahgaKiteTrigger : public Trigger
{
public:
    GbDrahgaKiteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb drahga kite") {}
    bool IsActive() override;
};

// Drahga Shadowburner - Invoked Flaming Spirit (Wowpedia: "DPS focuses on the elemental after it spawns"): skull
// the spirit so "dps assist" (RtiTargetValue) switches every dps bot to it. The AC pattern for an add that must
// die first: FoSActions.cpp:24-52 (AttackCorruptedSoulFragmentAction), same helper EncounterHelpers::MarkTargetWithSkull.
class GbDrahgaSpiritTrigger : public Trigger
{
public:
    GbDrahgaSpiritTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb drahga spirit") {}
    bool IsActive() override;
};

// Drahga Shadowburner / Valiona - Shredding Swipe -> Seeping Twilight (Wowpedia: "stay out of Shredding Swipe
// (a ground-based AoE zone)", a 10 yd pool for 20 s; Wowhead 75317 9.3-9.7k and -50% speed): leave the pool (npc
// 40365). The tank leaves too, which drags Valiona out of it.
class GbDrahgaSeepingTwilightTrigger : public Trigger
{
public:
    GbDrahgaSeepingTwilightTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb drahga seeping twilight") {}
    bool IsActive() override;
};

// Erudax - Shadow Gale 75664 (Wowpedia Erudax 2026-10-06: "party members must move to the center of the swirl";
// Wowhead 75694 "moving to the eye of the Shadow Gale will prevent the damage"): walk to the eye (npc 40567). The
// script zeroes the damage within 5 yd of it (boss_erudax.cpp:483).
class GbErudaxGaleTrigger : public Trigger
{
public:
    GbErudaxGaleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb erudax gale") {}
    bool IsActive() override;
};

// Erudax - Binding Shadows 79466 (Wowpedia Erudax: "a projectile-based AoE attack that should be avoided"; Wowhead
// 1.5 s cast, drains 7.8-8.2k/s and roots within 8 yd for 6 s): the bot it is aimed at runs away from the group,
// everyone within 9.5 yd of that bot leaves the circle.
class GbErudaxShadowsTrigger : public Trigger
{
public:
    GbErudaxShadowsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb erudax shadows") {}
    bool IsActive() override;
};

// Erudax - Faceless Corruptor (Wowpedia Erudax: "It is imperative that the Corruptor adds die before hatching any
// eggs ... should be snared and killed immediately", Umbral Mending is interruptible): skull the Faceless
// Corruptor (npc 40600, heroic 48844) so every dps bot focuses it; the class interrupt triggers then use their
// kicks on its Umbral Mending because it is their target.
class GbErudaxCorruptorTrigger : public Trigger
{
public:
    GbErudaxCorruptorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gb erudax corruptor") {}
    bool IsActive() override;
};

#endif
