/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/LootAction.h (issue #151), replacing the #62
 * stand-in that opened a nearby corpse directly. This now drives AC's real LootObjectStack flow:
 * LootAction only *selects* the nearest possible loot target; OpenLootAction/StoreLootAction/
 * ReleaseLootAction below (and MoveToLootAction at the bottom) do the rest.
 */

#ifndef _PLAYERBOT_LOOTACTION_H
#define _PLAYERBOT_LOOTACTION_H

#include "InventoryAction.h"
#include "MovementActions.h"

class GameObject;
class LootObject;
class PlayerbotAI;
class SpellInfo;

// AC LootAction.h:18-25.
class LootAction : public MovementAction
{
public:
    LootAction(PlayerbotAI* botAI) : MovementAction(botAI, "loot") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// AC LootAction.h:27-40.
class OpenLootAction : public MovementAction
{
public:
    OpenLootAction(PlayerbotAI* botAI) : MovementAction(botAI, "open loot") {}

    bool Execute(Event event) override;

private:
    bool DoLoot(LootObject& lootObject);
    uint32 GetOpeningSpell(LootObject& lootObject);
    uint32 GetOpeningSpell(LootObject& lootObject, GameObject* go);
    bool CanOpenLock(LootObject& lootObject, SpellInfo const* spellInfo, GameObject* go);
    bool CanOpenLock(uint32 skillId, uint32 reqSkillValue);
};

// AC LootAction.h:42-49. Fed by the WorldPacket action path (WorldPacketActionContext.h) on the
// "loot response" trigger (SMSG_LOOT_RESPONSE - see AI/PlayerbotAI.cpp's handler registration).
class StoreLootAction : public InventoryAction
{
public:
    StoreLootAction(PlayerbotAI* botAI) : InventoryAction(botAI, "store loot") {}

    bool Execute(Event event) override;
    static bool IsLootAllowed(uint32 itemid, PlayerbotAI* botAI);
};

// AC LootAction.h:51-57.
class ReleaseLootAction : public InventoryAction
{
public:
    ReleaseLootAction(PlayerbotAI* botAI) : InventoryAction(botAI, "release loot") {}

    bool Execute(Event event) override;
};

// AC MovementActions.cpp:2602-2609. Defined here rather than in MovementActions.h (15 includers -
// see loot-stack-port.md step 4.3) so only ActionContext.h (which already includes LootAction.h)
// pulls it in.
class MoveToLootAction : public MovementAction
{
public:
    MoveToLootAction(PlayerbotAI* botAI) : MovementAction(botAI, "move to loot") {}

    bool Execute(Event event) override;
};

#endif
