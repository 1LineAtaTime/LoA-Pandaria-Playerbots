/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight bot strategy "cata-hot" - action context (issue #673, epic #373).
// Pattern: AC Ai/Dungeon/HoS/HoSActionContext.h.

#ifndef PLAYERBOTS_CATAHOTACTIONCONTEXT_H
#define PLAYERBOTS_CATAHOTACTIONCONTEXT_H

#include "Action.h"
#include "CataHotActions.h"
#include "NamedObjectContext.h"

class CataDungeonHourOfTwilightActionContext : public NamedObjectContext<Action>
{
    public:
        CataDungeonHourOfTwilightActionContext()
        {
            creators["hot kill icy tomb"] = &CataDungeonHourOfTwilightActionContext::kill_icy_tomb;
            creators["hot leave smoke bomb"] = &CataDungeonHourOfTwilightActionContext::leave_smoke_bomb;
            creators["hot leave light orb"] = &CataDungeonHourOfTwilightActionContext::leave_light_orb;
            creators["hot dodge wave"] = &CataDungeonHourOfTwilightActionContext::dodge_wave;
            creators["hot spread shear"] = &CataDungeonHourOfTwilightActionContext::spread_shear;
            creators["hot assist thrall"] = &CataDungeonHourOfTwilightActionContext::assist_thrall;
            creators["hot leave icy boulder"] = &CataDungeonHourOfTwilightActionContext::leave_icy_boulder;
            creators["hot knife cover"] = &CataDungeonHourOfTwilightActionContext::knife_cover;
            creators["hot enter water shell"] = &CataDungeonHourOfTwilightActionContext::enter_water_shell;
        }
    private:
        static Action* kill_icy_tomb(PlayerbotAI* ai) { return new CataHotKillTombAction(ai); }
        static Action* leave_smoke_bomb(PlayerbotAI* ai)
        {
            return new CataHotEscapeAction(ai, "hot leave smoke bomb", HourOfTwilight::HazardKind::SmokeBomb);
        }
        static Action* leave_light_orb(PlayerbotAI* ai)
        {
            return new CataHotEscapeAction(ai, "hot leave light orb", HourOfTwilight::HazardKind::LightOrb);
        }
        static Action* dodge_wave(PlayerbotAI* ai) { return new CataHotWaveDodgeAction(ai); }
        static Action* spread_shear(PlayerbotAI* ai) { return new CataHotShearSpreadAction(ai); }
        static Action* assist_thrall(PlayerbotAI* ai) { return new CataHotAssistThrallAction(ai); }
        static Action* leave_icy_boulder(PlayerbotAI* ai)
        {
            return new CataHotEscapeAction(ai, "hot leave icy boulder", HourOfTwilight::HazardKind::IcyBoulder);
        }
        static Action* knife_cover(PlayerbotAI* ai) { return new CataHotKnifeCoverAction(ai); }
        static Action* enter_water_shell(PlayerbotAI* ai) { return new CataHotWaterShellAction(ai); }
};

#endif
