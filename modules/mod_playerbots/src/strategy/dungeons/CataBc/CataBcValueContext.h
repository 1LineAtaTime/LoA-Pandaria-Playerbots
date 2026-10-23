/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#ifndef PLAYERBOTS_CATABCVALUECONTEXT_H
#define PLAYERBOTS_CATABCVALUECONTEXT_H

#include "CataBcShared.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "Unit.h"
#include "Value.h"

// Shadows of Obsidius near the bot (throttled: the target values read it several times a tick).
// Same shape as fork strategy/dungeons/UB/UBValueContext.h UnderbogMushroomsValue; 1000 means 1000 ms.
class CataBcShadowsValue : public CalculatedValue<GuidVector>
{
public:
    CataBcShadowsValue(PlayerbotAI* botAI) : CalculatedValue<GuidVector>(botAI, "cata bc shadows", 1000) {}

protected:
    GuidVector Calculate() override
    {
        if (!bot->IsInCombat())
            return {};

        return CataBc::FindShadowGuids(bot);
    }
};

class CataDungeonBcValueContext : public NamedObjectContext<UntypedValue>
{
public:
    CataDungeonBcValueContext() { creators["cata bc shadows"] = &CataDungeonBcValueContext::cata_bc_shadows; }

private:
    static UntypedValue* cata_bc_shadows(PlayerbotAI* botAI) { return new CataBcShadowsValue(botAI); }
};

#endif
