/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" (Scarlet Halls, epic #373, issue #657). See MopShShared.h for sources.

#ifndef PLAYERBOTS_MOPSHACTIONCONTEXT_H
#define PLAYERBOTS_MOPSHACTIONCONTEXT_H

#include "Action.h"
#include "MopDungeonShared.h"
#include "MopShActions.h"
#include "MopShShared.h"
#include "NamedObjectContext.h"

class MopDungeonScarletHallsActionContext : public NamedObjectContext<Action>
{
public:
    MopDungeonScarletHallsActionContext()
    {
        creators["sh leave knife line"] = &MopDungeonScarletHallsActionContext::sh_leave_knife_line;
        creators["sh flee blades of light"] = &MopDungeonScarletHallsActionContext::sh_flee_blades_of_light;
        creators["sh koegler interrupt"] = &MopDungeonScarletHallsActionContext::sh_koegler_interrupt;
        creators["sh leave burning books"] = &MopDungeonScarletHallsActionContext::sh_leave_burning_books;
        creators["sh intercept book burner"] = &MopDungeonScarletHallsActionContext::sh_intercept_book_burner;
        creators["sh behind koegler"] = &MopDungeonScarletHallsActionContext::sh_behind_koegler;
        creators["sh hounds"] = &MopDungeonScarletHallsActionContext::sh_hounds;
        creators["sh tank hounds"] = &MopDungeonScarletHallsActionContext::sh_tank_hounds;
        creators["sh defenders"] = &MopDungeonScarletHallsActionContext::sh_defenders;
        creators["sh tank defenders"] = &MopDungeonScarletHallsActionContext::sh_tank_defenders;
    }

private:
    static Action* sh_leave_knife_line(PlayerbotAI* botAI) { return new MopShLeaveKnifeLineAction(botAI); }
    static Action* sh_flee_blades_of_light(PlayerbotAI* botAI) { return new MopShFleeBladesAction(botAI); }
    static Action* sh_koegler_interrupt(PlayerbotAI* botAI) { return new MopShKoeglerInterruptAction(botAI); }
    static Action* sh_leave_burning_books(PlayerbotAI* botAI) { return new MopShLeaveBurningBooksAction(botAI); }
    static Action* sh_intercept_book_burner(PlayerbotAI* botAI) { return new MopShInterceptBookBurnerAction(botAI); }
    static Action* sh_behind_koegler(PlayerbotAI* botAI) { return new MopShBehindKoeglerAction(botAI); }

    static Action* sh_hounds(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "sh hounds", MopScarletHalls::HoundWave(MopDungeon::WaveRole::Dps));
    }

    static Action* sh_tank_hounds(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "sh tank hounds",
                                             MopScarletHalls::HoundWave(MopDungeon::WaveRole::Tank));
    }

    static Action* sh_defenders(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "sh defenders",
                                             MopScarletHalls::DefenderWave(MopDungeon::WaveRole::Dps));
    }

    static Action* sh_tank_defenders(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "sh tank defenders",
                                             MopScarletHalls::DefenderWave(MopDungeon::WaveRole::Tank));
    }
};

#endif
