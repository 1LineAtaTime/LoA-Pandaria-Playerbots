/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// End Time bot strategy "cata-et" - action context (issue #671, epic #373). Pattern: AC Ai/Dungeon/HoS/HoSActionContext.h.

#ifndef PLAYERBOTS_CATAETACTIONCONTEXT_H
#define PLAYERBOTS_CATAETACTIONCONTEXT_H

#include "Action.h"
#include "CataEtActions.h"
#include "NamedObjectContext.h"

class CataDungeonEndTimeActionContext : public NamedObjectContext<Action>
{
    public:
        CataDungeonEndTimeActionContext()
        {
            creators["et sylvanas safe spot"] = &CataDungeonEndTimeActionContext::sylvanas_safe_spot;
            creators["et sylvanas stay inside"] = &CataDungeonEndTimeActionContext::sylvanas_stay_inside;
            creators["et leave blighted arrows"] = &CataDungeonEndTimeActionContext::leave_blighted_arrows;
            creators["et sylvanas kill ghoul"] = &CataDungeonEndTimeActionContext::sylvanas_kill_ghoul;
            creators["et ranged spread"] = &CataDungeonEndTimeActionContext::ranged_spread;
            creators["et leave moonlance"] = &CataDungeonEndTimeActionContext::leave_moonlance;
            creators["et leave eye of elune"] = &CataDungeonEndTimeActionContext::leave_eye_of_elune;
            creators["et tyrande interrupt"] = &CataDungeonEndTimeActionContext::tyrande_interrupt;
            creators["et enter moonlight pool"] = &CataDungeonEndTimeActionContext::enter_moonlight_pool;
            creators["et tyrande ranged band"] = &CataDungeonEndTimeActionContext::tyrande_ranged_band;
            creators["et dodge frost blades"] = &CataDungeonEndTimeActionContext::dodge_frost_blades;
            creators["et handle flarecore"] = &CataDungeonEndTimeActionContext::handle_flarecore;
            creators["et jaina interrupt"] = &CataDungeonEndTimeActionContext::jaina_interrupt;
            creators["et collect fragment"] = &CataDungeonEndTimeActionContext::collect_fragment;
            creators["et baine leave lava"] = &CataDungeonEndTimeActionContext::baine_leave_lava;
            creators["et baine totem"] = &CataDungeonEndTimeActionContext::baine_totem;
            creators["et leave distortion bomb"] = &CataDungeonEndTimeActionContext::leave_distortion_bomb;
            creators["et dodge infinite breath"] = &CataDungeonEndTimeActionContext::dodge_infinite_breath;
            creators["et use hourglass"] = &CataDungeonEndTimeActionContext::use_hourglass;
        }
    private:
        static Action* sylvanas_safe_spot(PlayerbotAI* ai) { return new CataEtSafeSpotAction(ai); }
        static Action* sylvanas_stay_inside(PlayerbotAI* ai) { return new CataEtSylvanasInsideAction(ai); }
        static Action* leave_blighted_arrows(PlayerbotAI* ai)
        {
            return new CataEtEscapeAction(ai, "et leave blighted arrows", EndTime::HazardKind::BlightedArrows);
        }
        static Action* sylvanas_kill_ghoul(PlayerbotAI* ai) { return new CataEtKillGhoulAction(ai); }
        static Action* ranged_spread(PlayerbotAI* ai) { return new CataEtSpreadAction(ai); }
        static Action* leave_moonlance(PlayerbotAI* ai)
        {
            return new CataEtEscapeAction(ai, "et leave moonlance", EndTime::HazardKind::Moonlance);
        }
        static Action* leave_eye_of_elune(PlayerbotAI* ai)
        {
            return new CataEtEscapeAction(ai, "et leave eye of elune", EndTime::HazardKind::EyeOfElune);
        }
        static Action* tyrande_interrupt(PlayerbotAI* ai)
        {
            return new CataEtInterruptAction(ai, "et tyrande interrupt", &EndTime::GetTyrandeInterruptTarget);
        }
        static Action* enter_moonlight_pool(PlayerbotAI* ai) { return new CataEtMoonlightPoolAction(ai); }
        static Action* tyrande_ranged_band(PlayerbotAI* ai) { return new CataEtTyrandeRangedBandAction(ai); }
        static Action* dodge_frost_blades(PlayerbotAI* ai) { return new CataEtBladeDodgeAction(ai); }
        static Action* handle_flarecore(PlayerbotAI* ai) { return new CataEtFlarecoreAction(ai); }
        static Action* jaina_interrupt(PlayerbotAI* ai)
        {
            return new CataEtInterruptAction(ai, "et jaina interrupt", &EndTime::GetJainaInterruptTarget);
        }
        static Action* collect_fragment(PlayerbotAI* ai) { return new CataEtFragmentAction(ai); }
        static Action* baine_leave_lava(PlayerbotAI* ai) { return new CataEtLavaAction(ai); }
        static Action* baine_totem(PlayerbotAI* ai) { return new CataEtTotemAction(ai); }
        static Action* leave_distortion_bomb(PlayerbotAI* ai)
        {
            return new CataEtEscapeAction(ai, "et leave distortion bomb", EndTime::HazardKind::DistortionBomb);
        }
        static Action* dodge_infinite_breath(PlayerbotAI* ai) { return new CataEtBreathAction(ai); }
        static Action* use_hourglass(PlayerbotAI* ai) { return new CataEtHourglassAction(ai); }
};

#endif
