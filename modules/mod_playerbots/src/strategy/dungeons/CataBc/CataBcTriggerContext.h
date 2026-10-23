/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#ifndef PLAYERBOTS_CATABCTRIGGERCONTEXT_H
#define PLAYERBOTS_CATABCTRIGGERCONTEXT_H

#include "CataBcTriggers.h"
#include "NamedObjectContext.h"
#include "TriggerContext.h"

class CataDungeonBcTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CataDungeonBcTriggerContext()
    {
        creators["cata bc skullcracker"] = &CataDungeonBcTriggerContext::cata_bc_skullcracker;
        creators["cata bc chains"] = &CataDungeonBcTriggerContext::cata_bc_chains;
        creators["cata bc quake"] = &CataDungeonBcTriggerContext::cata_bc_quake;
        creators["cata bc lava drool"] = &CataDungeonBcTriggerContext::cata_bc_lava_drool;
        creators["cata bc evolution"] = &CataDungeonBcTriggerContext::cata_bc_evolution;
        creators["cata bc interrupt"] = &CataDungeonBcTriggerContext::cata_bc_interrupt;
        creators["cata bc karsh pillar"] = &CataDungeonBcTriggerContext::cata_bc_karsh_pillar;
    }

private:
    static Trigger* cata_bc_skullcracker(PlayerbotAI* botAI) { return new CataBcSkullcrackerTrigger(botAI); }
    static Trigger* cata_bc_chains(PlayerbotAI* botAI) { return new CataBcChainsTrigger(botAI); }
    static Trigger* cata_bc_quake(PlayerbotAI* botAI) { return new CataBcQuakeTrigger(botAI); }
    static Trigger* cata_bc_lava_drool(PlayerbotAI* botAI) { return new CataBcLavaDroolTrigger(botAI); }
    static Trigger* cata_bc_evolution(PlayerbotAI* botAI) { return new CataBcEvolutionTrigger(botAI); }
    static Trigger* cata_bc_interrupt(PlayerbotAI* botAI) { return new CataBcInterruptTrigger(botAI); }
    static Trigger* cata_bc_karsh_pillar(PlayerbotAI* botAI) { return new CataBcKarshPillarTrigger(botAI); }
};

#endif
