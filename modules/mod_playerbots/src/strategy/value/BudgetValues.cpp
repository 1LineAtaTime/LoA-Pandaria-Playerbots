/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Value/BudgetValues.cpp for issue #332. See BudgetValues.h
 * for the full list of 5.4.8 divergences; the ones that matter inside a function body are repeated
 * at the call site.
 */

#include "BudgetValues.h"

// DBCStores.h FIRST: it pulls DBCStructure.h in, and ItemPrototype.h (via Creature.h) is not
// self-contained without it - DurabilityCostsEntry / DurabilityQualityEntry live in DBCStructure.h
// and ItemSubClassToDurabilityMultiplierId() is an inline in ItemPrototype.h.
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Playerbots.h"

#include <algorithm>
#include <unordered_set>

uint32 MaxGearRepairCostValue::Calculate()
{
    uint32 totalCost = 0;

    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        uint16 pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        Item* item = bot->GetItemByPos(pos);

        if (!item)
            continue;

        uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAX_DURABILITY);
        if (!maxDurability)
            continue;

        uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

        if (uint32(i) >= EQUIPMENT_SLOT_END && curDurability >= maxDurability)  // Only count items equiped or already damanged.
            continue;

        ItemTemplate const* ditemProto = item->GetTemplate();

        DurabilityCostsEntry const* dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
        if (!dcost)
            continue;

        uint32 dQualitymodEntryId = (ditemProto->Quality + 1) * 2;
        DurabilityQualityEntry const* dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
        if (!dQualitymodEntry)
            continue;

        uint32 dmultiplier =
            dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass)];

        uint32 costs = uint32(maxDurability * dmultiplier * double(dQualitymodEntry->quality_mod));

        totalCost += costs;
    }

    return totalCost;
}

uint32 RepairCostValue::Calculate()
{
    uint32 totalCost = 0;

    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        uint16 pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        Item* item = bot->GetItemByPos(pos);

        if (!item)
            continue;

        uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAX_DURABILITY);
        if (!maxDurability)
            continue;

        uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

        uint32 LostDurability = maxDurability - curDurability;

        if (LostDurability == 0)
            continue;

        ItemTemplate const* ditemProto = item->GetTemplate();

        DurabilityCostsEntry const* dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
        if (!dcost)
            continue;

        uint32 dQualitymodEntryId = (ditemProto->Quality + 1) * 2;
        DurabilityQualityEntry const* dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
        if (!dQualitymodEntry)
            continue;

        uint32 dmultiplier =
            dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass)];
        uint32 costs = uint32(LostDurability * dmultiplier * double(dQualitymodEntry->quality_mod));

        totalCost += costs;
    }

    return totalCost;
}

// AC Ai/Base/Value/BudgetValues.cpp:97-134. 5.4.8: this core has no AC Trainer class
// (src/server/game/Entities/Creature/Trainer.h does not exist). The equivalent data is
// TrainerSpellData::spellList, a TrainerSpellMap = unordered_map<spellid, TrainerSpell>
// (Creature.h:223-234), fetched with sObjectMgr->GetNpcTrainerSpells(entry)
// (Globals/ObjectMgr.h:1640). The three differences are all noted inline.
uint32 TrainCostValue::Calculate()
{
    uint32 totalCost = 0;

    std::unordered_set<uint32> spells;

    // AC's Trainer::Type::Class == 0 (AC src/server/game/Entities/Creature/Trainer.h:33); the same
    // numbering is carried by TrainerSpellData::trainerType on this core, exactly as
    // TrainerAction.cpp:162-164 records. NPCFLAG_TRAINER lives in CreatureTemplate::npcflag
    // (Entities/Creature/CreatureData.h:172), same field AC iterates.
    uint32 const classTrainer = 0;

    CreatureTemplateContainer const* ctc = sObjectMgr->GetCreatureTemplates();
    for (CreatureTemplateContainer::const_iterator itr = ctc->begin(); itr != ctc->end(); ++itr)
    {
        if (!(itr->second.npcflag & UNIT_NPC_FLAG_TRAINER))
            continue;

        TrainerSpellData const* trainer = sObjectMgr->GetNpcTrainerSpells(itr->first);
        if (!trainer)
            continue;

        // AC's `trainer->GetTrainerType() != Trainer::Type::Class || !trainer->IsTrainerValidForPlayer(bot)`.
        // IsTrainerValidForPlayer() has no counterpart on this core; TrainerAction.cpp:216-227
        // establishes GetTrainerSpellState(...) == TRAINER_SPELL_GREEN as this fork's equivalent of
        // AC's CanTeachSpell/IsTrainerSpellAllowedForBot pair.
        if (trainer->trainerType != classTrainer)
            continue;

        // AC walks trainer->GetSpells() and re-looks-up each SpellId via GetSpell(); the map is
        // already keyed by spell id and yields the TrainerSpell directly, so one iteration replaces
        // both (same shape as the sweep at BotFactory.cpp:511). Iteration order is unordered, as in AC.
        for (TrainerSpellMap::const_iterator spell = trainer->spellList.begin(); spell != trainer->spellList.end();
             ++spell)
        {
            TrainerSpell const* trainerSpell = &spell->second;

            if (bot->GetTrainerSpellState(trainerSpell) != TRAINER_SPELL_GREEN)
                continue;

            if (spells.find(trainerSpell->spell) != spells.end())
                continue;

            totalCost += trainerSpell->spellCost;  // AC: trainerSpell->MoneyCost
            spells.insert(trainerSpell->spell);
        }
    }

    return totalCost;
}

uint32 MoneyNeededForValue::Calculate()
{
    NeedMoneyFor needMoneyFor = NeedMoneyFor(stoi(getQualifier()));

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);  // AC BudgetValues.cpp:140
    AiObjectContext* context = botAI->GetAiObjectContext();

    uint32 moneyWanted = 0;

    uint32 level = bot->GetLevel();

    switch (needMoneyFor)
    {
        case NeedMoneyFor::none:
            moneyWanted = 0;
            break;
        case NeedMoneyFor::repair:
            moneyWanted = AI_VALUE(uint32, "max repair cost");
            break;
        case NeedMoneyFor::ammo:
            moneyWanted = (bot->GetClass() == CLASS_HUNTER)
                              ? (level * level * level) / 10
                              : 0;  // Or level^3 (1s @ lvl10, 30s @ lvl30, 2g @ lvl60, 5g @ lvl80): Todo replace
                                    // (should be best ammo buyable x 8 stacks cost)
            break;
        case NeedMoneyFor::spells:
            moneyWanted = AI_VALUE(uint32, "train cost");
            break;
        case NeedMoneyFor::travel:
            moneyWanted =
                bot->isTaxiCheater()
                    ? 0
                    : 1500;  // 15s for traveling half a continent. Todo: Add better calculation (Should be ???)
            break;
        case NeedMoneyFor::gear:
            moneyWanted = level * level * level;  // Or level^3 (10s @ lvl10, 3g @ lvl30, 20g @ lvl60, 50g @ lvl80):
                                                  // Todo replace (Should be ~total cost of all >green gear equiped)
            break;
        case NeedMoneyFor::consumables:
            moneyWanted =
                (level * level * level) / 10;  // Or level^3 (1s @ lvl10, 30s @ lvl30, 2g @ lvl60, 5g @ lvl80): Todo
                                               // replace (Should be best food/drink x 2 stacks cost)
            break;
        case NeedMoneyFor::guild:
            if (botAI->HasStrategy("guild", BOT_STATE_NON_COMBAT))
            {
                // AC BudgetValues.cpp:183,186 use chat->FormatQItem(5976) / (5863). This fork's
                // ChatHelper has no FormatQItem and must not grow one (ChatHelper.h is included by
                // PlayerbotAI.h); its "item count" qualifier resolves a plain numeric item id through
                // InventoryAction::Find's FindItemByIdVisitor branch (#188), which counts the same
                // items. The literal ids and both amounts are AC's.
                if (bot->GetGuildId())
                    moneyWanted = AI_VALUE2(uint32, "item count", std::to_string(5976)) ? 0 : 10000;  // 1g (tabard)
                else
                    moneyWanted =
                        AI_VALUE2(uint32, "item count", std::to_string(5863)) ? 0 : 10000;  // 10s (guild charter)
            }
            break;
        case NeedMoneyFor::tradeskill:
            moneyWanted = (level * level *
                           level);  // Or level^3 (10s @ lvl10, 3g @ lvl30, 20g @ lvl60, 50g @ lvl80): Todo replace
                                    // (Should be buyable reagents that combined allow crafting of usefull items)
            break;
        default:
            break;
    }

    return moneyWanted;
};

uint32 TotalMoneyNeededForValue::Calculate()
{
    NeedMoneyFor needMoneyFor = NeedMoneyFor(stoi(getQualifier()));

    uint32 moneyWanted = AI_VALUE2(uint32, "money needed for", (uint32)needMoneyFor);

    auto needPtr = std::find(saveMoneyFor.begin(), saveMoneyFor.end(), needMoneyFor);

    while (needPtr != saveMoneyFor.begin())
    {
        needPtr--;

        NeedMoneyFor alsoNeed = *needPtr;

        moneyWanted = moneyWanted + AI_VALUE2(uint32, "money needed for", (uint32)alsoNeed);
    }

    return moneyWanted;
}

uint32 FreeMoneyForValue::Calculate()
{
    uint32 money = bot->GetMoney();

    // AC BudgetValues.cpp:225-226 `if (botAI->HasCheat(BotCheatMask::gold)) return 10000000;`. The
    // bot-cheat-mask system does not exist on this fork (AI/PlayerbotAI.h:319 has "cheatMask"
    // commented out), the same gap BuyAction.cpp:25-30, TrainerAction.cpp:67-71 and
    // RepairAllAction.cpp:10-15 already record. Omitted, not stubbed: without the cheat every bot
    // takes the budget path below, which is AC's behaviour for a non-cheating bot.
    //
    // AC BudgetValues.cpp:228-229 `if (IsRealPlayer(botAI->GetMaster())) return money;` ->
    // HasRealPlayerMaster(): AC's IsRealPlayer (Bot/PlayerbotAI.cpp:4586-4592) is
    // `player && !GET_PLAYERBOT_AI(player)`, and the fork's HasRealPlayerMaster() has the same
    // shape (false for a masterless bot).
    if (botAI->HasRealPlayerMaster())
        return money;

    uint32 savedMoney = AI_VALUE2(uint32, "total money needed for", getQualifier()) -
                        AI_VALUE2(uint32, "money needed for", getQualifier());

    if (savedMoney > money)
        return 0;

    return money - savedMoney;
};

bool ShouldGetMoneyValue::Calculate() { return !AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::anything); }
