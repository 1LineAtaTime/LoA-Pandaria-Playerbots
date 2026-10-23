/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" (Scholomance, epic #373, issue #659). See MopScholoShared.h for sources.

#ifndef PLAYERBOTS_MOPSCHOLOTRIGGERCONTEXT_H
#define PLAYERBOTS_MOPSCHOLOTRIGGERCONTEXT_H

#include "MopDungeonShared.h"
#include "MopScholoShared.h"
#include "MopScholoTriggers.h"
#include "NamedObjectContext.h"
#include "TriggerContext.h"

class MopDungeonScholomanceTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MopDungeonScholomanceTriggerContext()
    {
        creators["scholo ice wall"] = &MopDungeonScholomanceTriggerContext::scholo_ice_wall;
        creators["scholo tank spot"] = &MopDungeonScholomanceTriggerContext::scholo_tank_spot;
        creators["scholo frigid grasp"] = &MopDungeonScholomanceTriggerContext::scholo_frigid_grasp;
        creators["scholo ice wrath"] = &MopDungeonScholomanceTriggerContext::scholo_ice_wrath;
        creators["scholo arcane bomb"] = &MopDungeonScholomanceTriggerContext::scholo_arcane_bomb;
        creators["scholo fire tome"] = &MopDungeonScholomanceTriggerContext::scholo_fire_tome;
        creators["scholo focus target"] = &MopDungeonScholomanceTriggerContext::scholo_focus_target;
        creators["scholo gravity flux"] = &MopDungeonScholomanceTriggerContext::scholo_gravity_flux;
        creators["scholo priority interrupt"] = &MopDungeonScholomanceTriggerContext::scholo_priority_interrupt;
        creators["scholo harsh lesson"] = &MopDungeonScholomanceTriggerContext::scholo_harsh_lesson;
        creators["scholo lesson exit"] = &MopDungeonScholomanceTriggerContext::scholo_lesson_exit;
        creators["scholo bone armor"] = &MopDungeonScholomanceTriggerContext::scholo_bone_armor;
        creators["scholo soulflame"] = &MopDungeonScholomanceTriggerContext::scholo_soulflame;
        creators["scholo dark blaze"] = &MopDungeonScholomanceTriggerContext::scholo_dark_blaze;
        creators["scholo lilian spread"] = &MopDungeonScholomanceTriggerContext::scholo_lilian_spread;
        creators["scholo fixated"] = &MopDungeonScholomanceTriggerContext::scholo_fixated;
        creators["scholo tank students"] = &MopDungeonScholomanceTriggerContext::scholo_tank_students;
        creators["scholo rapidity tank"] = &MopDungeonScholomanceTriggerContext::scholo_rapidity_tank;
        creators["scholo rapidity tank return"] = &MopDungeonScholomanceTriggerContext::scholo_rapidity_tank_return;
    }

private:
    static Trigger* scholo_ice_wall(PlayerbotAI* botAI) { return new MopScholoIceWallTrigger(botAI); }
    static Trigger* scholo_tank_spot(PlayerbotAI* botAI) { return new MopScholoTankSpotTrigger(botAI); }

    static Trigger* scholo_frigid_grasp(PlayerbotAI* botAI)
    {
        return new MopScholoHazardTrigger(botAI, "scholo frigid grasp", MopScholomance::HAZARD_FRIGID_GRASP);
    }

    static Trigger* scholo_ice_wrath(PlayerbotAI* botAI) { return new MopScholoIceWrathTrigger(botAI); }

    static Trigger* scholo_arcane_bomb(PlayerbotAI* botAI)
    {
        return new MopScholoHazardTrigger(botAI, "scholo arcane bomb", MopScholomance::HAZARD_ARCANE_BOMB);
    }

    static Trigger* scholo_fire_tome(PlayerbotAI* botAI)
    {
        return new MopScholoHazardTrigger(botAI, "scholo fire tome", MopScholomance::HAZARD_FIRE_TOME);
    }

    static Trigger* scholo_focus_target(PlayerbotAI* botAI) { return new MopScholoFocusTrigger(botAI); }

    static Trigger* scholo_gravity_flux(PlayerbotAI* botAI)
    {
        return new MopScholoHazardTrigger(botAI, "scholo gravity flux", MopScholomance::HAZARD_GRAVITY_FLUX);
    }

    static Trigger* scholo_priority_interrupt(PlayerbotAI* botAI) { return new MopScholoInterruptTrigger(botAI); }
    static Trigger* scholo_harsh_lesson(PlayerbotAI* botAI) { return new MopScholoHarshLessonTrigger(botAI); }
    static Trigger* scholo_lesson_exit(PlayerbotAI* botAI) { return new MopScholoLessonExitTrigger(botAI); }
    static Trigger* scholo_bone_armor(PlayerbotAI* botAI) { return new MopScholoBoneArmorTrigger(botAI); }

    static Trigger* scholo_soulflame(PlayerbotAI* botAI)
    {
        return new MopScholoHazardTrigger(botAI, "scholo soulflame", MopScholomance::HAZARD_SOULFLAME);
    }

    static Trigger* scholo_dark_blaze(PlayerbotAI* botAI)
    {
        return new MopScholoHazardTrigger(botAI, "scholo dark blaze", MopScholomance::HAZARD_DARK_BLAZE);
    }

    static Trigger* scholo_lilian_spread(PlayerbotAI* botAI) { return new MopScholoLilianSpreadTrigger(botAI); }
    static Trigger* scholo_fixated(PlayerbotAI* botAI) { return new MopScholoFixatedTrigger(botAI); }

    static Trigger* scholo_rapidity_tank(PlayerbotAI* botAI) { return new MopScholoRapidityTankTrigger(botAI); }

    static Trigger* scholo_rapidity_tank_return(PlayerbotAI* botAI)
    {
        return new MopScholoRapidityTankReturnTrigger(botAI);
    }

    static Trigger* scholo_tank_students(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveTrigger(botAI, "scholo tank students", MopScholomance::GandlingTankWave());
    }
};

#endif
