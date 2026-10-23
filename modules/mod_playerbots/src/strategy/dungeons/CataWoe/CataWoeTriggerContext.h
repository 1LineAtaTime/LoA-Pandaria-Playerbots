/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity bot strategy "cata-woe" - trigger context (issue #672, epic #373).
// Pattern: AC Ai/Dungeon/HoS/HoSTriggerContext.h.

#ifndef PLAYERBOTS_CATAWOETRIGGERCONTEXT_H
#define PLAYERBOTS_CATAWOETRIGGERCONTEXT_H

#include "CataWoeTriggers.h"
#include "NamedObjectContext.h"

class CataDungeonWellOfEternityTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        CataDungeonWellOfEternityTriggerContext()
        {
            creators["woe fel flames"] = &CataDungeonWellOfEternityTriggerContext::fel_flames;
            creators["woe eye of perotharn"] = &CataDungeonWellOfEternityTriggerContext::eye_of_perotharn;
            creators["woe total obedience"] = &CataDungeonWellOfEternityTriggerContext::total_obedience;
            creators["woe arcane bomb"] = &CataDungeonWellOfEternityTriggerContext::arcane_bomb;
            creators["woe debilitator"] = &CataDungeonWellOfEternityTriggerContext::debilitator;
            creators["woe hand of the queen"] = &CataDungeonWellOfEternityTriggerContext::hand_of_the_queen;
            creators["woe fel firestorm"] = &CataDungeonWellOfEternityTriggerContext::fel_firestorm;
            creators["woe portal energy focus"] = &CataDungeonWellOfEternityTriggerContext::portal_energy_focus;
        }
    private:
        static Trigger* fel_flames(PlayerbotAI* ai)
        {
            return new CataWoeHazardTrigger(ai, "woe fel flames", WellOfEternity::HazardKind::FelFlames);
        }
        static Trigger* eye_of_perotharn(PlayerbotAI* ai)
        {
            return new CataWoeHazardTrigger(ai, "woe eye of perotharn", WellOfEternity::HazardKind::EyeOfPerotharn);
        }
        static Trigger* total_obedience(PlayerbotAI* ai)
        {
            return new CataWoeConditionTrigger(ai, "woe total obedience", &WellOfEternity::ObedienceNeedsInterrupt);
        }
        static Trigger* arcane_bomb(PlayerbotAI* ai)
        {
            return new CataWoeHazardTrigger(ai, "woe arcane bomb", WellOfEternity::HazardKind::ArcaneBomb);
        }
        static Trigger* debilitator(PlayerbotAI* ai)
        {
            return new CataWoeConditionTrigger(ai, "woe debilitator", &WellOfEternity::NeedsDebilitatorKill);
        }
        static Trigger* hand_of_the_queen(PlayerbotAI* ai)
        {
            return new CataWoeConditionTrigger(ai, "woe hand of the queen", &WellOfEternity::NeedsHandKill);
        }
        static Trigger* fel_firestorm(PlayerbotAI* ai)
        {
            return new CataWoeHazardTrigger(ai, "woe fel firestorm", WellOfEternity::HazardKind::FelFirestorm);
        }
        static Trigger* portal_energy_focus(PlayerbotAI* ai)
        {
            return new CataWoeConditionTrigger(ai, "woe portal energy focus", &WellOfEternity::NeedsPortalFocusUse);
        }
};

#endif
