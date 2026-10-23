/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity bot strategy "cata-woe" - action context (issue #672, epic #373).
// Pattern: AC Ai/Dungeon/HoS/HoSActionContext.h.

#ifndef PLAYERBOTS_CATAWOEACTIONCONTEXT_H
#define PLAYERBOTS_CATAWOEACTIONCONTEXT_H

#include "Action.h"
#include "CataWoeActions.h"
#include "NamedObjectContext.h"

class CataDungeonWellOfEternityActionContext : public NamedObjectContext<Action>
{
    public:
        CataDungeonWellOfEternityActionContext()
        {
            creators["woe leave fel flames"] = &CataDungeonWellOfEternityActionContext::leave_fel_flames;
            creators["woe leave eye of perotharn"] = &CataDungeonWellOfEternityActionContext::leave_eye_of_perotharn;
            creators["woe interrupt total obedience"] = &CataDungeonWellOfEternityActionContext::interrupt_total_obedience;
            creators["woe leave arcane bomb"] = &CataDungeonWellOfEternityActionContext::leave_arcane_bomb;
            creators["woe kill debilitator"] = &CataDungeonWellOfEternityActionContext::kill_debilitator;
            creators["woe kill hand of the queen"] = &CataDungeonWellOfEternityActionContext::kill_hand_of_the_queen;
            creators["woe leave fel firestorm"] = &CataDungeonWellOfEternityActionContext::leave_fel_firestorm;
            creators["woe use portal energy focus"] = &CataDungeonWellOfEternityActionContext::use_portal_energy_focus;
        }
    private:
        static Action* leave_fel_flames(PlayerbotAI* ai)
        {
            return new CataWoeEscapeAction(ai, "woe leave fel flames", WellOfEternity::HazardKind::FelFlames);
        }
        static Action* leave_eye_of_perotharn(PlayerbotAI* ai)
        {
            return new CataWoeEscapeAction(ai, "woe leave eye of perotharn", WellOfEternity::HazardKind::EyeOfPerotharn);
        }
        static Action* interrupt_total_obedience(PlayerbotAI* ai) { return new CataWoeObedienceInterruptAction(ai); }
        static Action* leave_arcane_bomb(PlayerbotAI* ai)
        {
            return new CataWoeEscapeAction(ai, "woe leave arcane bomb", WellOfEternity::HazardKind::ArcaneBomb);
        }
        static Action* kill_debilitator(PlayerbotAI* ai) { return new CataWoeKillDebilitatorAction(ai); }
        static Action* kill_hand_of_the_queen(PlayerbotAI* ai) { return new CataWoeKillHandAction(ai); }
        static Action* leave_fel_firestorm(PlayerbotAI* ai)
        {
            return new CataWoeEscapeAction(ai, "woe leave fel firestorm", WellOfEternity::HazardKind::FelFirestorm);
        }
        static Action* use_portal_energy_focus(PlayerbotAI* ai) { return new CataWoeUsePortalFocusAction(ai); }
};

#endif
