/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Full port of AC mod-playerbots Ai/Base/Value/ItemUsageValue.{h,cpp} for issue #188 (S6 of epic
 * #163), replacing the #151 stub (see the old file header, preserved in git history). The ItemUsage
 * enum and Qualified/CalculatedValue<T> plumbing already existed on this fork (strategy/ItemVisitors.h,
 * strategy/value/Value.h) and are unchanged - only Calculate() and its helpers are new. See
 * ItemUsageValue.cpp for the full list of 5.4.8 adaptations (RandomItemManager signature, no
 * GuildTaskMgr, no ammo, quest objective system, etc).
 */

#ifndef _PLAYERBOT_ITEMUSAGEVALUE_H
#define _PLAYERBOT_ITEMUSAGEVALUE_H

#include "ItemVisitors.h"
#include "NamedObjectContext.h"
#include "Value.h"

class Item;
class Player;
class PlayerbotAI;
struct ItemTemplate;

struct ParsedItemUsage
{
    uint32 itemId = 0;
    int32 randomPropertyId = 0;
};

class ItemUsageValue : public CalculatedValue<ItemUsage>, public Qualified
{
public:
    ItemUsageValue(PlayerbotAI* botAI, std::string const name = "item usage") : CalculatedValue<ItemUsage>(botAI, name)
    {
    }

    ItemUsage Calculate() override;

protected:
    ItemUsage QueryItemUsageForEquip(ItemTemplate const* proto, int32 randomPropertyId = 0);
    ItemUsage QueryItemUsageForAmmo(ItemTemplate const* proto);
    ParsedItemUsage GetItemIdFromQualifier();

private:
    uint32 GetSmallestBagSize();
    bool IsItemUsefulForQuest(Player* player, ItemTemplate const* proto);
    bool IsItemNeededForSkill(ItemTemplate const* proto);
    bool IsItemUsefulForSkill(ItemTemplate const* proto);
    bool IsItemNeededForUsefullSpell(ItemTemplate const* proto, bool checkAllReagents = false);
    bool HasItemsNeededForSpell(uint32 spellId, ItemTemplate const* proto);
    Item* CurrentItem(ItemTemplate const* proto);
    float CurrentStacks(ItemTemplate const* proto);
    float BetterStacks(ItemTemplate const* proto, std::string const usageType = "");

public:
    static std::vector<uint32> SpellsUsingItem(uint32 itemId, Player* bot);
    static bool SpellGivesSkillUp(uint32 spellId, Player* bot);

    static std::string const GetConsumableType(ItemTemplate const* proto, bool hasMana);
};

class ItemUpgradeValue : public ItemUsageValue
{
public:
    ItemUpgradeValue(PlayerbotAI* botAI, std::string const name = "item upgrade") : ItemUsageValue(botAI, name)
    {
    }

    ItemUsage Calculate() override;
};

#endif
