/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/BankAction.h/.cpp (#258).
 *
 *  - AC's InventoryAction::parseItems(text, mask) does not exist on this fork's cut-down
 *    InventoryAction (strategy/actions/InventoryAction.h): the surviving entry point is
 *    InventoryAction::Find(qualifier), which takes no mask and only ever walks bags/equipment
 *    (IterateItems supports ITERATE_ITEMS_IN_EQUIP only - ITERATE_ITEMS_IN_BANK is declared in
 *    ItemVisitors.h:24 but has no branch in InventoryAction::IterateItems). So the deposit path uses
 *    Find() (bags, unchanged behaviour) and the withdraw path resolves its qualifier against the bank
 *    through the file-local ForEachItemInBank() helper in BankAction.cpp. Implementing the
 *    ITERATE_ITEMS_IN_BANK branch inside InventoryAction is a shared-header change deliberately left
 *    out of this slice (filed as a gap).
 *  - AC's PlayerbotAI::TellItems() is not on this fork (see TradeStatusAction.cpp:76-79 for the same
 *    gap); the file-local TellBankList() below is the same simplification.
 *  - PlayerbotTextMgr is not ported: BankAction.cpp's one call site keeps AC's literal fallback
 *    string.
 */

#ifndef _PLAYERBOT_BANKACTION_H
#define _PLAYERBOT_BANKACTION_H

#include "InventoryAction.h"

class Item;
class PlayerbotAI;
class Unit;

class BankAction : public InventoryAction
{
public:
    BankAction(PlayerbotAI* botAI) : InventoryAction(botAI, "bank") {}

    bool Execute(Event event) override;

private:
    bool ExecuteBank(std::string const text, Unit* bank);
    void ListItems();
    bool Withdraw(uint32 itemid);
    bool Deposit(Item* pItem);
    Item* FindItemInBank(uint32 itemid);
};

#endif