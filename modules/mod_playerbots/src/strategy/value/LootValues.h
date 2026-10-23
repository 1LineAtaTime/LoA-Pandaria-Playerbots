/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #151: replaces the #149 neutral stand-ins that used to live in this file (empty LootObject,
 * forward-declared LootObjectStack) with AC's real loot values, ported from
 * Ai/Base/Value/AvailableLootValue.h/.cpp and Ai/Base/Value/HasAvailableLootValue.h/.cpp. The file
 * name and include guard stay the same as before so ValueContext.h's #include "LootValues.h" does
 * not need to change.
 */

#ifndef _PLAYERBOT_LOOTVALUES_H
#define _PLAYERBOT_LOOTVALUES_H

#include "ItemUsageValue.h"
#include "LootMgr.h"
#include "LootObjectStack.h"
#include "Value.h"

class PlayerbotAI;

// AC Ai/Base/Value/LootValues.h:18-25. Cheat class copy to hack into the loot system.
// 5.4.8 divergence: the core declares `LootGroups` as `std::vector<LootGroup*>`
// (LootMgr.h:272), not AC's `std::vector<LootGroup>`, so the mirror must match the core.
class LootTemplateAccess
{
public:
    class LootGroup;  // A set of loot definitions for items (refs are not allowed inside)
    typedef std::vector<LootGroup*> LootGroups;
    LootStoreItemList Entries;  // not grouped only
    LootGroups Groups;          // groups have own (optimized) processing, grouped entries go there
};

//                   itemId, entry
// Multi: one item has many droppers.
typedef std::unordered_multimap<uint32, int32> DropMap;

// Returns the loot map of all entries
class DropMapValue : public SingleCalculatedValue<DropMap*>
{
public:
    DropMapValue(PlayerbotAI* botAI) : SingleCalculatedValue(botAI, "drop map") {}

    static LootTemplateAccess const* GetLootTemplate(ObjectGuid guid, LootType type = LOOT_CORPSE);

    DropMap* Calculate() override;
};

// Returns the items a specific entry can drop
// (AC LootValues.h:52-58; AC also declares ItemDropListValue here, which is not ported - see
//  strategy/value/QuestValues.cpp file header, which documents the deliberate omission.)
class EntryLootListValue : public SingleCalculatedValue<std::vector<uint32>>, public Qualified
{
public:
    EntryLootListValue(PlayerbotAI* botAI) : SingleCalculatedValue(botAI, "entry loot list") {}

    std::vector<uint32> Calculate() override;
};

typedef std::unordered_map<ItemUsage, std::vector<uint32>> itemUsageMap;

class EntryLootUsageValue : public CalculatedValue<itemUsageMap>, public Qualified
{
public:
    EntryLootUsageValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "entry loot usage", 2 * 1000) {}

    itemUsageMap Calculate() override;
};

class HasUpgradeValue : public BoolCalculatedValue, public Qualified
{
public:
    HasUpgradeValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "has upgrade", 2 * 1000) {}

    bool Calculate() override;
};

// AC AvailableLootValue.h:15-20. Owns the bot's LootObjectStack for its whole lifetime.
class AvailableLootValue : public ManualSetValue<LootObjectStack*>
{
public:
    AvailableLootValue(PlayerbotAI* botAI, std::string const name = "available loot");
    virtual ~AvailableLootValue();
};

// AC AvailableLootValue.h:22-26.
class LootTargetValue : public ManualSetValue<LootObject>
{
public:
    LootTargetValue(PlayerbotAI* botAI, std::string const name = "loot target");
};

// AC AvailableLootValue.h:28-34.
class CanLootValue : public BoolCalculatedValue
{
public:
    CanLootValue(PlayerbotAI* botAI, std::string const name = "can loot") : BoolCalculatedValue(botAI, name) {}

    bool Calculate() override;
};

// AC Ai/Base/Value/HasAvailableLootValue.h/.cpp.
class HasAvailableLootValue : public BoolCalculatedValue
{
public:
    HasAvailableLootValue(PlayerbotAI* botAI, std::string const name = "has available loot")
        : BoolCalculatedValue(botAI, name)
    {
    }

    bool Calculate() override;
};

#endif
