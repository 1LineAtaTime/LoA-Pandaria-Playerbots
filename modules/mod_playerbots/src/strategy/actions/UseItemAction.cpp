/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "UseItemAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ObjectDefines.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "SpellDefines.h"
#include "WorldPacket.h"

// Ported from AC mod-playerbots UseItemAction.cpp:15-42 (Execute), dropping the gameobject branch
// (chat->parseGameobjects) - none of #149's actions target a gameobject.
bool UseItemAction::Execute(Event event)
{
    std::string name = event.getParam();
    if (name.empty())
        name = getName();

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", name);
    if (items.empty())
        return false;

    return UseItem(*items.begin());
}

// Self-only distillation of AC's UseItemAction::UseItem()/UseItemAuto() (UseItemAction.cpp:60-321):
// no gameobject/item/unit target, no trade-item or quest-start handling, no consumable
// eat/drink timing - just cast the item's first on-use spell at the bot itself. Mirrors the
// CMSG_USE_ITEM packet this module already builds for UseTrinketAction
// (GenericSpellActions.cpp:342-389).
bool UseItemAction::UseItem(Item* item)
{
    if (bot->CanUseItem(item) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCasted(false))
        return false;

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 cast_count = 1;
    ObjectGuid item_guid = item->GetGUID();
    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint32 spellId = 0;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetTemplate()->Spells[i].SpellId > 0)
        {
            spellId = item->GetTemplate()->Spells[i].SpellId;
            if (!botAI->CanCastSpell(spellId, bot, false))
                return false;
            break;
        }
    }

    if (!spellId)
        return false;

    // 5.4.8: CMSG_USE_ITEM is bit-packed on this core. The WotLK-shaped packet AC builds here made
    // HandleUseItemOpcode read past the end and abort the process (#194 S8, once "use trinket" and
    // "potions" started firing). Do what the handler does after parsing the packet instead.
    if (item->GetTemplate()->Bonding == BIND_ON_USE && !item->IsSoulBound())
    {
        item->SetState(ITEM_CHANGED, bot);
        item->SetBinding(true);
    }
    SpellCastTargets targets;
    targets.SetUnitTarget(bot);
    if (!sScriptMgr->OnItemUse(bot, item, targets))
        bot->CastItemUseSpell(item, targets, cast_count, glyphIndex);
    return true;
}

// Ported from AC mod-playerbots UseItemAction.cpp:396: AC also allows a bare "use" without a
// qualifier; this fork never registers that name, so it is dropped.
bool UseItemAction::isPossible() { return AI_VALUE2(uint32, "item count", getName()) > 0; }

// Ported from AC mod-playerbots UseItemAction.cpp:400,402, unchanged.
bool UseHealingPotion::isUseful() { return AI_VALUE2(bool, "combat", "self target"); }

bool UseManaPotion::isUseful() { return AI_VALUE2(bool, "combat", "self target"); }

// #257: AC UseItemAction.cpp:455-491. AC's UseItem() accepts the quest through
// HandleQuestgiverAcceptQuestOpcode with the item as giver (AC UseItemAction.cpp:171-184); this fork's
// UseItem() is self-spell only, so the same accept packet is built here in the 5.4.8 bit-packed layout
// (the reader is WorldSession::HandleQuestgiverAcceptQuestOpcode, QuestHandler.cpp:148-170).
bool UseRandomQuestItem::Execute(Event /*event*/)
{
    std::vector<Item*> questItems = AI_VALUE2(std::vector<Item*>, "inventory items", "quest");
    if (questItems.empty())
        return false;

    Item* item = nullptr;
    Quest const* qInfo = nullptr;
    for (uint8 i = 0; i < 5; i++)
    {
        auto itr = questItems.begin();
        std::advance(itr, urand(0, questItems.size() - 1));
        Item* questItem = *itr;

        ItemTemplate const* proto = questItem->GetTemplate();
        if (proto->StartQuest)
        {
            Quest const* quest = sObjectMgr->GetQuestTemplate(proto->StartQuest);
            if (quest && bot->CanTakeQuest(quest, false))
            {
                item = questItem;
                qInfo = quest;
                break;
            }
        }
    }

    if (!item)
        return false;

    ObjectGuid const guid = item->GetGUID();
    WorldPacket packet(CMSG_QUESTGIVER_ACCEPT_QUEST);
    packet << uint32(qInfo->GetQuestId());
    packet.WriteBit(guid[6]);
    packet.WriteBit(guid[0]);
    packet.WriteBit(false);  // startCheat
    packet.WriteBit(guid[2]);
    packet.WriteBit(guid[7]);
    packet.WriteBit(guid[5]);
    packet.WriteBit(guid[4]);
    packet.WriteBit(guid[3]);
    packet.WriteBit(guid[1]);
    packet.FlushBits();
    packet.WriteByteSeq(guid[5]);
    packet.WriteByteSeq(guid[4]);
    packet.WriteByteSeq(guid[0]);
    packet.WriteByteSeq(guid[1]);
    packet.WriteByteSeq(guid[6]);
    packet.WriteByteSeq(guid[2]);
    packet.WriteByteSeq(guid[3]);
    packet.WriteByteSeq(guid[7]);
    packet.rpos(0);
    bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(packet);

    botAI->TellMasterNoFacing("Got quest " + ChatHelper::FormatQuest(qInfo));
    botAI->SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
    return true;
}

bool UseRandomQuestItem::isUseful()
{
    return !botAI->HasRealPlayerMaster() && !bot->InBattleground() && !bot->HasUnitState(UNIT_STATE_IN_FLIGHT);
}

bool UseRandomQuestItem::isPossible() { return AI_VALUE2(uint32, "item count", "quest") > 0; }
