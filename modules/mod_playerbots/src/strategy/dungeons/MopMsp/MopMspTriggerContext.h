/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace strategy "mop-msp" (epic #373, issue #654) - trigger context.

#ifndef PLAYERBOTS_MOPMSPTRIGGERCONTEXT_H
#define PLAYERBOTS_MOPMSPTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "MopMspTriggers.h"
#include "TriggerContext.h"

class MopMspTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MopMspTriggerContext()
    {
        creators["mop msp dervish"] = &MopMspTriggerContext::mop_msp_dervish;
        creators["mop msp shockwave"] = &MopMspTriggerContext::mop_msp_shockwave;
        creators["mop msp conflagrate"] = &MopMspTriggerContext::mop_msp_conflagrate;
        creators["mop msp meteor stack"] = &MopMspTriggerContext::mop_msp_meteor_stack;
        creators["mop msp ground slam"] = &MopMspTriggerContext::mop_msp_ground_slam;
        creators["mop msp axe"] = &MopMspTriggerContext::mop_msp_axe;
        creators["mop msp fire ring"] = &MopMspTriggerContext::mop_msp_fire_ring;
        creators["mop msp healer leash"] = &MopMspTriggerContext::mop_msp_healer_leash;
        creators["mop msp ming interrupt"] = &MopMspTriggerContext::mop_msp_ming_interrupt;
        creators["mop msp gekkan interrupt"] = &MopMspTriggerContext::mop_msp_gekkan_interrupt;
        creators["mop msp focus target"] = &MopMspTriggerContext::mop_msp_focus_target;
    }

private:
    static Trigger* mop_msp_dervish(PlayerbotAI* botAI)
    {
        return new MopMspHazardTrigger(botAI, "mop msp dervish", MopMsp::HazardKind::Dervish);
    }

    static Trigger* mop_msp_shockwave(PlayerbotAI* botAI)
    {
        return new MopMspHazardTrigger(botAI, "mop msp shockwave", MopMsp::HazardKind::Shockwave);
    }

    static Trigger* mop_msp_conflagrate(PlayerbotAI* botAI)
    {
        return new MopMspHazardTrigger(botAI, "mop msp conflagrate", MopMsp::HazardKind::Conflagrate);
    }

    static Trigger* mop_msp_meteor_stack(PlayerbotAI* botAI)
    {
        return new MopMspHazardTrigger(botAI, "mop msp meteor stack", MopMsp::HazardKind::MeteorStack);
    }

    static Trigger* mop_msp_ground_slam(PlayerbotAI* botAI)
    {
        return new MopMspHazardTrigger(botAI, "mop msp ground slam", MopMsp::HazardKind::GroundSlam);
    }

    static Trigger* mop_msp_axe(PlayerbotAI* botAI)
    {
        return new MopMspHazardTrigger(botAI, "mop msp axe", MopMsp::HazardKind::Axe);
    }

    static Trigger* mop_msp_fire_ring(PlayerbotAI* botAI)
    {
        return new MopMspHazardTrigger(botAI, "mop msp fire ring", MopMsp::HazardKind::FireRing);
    }

    static Trigger* mop_msp_healer_leash(PlayerbotAI* botAI)
    {
        return new MopMspHazardTrigger(botAI, "mop msp healer leash", MopMsp::HazardKind::HealerLeash);
    }

    static Trigger* mop_msp_ming_interrupt(PlayerbotAI* botAI)
    {
        return new MopMspInterruptTrigger(botAI, "mop msp ming interrupt", MopMsp::InterruptGroup::Ming);
    }

    static Trigger* mop_msp_gekkan_interrupt(PlayerbotAI* botAI)
    {
        return new MopMspInterruptTrigger(botAI, "mop msp gekkan interrupt", MopMsp::InterruptGroup::Gekkan);
    }

    static Trigger* mop_msp_focus_target(PlayerbotAI* botAI) { return new MopMspFocusTrigger(botAI); }
};

#endif
