/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#ifndef PLAYERBOTS_CATABCACTIONCONTEXT_H
#define PLAYERBOTS_CATABCACTIONCONTEXT_H

#include "Action.h"
#include "CataBcActions.h"
#include "NamedObjectContext.h"

class CataDungeonBcActionContext : public NamedObjectContext<Action>
{
public:
    CataDungeonBcActionContext()
    {
        creators["cata bc flee skullcracker"] = &CataDungeonBcActionContext::cata_bc_flee_skullcracker;
        creators["cata bc break chains"] = &CataDungeonBcActionContext::cata_bc_break_chains;
        creators["cata bc leave quake"] = &CataDungeonBcActionContext::cata_bc_leave_quake;
        creators["cata bc leave beam"] = &CataDungeonBcActionContext::cata_bc_leave_beam;
        creators["cata bc interrupt"] = &CataDungeonBcActionContext::cata_bc_interrupt;
        creators["cata bc leave lava drool"] = &CataDungeonBcActionContext::cata_bc_leave_lava_drool;
        creators["cata bc karsh pillar"] = &CataDungeonBcActionContext::cata_bc_karsh_pillar;
    }

private:
    static Action* cata_bc_flee_skullcracker(PlayerbotAI* botAI) { return new CataBcFleeSkullcrackerAction(botAI); }
    static Action* cata_bc_break_chains(PlayerbotAI* botAI) { return new CataBcBreakChainsAction(botAI); }
    static Action* cata_bc_leave_quake(PlayerbotAI* botAI) { return new CataBcLeaveQuakeAction(botAI); }
    static Action* cata_bc_leave_beam(PlayerbotAI* botAI) { return new CataBcLeaveBeamAction(botAI); }
    static Action* cata_bc_interrupt(PlayerbotAI* botAI) { return new CataBcInterruptAction(botAI); }
    static Action* cata_bc_leave_lava_drool(PlayerbotAI* botAI) { return new CataBcLeaveLavaDroolAction(botAI); }
    static Action* cata_bc_karsh_pillar(PlayerbotAI* botAI) { return new CataBcKarshPillarAction(botAI); }
};

#endif
