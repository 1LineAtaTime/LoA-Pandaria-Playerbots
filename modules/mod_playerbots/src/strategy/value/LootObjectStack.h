/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots (Mgr/Item/LootObjectStack.h, issue #151) into the Legends of Azeroth
 * Pandaria fork. Replaces the #149 neutral stand-ins that used to live in LootValues.h (a bot could
 * never actually track corpses/game objects to loot). 5.4.8 adaptations are documented at each call
 * site in LootObjectStack.cpp.
 */

#ifndef _PLAYERBOT_LOOTOBJECTSTACK_H
#define _PLAYERBOT_LOOTOBJECTSTACK_H

#include "ObjectGuid.h"

#include <set>

class AiObjectContext;
class Player;
class WorldObject;

struct ItemTemplate;

// AC Mgr/Item/LootObjectStack.h:18-25. Interface for "loot strategy" (normal/gray/disenchant/all -
// see LootStrategyValue.h). Not the same class as the old strategy/generic/NonCombatStrategy.h
// "LootStrategy" *Strategy* - that one is renamed LootNonCombatStrategy in this port (see
// strategy/generic/LootNonCombatStrategy.h) to resolve the name clash across the two headers
// AiObjectContext.cpp pulls in (loot-stack-port.md section 2 step 6).
class LootStrategy
{
public:
    LootStrategy() {}
    virtual ~LootStrategy() {}
    virtual bool CanLoot(ItemTemplate const* proto, AiObjectContext* context) = 0;
    virtual std::string const GetName() = 0;
};

// AC LootObjectStack.h:27-47 / .cpp:48-349.
class LootObject
{
public:
    LootObject() : skillId(0), reqSkillValue(0), reqItem(0) {}
    LootObject(Player* bot, ObjectGuid guid);
    LootObject(LootObject const& other);
    LootObject& operator=(LootObject const& other) = default;

    bool IsEmpty() { return !guid; }
    bool IsLootPossible(Player* bot);
    void Refresh(Player* bot, ObjectGuid guid);
    WorldObject* GetWorldObject(Player* bot);
    ObjectGuid guid;

    uint32 skillId;
    uint32 reqSkillValue;
    uint32 reqItem;

private:
    static bool IsNeededForQuest(Player* bot, uint32 itemId);
};

// AC LootObjectStack.h:49-62.
class LootTarget
{
public:
    LootTarget(ObjectGuid guid);
    LootTarget(LootTarget const& other);

public:
    LootTarget& operator=(LootTarget const& other);
    bool operator<(LootTarget const& other) const;

public:
    ObjectGuid guid;
    time_t asOfTime;
};

class LootTargetList : public std::set<LootTarget>
{
public:
    void shrink(time_t fromTime);
};

// AC LootObjectStack.h:70-86.
class LootObjectStack
{
public:
    LootObjectStack(Player* bot) : bot(bot) {}

    bool Add(ObjectGuid guid);
    void Remove(ObjectGuid guid);
    void Clear();
    bool CanLoot(float maxDistance);
    LootObject GetLoot(float maxDistance = 0);

private:
    LootObject GetNearest(float maxDistance = 0);

    Player* bot;
    LootTargetList availableLoot;
};

#endif
