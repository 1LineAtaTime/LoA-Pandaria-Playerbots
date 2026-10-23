/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/EquipAction.{h,cpp} for issue #188 (S6 of epic #163).
 * See EquipAction.cpp for the 5.4.8 adaptations (no ChatHelper item-link formatter/parser, no
 * WorldPackets::Item packet-struct handlers on this core - equips go through Player::SwapItem()
 * directly instead of a synthesized CMSG_AUTOEQUIP_ITEM_SLOT).
 */

#ifndef PLAYERBOTS_EQUIPACTION_H
#define PLAYERBOTS_EQUIPACTION_H

#include "InventoryAction.h"
#include "ItemVisitors.h"

class FindItemVisitor;
class Item;
class PlayerbotAI;

class EquipAction : public InventoryAction
{
public:
    EquipAction(PlayerbotAI* botAI, std::string const name = "equip") : InventoryAction(botAI, name) {}

    bool Execute(Event event) override;
    void EquipItems(ItemIds ids);
    ItemIds SelectInventoryItemsToEquip();

private:
    void EquipItem(FindItemVisitor* visitor);
    uint8 GetSmallestBagSlot();
    void EquipItem(Item* item);
};

class EquipUpgradesPacketAction : public EquipAction
{
public:
    explicit EquipUpgradesPacketAction(PlayerbotAI* botAI, std::string const name = "equip upgrades packet action") : EquipAction(botAI, name) {}

    bool Execute(Event event) override;
};

class EquipUpgradeAction : public EquipAction
{
public:
    explicit EquipUpgradeAction(PlayerbotAI* botAI, std::string const name = "equip upgrade") : EquipAction(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
