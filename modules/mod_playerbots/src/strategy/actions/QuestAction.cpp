/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/QuestAction.cpp for issue #257 slice B (the
 * master-commanded quest layer).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - SMSG_QUESTUPDATE_ADD_KILL does not exist in 5.4.8; its successor is SMSG_QUESTUPDATE_ADD_CREDIT
 *    (Opcodes.h:994) and the layout is different, so QuestUpdateAddKillAction::Execute() is rewritten,
 *    not copied. Player::SendQuestUpdateAddCredit() (src/server/game/Entities/Player/Player.cpp:18302-18329)
 *    writes, in order: available(u16), objectiveType(u8), questId(u32), required(u16), objectId(u32), then 8
 *    guid mask bits followed by the masked guid bytes. AC instead read AC's WotLK layout
 *    "questId, entry, available, required" and inferred the NPC/GO case from `entry & 0x80000000`; here the
 *    objectiveType byte says it outright (QUEST_OBJECTIVE_MONSTER vs QUEST_OBJECTIVE_GAMEOBJECT), and
 *    objectId carries the entry.
 *  - AC's SMSG_QUESTUPDATE_COMPLETE is SMSG_QUEST_UPDATE_COMPLETE here (Opcodes.h:997, body = uint32 questId,
 *    QuestPackets.cpp:56), so QuestUpdateCompleteAction (parses the bare questId, tells the master, bumps
 *    GetRpgStatistic().questCompleted) is ported 1:1 below. AC's p.print_storage() debug dump is dropped.
 *  - AC's SMSG_QUESTITEM_PUSH_RESULT is SMSG_ITEM_PUSH_RESULT here (Opcodes.h:809) and its body is
 *    bit-masked + byte-seq interleaved (Player::SendNewItem(), Player.cpp:15378-15437), not the flat
 *    "itemId, count" AC read. QuestUpdateAddItemAction::Execute() therefore reads the packet in the exact
 *    order Player::SendNewItem() writes it and pulls itemId / totalInventoryCount out of it.
 *  - Quest data moved to objectives: AC walked the WotLK Quest::RequiredItemId[]/RequiredItemCount[] and
 *    Quest::RequiredNpcOrGo[]/RequiredNpcOrGoCount[] arrays, which do not exist in 5.4.8 - Quest now exposes
 *    GetObjectives() (QuestDef.h:376) returning QuestObjective{Type, ObjectID, Amount} records. Both loops in
 *    QuestAction::CompleteQuest() are rewritten over those, and they follow the shape the core itself uses for
 *    the same job in Player::IncompleteQuest(bool, uint32) (Player.cpp:16457-16506), which is the MoP-native
 *    equivalent of AC's item / creature / GO / reputation / money blocks.
 *  - AC's GetRepObjectiveFaction()/GetRepObjectiveValue() and GetRewOrReqMoney() have no 5.4.8 equivalent
 *    (the latter now takes a Player const*, QuestDef.h:377). The reputation requirement is read off the
 *    QUEST_OBJECTIVE_MIN_REPUTATION objective's ObjectID/Amount and the money requirement off the
 *    QUEST_OBJECTIVE_MONEY objective's Amount - see the Player.cpp:16492-16506 lines above.
 *  - bot->AbandonQuest(questId) -> bot->FailQuest(questId): Player::AbandonQuest() does not exist in 5.4.8;
 *    FailQuest() (Player.h:1719, Player.cpp:16843) is the function that clears the quest (failed status,
 *    QUEST_STATE_FAIL, timer) that AC's AbandonQuest() did.
 *  - botAI->GetLocalizedCreatureName()/GetLocalizedGameObjectName(): not exposed on this fork and
 *    PlayerbotAI.h is off-limits for this slice, so two file-local helpers read the template name
 *    (sObjectMgr->GetCreatureTemplate()->Name / sObjectMgr->GetGameObjectTemplate()->Name) - the same
 *    unlocalized access the fork already uses in Travel/TravelNode.cpp:1654 and strategy/Value.cpp:62.
 *  - botAI->GetInventoryItemsCountWithId(): not exposed on this fork; the file-local helper uses
 *    Player::GetItemCount(id, true), which is what AC's helper does internally.
 *  - botAI->GetCurrentQuestsRequiringItemId(): not exposed on this fork; the file-local helper rebuilds the
 *    same (Quest const*, requiredCount) list from the bot's quest log against the 5.4.8 objective data.
 *  - botAI->rpgStatistic.questCompleted -> botAI->GetRpgStatistic().questCompleted (PlayerbotAI.h:199).
 *  - BroadcastHelper (AC Bot/Broadcast/BroadcastHelper.h) is not ported on this fork (issue #322); every
 *    AC broadcast call below is replaced by a single marker comment at the same site.
 *  - PlayerbotTextMgr::instance().GetBotText(): not ported on this fork (same as every other chat/tell string
 *    ported so far) - plain default strings are used directly.
 *  - sPlayerbotAIConfig.x -> sPlayerbotAIConfig->x (pointer singleton on this fork).
 */

#include "QuestAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ReputationMgr.h"
#include "GameObject.h"
#include "GossipDef.h"
#include "DBCStructure.h"
#include "ItemPrototype.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Opcodes.h"      // lane Q2: ReadQuestEventGuid
#include "WorldPacket.h"  // lane Q2: ReadQuestEventGuid
#include "Playerbots.h"
#include "BotArc.h"
#include "BroadcastHelper.h"
#include "ReputationMgr.h"
#include <algorithm>
#include <sstream>

namespace
{
// 5.4.8: see the GetLocalizedCreatureName()/GetLocalizedGameObjectName() note in the file header.
std::string GetCreatureName(uint32 entry)
{
    if (CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(entry))
        return cInfo->Name;

    return std::string();
}

// 5.4.8: see the GetLocalizedCreatureName()/GetLocalizedGameObjectName() note in the file header.
std::string GetGameObjectName(uint32 entry)
{
    if (GameObjectTemplate const* info = sObjectMgr->GetGameObjectTemplate(entry))
        return info->name;

    return std::string();
}

// 5.4.8: see the GetInventoryItemsCountWithId() note in the file header.
uint32 GetInventoryItemsCount(Player* player, uint32 itemId)
{
    return player->GetItemCount(itemId, true);
}

// 5.4.8: see the GetCurrentQuestsRequiringItemId() note in the file header. AC returns a list of
// (Quest const*, requiredCount) pairs for every quest in the bot's log that needs this item; on 5.4.8 the
// requirement lives in the quest's QUEST_OBJECTIVE_ITEM objectives, so that is what is scanned here.
std::vector<std::pair<Quest const*, uint32>> GetCurrentQuestsRequiringItemId(Player* player, uint32 itemId)
{
    std::vector<std::pair<Quest const*, uint32>> result;

    for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questId = player->GetQuestSlotQuestId(i);
        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        for (QuestObjective const& objective : quest->GetObjectives())
        {
            if (objective.Type == QUEST_OBJECTIVE_ITEM && objective.ObjectID == itemId && objective.Amount > 0)
                result.push_back(std::make_pair(quest, uint32(objective.Amount)));
        }
    }

    return result;
}

// 5.4.8: SMSG_ITEM_PUSH_RESULT is bit-masked; this consumes the N guid mask bits that precede the guid
// bytes and then skips the bytes of the guid indexes whose mask bit is set (Player::SendNewItem(),
// Player.cpp:15389-15437). Returns the mask so the caller can check a specific index afterwards.
uint64 ReadGuidMask(WorldPacket& p)
{
    return p.ReadBits(8);
}

void SkipMaskedGuidBytes(WorldPacket& p, uint64 mask, uint8 index)
{
    if (mask & (uint64(1) << index))
    {
        uint8 dummy = 0;
        p >> dummy;
    }
}

// #257/#258 lane Q2: AC QuestAction.cpp:23 `ObjectGuid guid = event.getObject();` reads the master's packet as a raw
// uint64 guid, which is what 3.3.5 CMSG_GOSSIP_HELLO / CMSG_QUESTGIVER_HELLO / CMSG_QUESTGIVER_COMPLETE_QUEST /
// CMSG_GAMEOBJ_USE carry (AC PlayerbotAI.cpp:286-287,335 route them to "gossip hello" / "complete quest", whose
// action is this Execute). On 5.4.8 all four bit-pack the guid, so Event::getObject() returned a garbage guid
// whenever the packet reached 8 bytes (a creature guid with 7 nonzero bytes = 1 mask byte + 7 = 8) and
// ProcessQuests() then found no creature ("talk to quest giver - FAILED", t257 wave 86 Q3). Each reader below
// mirrors the core handler field for field; any other packet keeps AC's getObject().
ObjectGuid ReadQuestEventGuid(Event& event)
{
    WorldPacket& packet = event.getPacket();
    uint32 const opcode = packet.GetOpcode();
    if (packet.empty() || (opcode != CMSG_GOSSIP_HELLO && opcode != CMSG_QUESTGIVER_HELLO &&
                           opcode != CMSG_QUESTGIVER_COMPLETE_QUEST && opcode != CMSG_GAMEOBJ_USE))
        return event.getObject();

    WorldPacket p(packet);
    p.rpos(0);
    p.ResetBitPos();
    ObjectGuid guid;
    try
    {
        if (opcode == CMSG_GOSSIP_HELLO)
        {
            // WorldSession::HandleGossipHelloOpcode (Handlers/NPCHandler.cpp:447-468)
            guid[2] = p.ReadBit(); guid[4] = p.ReadBit(); guid[0] = p.ReadBit(); guid[3] = p.ReadBit();
            guid[6] = p.ReadBit(); guid[7] = p.ReadBit(); guid[5] = p.ReadBit(); guid[1] = p.ReadBit();
            p.ReadByteSeq(guid[4]); p.ReadByteSeq(guid[7]); p.ReadByteSeq(guid[1]); p.ReadByteSeq(guid[0]);
            p.ReadByteSeq(guid[5]); p.ReadByteSeq(guid[3]); p.ReadByteSeq(guid[6]); p.ReadByteSeq(guid[2]);
        }
        else if (opcode == CMSG_QUESTGIVER_HELLO)
        {
            // WorldSession::HandleQuestgiverHelloOpcode (Handlers/QuestHandler.cpp:102-122)
            guid[5] = p.ReadBit(); guid[6] = p.ReadBit(); guid[7] = p.ReadBit(); guid[3] = p.ReadBit();
            guid[4] = p.ReadBit(); guid[2] = p.ReadBit(); guid[1] = p.ReadBit(); guid[0] = p.ReadBit();
            p.ReadByteSeq(guid[4]); p.ReadByteSeq(guid[1]); p.ReadByteSeq(guid[7]); p.ReadByteSeq(guid[3]);
            p.ReadByteSeq(guid[6]); p.ReadByteSeq(guid[0]); p.ReadByteSeq(guid[5]); p.ReadByteSeq(guid[2]);
        }
        else if (opcode == CMSG_QUESTGIVER_COMPLETE_QUEST)
        {
            // WorldSession::HandleQuestgiverCompleteQuest (Handlers/QuestHandler.cpp:673-698): questId, 7 guid bits,
            // autoCompleteMode, guid bit 0, then the bytes.
            p.read_skip<uint32>();
            guid[4] = p.ReadBit(); guid[2] = p.ReadBit(); guid[1] = p.ReadBit(); guid[5] = p.ReadBit();
            guid[6] = p.ReadBit(); guid[7] = p.ReadBit(); guid[3] = p.ReadBit(); p.ReadBit(); guid[0] = p.ReadBit();
            p.ReadByteSeq(guid[0]); p.ReadByteSeq(guid[2]); p.ReadByteSeq(guid[1]); p.ReadByteSeq(guid[4]);
            p.ReadByteSeq(guid[3]); p.ReadByteSeq(guid[6]); p.ReadByteSeq(guid[7]); p.ReadByteSeq(guid[5]);
        }
        else
        {
            // WorldSession::HandleGameObjectUseOpcode (Handlers/SpellHandler.cpp:596-616)
            guid[6] = p.ReadBit(); guid[1] = p.ReadBit(); guid[3] = p.ReadBit(); guid[4] = p.ReadBit();
            guid[0] = p.ReadBit(); guid[5] = p.ReadBit(); guid[7] = p.ReadBit(); guid[2] = p.ReadBit();
            p.ReadByteSeq(guid[0]); p.ReadByteSeq(guid[1]); p.ReadByteSeq(guid[6]); p.ReadByteSeq(guid[2]);
            p.ReadByteSeq(guid[3]); p.ReadByteSeq(guid[4]); p.ReadByteSeq(guid[5]); p.ReadByteSeq(guid[7]);
        }
    }
    catch (ByteBufferException const&)
    {
        return ObjectGuid::Empty;
    }

    return guid;
}
}

bool QuestAction::Execute(Event event)
{
    // AC QuestAction.cpp:23 `event.getObject()`; 5.4.8 bit-packed master packets are decoded first (see above).
    ObjectGuid guid = ReadQuestEventGuid(event);
    Player* master = GetMaster();

    // Checks if the bot and botAI are valid
    if (!bot || !botAI)
        return false;

    // Sets guid based on bot or master target
    if (!guid)
    {
        if (!master)
        {
            guid = bot->GetTarget();
        }
        else
        {
            guid = master->GetTarget();
        }
    }

    if (guid)
    {
        return ProcessQuests(guid);
    }

    bool result = false;

    // Check the nearest NPCs
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && bot->GetDistance(unit) <= INTERACTION_DISTANCE)
        {
            result |= ProcessQuests(unit);
        }
    }

    // Checks the nearest game objects
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (auto const& go : gos)
    {
        GameObject* gameobj = botAI->GetGameObject(go);
        if (gameobj && bot->GetDistance(gameobj) <= INTERACTION_DISTANCE)
        {
            result |= ProcessQuests(gameobj);
        }
    }

    return result;
}

bool QuestAction::CompleteQuest(Player* player, uint32 entry)
{
    Quest const* pQuest = sObjectMgr->GetQuestTemplate(entry);

    // If player doesn't have the quest
    if (!pQuest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
    {
        return false;
    }

    // 5.4.8: see the "quest data moved to objectives" note in the file header - AC's three parallel
    // RequiredItemId/RequiredItemCount, RequiredNpcOrGo/RequiredNpcOrGoCount and GetRepObjectiveFaction/
    // GetRepObjectiveValue blocks become one pass over Quest::GetObjectives(), shaped like the core's own
    // Player::IncompleteQuest(bool, uint32) (Player.cpp:16457-16506).
    for (QuestObjective const& objective : pQuest->GetObjectives())
    {
        switch (objective.Type)
        {
            // Add quest items for quests that require items
            case QUEST_OBJECTIVE_ITEM:
            {
                uint32 id = objective.ObjectID;
                int32 amount = objective.Amount;
                if (!id || amount <= 0)
                {
                    break;
                }

                uint32 count = uint32(amount);
                uint32 curItemCount = player->GetItemCount(id, true);

                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count - curItemCount);
                if (msg == EQUIP_ERR_OK)
                {
                    if (Item* item = player->StoreNewItem(dest, id, true))
                    {
                        player->SendNewItem(item, count - curItemCount, true, false);
                    }
                }
                break;
            }

            // All creature/GO slain (not required, but otherwise it will display "Creature slain 0/10")
            case QUEST_OBJECTIVE_MONSTER:
            {
                int32 creature = int32(objective.ObjectID);
                int32 creaturecount = objective.Amount;
                if (creature <= 0 || creaturecount <= 0)
                {
                    break;
                }

                if (CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(creature))
                    for (int32 z = 0; z < creaturecount; ++z)
                    {
                        player->KilledMonster(cInfo, ObjectGuid::Empty);
                    }
                break;
            }

            case QUEST_OBJECTIVE_GAMEOBJECT:
            {
                uint32 go = objective.ObjectID;
                int32 gocount = objective.Amount;
                if (!go || gocount <= 0)
                {
                    break;
                }

                for (int32 z = 0; z < gocount; ++z)
                {
                    player->KillCreditGO(go, ObjectGuid::Empty);
                }
                break;
            }

            // If the quest requires reputation to complete
            case QUEST_OBJECTIVE_MIN_REPUTATION:
            case QUEST_OBJECTIVE_MAX_REPUTATION:
            {
                uint32 repFaction = objective.ObjectID;
                int32 repValue = objective.Amount;
                if (!repFaction || repValue <= 0)
                {
                    break;
                }

                uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
                if (int32(curRep) < repValue)
                    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                    {
                        player->GetReputationMgr().SetReputation(factionEntry, repValue);
                    }
                break;
            }

            // If the quest requires money
            case QUEST_OBJECTIVE_MONEY:
            {
                int32 reqMoney = objective.Amount;
                if (reqMoney <= 0)
                {
                    break;
                }

                player->ModifyMoney(reqMoney);
                break;
            }

            default:
                break;
        }
    }

    std::string const text_quest = ChatHelper::FormatQuest(pQuest);
    if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) ||
        botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
    {
        TC_LOG_INFO("playerbots", "{} => Quest [ {} ] completed", bot->GetName(), pQuest->GetLogTitle());
        bot->Say("Quest [ " + text_quest + " ] completed", LANG_UNIVERSAL);
    }
    botAI->TellMasterNoFacing("Quest completed " + text_quest);

    player->CompleteQuest(entry);

    return true;
}

bool QuestAction::ProcessQuests(ObjectGuid questGiver)
{
    if (GameObject* gameObject = botAI->GetGameObject(questGiver))
        if (gameObject->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
            return ProcessQuests(gameObject);

    Creature* creature = botAI->GetCreature(questGiver);
    if (creature)
        return ProcessQuests(creature);

    return false;
}

bool QuestAction::ProcessQuests(WorldObject* questGiver)
{
    ObjectGuid guid = questGiver->GetGUID();

    if (bot->GetDistance(questGiver) > INTERACTION_DISTANCE && !sPlayerbotAIConfig->syncQuestWithPlayer)
    {
        botAI->TellError("Cannot talk to quest giver");
        return false;
    }

    if (!BotHasInArcAC(bot, CAST_ANGLE_IN_FRONT, questGiver, sPlayerbotAIConfig->sightDistance))
        bot->SetFacingToObject(questGiver);

    bot->SetTarget(guid);
    bot->PrepareQuestMenu(guid);

    QuestMenu& questMenu = bot->PlayerTalkClass->GetQuestMenu();
    for (uint32 i = 0; i < questMenu.GetMenuItemCount(); ++i)
    {
        QuestMenuItem const& menuItem = questMenu.GetItem(i);
        uint32 questID = menuItem.QuestId;
        Quest const* quest = sObjectMgr->GetQuestTemplate(questID);
        if (!quest)
            continue;

        ProcessQuest(quest, questGiver);
    }

    return true;
}

bool QuestAction::AcceptQuest(Quest const* quest, ObjectGuid questGiver)
{
    std::ostringstream out;

    uint32 questId = quest->GetQuestId();

    if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
        out << "Already completed";
    else if (!bot->CanTakeQuest(quest, false))
    {
        if (!bot->SatisfyQuestStatus(quest, false))
            out << "Already on";
        else
            out << "Can't take";
    }
    else if (!bot->SatisfyQuestLog(false))
        out << "Quest log is full";
    else if (!bot->CanAddQuest(quest, false))
        out << "Bags are full";
    else
    {
        WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
        // #257: the 5.4.8 wire format is NOT AC's flat `guid, questId, unk1`. The core's own reader
        // (src/server/game/Handlers/QuestHandler.cpp:149-170) is `questId(u32)` FIRST, then NINE bits
        // (guid[6], guid[0], startCheat, guid[2], guid[7], guid[5], guid[4], guid[3], guid[1]), then eight
        // byte-seq writes in the order 5,4,0,1,6,2,3,7. Building it any other way makes the handler read
        // garbage for the guid and the quest, so every accept silently fails. Mirror the reader exactly;
        // AcceptQuestAction.cpp has the same pattern on the parse side.
        p << questId;
        p.WriteBit(questGiver[6]);
        p.WriteBit(questGiver[0]);
        p.WriteBit(false); // startCheat
        p.WriteBit(questGiver[2]);
        p.WriteBit(questGiver[7]);
        p.WriteBit(questGiver[5]);
        p.WriteBit(questGiver[4]);
        p.WriteBit(questGiver[3]);
        p.WriteBit(questGiver[1]);
        p.FlushBits();
        p.WriteByteSeq(questGiver[5]);
        p.WriteByteSeq(questGiver[4]);
        p.WriteByteSeq(questGiver[0]);
        p.WriteByteSeq(questGiver[1]);
        p.WriteByteSeq(questGiver[6]);
        p.WriteByteSeq(questGiver[2]);
        p.WriteByteSeq(questGiver[3]);
        p.WriteByteSeq(questGiver[7]);
        p.rpos(0);
        bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(p);

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_NONE && sPlayerbotAIConfig->syncQuestWithPlayer)
        {
            Object* pObject = ObjectAccessor::GetObjectByTypeMask(
                *bot, questGiver, TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT | TYPEMASK_ITEM);
            bot->AddQuest(quest, pObject);
        }

        if (bot->GetQuestStatus(questId) != QUEST_STATUS_NONE && bot->GetQuestStatus(questId) != QUEST_STATUS_REWARDED)
        {
            BroadcastHelper::BroadcastQuestAccepted(botAI, bot, quest);  // AC QuestAction.cpp:252
            out << "Accepted " << chat->FormatQuest(quest);
            botAI->TellMaster(out);
            return true;
        }
        out << "Cannot accept";
    }

    out << " " << chat->FormatQuest(quest);
    botAI->TellMaster(out);

    return false;
}

// AC QuestAction.cpp:266-299. 5.4.8: the body is the same bare uint32 questId (QuestPackets.cpp:56).
bool QuestUpdateCompleteAction::Execute(Event event)
{
    // the action can hardly be triggered
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 questId = 0;
    p >> questId;

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId);
    if (qInfo)
    {
        const auto format = ChatHelper::FormatQuest(qInfo);
        if (botAI->GetMaster())
            botAI->TellMasterNoFacing("Quest completed " + format);
        BroadcastHelper::BroadcastQuestUpdateComplete(botAI, bot, qInfo);  // AC QuestAction.cpp:292
        botAI->GetRpgStatistic().questCompleted++;
    }

    return true;
}

/*
 * For creature or gameobject
 */
bool QuestUpdateAddKillAction::Execute(Event event)
{
    // 5.4.8: SMSG_QUESTUPDATE_ADD_KILL does not exist here; SMSG_QUESTUPDATE_ADD_CREDIT replaces it with a
    // different layout (Player::SendQuestUpdateAddCredit(), Player.cpp:18302-18329) - see the file header.
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint16 available = 0;
    uint8 objectiveType = 0;
    uint32 questId = 0;
    uint16 required = 0;
    uint32 objectId = 0;
    p >> available >> objectiveType >> questId >> required >> objectId;

    // Trailing guid: 8 mask bits then the masked bytes. Nothing reads past it, but consume it so the read
    // position stays honest for anything appended later.
    uint64 guidMask = ReadGuidMask(p);
    for (uint8 i = 0; i < 8; ++i)
        SkipMaskedGuidBytes(p, guidMask, i);

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId);
    if (!qInfo)
        return false;

    std::string infoName;
    if (objectiveType == QUEST_OBJECTIVE_GAMEOBJECT)
        infoName = GetGameObjectName(objectId);
    else if (objectiveType == QUEST_OBJECTIVE_MONSTER)
        infoName = GetCreatureName(objectId);

    if (infoName.empty())
        return false;

    BroadcastHelper::BroadcastQuestUpdateAddKill(botAI, bot, qInfo, available, required, infoName);  // AC QuestAction.cpp:320/335
    if (botAI->GetMaster())
    {
        std::ostringstream out;
        out << infoName << " " << available << "/" << required << " " << ChatHelper::FormatQuest(qInfo);
        botAI->TellMasterNoFacing(out.str());
    }

    return false;
}

bool QuestUpdateAddItemAction::Execute(Event event)
{
    // 5.4.8: SMSG_QUESTITEM_PUSH_RESULT is SMSG_ITEM_PUSH_RESULT here and the body is bit-masked and
    // byte-seq interleaved (Player::SendNewItem(), Player.cpp:15378-15437) - see the file header. This
    // reads it strictly in the order that function writes it.
    WorldPacket p(event.getPacket());
    p.rpos(0);

    // 20 mask bits, written in this exact order by Player::SendNewItem().
    uint64 playerMask = 0;
    uint64 itemMask = 0;
    bool received = false;
    bool created = false;
    for (uint8 bit = 0; bit < 20; ++bit)
    {
        bool value = p.ReadBit() != 0;
        switch (bit)
        {
            case 0:
                itemMask |= value ? (uint64(1) << 2) : 0;
                break;
            case 1:
                playerMask |= value ? (uint64(1) << 4) : 0;
                break;
            case 2:
                itemMask |= value ? (uint64(1) << 5) : 0;
                break;
            case 3:
                break;  // "display in chat"
            case 4:
                playerMask |= value ? (uint64(1) << 1) : 0;
                break;
            case 5:
                received = value;
                break;
            case 6:
                itemMask |= value ? (uint64(1) << 4) : 0;
                break;
            case 7:
                playerMask |= value ? (uint64(1) << 6) : 0;
                break;
            case 8:
                playerMask |= value ? (uint64(1) << 5) : 0;
                break;
            case 9:
                playerMask |= value ? (uint64(1) << 7) : 0;
                break;
            case 10:
                playerMask |= value ? (uint64(1) << 0) : 0;
                break;
            case 11:
                itemMask |= value ? (uint64(1) << 0) : 0;
                break;
            case 12:
                itemMask |= value ? (uint64(1) << 7) : 0;
                break;
            case 13:
                playerMask |= value ? (uint64(1) << 2) : 0;
                break;
            case 14:
                playerMask |= value ? (uint64(1) << 6) : 0;
                break;
            case 15:
                break;  // "bonus loot"
            case 16:
                playerMask |= value ? (uint64(1) << 3) : 0;
                break;
            case 17:
                itemMask |= value ? (uint64(1) << 1) : 0;
                break;
            case 18:
                created = value;
                break;
            case 19:
                itemMask |= value ? (uint64(1) << 3) : 0;
                break;
            default:
                break;
        }
    }

    uint8 dummy8 = 0;
    uint32 petSpecies = 0;
    uint32 suffixFactor = 0;
    uint32 quality = 0;
    uint32 itemId = 0;
    int32 itemRandomPropertyId = 0;
    uint32 breed = 0;
    uint32 itemCount = 0;
    uint32 count = 0;
    uint32 itemSlot = 0;
    uint32 petLevel = 0;

    SkipMaskedGuidBytes(p, playerMask, 1);
    SkipMaskedGuidBytes(p, itemMask, 1);
    p >> petSpecies;
    SkipMaskedGuidBytes(p, itemMask, 0);
    SkipMaskedGuidBytes(p, playerMask, 5);
    SkipMaskedGuidBytes(p, playerMask, 2);
    p >> suffixFactor;
    SkipMaskedGuidBytes(p, itemMask, 7);
    p >> quality;
    p >> itemId;
    p >> itemRandomPropertyId;
    SkipMaskedGuidBytes(p, itemMask, 6);
    p >> breed;
    p >> itemCount;
    SkipMaskedGuidBytes(p, playerMask, 0);
    p >> count;
    SkipMaskedGuidBytes(p, itemMask, 7);
    SkipMaskedGuidBytes(p, itemMask, 5);
    SkipMaskedGuidBytes(p, playerMask, 4);
    p >> itemSlot;
    p >> dummy8;  // bag slot
    SkipMaskedGuidBytes(p, playerMask, 3);
    SkipMaskedGuidBytes(p, itemMask, 6);
    p >> petLevel;
    SkipMaskedGuidBytes(p, itemMask, 3);
    SkipMaskedGuidBytes(p, itemMask, 4);

    if (!received && !created)
        return false;

    auto const* itemPrototype = sObjectMgr->GetItemTemplate(itemId);
    if (!itemPrototype)
        return false;

    std::string itemLink = ChatHelper::FormatItem(itemPrototype);
    uint32 availableItemsCount = GetInventoryItemsCount(bot, itemId);

    for (auto const& pair : GetCurrentQuestsRequiringItemId(bot, itemId))
    {
        uint32 requiredItemsCount = pair.second;

        if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_COMBAT) ||
            botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT))
        {
            std::ostringstream text;
            text << ChatHelper::FormatQuest(pair.first) << " - " << itemLink << " "
                 << ChatHelper::FormatQuestObjective(itemLink, availableItemsCount, requiredItemsCount);
            botAI->Say(text.str());
            TC_LOG_INFO("playerbots", "{} => {}", bot->GetName(), text.str());
        }

        BroadcastHelper::BroadcastQuestUpdateAddItem(botAI, bot, pair.first, availableItemsCount, requiredItemsCount,
                                                     itemPrototype);  // AC QuestAction.cpp:375
    }

    return false;
}

bool QuestItemPushResultAction::Execute(Event event)
{
    // 5.4.8: same SMSG_ITEM_PUSH_RESULT layout as QuestUpdateAddItemAction above - see the file header.
    WorldPacket packet(event.getPacket());
    packet.rpos(0);

    uint64 playerMask = 0;
    uint64 itemMask = 0;
    bool received = false;
    bool created = false;
    for (uint8 bit = 0; bit < 20; ++bit)
    {
        bool value = packet.ReadBit() != 0;
        switch (bit)
        {
            case 0:
                itemMask |= value ? (uint64(1) << 2) : 0;
                break;
            case 1:
                playerMask |= value ? (uint64(1) << 4) : 0;
                break;
            case 2:
                itemMask |= value ? (uint64(1) << 5) : 0;
                break;
            case 3:
                break;
            case 4:
                playerMask |= value ? (uint64(1) << 1) : 0;
                break;
            case 5:
                received = value;
                break;
            case 6:
                itemMask |= value ? (uint64(1) << 4) : 0;
                break;
            case 7:
                playerMask |= value ? (uint64(1) << 6) : 0;
                break;
            case 8:
                playerMask |= value ? (uint64(1) << 5) : 0;
                break;
            case 9:
                playerMask |= value ? (uint64(1) << 7) : 0;
                break;
            case 10:
                playerMask |= value ? (uint64(1) << 0) : 0;
                break;
            case 11:
                itemMask |= value ? (uint64(1) << 0) : 0;
                break;
            case 12:
                itemMask |= value ? (uint64(1) << 7) : 0;
                break;
            case 13:
                playerMask |= value ? (uint64(1) << 2) : 0;
                break;
            case 14:
                playerMask |= value ? (uint64(1) << 6) : 0;
                break;
            case 15:
                break;
            case 16:
                playerMask |= value ? (uint64(1) << 3) : 0;
                break;
            case 17:
                itemMask |= value ? (uint64(1) << 1) : 0;
                break;
            case 18:
                created = value;
                break;
            case 19:
                itemMask |= value ? (uint64(1) << 3) : 0;
                break;
            default:
                break;
        }
    }

    uint8 dummy8 = 0;
    uint32 petSpecies = 0;
    uint32 suffixFactor = 0;
    uint32 quality = 0;
    uint32 itemEntry = 0;
    int32 itemRandomPropertyId = 0;
    uint32 breed = 0;
    uint32 itemCount = 0;
    uint32 count = 0;
    uint32 itemSlot = 0;
    uint32 petLevel = 0;

    SkipMaskedGuidBytes(packet, playerMask, 1);
    SkipMaskedGuidBytes(packet, itemMask, 1);
    packet >> petSpecies;
    SkipMaskedGuidBytes(packet, itemMask, 0);
    SkipMaskedGuidBytes(packet, playerMask, 5);
    SkipMaskedGuidBytes(packet, playerMask, 2);
    packet >> suffixFactor;
    SkipMaskedGuidBytes(packet, itemMask, 7);
    packet >> quality;
    packet >> itemEntry;
    packet >> itemRandomPropertyId;
    SkipMaskedGuidBytes(packet, itemMask, 6);
    packet >> breed;
    packet >> itemCount;
    SkipMaskedGuidBytes(packet, playerMask, 0);
    packet >> count;
    SkipMaskedGuidBytes(packet, itemMask, 7);
    SkipMaskedGuidBytes(packet, itemMask, 5);
    SkipMaskedGuidBytes(packet, playerMask, 4);
    packet >> itemSlot;
    packet >> dummy8;
    SkipMaskedGuidBytes(packet, playerMask, 3);
    SkipMaskedGuidBytes(packet, itemMask, 6);
    packet >> petLevel;
    SkipMaskedGuidBytes(packet, itemMask, 3);
    SkipMaskedGuidBytes(packet, itemMask, 4);

    if (received || created)
        return false;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemEntry);
    if (!proto)
        return false;

    for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questId = bot->GetQuestSlotQuestId(i);
        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            return false;

        // 5.4.8: AC's QUEST_ITEM_OBJECTIVES_COUNT loop over quest->RequiredItemId/RequiredItemCount - see
        // the "quest data moved to objectives" note in the file header.
        for (QuestObjective const& objective : quest->GetObjectives())
        {
            if (objective.Type != QUEST_OBJECTIVE_ITEM || objective.ObjectID != itemEntry || objective.Amount <= 0)
                continue;

            int32 required = objective.Amount;
            int32 previousCount = int32(itemCount) - int32(count);
            if (uint32(previousCount) < uint32(required))
            {
                if (botAI->GetMaster())
                {
                    std::string itemLink = ChatHelper::FormatItem(proto);
                    std::ostringstream out;
                    int32 available = std::min(int32(itemCount), required);
                    out << itemLink << " " << available << "/" << required << " " << ChatHelper::FormatQuest(quest);
                    botAI->TellMasterNoFacing(out.str());
                }
            }
        }
    }

    return false;
}

bool QuestUpdateFailedAction::Execute(Event /*event*/)
{
    // SMSG_QUESTUPDATE_FAILED is never sent on 5.4.8 either (only the opcode constant exists,
    // Opcodes.h:998, with no sender anywhere in src/) - same situation AC documented.
    return false;
}

bool QuestUpdateFailedTimerAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 questId;
    p >> questId;

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId);

    if (qInfo)
    {
        botAI->TellMaster("Failed timer for " + ChatHelper::FormatQuest(qInfo) + ", abandoning");
        BroadcastHelper::BroadcastQuestUpdateFailedTimer(botAI, bot, qInfo);  // AC QuestAction.cpp:455
    }
    else
    {
        botAI->TellMaster("Failed timer for " + std::to_string(questId));
    }

    // drop quest - 5.4.8: bot->AbandonQuest() -> bot->FailQuest(), see the file header
    bot->FailQuest(questId);

    return false;
}