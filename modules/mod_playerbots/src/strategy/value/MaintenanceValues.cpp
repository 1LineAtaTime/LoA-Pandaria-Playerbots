/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MaintenanceValues.h"

// #332: the budget values (and with them the NeedMoneyFor enum) moved out of this file into AC's own
// BudgetValues.{h,cpp} split; CanRepairValue::Calculate() below needs both.
#include "BudgetValues.h"
#include "ItemVisitors.h"
#include "Playerbots.h"

bool CanMoveAroundValue::Calculate()
{
    if (bot->IsInCombat())
        return false;

    if (bot->GetTradeData())
        return false;

    if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT))
        return false;

    if (!AI_VALUE(bool, "group ready"))
        return false;

    return true;
}

// #272 (AC MaintenanceValues.cpp:29): body restored verbatim; it has no reader on either tree. Needs the
// "home bind" value (HomeBindValue, TargetValue.h) which the lead registers.
bool ShouldHomeBindValue::Calculate() { return AI_VALUE2(float, "distance", "home bind") > 1000.0f; }

// #611 item 6 (AC MaintenanceValues.cpp:28): live body restored.
bool ShouldRepairValue::Calculate() { return AI_VALUE(uint8, "durability") < 80; }

// #332: AC MaintenanceValues.cpp:33-37 verbatim. This was stubbed to false by #149 because both of its
// inputs were stubs; BudgetValues.{h,cpp} now carries the real "repair cost" and "free money for"
// (and NeedMoneyFor with them), so the body is live. "durability" was already registered
// (ValueContext.h:101).
bool CanRepairValue::Calculate()
{
    return AI_VALUE(uint8, "durability") < 100 &&
           AI_VALUE(uint32, "repair cost") < AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::repair);
}

// #258 (AC MaintenanceValues.cpp:50,52-56): bodies restored verbatim. Both names were already
// registered in ValueContext.h before this pass; only the bodies were stubbed to false, which is
// why no bot ever emptied its bags. "bag space" (ValueContext.h) and "item count" (ValueContext.h)
// are both registered.
// CAVEAT: AC's CanSellValue needs InventoryAction::Find to understand a "usage <n>" qualifier.
// This fork's Find (InventoryAction.cpp:52-119) has no such branch - "usage 12" falls through
// to the bare-name FindNamedItemVisitor and matches nothing - so on this fork the sum is 0 and
// "can sell" is false until that branch is ported. Reported as a #258 gap, not worked around.
bool ShouldSellValue::Calculate()
{
    return AI_VALUE(uint8, "bag space") > 80;
}

bool CanSellValue::Calculate()
{
    return (AI_VALUE2(uint32, "item count", "usage " + std::to_string(ITEM_USAGE_VENDOR)) +
            AI_VALUE2(uint32, "item count", "usage " + std::to_string(ITEM_USAGE_AH))) > 1;
}

bool CanFightEqualValue::Calculate()
{
    return AI_VALUE(uint8, "durability") > 20;
}

// #611 item 6 (AC MaintenanceValues.cpp:54-58, 60-65): live bodies restored.
bool CanFightEliteValue::Calculate()
{
    return bot->GetGroup() && AI_VALUE2(bool, "group and", "can fight equal") &&
           AI_VALUE2(bool, "group and", "following party") && !AI_VALUE2(bool, "group or", "should sell,can sell");
}

bool CanFightBossValue::Calculate()
{
    return bot->GetGroup() && bot->GetGroup()->GetMembersCount() > 3 &&
           AI_VALUE2(bool, "group and", "can fight equal") && AI_VALUE2(bool, "group and", "following party") &&
           !AI_VALUE2(bool, "group or", "should sell,can sell");
}

// #332 (AC MaintenanceValues.cpp:47-50): CanTrainValue was missing from this fork entirely.
bool CanTrainValue::Calculate() { return AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::spells) > 0; }
