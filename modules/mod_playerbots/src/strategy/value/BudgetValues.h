/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Value/BudgetValues.h for issue #332.
 *
 * AC's gold-budget system. Before this port the fork had none of it: RepairCostValue and
 * FreeMoneyForValue were hard-coded `return 0;` stubs in MaintenanceValues.cpp, so every consumer
 * of "free money for" (BuyAction, TrainerAction) either rejected everything or had to fall back to
 * the bot's full purse through a file-local helper. This file is AC's own file split, verbatim.
 *
 * 5.4.8 / this-fork adaptations vs AC (all are in BudgetValues.cpp next to the code they affect):
 *   - botAI->HasCheat(BotCheatMask::gold): no bot-cheat-mask system on this fork (AI/PlayerbotAI.h:319
 *     has "cheatMask" commented out), same gap already recorded in BuyAction.cpp / TrainerAction.cpp /
 *     RepairAllAction.cpp. The cheat branch is omitted; without the cheat every bot takes the budget
 *     path, which is what AC does for non-cheating bots anyway.
 *   - IsRealPlayer(botAI->GetMaster()) -> botAI->HasRealPlayerMaster() (AC Bot/PlayerbotAI.cpp:4586-4592:
 *     `player && !GET_PLAYERBOT_AI(player)`; the fork's HasRealPlayerMaster() has the same shape).
 *   - AC's Trainer class does not exist on this core; TrainCostValue walks
 *     sObjectMgr->GetNpcTrainerSpells() / TrainerSpellData::spellList / bot->GetTrainerSpellState().
 *   - chat->FormatQItem(5976)/(5863) -> std::to_string(id), the fork's plain-numeric item-id
 *     qualifier resolved by InventoryAction::Find (#188).
 */

#ifndef _PLAYERBOT_BUDGETVALUES_H
#define _PLAYERBOT_BUDGETVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;

enum class NeedMoneyFor : uint32
{
    none = 0,
    repair = 1,
    ammo = 2,
    spells = 3,
    travel = 4,
    consumables = 5,
    gear = 6,
    guild = 7,
    tradeskill = 8,
    anything = 9
};

class MaxGearRepairCostValue : public Uint32CalculatedValue
{
public:
    MaxGearRepairCostValue(PlayerbotAI* botAI) : Uint32CalculatedValue(botAI, "max repair cost", 60) {}

    uint32 Calculate() override;
};

class RepairCostValue : public Uint32CalculatedValue
{
public:
    RepairCostValue(PlayerbotAI* botAI) : Uint32CalculatedValue(botAI, "repair cost", 60) {}

    uint32 Calculate() override;
};

class TrainCostValue : public Uint32CalculatedValue
{
public:
    TrainCostValue(PlayerbotAI* botAI) : Uint32CalculatedValue(botAI, "train cost", 60) {}

    uint32 Calculate() override;
};

class MoneyNeededForValue : public Uint32CalculatedValue, public Qualified
{
public:
    MoneyNeededForValue(PlayerbotAI* botAI) : Uint32CalculatedValue(botAI, "money needed for", 60) {}

    uint32 Calculate() override;
};

class TotalMoneyNeededForValue : public Uint32CalculatedValue, public Qualified
{
public:
    TotalMoneyNeededForValue(PlayerbotAI* botAI) : Uint32CalculatedValue(botAI, "total money needed for", 60) {}

    uint32 Calculate() override;

private:
    std::vector<NeedMoneyFor> saveMoneyFor = {NeedMoneyFor::guild, NeedMoneyFor::repair, NeedMoneyFor::ammo,
                                              NeedMoneyFor::spells, NeedMoneyFor::travel};
};

class FreeMoneyForValue : public Uint32CalculatedValue, public Qualified
{
public:
    FreeMoneyForValue(PlayerbotAI* botAI) : Uint32CalculatedValue(botAI, "free money for") {}

    uint32 Calculate() override;
};

class ShouldGetMoneyValue : public BoolCalculatedValue
{
public:
    ShouldGetMoneyValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should get money", 2) {}

    bool Calculate() override;
};

#endif
