/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAVPTRIGGERS_H
#define PLAYERBOTS_CATAVPTRIGGERS_H

#include "Trigger.h"

// Grand Vizier Ertan - Cyclone Shield / Storm's Edge (Wowpedia Grand_Vizier_Ertan 2026-10-06; Wowhead spell
// 86309 "enemies outside the storm's circumference", 86292 cyclone touch 9.6-11.7k and -85% speed): stay inside
// the ring of cyclones (npc 46007); when it is called in, stack under the boss. Script: boss_grand_vizier_ertan.cpp
// :168-190 (Storm's Edge on every player farther than a cyclone), :231-239 (ring pulled in to ~5 yd).
class VpErtanRingTrigger : public Trigger
{
public:
    VpErtanRingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vp ertan ring") {}
    bool IsActive() override;
};

// Altairus - Chilling Breath 88308 (Wowpedia Altairus 2026-10-06; Wowhead: 2 s cast, 47.5-52.5k frost to every
// enemy in a 120 degree cone, SpellTargetRestrictions.dbc cone 120): while the boss casts it, a bot inside the
// cone (other than the target, whom the boss turns to) steps sideways out of it when that is reachable in time.
class VpAltairusBreathTrigger : public Trigger
{
public:
    VpAltairusBreathTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vp altairus breath") {}
    bool IsActive() override;
};

// Altairus (heroic only) - Twisting Winds (Wowpedia Altairus 2026-10-06; Wowhead 88313/88314: 11.7-12.3k nature
// and knock up within 7 yd of each twister, pulsing every 0.5 s): keep clear of the twisters (npc 47342). They
// only exist on heroic (boss_altairus.cpp:99, :207), so the normal-difficulty driver cannot exercise this edge.
class VpAltairusTwisterTrigger : public Trigger
{
public:
    VpAltairusTwisterTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vp altairus twister") {}
    bool IsActive() override;
};

// Altairus - Call of the Wind (Wowpedia Altairus 2026-10-06: dps and healers stay upwind of the boss; Wowhead
// 88286/88282 +-100% attack and cast speed, +-30% movement): a non-tank bot that carries Downwind of Altairus
// (88286) and stands off the air current's heading (npc 47305 orientation) walks round the boss to that side.
// The script grants the aura from the player's FACING (boss_altairus.cpp:129-205), see CataVpShared.h.
class VpAltairusDownwindTrigger : public Trigger
{
public:
    VpAltairusDownwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vp altairus downwind") {}
    bool IsActive() override;
};

// Asaad - Unstable Grounding Field / Supremacy of the Storm (Wowpedia Asaad 2026-10-06: "everyone must group up
// within the area ... an AoE which will wipe out anybody not under the boss"; Wowhead 86930 channel 6 s, 86911
// channel 20 s): during the field phase everyone stacks at the field and then directly under the boss. The script
// zeroes the storm's damage within 5 yd of him (boss_asaad.cpp:297).
class VpAsaadStackTrigger : public Trigger
{
public:
    VpAsaadStackTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vp asaad stack") {}
    bool IsActive() override;
};

// #692 (BEYOND AC) - the Slipstream rides between the platforms (Cata: a chain of slipstream vehicles to the landing zone,
// Cataclysm Preservation vortex_pinnacle.cpp npc_slipstream): the leader rides a slipstream, or already stands on a later platform, and the bot is still on an
// earlier one (or a leader bot stands by the slipstream to its next boss). See CataVpShared.h SlipstreamTarget.
class VpSlipstreamTrigger : public Trigger
{
public:
    VpSlipstreamTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vp slipstream") {}
    bool IsActive() override;
};

#endif
