/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple strategy "mop-snt" (epic #373, issue #656) - trigger context.

#ifndef PLAYERBOTS_MOPSNTTRIGGERCONTEXT_H
#define PLAYERBOTS_MOPSNTTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "MopDungeonShared.h"
#include "MopSntTriggers.h"
#include "TriggerContext.h"

class MopSntTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MopSntTriggerContext()
    {
        creators["mop snt blades"] = &MopSntTriggerContext::mop_snt_blades;
        creators["mop snt dash"] = &MopSntTriggerContext::mop_snt_dash;
        creators["mop snt blade rush"] = &MopSntTriggerContext::mop_snt_blade_rush;
        creators["mop snt bomb"] = &MopSntTriggerContext::mop_snt_bomb;
        creators["mop snt pitch"] = &MopSntTriggerContext::mop_snt_pitch;
        creators["mop snt puddle"] = &MopSntTriggerContext::mop_snt_puddle;
        creators["mop snt healer leash"] = &MopSntTriggerContext::mop_snt_healer_leash;
        creators["mop snt neronok interrupt"] = &MopSntTriggerContext::mop_snt_neronok_interrupt;
        creators["mop snt focus target"] = &MopSntTriggerContext::mop_snt_focus_target;
        creators["mop snt siege bomb"] = &MopSntTriggerContext::mop_snt_siege_bomb;
        creators["mop snt tar keg"] = &MopSntTriggerContext::mop_snt_tar_keg;
        creators["mop snt soak puddle"] = &MopSntTriggerContext::mop_snt_soak_puddle;
        creators["mop snt tank reinforcements"] = &MopSntTriggerContext::mop_snt_tank_reinforcements;
        creators["mop snt resin"] = &MopSntTriggerContext::mop_snt_resin;
        creators["mop snt neronok far"] = &MopSntTriggerContext::mop_snt_neronok_far;
    }

private:
    static Trigger* mop_snt_blades(PlayerbotAI* botAI)
    {
        return new MopSntHazardTrigger(botAI, "mop snt blades", MopSnt::HazardKind::Blades);
    }

    static Trigger* mop_snt_dash(PlayerbotAI* botAI)
    {
        return new MopSntHazardTrigger(botAI, "mop snt dash", MopSnt::HazardKind::Dash);
    }

    static Trigger* mop_snt_blade_rush(PlayerbotAI* botAI)
    {
        return new MopSntHazardTrigger(botAI, "mop snt blade rush", MopSnt::HazardKind::BladeRush);
    }

    static Trigger* mop_snt_bomb(PlayerbotAI* botAI)
    {
        return new MopSntHazardTrigger(botAI, "mop snt bomb", MopSnt::HazardKind::Bomb);
    }

    static Trigger* mop_snt_pitch(PlayerbotAI* botAI)
    {
        return new MopSntHazardTrigger(botAI, "mop snt pitch", MopSnt::HazardKind::Pitch);
    }

    static Trigger* mop_snt_puddle(PlayerbotAI* botAI)
    {
        return new MopSntHazardTrigger(botAI, "mop snt puddle", MopSnt::HazardKind::Puddle);
    }

    static Trigger* mop_snt_healer_leash(PlayerbotAI* botAI)
    {
        return new MopSntHazardTrigger(botAI, "mop snt healer leash", MopSnt::HazardKind::HealerLeash);
    }

    static Trigger* mop_snt_neronok_interrupt(PlayerbotAI* botAI)
    {
        return new MopSntInterruptTrigger(botAI, "mop snt neronok interrupt", MopSnt::InterruptGroup::Neronok);
    }

    static Trigger* mop_snt_focus_target(PlayerbotAI* botAI) { return new MopSntFocusTrigger(botAI); }
    static Trigger* mop_snt_siege_bomb(PlayerbotAI* botAI) { return new MopSntSiegeBombTrigger(botAI); }
    static Trigger* mop_snt_tar_keg(PlayerbotAI* botAI) { return new MopSntTarKegTrigger(botAI); }
    static Trigger* mop_snt_soak_puddle(PlayerbotAI* botAI) { return new MopSntSoakPuddleTrigger(botAI); }
    static Trigger* mop_snt_resin(PlayerbotAI* botAI) { return new MopSntResinTrigger(botAI); }
    static Trigger* mop_snt_neronok_far(PlayerbotAI* botAI) { return new MopSntNeronokFarTrigger(botAI); }

    static Trigger* mop_snt_tank_reinforcements(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveTrigger(botAI, "mop snt tank reinforcements", MopSnt::ReinforcementsWave());
    }
};

#endif
