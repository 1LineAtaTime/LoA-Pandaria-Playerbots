/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/StatsAction.cpp. 5.4.8 differences, each noted inline: the
 * update-field names (PLAYER_FIELD_XP / PLAYER_FIELD_NEXT_LEVEL_XP / PLAYER_FIELD_REST_STATE_BONUS_POOL /
 * ITEM_FIELD_MAX_DURABILITY), the TC_LOG_ERROR call style, and a divide-by-zero guard in ListRepairCost.
 */

#include "StatsAction.h"

// DBCStores.h FIRST: it pulls DBCStructure.h in, and ItemPrototype.h (via Creature.h) is not
// self-contained without it (same ordering as BudgetValues.cpp).
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Playerbots.h"

bool StatsAction::Execute(Event /*event*/)
{
    std::ostringstream out;

    ListGold(out);

    out << ", ";
    ListBagSlots(out);

    out << ", ";
    ListRepairCost(out);

    if (bot->GetUInt32Value(PLAYER_FIELD_NEXT_LEVEL_XP))
    {
        out << ", ";
        ListXP(out);
    }

    botAI->TellMaster(out);
    return true;
}

void StatsAction::ListGold(std::ostringstream& out) { out << chat->formatMoney(bot->GetMoney()); }

void StatsAction::ListBagSlots(std::ostringstream& out)
{
    uint32 totalused = 0, total = 16;

    // list out items in main backpack
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
    {
        if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            ++totalused;
        }
    }

    uint32 totalfree = 16 - totalused;

    // list out items in other removable backpacks
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            ItemTemplate const* pBagProto = pBag->GetTemplate();
            if (pBagProto->Class == ITEM_CLASS_CONTAINER && pBagProto->SubClass == ITEM_SUBCLASS_CONTAINER)
            {
                total += pBag->GetBagSize();
                totalfree += pBag->GetFreeSlots();
            }
        }
    }

    std::string color = "ff00ff00";
    if (totalfree < total / 2)
        color = "ffffff00";

    if (totalfree < total / 4)
        color = "ffff0000";

    out << "|h|c" << color << totalfree << "/" << total << "|h|cffffffff Bag";
}

void StatsAction::ListXP(std::ostringstream& out)
{
    uint32 curXP = bot->GetUInt32Value(PLAYER_FIELD_XP);
    uint32 nextLevelXP = bot->GetUInt32Value(PLAYER_FIELD_NEXT_LEVEL_XP);
    uint32 restXP = bot->GetUInt32Value(PLAYER_FIELD_REST_STATE_BONUS_POOL);
    uint32 xpPercent = 0;

    if (nextLevelXP)
        xpPercent = 100 * curXP / nextLevelXP;

    uint32 restPercent = 0;
    if (restXP && nextLevelXP)
        restPercent = 2 * (100 * restXP / nextLevelXP);

    out << "|cff00ff00" << xpPercent << "|cffffd333/|cff00ff00" << restPercent << "%|cffffffff XP";
}

void StatsAction::ListRepairCost(std::ostringstream& out)
{
    uint32 totalCost = 0;
    double repairPercent = 0;
    double repairCount = 0;

    for (uint32 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        uint16 pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        totalCost += EstRepair(pos);
        double repair = RepairPercent(pos);
        if (repair < 100)
        {
            repairPercent += repair;
            ++repairCount;
        }
    }

    // AC divides unconditionally, which is NaN when nothing is damaged (and a UB cast to uint32 below). With
    // nothing damaged the durability is, by the same definition, 100%.
    if (repairCount > 0)
        repairPercent /= repairCount;
    else
        repairPercent = 100;

    std::string color = "ff00ff00";
    if (repairPercent < 50)
        color = "ffffff00";

    if (repairPercent < 25)
        color = "ffff0000";

    out << "|c" << color << (uint32)ceil(repairPercent) << "% (" << chat->formatMoney(totalCost) << ")|cffffffff Dur";
}

uint32 StatsAction::EstRepair(uint16 pos)
{
    Item* item = bot->GetItemByPos(pos);

    uint32 TotalCost = 0;
    if (!item)
        return TotalCost;

    uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAX_DURABILITY);
    if (!maxDurability)
        return TotalCost;

    uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

    uint32 LostDurability = maxDurability - curDurability;
    if (LostDurability > 0)
    {
        ItemTemplate const* ditemProto = item->GetTemplate();

        DurabilityCostsEntry const* dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
        if (!dcost)
        {
            TC_LOG_ERROR("playerbots", "RepairDurability: Wrong item lvl %u", ditemProto->ItemLevel);
            return TotalCost;
        }

        uint32 dQualitymodEntryId = (ditemProto->Quality + 1) * 2;
        DurabilityQualityEntry const* dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
        if (!dQualitymodEntry)
        {
            TC_LOG_ERROR("playerbots", "RepairDurability: Wrong dQualityModEntry %u", dQualitymodEntryId);
            return TotalCost;
        }

        uint32 dmultiplier =
            dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass)];
        uint32 costs = uint32(LostDurability * dmultiplier * double(dQualitymodEntry->quality_mod));

        if (!costs)  // fix for ITEM_QUALITY_ARTIFACT
            costs = 1;

        TotalCost = costs;
    }

    return TotalCost;
}

double StatsAction::RepairPercent(uint16 pos)
{
    Item* item = bot->GetItemByPos(pos);
    if (!item)
        return 100;

    uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAX_DURABILITY);
    if (!maxDurability)
        return 100;

    uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);
    if (!curDurability)
        return 0;

    return curDurability * 100.0 / maxDurability;
}
