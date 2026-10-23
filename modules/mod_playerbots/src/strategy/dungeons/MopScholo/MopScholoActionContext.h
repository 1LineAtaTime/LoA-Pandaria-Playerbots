/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" (Scholomance, epic #373, issue #659). See MopScholoShared.h for sources.

#ifndef PLAYERBOTS_MOPSCHOLOACTIONCONTEXT_H
#define PLAYERBOTS_MOPSCHOLOACTIONCONTEXT_H

#include "Action.h"
#include "MopDungeonShared.h"
#include "MopScholoActions.h"
#include "MopScholoShared.h"
#include "NamedObjectContext.h"

class MopDungeonScholomanceActionContext : public NamedObjectContext<Action>
{
public:
    MopDungeonScholomanceActionContext()
    {
        creators["scholo outrun ice wall"] = &MopDungeonScholomanceActionContext::scholo_outrun_ice_wall;
        creators["scholo tank spot"] = &MopDungeonScholomanceActionContext::scholo_tank_spot;
        creators["scholo leave frigid grasp"] = &MopDungeonScholomanceActionContext::scholo_leave_frigid_grasp;
        creators["scholo spread ice wrath"] = &MopDungeonScholomanceActionContext::scholo_spread_ice_wrath;
        creators["scholo leave arcane bomb"] = &MopDungeonScholomanceActionContext::scholo_leave_arcane_bomb;
        creators["scholo leave fire tome"] = &MopDungeonScholomanceActionContext::scholo_leave_fire_tome;
        creators["scholo focus target"] = &MopDungeonScholomanceActionContext::scholo_focus_target;
        creators["scholo leave gravity flux"] = &MopDungeonScholomanceActionContext::scholo_leave_gravity_flux;
        creators["scholo priority interrupt"] = &MopDungeonScholomanceActionContext::scholo_priority_interrupt;
        creators["scholo harsh lesson"] = &MopDungeonScholomanceActionContext::scholo_harsh_lesson;
        creators["scholo lesson exit"] = &MopDungeonScholomanceActionContext::scholo_lesson_exit;
        creators["scholo get bone armor"] = &MopDungeonScholomanceActionContext::scholo_get_bone_armor;
        creators["scholo leave soulflame"] = &MopDungeonScholomanceActionContext::scholo_leave_soulflame;
        creators["scholo leave dark blaze"] = &MopDungeonScholomanceActionContext::scholo_leave_dark_blaze;
        creators["scholo spread from group"] = &MopDungeonScholomanceActionContext::scholo_spread_from_group;
        creators["scholo run from soul"] = &MopDungeonScholomanceActionContext::scholo_run_from_soul;
        creators["scholo tank students"] = &MopDungeonScholomanceActionContext::scholo_tank_students;
        creators["scholo dodge rapidity"] = &MopDungeonScholomanceActionContext::scholo_dodge_rapidity;
        creators["scholo rapidity return"] = &MopDungeonScholomanceActionContext::scholo_rapidity_return;
    }

private:
    static Action* scholo_outrun_ice_wall(PlayerbotAI* botAI) { return new MopScholoOutrunIceWallAction(botAI); }
    static Action* scholo_tank_spot(PlayerbotAI* botAI) { return new MopScholoTankSpotAction(botAI); }

    static Action* scholo_leave_frigid_grasp(PlayerbotAI* botAI)
    {
        return new MopScholoLeaveHazardAction(botAI, "scholo leave frigid grasp", MopScholomance::HAZARD_FRIGID_GRASP);
    }

    static Action* scholo_spread_ice_wrath(PlayerbotAI* botAI)
    {
        return new MopScholoSpreadAction(botAI, "scholo spread ice wrath", MopScholomance::ICE_WRATH_SPREAD);
    }

    static Action* scholo_leave_arcane_bomb(PlayerbotAI* botAI)
    {
        return new MopScholoLeaveHazardAction(botAI, "scholo leave arcane bomb", MopScholomance::HAZARD_ARCANE_BOMB);
    }

    static Action* scholo_leave_fire_tome(PlayerbotAI* botAI)
    {
        return new MopScholoLeaveHazardAction(botAI, "scholo leave fire tome", MopScholomance::HAZARD_FIRE_TOME);
    }

    static Action* scholo_focus_target(PlayerbotAI* botAI) { return new MopScholoFocusAction(botAI); }

    static Action* scholo_leave_gravity_flux(PlayerbotAI* botAI)
    {
        return new MopScholoLeaveHazardAction(botAI, "scholo leave gravity flux", MopScholomance::HAZARD_GRAVITY_FLUX);
    }

    static Action* scholo_priority_interrupt(PlayerbotAI* botAI) { return new MopScholoInterruptAction(botAI); }
    static Action* scholo_harsh_lesson(PlayerbotAI* botAI) { return new MopScholoHarshLessonAction(botAI); }
    static Action* scholo_lesson_exit(PlayerbotAI* botAI) { return new MopScholoLessonExitAction(botAI); }
    static Action* scholo_get_bone_armor(PlayerbotAI* botAI) { return new MopScholoBoneArmorAction(botAI); }

    static Action* scholo_leave_soulflame(PlayerbotAI* botAI)
    {
        return new MopScholoLeaveHazardAction(botAI, "scholo leave soulflame", MopScholomance::HAZARD_SOULFLAME);
    }

    static Action* scholo_leave_dark_blaze(PlayerbotAI* botAI)
    {
        return new MopScholoLeaveHazardAction(botAI, "scholo leave dark blaze", MopScholomance::HAZARD_DARK_BLAZE);
    }

    static Action* scholo_spread_from_group(PlayerbotAI* botAI)
    {
        return new MopScholoSpreadAction(botAI, "scholo spread from group", MopScholomance::LILIAN_SPREAD);
    }

    static Action* scholo_run_from_soul(PlayerbotAI* botAI) { return new MopScholoFixatedAction(botAI); }

    static Action* scholo_dodge_rapidity(PlayerbotAI* botAI) { return new MopScholoDodgeRapidityAction(botAI); }
    static Action* scholo_rapidity_return(PlayerbotAI* botAI) { return new MopScholoRapidityReturnAction(botAI); }

    static Action* scholo_tank_students(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "scholo tank students", MopScholomance::GandlingTankWave());
    }
};

#endif
