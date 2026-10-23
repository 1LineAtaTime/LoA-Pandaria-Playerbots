/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Full port of AC mod-playerbots Ai/Base/Value/ItemUsageValue.cpp for issue #188 (S6 of epic #163).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - bot->getClass() -> bot->GetClass().
 *  - IsRealPlayer(Player*) (AC free function: "is this an actual human, not a bot") has no equivalent
 *    on this fork; the established pattern elsewhere in this module (PassLeadershipToMasterAction.cpp:57)
 *    is `!GET_PLAYERBOT_AI(player)`. Used here as MasterIsRealPlayer(botAI).
 *  - IsSelfBot(Player*) (AC free function: "is this bot mastered by no one, i.e. a standalone random
 *    bot") maps to this fork's (confusingly named) PlayerbotAI::IsRealPlayer() - an established
 *    mapping documented at AreaTriggerAction.cpp:15, LootAction.cpp:121, PassLeadershipToMasterAction.cpp:14-16.
 *  - bot->BotCanUseItem(proto) (AC-patched Player method) does not exist on this fork's Player;
 *    replaced with the stock bot->CanUseItem(proto) also already used elsewhere in this function.
 *  - botAI->HasSkill(SkillType) does not exist on this fork's PlayerbotAI (see
 *    strategy/value/LootObjectStack.cpp's own note on the same gap); ported as a local BotHasSkill()
 *    helper using Player::HasSkill()+GetSkillValue(), the same fix LootObjectStack.cpp already uses.
 *  - GuildTaskMgr does not exist anywhere on this fork (no guild-task system ported at all yet) -
 *    the whole ITEM_USAGE_GUILD_TASK branch is dropped. Flagged as an AC feature that could not be
 *    ported (needs its own guild-task-system port first, out of scope for #188).
 *  - RandomItemMgr (AC) is named RandomItemManager here (Manager/RandomItemManager.h) with a
 *    different signature: `sRandomItemMgr->CanEquipWeapon(Player*, ItemTemplate const*)` and
 *    `CanEquipArmor(Player*, ItemTemplate const*)` (pointer singleton, no separate class/level args -
 *    it derives them from the Player* itself). `RandomItemMgr::IsUsedBySkill(proto, skill)` has no
 *    equivalent at all (that reagent/recipe-usage table does not exist on this fork) - the
 *    IsItemUsefulForSkill() trade-goods/reagent branch is dropped (always returns false for that
 *    item-class group); the recipe branch, which doesn't need it, is kept. Flagged as an AC feature
 *    that could not be ported without adding a new reagent-usage table (out of scope for #188).
 *  - sRandomPlayerbotMgr.IsSpecPvp(...) (per-bot "is this bot specced for PvP" flag) does not exist
 *    on this fork's RandomPlayerbotMgr. isPvp is hardcoded false here (StatsWeightCalculator's PvP
 *    Power/Resilience weighting - see StatsWeightCalculator.cpp - is wired up and ready, just never
 *    triggered from this call site). Flagged for a follow-up once/if a per-bot PvP-spec flag exists.
 *  - sPlayerbotAIConfig.equipUpgradeThreshold does not exist either (AiPlayerbot.EquipUpgradeThreshold
 *    was never ported); read locally via sConfigMgr the same way as AiPlayerbot.PreferredSpecWeapons.
 *  - Item::CreateItem() on this fork takes (item, count, player, temp) - no randomPropertyId/extra
 *    bool params like AC's 6-arg call; adapted to the 3-arg form (temp defaults false).
 *  - ITEM_FLAG_UNIQUE_EQUIPPABLE was renamed ITEM_PROTO_FLAG_UNIQUE_EQUIPPED on this fork, and
 *    ItemTemplate has no generic HasFlag() helper, so the check is a direct bitwise test.
 *  - Ammo/quivers were removed from the game entirely in Cataclysm (ranged weapons no longer consume
 *    ammunition; PLAYER_AMMO_ID does not exist on this fork). QueryItemUsageForAmmo() is neutered to
 *    always return ITEM_USAGE_NONE; the ITEM_USAGE_AMMO enum value (inherited from ItemVisitors.h,
 *    shared with the rest of the module) simply never fires from this file any more.
 *  - Quest required-items are a fixed 4-slot RequiredItemId[]/RequiredItemCount[] array in AC (WotLK).
 *    This fork's Quest class uses the later, generic QuestObjective/QuestObjectives system
 *    (Quests/QuestDef.h) - IsItemUsefulForQuest() iterates quest->GetObjectives() and matches
 *    QUEST_OBJECTIVE_ITEM objectives by ObjectID/Amount instead.
 *  - PlayerSpell's AC fields State/Active are lowercase state/active on this fork (SpellGivesSkillUp
 *    is unaffected; SpellsUsingItem uses these).
 *  - chat->FormatItem(proto) (AC's ChatHelper item-link formatter, used as the "inventory items" AI
 *    value qualifier) does not exist on this fork; this fork's InventoryAction::Find() also only
 *    understood four consumable-category qualifiers ("food"/"drink"/"mana potion"/"healing potion").
 *    Extended InventoryAction::Find() (strategy/actions/InventoryAction.cpp) with a numeric-qualifier
 *    branch that matches items by id, and pass std::to_string(proto->ItemId) here instead of a
 *    formatted link. See InventoryAction.cpp for that change.
 */

#include "ItemUsageValue.h"

#include "Config.h"
#include "Item.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "PlayerbotAIConfig.h"
#include "QuestDef.h"
#include "RandomItemManager.h"
#include "StatsWeightCalculator.h"
#include "World.h"

#include <unordered_set>

namespace
{
// 5.4.8: AC PlayerbotAI::HasSkill(SkillType) has no equivalent on this fork's PlayerbotAI (see
// strategy/value/LootObjectStack.cpp's identical local helper for the same gap). Player::HasSkill()
// exists (Entities/Player/Player.h).
bool BotHasSkill(Player* bot, uint32 skillId)
{
    return skillId != SKILL_NONE && bot->HasSkill(skillId) && bot->GetSkillValue(skillId) > 0;
}

// 5.4.8: see the free-function IsRealPlayer(Player*) note in the file header.
bool MasterIsRealPlayer(PlayerbotAI* botAI)
{
    Player* master = botAI->GetMaster();
    return master && !GET_PLAYERBOT_AI(master);
}

// 5.4.8: AiPlayerbot.EquipUpgradeThreshold is an AC config key (PlayerbotAIConfig.cpp:713, default
// 1.1f) that was never ported into this fork's PlayerbotAIConfig.h, which is off-limits for this
// issue (a ~48-minute full-module rebuild). Read locally with AC's own key name/default, the same
// pattern used for AiPlayerbot.PreferredSpecWeapons in StatsWeightCalculator.cpp.
float EquipUpgradeThreshold()
{
    static float const v = sConfigMgr->GetFloatDefault("AiPlayerbot.EquipUpgradeThreshold", 1.1f);
    return v;
}
}

ItemUsage ItemUsageValue::Calculate()
{
    ParsedItemUsage const parsed = GetItemIdFromQualifier();
    uint32 itemId = parsed.itemId;
    uint32 randomPropertyId = parsed.randomPropertyId;
    if (!itemId)
        return ITEM_USAGE_NONE;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return ITEM_USAGE_NONE;

    if (MasterIsRealPlayer(botAI))
    {
        if (IsItemUsefulForSkill(proto) || IsItemNeededForSkill(proto))
            return ITEM_USAGE_SKILL;
    }
    else
    {
        bool needItem = false;

        if (IsItemNeededForSkill(proto))
            needItem = true;
        else
        {
            bool lowBagSpace = AI_VALUE(uint8, "bag space") > 50;

            if (proto->Class == ITEM_CLASS_TRADE_GOODS || proto->Class == ITEM_CLASS_MISCELLANEOUS ||
                proto->Class == ITEM_CLASS_REAGENT)
                needItem = IsItemNeededForUsefullSpell(proto, lowBagSpace);
            else if (proto->Class == ITEM_CLASS_RECIPE)
            {
                if (bot->HasSpell(proto->Spells[2].SpellId))
                    needItem = false;
                else
                    needItem = bot->CanUseItem(proto) == EQUIP_ERR_OK;
            }
        }

        if (needItem)
        {
            float stacks = CurrentStacks(proto);
            if (stacks < 1)
                return ITEM_USAGE_SKILL;  // Buy more.
            if (stacks < 2)
                return ITEM_USAGE_KEEP;  // Keep current amount.
        }
    }

    if (proto->Class == ITEM_CLASS_KEY)
        return ITEM_USAGE_USE;

    const uint32_t maxCount = proto->MaxCount;

    if (proto->Class == ITEM_CLASS_CONSUMABLE &&
        (maxCount == 0 || bot->GetItemCount(itemId, false) < maxCount))
    {
        std::string const foodType = GetConsumableType(proto, bot->GetPower(POWER_MANA));

        if (!foodType.empty() && bot->CanUseItem(proto) == EQUIP_ERR_OK)
        {
            float stacks = BetterStacks(proto, foodType);
            if (stacks < 2)
            {
                stacks += CurrentStacks(proto);

                if (stacks < 2)
                    return ITEM_USAGE_USE;  // Buy some to get to 2 stacks
                else if (stacks < 3)        // Keep the item if less than 3 stacks
                    return ITEM_USAGE_KEEP;
            }
        }
    }

    // 5.4.8: no GuildTaskMgr on this fork (see file header) - ITEM_USAGE_GUILD_TASK never fires.

    ItemUsage equip = QueryItemUsageForEquip(proto, randomPropertyId);
    if (equip != ITEM_USAGE_NONE)
        return equip;

    // Get item instance to check if it's soulbound
    Item* item = bot->GetItemByEntry(proto->ItemId);
    bool isSoulbound = item && item->IsSoulBound();

    if ((proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON) &&
        BotHasSkill(bot, SKILL_ENCHANTING) &&
        proto->Quality >= ITEM_QUALITY_UNCOMMON)
    {
        // Retrieve the bot's Enchanting skill level
        uint32 enchantingSkill = bot->GetSkillValue(SKILL_ENCHANTING);

        // Only disenchant if skilled enough and binding allows it
        if (enchantingSkill >= proto->RequiredDisenchantSkill &&
            (proto->Bonding == BIND_ON_ACQUIRE || (proto->Bonding == BIND_ON_EQUIP && isSoulbound)))
            return ITEM_USAGE_DISENCHANT;
    }

    Player* master = botAI->GetMaster();
    bool botNeedsItemForQuest = IsItemUsefulForQuest(bot, proto);
    bool masterNeedsItemForQuest = master && sPlayerbotAIConfig->syncQuestWithPlayer && IsItemUsefulForQuest(master, proto);

    // Identify the source of loot
    LootObject lootObject = AI_VALUE(LootObject, "loot target");

    // Get GUID of loot source
    ObjectGuid lootGuid = lootObject.guid;

    // Check if loot source is an item
    bool isLootFromItem = lootGuid.IsItem();

    // If the loot is from an item in the bot's bags, ignore syncQuestWithPlayer
    if (isLootFromItem && botNeedsItemForQuest)
        return ITEM_USAGE_QUEST;

    // If this is not a selfbot acting alone and the master needs this quest item, defer to the master
    if (!botAI->IsRealPlayer() && masterNeedsItemForQuest)
        return ITEM_USAGE_NONE;

    // If the bot itself needs the item for a quest, allow looting
    if (botNeedsItemForQuest)
        return ITEM_USAGE_QUEST;

    if (proto->Class == ITEM_CLASS_PROJECTILE && bot->CanUseItem(proto) == EQUIP_ERR_OK)
    {
        ItemUsage ammoUsage = QueryItemUsageForAmmo(proto);
        if (ammoUsage != ITEM_USAGE_NONE)
            return ammoUsage;
    }
    // Need to add something like free bagspace or item value.
    if (proto->SellPrice > 0)
    {
        if (proto->Quality >= ITEM_QUALITY_NORMAL && !isSoulbound && proto->Bonding != BIND_ON_ACQUIRE)
            return ITEM_USAGE_AH;
        else
            return ITEM_USAGE_VENDOR;
    }

    return ITEM_USAGE_NONE;
}

ItemUsage ItemUsageValue::QueryItemUsageForEquip(ItemTemplate const* itemProto, int32 randomPropertyId)
{
    if (bot->CanUseItem(itemProto) != EQUIP_ERR_OK)
        return ITEM_USAGE_NONE;

    if (itemProto->InventoryType == INVTYPE_NON_EQUIP)
        return ITEM_USAGE_NONE;

    Item* pItem = Item::CreateItem(itemProto->ItemId, 1, bot);
    if (!pItem)
        return ITEM_USAGE_NONE;

    uint16 dest;
    InventoryResult result = botAI->CanEquipItem(NULL_SLOT, dest, pItem, true, true);
    pItem->RemoveFromUpdateQueueOf(bot);
    delete pItem;

    if (result != EQUIP_ERR_OK && result != EQUIP_ERR_ITEM_MAX_COUNT)
        return ITEM_USAGE_NONE;

    // Check if unique items are equipped or not
    // 5.4.8: ITEM_FLAG_UNIQUE_EQUIPPABLE -> ITEM_PROTO_FLAG_UNIQUE_EQUIPPED (see file header); no
    // generic ItemTemplate::HasFlag() helper on this fork, so a direct bitwise test.
    bool needToCheckUnique = result == EQUIP_ERR_ITEM_MAX_COUNT ||
         (itemProto->Flags & ITEM_PROTO_FLAG_UNIQUE_EQUIPPED) != 0;

    if (needToCheckUnique)
    {
        // Count the total number of the item (equipped + in bags)
        uint32 totalItemCount = bot->GetItemCount(itemProto->ItemId, true);

        // Count the number of the item in bags only
        uint32 bagItemCount = bot->GetItemCount(itemProto->ItemId, false);

        // Determine if the unique item is already equipped
        bool isEquipped = (totalItemCount > bagItemCount);

        if (isEquipped)
            return ITEM_USAGE_NONE;  // Item is already equipped
        // If not equipped, continue processing
    }

    if (itemProto->Class == ITEM_CLASS_QUIVER && bot->GetClass() != CLASS_HUNTER)
        return ITEM_USAGE_NONE;

    if (itemProto->Class == ITEM_CLASS_CONTAINER)
    {
        if (itemProto->SubClass != ITEM_SUBCLASS_CONTAINER)
            return ITEM_USAGE_NONE;  // Todo add logic for non-bag containers. We want to look at professions/class and
                                     // only replace if non-bag is larger than bag.
        if (GetSmallestBagSize() >= itemProto->ContainerSlots)
            return ITEM_USAGE_NONE;

        return ITEM_USAGE_EQUIP;
    }

    if (itemProto->Class == ITEM_CLASS_WEAPON && itemProto->SubClass == ITEM_SUBCLASS_WEAPON_MISCELLANEOUS)
        return ITEM_USAGE_NONE;

    bool shouldEquip = false;
    StatsWeightCalculator calculator(bot);
    calculator.SetItemSetBonus(false);
    calculator.SetOverflowPenalty(false);

    // 5.4.8: AC applies PvP weights via sRandomPlayerbotMgr.IsSpecPvp(), a per-bot flag that does not
    // exist on this fork's RandomPlayerbotMgr (see file header) - hardcoded false for now.
    bool isPvp = false;
    if (isPvp)
        calculator.SetPvpSpec(true);

    float itemScore = calculator.CalculateItem(itemProto->ItemId, randomPropertyId);

    if (itemScore)
        shouldEquip = true;

    // 5.4.8: RandomItemMgr -> RandomItemManager, signature (Player*, ItemTemplate const*) (see file header).
    if (itemProto->Class == ITEM_CLASS_WEAPON && !sRandomItemMgr->CanEquipWeapon(bot, itemProto))
        shouldEquip = false;
    if (itemProto->Class == ITEM_CLASS_ARMOR && !sRandomItemMgr->CanEquipArmor(bot, itemProto))
        shouldEquip = false;

    uint8 possibleSlots = 1;
    uint8 dstSlot = botAI->FindEquipSlot(itemProto, NULL_SLOT, true);
    // Check if dest wasn't set correctly by CanEquipItem and use FindEquipSlot instead
    // This occurs with unique items that are already in the bots bags when CanEquipItem is called
    if (dest == 0 && dstSlot != NULL_SLOT)
    {
        // Construct dest from dstSlot
        dest = (INVENTORY_SLOT_BAG_0 << 8) | dstSlot;
    }

    if (dstSlot == EQUIPMENT_SLOT_FINGER1 || dstSlot == EQUIPMENT_SLOT_TRINKET1)
        possibleSlots = 2;

    // Check weapon case separately to keep things a bit cleaner
    bool have2HWeapon = false;
    bool isValidTGWeapon = false;

    if (dstSlot == EQUIPMENT_SLOT_MAINHAND)
    {
        Item* currentWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        have2HWeapon = currentWeapon && currentWeapon->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;

        // Determine if the new weapon is a valid Titan Grip weapon
        isValidTGWeapon = (itemProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 ||
                           itemProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2 ||
                           itemProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2);

        // If the bot can Titan Grip, ignore any 2H weapon that isn't a 2H sword, mace, or axe.
        // If this weapon is 2H but not one of the valid TG weapon types, do not equip it at all.
        if (bot->CanTitanGrip() && itemProto->InventoryType == INVTYPE_2HWEAPON && !isValidTGWeapon)
            return ITEM_USAGE_NONE;

        // Now handle the logic for equipping and possible offhand slots
        // If the bot can Dual Wield and:
        // - The weapon is not 2H and we currently don't have a 2H weapon equipped
        // OR
        // - The bot can Titan Grip and it is a valid TG weapon
        // Then we can consider the offhand slot as well.
        if (bot->CanDualWield() &&
            ((itemProto->InventoryType != INVTYPE_2HWEAPON && !have2HWeapon) ||
             (bot->CanTitanGrip() && isValidTGWeapon)))
        {
            possibleSlots = 2;
        }
    }

    for (uint8 i = 0; i < possibleSlots; i++)
    {
        bool shouldEquipInSlot = shouldEquip;
        Item* oldItem = bot->GetItemByPos(dest + i);

        // No item equipped
        if (!oldItem)
        {
            if (shouldEquipInSlot)
                return ITEM_USAGE_EQUIP;
            else
                return ITEM_USAGE_BAD_EQUIP;
        }

        ItemTemplate const* oldItemProto = oldItem->GetTemplate();
        float oldScore = calculator.CalculateItem(oldItemProto->ItemId, oldItem->GetInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID));
        if (oldItem)
        {
            if (itemScore || oldScore)
                shouldEquipInSlot = itemScore > oldScore * EquipUpgradeThreshold();
        }

        // Bigger quiver
        if (itemProto->Class == ITEM_CLASS_QUIVER)
        {
            if (!oldItem || oldItemProto->ContainerSlots < itemProto->ContainerSlots)
                return ITEM_USAGE_EQUIP;
            else
                return ITEM_USAGE_NONE;
        }

        bool existingShouldEquip = true;
        if (oldItemProto->Class == ITEM_CLASS_WEAPON && !sRandomItemMgr->CanEquipWeapon(bot, oldItemProto))
            existingShouldEquip = false;

        if (oldItemProto->Class == ITEM_CLASS_ARMOR && !sRandomItemMgr->CanEquipArmor(bot, oldItemProto))
            existingShouldEquip = false;

        // Compare items based on item level, quality or itemId.
        bool isBetter = false;
        if (itemScore > oldScore)
            isBetter = true;

        Item* item = CurrentItem(itemProto);
        bool itemIsBroken =
            item && item->GetUInt32Value(ITEM_FIELD_DURABILITY) == 0 && item->GetUInt32Value(ITEM_FIELD_MAX_DURABILITY) > 0;
        bool oldItemIsBroken =
            oldItem->GetUInt32Value(ITEM_FIELD_DURABILITY) == 0 && oldItem->GetUInt32Value(ITEM_FIELD_MAX_DURABILITY) > 0;

        if (itemProto->ItemId != oldItemProto->ItemId && (shouldEquipInSlot || !existingShouldEquip) && isBetter)
        {
            switch (itemProto->Class)
            {
                case ITEM_CLASS_ARMOR:
                    if (oldItemProto->SubClass <= itemProto->SubClass)
                    {
                        // Need to add some logic to check second slot before returning, but as it happens, all three of these
                        // return vals will result in an attempted equip action so it wouldn't have much effect currently
                        if (itemIsBroken && !oldItemIsBroken)
                            return ITEM_USAGE_BROKEN_EQUIP;
                        else if (shouldEquipInSlot)
                            return ITEM_USAGE_REPLACE;
                        else
                            return ITEM_USAGE_BAD_EQUIP;
                    }
                    // A lower-subclass armor piece is handled by the default case below.
                    [[fallthrough]];
                default:
                {
                    if (itemIsBroken && !oldItemIsBroken)
                        return ITEM_USAGE_BROKEN_EQUIP;
                    else if (shouldEquipInSlot)
                        return ITEM_USAGE_EQUIP;
                    else
                        return ITEM_USAGE_BAD_EQUIP;
                }
            }
        }

        // Item is not better but current item is broken and new one is not.
        if (oldItemIsBroken && !itemIsBroken)
            return ITEM_USAGE_EQUIP;
    }
    return ITEM_USAGE_NONE;
}

ItemUsage ItemUsageValue::QueryItemUsageForAmmo(ItemTemplate const* /*proto*/)
{
    // 5.4.8: ammunition was removed from the game in Cataclysm (ranged weapons no longer consume
    // arrows/bullets; PLAYER_AMMO_ID does not exist on this fork) - see file header. Nothing to do.
    return ITEM_USAGE_NONE;
}

ParsedItemUsage ItemUsageValue::GetItemIdFromQualifier()
{
    ParsedItemUsage parsed;

    size_t const pos = qualifier.find(",");
    if (pos != std::string::npos)
    {
        parsed.itemId = atoi(qualifier.substr(0, pos).c_str());
        parsed.randomPropertyId = atoi(qualifier.substr(pos + 1).c_str());
        return parsed;
    }
    else
        parsed.itemId = atoi(qualifier.c_str());
    return parsed;
}
// Return smaltest bag size equipped
uint32 ItemUsageValue::GetSmallestBagSize()
{
    int8 curSlot = 0;
    uint32 curSlots = 0;
    for (uint8 bag = INVENTORY_SLOT_BAG_START + 1; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            if (curSlot > 0 && curSlots < pBag->GetBagSize())
                continue;

            curSlot = pBag->GetSlot();
            curSlots = pBag->GetBagSize();
        }
        else
            return 0;
    }

    return curSlots;
}

bool ItemUsageValue::IsItemUsefulForQuest(Player* player, ItemTemplate const* proto)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    if (!botAI)
        return false;

    // 5.4.8: AC's fixed 4-slot Quest::RequiredItemId[]/RequiredItemCount[] (WotLK) was replaced by
    // the generic QuestObjective/QuestObjectives system on this fork (Quests/QuestDef.h) - see file
    // header.
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 entry = player->GetQuestSlotQuestId(slot);
        Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
        if (!quest)
            continue;

        for (QuestObjective const& objective : quest->GetObjectives())
        {
            if (objective.Type != QUEST_OBJECTIVE_ITEM)
                continue;

            // Check if the item itself is needed for the quest
            if (objective.ObjectID == proto->ItemId)
            {
                if (player->GetItemCount(proto->ItemId, false) >= uint32(objective.Amount))
                    continue;

                return true;  // Item is directly required for a quest
            }

            // Check if the item has spells that create this required quest item
            for (uint8 i = 0; i < MAX_ITEM_SPELLS; i++)
            {
                uint32 spellId = proto->Spells[i].SpellId;
                if (!spellId)
                    continue;

                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
                if (!spellInfo)
                    continue;

                for (uint8 effectIndex = 0; effectIndex < MAX_SPELL_EFFECTS; effectIndex++)
                {
                    if (spellInfo->Effects[effectIndex].Effect != SPELL_EFFECT_CREATE_ITEM)
                        continue;

                    uint32 createdItemId = spellInfo->Effects[effectIndex].ItemType;
                    if (createdItemId != objective.ObjectID)
                        continue;

                    if (player->GetItemCount(createdItemId, false) >= uint32(objective.Amount))
                        continue;

                    return true;  // Item is useful because it creates a required quest item
                }
            }
        }
    }

    return false;  // Item is not useful for any active quests
}

bool ItemUsageValue::IsItemNeededForSkill(ItemTemplate const* proto)
{
    switch (proto->ItemId)
    {
        case 756:  // Tunnel Pick
            return BotHasSkill(bot, SKILL_MINING);
        case 778:  // Kobold Excavation Pick
            return BotHasSkill(bot, SKILL_MINING);
        case 1819:  // Gouging Pick
            return BotHasSkill(bot, SKILL_MINING);
        case 1893:  // Miner's Revenge
            return BotHasSkill(bot, SKILL_MINING);
        case 1959:  // Cold Iron Pick
            return BotHasSkill(bot, SKILL_MINING);
        case 2901:  // Mining Pick
            return BotHasSkill(bot, SKILL_MINING);
        case 9465:  // Digmaster 5000
            return BotHasSkill(bot, SKILL_MINING);
        case 20723:  // Brann's Trusty Pick
            return BotHasSkill(bot, SKILL_MINING);
        case 40772:  // Gnomish Army Knife
            return BotHasSkill(bot, SKILL_MINING) || BotHasSkill(bot, SKILL_ENGINEERING) || BotHasSkill(bot, SKILL_BLACKSMITHING) || BotHasSkill(bot, SKILL_COOKING) || BotHasSkill(bot, SKILL_SKINNING);
        case 40892:  // Hammer Pick
            return BotHasSkill(bot, SKILL_MINING) || BotHasSkill(bot, SKILL_BLACKSMITHING);
        case 40893:  // Bladed Pickaxe
            return BotHasSkill(bot, SKILL_MINING) || BotHasSkill(bot, SKILL_SKINNING);
        case 5956:  // Blacksmith Hammer
            return BotHasSkill(bot, SKILL_BLACKSMITHING) || BotHasSkill(bot, SKILL_ENGINEERING);
        case 6219:  // Arclight Spanner
            return BotHasSkill(bot, SKILL_ENGINEERING);
        case 6218:  // Runed copper rod
            return BotHasSkill(bot, SKILL_ENCHANTING);
        case 6339:  // Runed silver rod
            return BotHasSkill(bot, SKILL_ENCHANTING);
        case 11130:  // Runed golden rod
            return BotHasSkill(bot, SKILL_ENCHANTING);
        case 11145:  // Runed truesilver rod
            return BotHasSkill(bot, SKILL_ENCHANTING);
        case 16207:  // Runed Arcanite Rod
            return BotHasSkill(bot, SKILL_ENCHANTING);
        case 7005:  // Skinning Knife
            return BotHasSkill(bot, SKILL_SKINNING);
        case 12709:
            return BotHasSkill(bot, SKILL_SKINNING);
        case 19901:
            return BotHasSkill(bot, SKILL_SKINNING);
        case 4471:  // Flint and Tinder
            return BotHasSkill(bot, SKILL_COOKING);
        case 4470:  // Simple Wood
            return BotHasSkill(bot, SKILL_COOKING);
        case 6256:  // Fishing Rod
            return BotHasSkill(bot, SKILL_FISHING);
    }

    return false;
}

bool ItemUsageValue::IsItemUsefulForSkill(ItemTemplate const* proto)
{
    switch (proto->Class)
    {
        case ITEM_CLASS_TRADE_GOODS:
        case ITEM_CLASS_MISCELLANEOUS:
        case ITEM_CLASS_REAGENT:
        case ITEM_CLASS_GEM:
        {
            // 5.4.8: RandomItemMgr::IsUsedBySkill(proto, skill) (a reagent/recipe-usage lookup table)
            // has no equivalent on this fork - see file header. This branch is dropped/always false
            // until such a table exists; flagged as an unported AC feature.
            return false;
        }
        case ITEM_CLASS_RECIPE:
        {
            if (bot->HasSpell(proto->Spells[2].SpellId))
                break;

            switch (proto->SubClass)
            {
                case ITEM_SUBCLASS_LEATHERWORKING_PATTERN:
                    return BotHasSkill(bot, SKILL_LEATHERWORKING);
                case ITEM_SUBCLASS_TAILORING_PATTERN:
                    return BotHasSkill(bot, SKILL_TAILORING);
                case ITEM_SUBCLASS_ENGINEERING_SCHEMATIC:
                    return BotHasSkill(bot, SKILL_ENGINEERING);
                case ITEM_SUBCLASS_BLACKSMITHING:
                    return BotHasSkill(bot, SKILL_BLACKSMITHING);
                case ITEM_SUBCLASS_COOKING_RECIPE:
                    return BotHasSkill(bot, SKILL_COOKING);
                case ITEM_SUBCLASS_ALCHEMY_RECIPE:
                    return BotHasSkill(bot, SKILL_ALCHEMY);
                case ITEM_SUBCLASS_FIRST_AID_MANUAL:
                    return BotHasSkill(bot, SKILL_FIRST_AID);
                case ITEM_SUBCLASS_ENCHANTING_FORMULA:
                    return BotHasSkill(bot, SKILL_ENCHANTING);
                case ITEM_SUBCLASS_FISHING_MANUAL:
                    return BotHasSkill(bot, SKILL_FISHING);
            }
        }
    }

    return false;
}

namespace
{
    // Crafting chains can be cyclic: Remember which items are already being evaluated
    // further up the stack and stop as soon as a cycle closes.
    thread_local std::unordered_set<uint32> itemsBeingEvaluated;

    class UsefullSpellRecursionGuard
    {
    public:
        explicit UsefullSpellRecursionGuard(uint32 itemId)
            : _itemId(itemId), _entered(itemsBeingEvaluated.insert(itemId).second)
        {
        }

        ~UsefullSpellRecursionGuard()
        {
            if (_entered)
                itemsBeingEvaluated.erase(_itemId);
        }

        UsefullSpellRecursionGuard(UsefullSpellRecursionGuard const&) = delete;
        UsefullSpellRecursionGuard& operator=(UsefullSpellRecursionGuard const&) = delete;

        bool Entered() const { return _entered; }

    private:
        uint32 _itemId;
        bool _entered;
    };
}

bool ItemUsageValue::IsItemNeededForUsefullSpell(ItemTemplate const* proto, bool checkAllReagents)
{
    UsefullSpellRecursionGuard guard(proto->ItemId);
    if (!guard.Entered())
        return false;

    for (auto spellId : SpellsUsingItem(proto->ItemId, bot))
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (checkAllReagents && !HasItemsNeededForSpell(spellId, proto))
            continue;

        if (SpellGivesSkillUp(spellId, bot))
            return true;

        uint32 newItemId = spellInfo->Effects[EFFECT_0].ItemType;
        if (newItemId && newItemId != proto->ItemId)
        {
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", newItemId);

            if (usage != ITEM_USAGE_REPLACE && usage != ITEM_USAGE_EQUIP && usage != ITEM_USAGE_AMMO &&
                usage != ITEM_USAGE_QUEST && usage != ITEM_USAGE_SKILL && usage != ITEM_USAGE_USE)
                continue;

            return true;
        }
    }

    return false;
}

bool ItemUsageValue::HasItemsNeededForSpell(uint32 spellId, ItemTemplate const* proto)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
        if (spellInfo->ReagentCount[i] > 0 && spellInfo->Reagent[i])
        {
            if (proto && proto->ItemId == uint32(spellInfo->Reagent[i]) &&
                spellInfo->ReagentCount[i] == 1)  // If we only need 1 item then current item does not need to be
                                                  // checked since we are looting/buying or already have it.
                continue;

            ItemTemplate const* reqProto = sObjectMgr->GetItemTemplate(spellInfo->Reagent[i]);
            if (!reqProto)
                continue;

            uint32 count = AI_VALUE2(uint32, "item count", std::to_string(reqProto->ItemId));

            if (count < uint32(spellInfo->ReagentCount[i]))
                return false;
        }

    return true;
}

Item* ItemUsageValue::CurrentItem(ItemTemplate const* proto)
{
    Item* bestItem = nullptr;
    // 5.4.8: chat->FormatItem(proto) -> std::to_string(proto->ItemId), see file header.
    std::vector<Item*> found = AI_VALUE2(std::vector<Item*>, "inventory items", std::to_string(proto->ItemId));
    for (auto item : found)
    {
        if (bestItem && item->GetUInt32Value(ITEM_FIELD_DURABILITY) < bestItem->GetUInt32Value(ITEM_FIELD_DURABILITY))
            continue;

        if (bestItem && item->GetCount() < bestItem->GetCount())
            continue;

        bestItem = item;
    }

    return bestItem;
}

float ItemUsageValue::CurrentStacks(ItemTemplate const* proto)
{
    uint32 maxStack = proto->GetMaxStackSize();

    // 5.4.8: chat->FormatItem(proto) -> std::to_string(proto->ItemId), see file header.
    std::vector<Item*> found = AI_VALUE2(std::vector<Item*>, "inventory items", std::to_string(proto->ItemId));

    float itemCount = 0;

    for (auto stack : found)
    {
        itemCount += stack->GetCount();
    }

    return itemCount / maxStack;
}

float ItemUsageValue::BetterStacks(ItemTemplate const* proto, std::string const itemType)
{
    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", itemType);

    float stacks = 0;

    for (auto& otherItem : items)
    {
        ItemTemplate const* otherProto = otherItem->GetTemplate();

        if (otherProto->Class != proto->Class || otherProto->SubClass != proto->SubClass)
            continue;

        if (otherProto->ItemLevel < proto->ItemLevel)
            continue;

        if (otherProto->ItemId == proto->ItemId)
            continue;

        stacks += CurrentStacks(otherProto);
    }

    return stacks;
}

std::vector<uint32> ItemUsageValue::SpellsUsingItem(uint32 itemId, Player* bot)
{
    std::vector<uint32> retSpells;

    PlayerSpellMap const& spellMap = bot->GetSpellMap();

    for (auto& spell : spellMap)
    {
        uint32 spellId = spell.first;

        // 5.4.8: PlayerSpell's fields are lowercase (state/active) on this fork, see file header.
        if (spell.second->state == PLAYERSPELL_REMOVED || !spell.second->active)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        if (spellInfo->IsPassive())
            continue;

        if (spellInfo->Effects[EFFECT_0].Effect != SPELL_EFFECT_CREATE_ITEM)
            continue;

        for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
            if (spellInfo->ReagentCount[i] > 0 && uint32(spellInfo->Reagent[i]) == itemId)
                retSpells.push_back(spellId);
    }

    return retSpells;
}

inline int32 SkillGainChance(uint32 SkillValue, uint32 GrayLevel, uint32 GreenLevel, uint32 YellowLevel)
{
    if (SkillValue >= GrayLevel)
        return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_GREY) * 10;

    if (SkillValue >= GreenLevel)
        return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_GREEN) * 10;

    if (SkillValue >= YellowLevel)
        return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_YELLOW) * 10;

    return sWorld->getIntConfig(CONFIG_SKILL_CHANCE_ORANGE) * 10;
}

bool ItemUsageValue::SpellGivesSkillUp(uint32 spellId, Player* bot)
{
    SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);

    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        SkillLineAbilityEntry const* skill = _spell_idx->second;
        if (skill->SkillLine)
        {
            uint32 SkillValue = bot->GetPureSkillValue(skill->SkillLine);

            if (SkillGainChance(SkillValue, skill->max_value,
                                (skill->max_value + skill->min_value) / 2,
                                skill->min_value) > 0)
                return true;
        }
    }

    return false;
}

std::string const ItemUsageValue::GetConsumableType(ItemTemplate const* proto, bool hasMana)
{
    std::string const foodType = "";

    if ((proto->SubClass == ITEM_SUBCLASS_CONSUMABLE || proto->SubClass == ITEM_SUBCLASS_FOOD_DRINK))
    {
        if (proto->Spells[0].SpellCategory == 11)
            return "food";
        else if (proto->Spells[0].SpellCategory == 59 && hasMana)
            return "drink";
    }

    if (proto->SubClass == ITEM_SUBCLASS_POTION || proto->SubClass == ITEM_SUBCLASS_FLASK)
    {
        for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
        {
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[j].SpellId);
            if (spellInfo)
                for (int i = 0; i < 3; i++)
                {
                    if (spellInfo->Effects[i].Effect == SPELL_EFFECT_ENERGIZE && hasMana)
                        return "mana potion";

                    if (spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL)
                        return "healing potion";
                }
        }
    }

    if (proto->SubClass == ITEM_SUBCLASS_BANDAGE)
    {
        return "bandage";
    }

    return "";
}

ItemUsage ItemUpgradeValue::Calculate()
{
    ParsedItemUsage parsed = GetItemIdFromQualifier();
    uint32 itemId = parsed.itemId;
    uint32 randomPropertyId = parsed.randomPropertyId;
    if (!itemId)
        return ITEM_USAGE_NONE;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return ITEM_USAGE_NONE;

    ItemUsage equip = QueryItemUsageForEquip(proto, randomPropertyId);
    if (equip != ITEM_USAGE_NONE)
        return equip;

    if (proto->Class == ITEM_CLASS_PROJECTILE && bot->CanUseItem(proto) == EQUIP_ERR_OK)
        return QueryItemUsageForAmmo(proto);

    return ITEM_USAGE_NONE;
}
