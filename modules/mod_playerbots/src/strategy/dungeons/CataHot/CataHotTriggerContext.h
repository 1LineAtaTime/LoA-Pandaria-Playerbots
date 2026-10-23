/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight bot strategy "cata-hot" - trigger context (issue #673, epic #373).
// Pattern: AC Ai/Dungeon/HoS/HoSTriggerContext.h.

#ifndef PLAYERBOTS_CATAHOTTRIGGERCONTEXT_H
#define PLAYERBOTS_CATAHOTTRIGGERCONTEXT_H

#include "CataHotTriggers.h"
#include "NamedObjectContext.h"

class CataDungeonHourOfTwilightTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        CataDungeonHourOfTwilightTriggerContext()
        {
            creators["hot icy tomb"] = &CataDungeonHourOfTwilightTriggerContext::icy_tomb;
            creators["hot smoke bomb"] = &CataDungeonHourOfTwilightTriggerContext::smoke_bomb;
            creators["hot light orbs"] = &CataDungeonHourOfTwilightTriggerContext::light_orbs;
            creators["hot wave"] = &CataDungeonHourOfTwilightTriggerContext::wave;
            creators["hot shear"] = &CataDungeonHourOfTwilightTriggerContext::shear;
            creators["hot thrall ambush"] = &CataDungeonHourOfTwilightTriggerContext::thrall_ambush;
            creators["hot icy boulder"] = &CataDungeonHourOfTwilightTriggerContext::icy_boulder;
            creators["hot mark of silence"] = &CataDungeonHourOfTwilightTriggerContext::mark_of_silence;
            creators["hot water shell"] = &CataDungeonHourOfTwilightTriggerContext::water_shell;
        }
    private:
        static Trigger* icy_tomb(PlayerbotAI* ai)
        {
            return new CataHotConditionTrigger(ai, "hot icy tomb", &HourOfTwilight::NeedsIcyTombKill);
        }
        static Trigger* smoke_bomb(PlayerbotAI* ai)
        {
            return new CataHotHazardTrigger(ai, "hot smoke bomb", HourOfTwilight::HazardKind::SmokeBomb);
        }
        static Trigger* light_orbs(PlayerbotAI* ai)
        {
            return new CataHotHazardTrigger(ai, "hot light orbs", HourOfTwilight::HazardKind::LightOrb);
        }
        static Trigger* wave(PlayerbotAI* ai)
        {
            return new CataHotConditionTrigger(ai, "hot wave", &HourOfTwilight::InWavePath);
        }
        static Trigger* shear(PlayerbotAI* ai)
        {
            return new CataHotConditionTrigger(ai, "hot shear", &HourOfTwilight::NeedsShearSpread);
        }
        static Trigger* thrall_ambush(PlayerbotAI* ai)
        {
            return new CataHotConditionTrigger(ai, "hot thrall ambush", &HourOfTwilight::NeedsEscortAssist);
        }
        static Trigger* icy_boulder(PlayerbotAI* ai)
        {
            return new CataHotHazardTrigger(ai, "hot icy boulder", HourOfTwilight::HazardKind::IcyBoulder);
        }
        static Trigger* mark_of_silence(PlayerbotAI* ai)
        {
            return new CataHotConditionTrigger(ai, "hot mark of silence", &HourOfTwilight::NeedsKnifeCover);
        }
        static Trigger* water_shell(PlayerbotAI* ai)
        {
            return new CataHotConditionTrigger(ai, "hot water shell", &HourOfTwilight::NeedsWaterShell);
        }
};

#endif
