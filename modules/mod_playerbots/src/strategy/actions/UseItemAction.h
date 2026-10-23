/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/UseItemAction.h/.cpp (#149), cut down to
 * "use this item on myself" - all EatAction/DrinkAction/UseHealingPotion/UseManaPotion need. AC's
 * UseItemAction also supports gameobject-targeted use, item-on-item (socketing), starting a quest
 * from an item, a human master's current target and a trade-item target; none of that is ported
 * per #149's instructions ("you do not need to port every AC feature"). isPossible()/Execute()
 * mirror AC's; UseItem() is a self-only distillation of AC's UseItem()/UseItemAuto().
 */

#ifndef _PLAYERBOT_USEITEMACTION_H
#define _PLAYERBOT_USEITEMACTION_H

#include "Action.h"

class Item;
class PlayerbotAI;

class UseItemAction : public Action
{
public:
    UseItemAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name) {}

    bool Execute(Event event) override;
    bool isPossible() override;

protected:
    // Casts the item's on-use spell targeting the bot itself, the same CMSG_USE_ITEM packet +
    // HandleUseItemOpcode() flow this module already uses for UseTrinketAction
    // (GenericSpellActions.cpp:342-389), just without a unit/gameobject target.
    bool UseItem(Item* item);
};

class UseHealingPotion : public UseItemAction
{
public:
    UseHealingPotion(PlayerbotAI* botAI) : UseItemAction(botAI, "healing potion") {}

    bool isUseful() override;
};

class UseManaPotion : public UseItemAction
{
public:
    UseManaPotion(PlayerbotAI* botAI) : UseItemAction(botAI, "mana potion") {}

    bool isUseful() override;
};

// #257 (AC UseItemAction.h:87-95 / UseItemAction.cpp:455-498): picks a random quest-class bag item that starts a
// quest the bot can take and accepts that quest from the item. AC's MaintenanceStrategy.cpp:66 schedules it
// ("random" -> 0.9); this fork has no MaintenanceStrategy, so it is reachable by name only
// (DoSpecificAction("use random quest item")).
class UseRandomQuestItem : public UseItemAction
{
public:
    UseRandomQuestItem(PlayerbotAI* botAI) : UseItemAction(botAI, "random quest item") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

#endif
