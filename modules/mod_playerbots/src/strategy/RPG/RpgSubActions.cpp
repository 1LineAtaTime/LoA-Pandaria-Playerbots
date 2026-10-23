/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Action/RpgSubActions.cpp)
 * into the Legends of Azeroth Pandaria fork, issue #272 (RpgDuelAction: issue #201, dormant parity).
 *
 * 5.4.8 / this-fork adaptations vs AC (each is also marked at the site):
 *  - botAI->HasGameClientMaster() -> botAI->HasRealPlayerMaster() (AI/PlayerbotAI.h:~214, AI/PlayerbotAI.cpp:2261;
 *    same predicate AC's HasGameClientMaster is: a real client or a self-bot master; see also
 *    strategy/actions/AutoMaintenanceOnLevelupAction.cpp:141).
 *  - botAI->GetGroupLeader() -> botAI->GetGroupMaster() (AI/PlayerbotAI.cpp:2438, identical body to AC 4663-4671).
 *  - IsRealPlayer(botAI->GetMaster()) (AC free function: master exists and has no PlayerbotAI) ->
 *    botAI->HasActivePlayerMaster() (AI/PlayerbotAI.cpp:2271, `master && !GET_PLAYERBOT_AI(master)`).
 *  - sPlayerbotAIConfig.x -> sPlayerbotAIConfig->x ; PlayerbotTextMgr::instance() exists (Mgr/Text/PlayerbotTextMgr.h:68).
 *  - chat->FormatWorldobject -> RpgFormatWorldobject (RpgAction.h; ChatHelper has none on this fork).
 *  - TalkAction::GetRandomEmote(unit) (AC EmoteAction.cpp:815-877, static, textEmote=false animation branch): the
 *    fork's TalkAction::GetRandomEmote (strategy/actions/TalkAction.h:28) is a private non-static member and only
 *    has the text-emote branch, so the animation branch is copied file-locally (RpgGetRandomEmote below).
 *  - CMSG_GOSSIP_HELLO packet: AC writes `p << guid`; 5.4.8 HandleGossipHelloOpcode (Handlers/NPCHandler.cpp:447-468)
 *    reads a bit-packed guid. File-local WriteGossipHelloGuid mirrors the core reader (same writer as
 *    strategy/actions/GossipHelloAction.cpp:108-126, which is in an anonymous namespace and cannot be reused).
 *  - HandleCancelMountAuraOpcode / HandleMountSpecialAnimOpcode ignore their packet on 5.4.8
 *    (Handlers/MovementHandler.cpp:590) - empty packets, same as AC.
 *  - HandleAcceptTradeOpcode(WorldPacket&) never reads the packet on 5.4.8 (Handlers/TradeHandler.cpp:333; see
 *    strategy/actions/TradeStatusAction.cpp:230). AC's TRADE_STATUS_TRADE_ACCEPT is TRADE_STATUS_ACCEPTED here
 *    (Miscellaneous/SharedDefines.h:4246); the payload is kept as AC builds it.
 *  - Taxi: sObjectMgr->GetNearestTaxiNode(x,y,z,mapId,uint32 team) takes the team mask (Globals/ObjectMgr.h:1089),
 *    so AC's bot->GetTeamId() is bot->GetTeam() (Entities/Player/Player.h:2522; same as Travel/TravelMgr.cpp:524).
 *    TaxiNodesEntry::name is a `char const*` (DataStores/DBCStructure.h:2638) instead of AC's name[16] array, so
 *    `nodeFrom->name[0]` becomes `nodeFrom->name`. TaxiPathEntry::from/to, sTaxiPathStore/sTaxiNodesStore,
 *    Player::m_taxi.IsTaximaskNodeKnown (PlayerTaxi.h:37), isTaxiCheater (Player.h:1371),
 *    ActivateTaxiPathTo(std::vector<uint32> const&, Creature*, uint32) (Player.h:1328) are as in AC.
 *  - RpgTrainAction::isPossible: AC's Trainer::Trainer / GetTrainer / IsTrainerValidForPlayer / CanTeachSpell do not
 *    exist on this core. Same adaptation as strategy/actions/TrainerAction.cpp:5-93 and its isPossible (:182-205):
 *    sObjectMgr->GetNpcTrainerSpells(entry) (ObjectMgr.h:1640) -> TrainerSpellData::spellList, spell validity =
 *    bot->GetTrainerSpellState(spell) == TRAINER_SPELL_GREEN (Player.h:2019; level/skill/class/race/prereq),
 *    MoneyCost -> spellCost, reputation discount via Player::GetReputationPriceDiscount(Creature const*)
 *    (Player.h:1650, takes the live creature instead of AC's FactionTemplateEntry; no loaded creature -> not possible).
 *  - AREA_FLAG_ALLOW_DUELS: AreaTableEntry::flags is `Flags` on 5.4.8 (the core duel check is
 *    Spells/SpellEffects.cpp:4663 `casterAreaEntry->Flags & AREA_FLAG_ALLOW_DUELS`, DBCEnums.h:381).
 *  - AC includes "GuildCreateActions.h" (not on this fork, nothing from it is used) - dropped.
 *  - LOG_ERROR/LOG_INFO -> TC_LOG_ERROR/TC_LOG_INFO printf style.
 */

#include "RpgSubActions.h"
#include "BudgetValues.h"
#include "ChooseRpgTargetAction.h"
#include "EmoteAction.h"
#include "Formations.h"
#include "QuestDef.h"  // QuestGiverStatus for GossipDef.h
#include "GossipDef.h"
#include "LastMovementValue.h"
#include "MovementActions.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "PossibleRpgTargetsValue.h"
#include "SocialMgr.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "DBCEnums.h"
#include "ItemVisitors.h"
#include "ObjectMgr.h"
#include "PlayerTaxi.h"
#include "RpgAction.h"
#include "TradeData.h"
#include "WorldPosition.h"
#include "WorldSession.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace
{
// 5.4.8 adaptation of AC `p1 << guid` for CMSG_GOSSIP_HELLO (AC RpgSubActions.cpp RpgEmoteAction::Execute): writes the
// guid with the bit layout WorldSession::HandleGossipHelloOpcode reads (Handlers/NPCHandler.cpp:451-468). Identical
// to WriteGossipHelloGuid in strategy/actions/GossipHelloAction.cpp:108-126.
void WriteGossipHelloGuid(WorldPacket& data, ObjectGuid const& guid)
{
    data.WriteBit(guid[2]);
    data.WriteBit(guid[4]);
    data.WriteBit(guid[0]);
    data.WriteBit(guid[3]);
    data.WriteBit(guid[6]);
    data.WriteBit(guid[7]);
    data.WriteBit(guid[5]);
    data.WriteBit(guid[1]);

    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[1]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[5]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[2]);
}

// AC TalkAction::GetRandomEmote(unit, textEmote = false), Ai/Base/Actions/EmoteAction.cpp:815-877, animation branch
// (the branch AC's RpgEmoteAction call takes). See the file header.
uint32 RpgGetRandomEmote(Unit* unit)
{
    std::vector<uint32> types;
    if (!urand(0, 20))
    {
        // expressions
        types.push_back(EMOTE_ONESHOT_BOW);
        types.push_back(EMOTE_ONESHOT_RUDE);
        types.push_back(EMOTE_ONESHOT_CRY);
        types.push_back(EMOTE_ONESHOT_LAUGH);
        types.push_back(EMOTE_ONESHOT_POINT);
        types.push_back(EMOTE_ONESHOT_CHEER);
        types.push_back(EMOTE_ONESHOT_SHY);
    }
    else
    {
        // talk
        types.push_back(EMOTE_ONESHOT_TALK);
        types.push_back(EMOTE_ONESHOT_EXCLAMATION);
        types.push_back(EMOTE_ONESHOT_QUESTION);

        if (unit && (unit->HasNpcFlag(UNIT_NPC_FLAG_TRAINER) || unit->HasNpcFlag(UNIT_NPC_FLAG_QUESTGIVER)))
        {
            types.push_back(EMOTE_ONESHOT_SALUTE);
        }
    }

    return types[urand(0, types.size() - 1)];
}
}  // namespace

void RpgHelper::OnExecute(std::string nextAction)
{
    if (botAI->HasRealPlayerMaster() && nextAction == "rpg")
        nextAction = "rpg cancel";

    SET_AI_VALUE(std::string, "next rpg action", nextAction);
}

void RpgHelper::BeforeExecute()
{
    OnExecute();

    bot->SetTarget(guidP());

    setFacingTo(guidP());
}

void RpgHelper::AfterExecute(bool doDelay, bool waitForGroup)
{
    OnExecute();

    bot->SetTarget(guidP());

    setFacingTo(guidP());

    if (doDelay)
        setDelay(waitForGroup);

    setFacing(guidP());
}

GuidPosition RpgHelper::guidP() { return AI_VALUE(GuidPosition, "rpg target"); }

ObjectGuid RpgHelper::guid() { return (ObjectGuid)guidP(); }

 bool RpgHelper::InRange()
 {
     GuidPosition targetGuid = guidP();
     if (!targetGuid)
         return false;

    return bot->GetExactDist2dSq(targetGuid.GetPositionX(), targetGuid.GetPositionY()) <
        INTERACTION_DISTANCE * INTERACTION_DISTANCE;
 }

void RpgHelper::setFacingTo(GuidPosition guidPosition)
{
    bot->SetFacingTo(guidPosition.getAngleTo(bot) + static_cast<float>(M_PI));
}

void RpgHelper::setFacing(GuidPosition guidPosition)
{
    if (!guidPosition.IsUnit())
        return;

    if (guidPosition.IsPlayer())
        return;

    //    Unit* unit = guidPosition.GetUnit();

    //    unit->SetFacingTo(unit->GetAngle(bot));
}

void RpgHelper::setDelay(bool waitForGroup)
{
    if (!botAI->HasRealPlayerMaster() || (waitForGroup && botAI->GetGroupMaster() == bot && bot->GetGroup()))
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
    else
        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay / 5);
}

bool RpgSubAction::isPossible() { return rpg->guidP() && rpg->guidP().GetWorldObject(); }

bool RpgSubAction::isUseful() { return rpg->InRange(); }

bool RpgSubAction::Execute(Event event)
{
    bool doAction = botAI->DoSpecificAction(ActionName(), ActionEvent(event), true);
    rpg->AfterExecute(doAction, true);
    return doAction;
}

std::string const RpgSubAction::ActionName() { return "none"; }

Event RpgSubAction::ActionEvent(Event event) { return event; }

bool RpgStayAction::isUseful() { return rpg->InRange() && !botAI->HasRealPlayerMaster(); }

bool RpgStayAction::Execute(Event /*event*/)
{
    bot->PlayerTalkClass->SendCloseGossip();

    rpg->AfterExecute();
    return true;
}

bool RpgWorkAction::isUseful() { return rpg->InRange() && !botAI->HasRealPlayerMaster(); }

bool RpgWorkAction::Execute(Event /*event*/)
{
    bot->HandleEmoteCommand(EMOTE_STATE_USE_STANDING);
    rpg->AfterExecute();
    return true;
}

bool RpgEmoteAction::isUseful() { return rpg->InRange() && !botAI->HasRealPlayerMaster(); }

bool RpgEmoteAction::Execute(Event /*event*/)
{
    uint32 type = RpgGetRandomEmote(rpg->guidP().GetUnit());

    WorldPacket p1;
    WriteGossipHelloGuid(p1, rpg->guid());  // 5.4.8 bit-packed guid; AC: p1 << rpg->guid();
    bot->GetSession()->HandleGossipHelloOpcode(p1);

    bot->HandleEmoteCommand(type);

    rpg->AfterExecute();

    return true;
}

bool RpgCancelAction::Execute(Event /*event*/)
{
    RESET_AI_VALUE(GuidPosition, "rpg target");
    rpg->OnExecute("");
    return true;
}

bool RpgTaxiAction::isUseful() { return rpg->InRange() && !botAI->HasRealPlayerMaster(); }

bool RpgTaxiAction::Execute(Event /*event*/)
{
    GuidPosition guidP = rpg->guidP();

    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    uint32 node =
        sObjectMgr->GetNearestTaxiNode(guidP.GetPositionX(), guidP.GetPositionY(), guidP.GetPositionZ(), guidP.GetMapId(), bot->GetTeam());  // 5.4.8: uint32 team mask (ObjectMgr.h:1089), AC: bot->GetTeamId()

    std::vector<uint32> nodes;
    for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
    {
        TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i);
        if (entry && entry->from == node && (bot->m_taxi.IsTaximaskNodeKnown(entry->to) || bot->isTaxiCheater()))
        {
            nodes.push_back(i);
        }
    }

    if (nodes.empty())
    {
        TC_LOG_ERROR("playerbots", "Bot %s - No flight paths available", bot->GetName().c_str());
        return false;
    }

    uint32 path = nodes[urand(0, nodes.size() - 1)];
    uint32 money = bot->GetMoney();
    bot->SetMoney(money + 100000);

    TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(path);
    if (!entry)
        return false;

    TaxiNodesEntry const* nodeFrom = sTaxiNodesStore.LookupEntry(entry->from);
    TaxiNodesEntry const* nodeTo = sTaxiNodesStore.LookupEntry(entry->to);

    Creature* flightMaster = bot->GetNPCIfCanInteractWith(guidP, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!flightMaster)
    {
        TC_LOG_ERROR("playerbots", "Bot %s cannot talk to flightmaster (%u location available)",
                     bot->GetName().c_str(), static_cast<uint32>(nodes.size()));
        return false;
    }

    if (!bot->ActivateTaxiPathTo({entry->from, entry->to}, flightMaster, 0))
    {
        TC_LOG_ERROR("playerbots", "Bot %s cannot fly %u (%u location available)", bot->GetName().c_str(), path,
                     static_cast<uint32>(nodes.size()));
        return false;
    }

    // 5.4.8: TaxiNodesEntry::name is a `char const*`, AC prints name[0] of a localized array.
    TC_LOG_INFO("playerbots", "Bot %s <%s> is flying from %s to %s (%u location available)",
                bot->GetGUID().ToString().c_str(), bot->GetName().c_str(), nodeFrom->name, nodeTo->name,
                static_cast<uint32>(nodes.size()));

    bot->SetMoney(money);

    rpg->AfterExecute();

    return true;
}

bool RpgDiscoverAction::Execute(Event /*event*/)
{
    GuidPosition guidP = rpg->guidP();

    uint32 node =
        sObjectMgr->GetNearestTaxiNode(guidP.GetPositionX(), guidP.GetPositionY(), guidP.GetPositionZ(), guidP.GetMapId(), bot->GetTeam());  // 5.4.8: uint32 team mask (ObjectMgr.h:1089), AC: bot->GetTeamId()

    if (!node)
        return false;

    Creature* flightMaster = bot->GetNPCIfCanInteractWith(guidP, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!flightMaster)
        return false;

    return bot->GetSession()->SendLearnNewTaxiNode(flightMaster);
}

std::string const RpgStartQuestAction::ActionName() { return "accept all quests"; }

Event RpgStartQuestAction::ActionEvent(Event /*event*/)
{
    WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
    p << rpg->guid();
    p.rpos(0);
    return Event("rpg action", p);
}

std::string const RpgEndQuestAction::ActionName() { return "talk to quest giver"; }

Event RpgEndQuestAction::ActionEvent(Event /*event*/)
{
    WorldPacket p(CMSG_QUESTGIVER_COMPLETE_QUEST);
    p << rpg->guid();
    p.rpos(0);
    return Event("rpg action", p);
}

std::string const RpgBuyAction::ActionName() { return "buy"; }

Event RpgBuyAction::ActionEvent(Event /*event*/) { return Event("rpg action", "vendor"); }

std::string const RpgSellAction::ActionName() { return "sell"; }

Event RpgSellAction::ActionEvent(Event /*event*/) { return Event("rpg action", "vendor"); }

std::string const RpgRepairAction::ActionName() { return "repair"; }

bool RpgTrainAction::isUseful()
{
    if (!rpg->InRange())
        return false;

    Creature* creature = rpg->guidP().GetCreature();
    if (!creature)
        return false;

    if (!creature->IsInWorld() || creature->IsDuringRemoveFromWorld() || !creature->IsAlive())
        return false;

    return true;
}

bool RpgTrainAction::isPossible()
{
    GuidPosition gp = rpg->guidP();

    CreatureTemplate const* cinfo = gp.GetCreatureTemplate();
    if (!cinfo)
        return false;

    // 5.4.8: AC `Trainer::Trainer* trainer = sObjectMgr->GetTrainer(cinfo->Entry)` -> the npc_trainer cache
    // (ObjectMgr.h:1640), see the file header.
    TrainerSpellData const* trainer = sObjectMgr->GetNpcTrainerSpells(cinfo->Entry);
    if (!trainer)
        return false;

    // AC trainer->IsTrainerValidForPlayer(bot) is subsumed by the per-spell Player::GetTrainerSpellState check below
    // (class/race/skill/level), as in strategy/actions/TrainerAction.cpp:5-93.

    // 5.4.8: GetReputationPriceDiscount takes the Creature (Player.h:1650); AC passes the template's faction.
    Creature* creature = gp.GetCreature();
    if (!creature)
        return false;

    float reputationDiscount = bot->GetReputationPriceDiscount(creature);
    uint32 currentGold = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::spells);

    for (TrainerSpellMap::const_iterator ref = trainer->spellList.begin(); ref != trainer->spellList.end(); ++ref)
    {
        TrainerSpell const* trainerSpell = &ref->second;

        // AC trainer->CanTeachSpell(bot, trainerSpell)
        if (bot->GetTrainerSpellState(trainerSpell) != TRAINER_SPELL_GREEN)
            continue;

        // AC trainerSpell->MoneyCost
        if (currentGold < static_cast<uint32>(floor(trainerSpell->spellCost * reputationDiscount)))
            continue;

        // we only check if at least one spell can be learned from the trainer;
        // otherwise, the train action should not be allowed
        return true;
    }

    return false;
}

std::string const RpgTrainAction::ActionName() { return "trainer"; }

bool RpgHealAction::Execute(Event /*event*/)
{
    bool retVal = false;

    switch (bot->GetClass())
    {
        case CLASS_PRIEST:
            retVal = botAI->DoSpecificAction("lesser heal on party", Event(), true);
            break;
        case CLASS_DRUID:
            retVal = botAI->DoSpecificAction("healing touch on party", Event(), true);
            break;
        case CLASS_PALADIN:
            retVal = botAI->DoSpecificAction("holy light on party", Event(), true);
            break;
        case CLASS_SHAMAN:
            retVal = botAI->DoSpecificAction("healing wave on party", Event(), true);
            break;
    }

    return retVal;
}

std::string const RpgHomeBindAction::ActionName() { return "home"; }

std::string const RpgQueueBgAction::ActionName()
{
    SET_AI_VALUE(uint32, "bg type", (uint32)AI_VALUE(BattlegroundTypeId, "rpg bg type"));
    return "free bg join";
}

std::string const RpgBuyPetitionAction::ActionName() { return "buy petition"; }

std::string const RpgUseAction::ActionName() { return "use"; }

Event RpgUseAction::ActionEvent(Event /*event*/)
{
    return Event("rpg action", RpgFormatWorldobject(rpg->guidP().GetWorldObject()));
}

std::string const RpgSpellAction::ActionName() { return "cast random spell"; }

Event RpgSpellAction::ActionEvent(Event /*event*/)
{
    return Event("rpg action", RpgFormatWorldobject(rpg->guidP().GetWorldObject()));
}

std::string const RpgCraftAction::ActionName() { return "craft random item"; }

Event RpgCraftAction::ActionEvent(Event /*event*/)
{
    return Event("rpg action", RpgFormatWorldobject(rpg->guidP().GetWorldObject()));
}

std::vector<Item*> RpgTradeUsefulAction::CanGiveItems(GuidPosition guidPosition)
{
    Player* player = guidPosition.GetPlayer();

    std::vector<Item*> giveItems;

    if (botAI->HasActivePlayerMaster() || !GET_PLAYERBOT_AI(player))
        return giveItems;

    std::vector<ItemUsage> myUsages = {ITEM_USAGE_NONE, ITEM_USAGE_VENDOR, ITEM_USAGE_AH, ITEM_USAGE_DISENCHANT};

    for (auto& myUsage : myUsages)
    {
        std::vector<Item*> myItems =
            AI_VALUE2(std::vector<Item*>, "inventory items", "usage " + std::to_string(myUsage));
        std::reverse(myItems.begin(), myItems.end());

        for (auto& item : myItems)
        {
            if (!item->CanBeTraded())
                continue;

            if (bot->GetTradeData() && bot->GetTradeData()->HasItem(item->GetGUID()))
                continue;

            ItemUsage otherUsage = PAI_VALUE2(ItemUsage, "item usage", item->GetEntry());

            if (std::find(myUsages.begin(), myUsages.end(), otherUsage) == myUsages.end())
                giveItems.push_back(item);
        }
    }

    return giveItems;
}

bool RpgTradeUsefulAction::Execute(Event /*event*/)
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    if (!player)
        return false;

    std::vector<Item*> items = CanGiveItems(guidP);

    if (items.empty())
        return false;

    Item* item = items.front();

    std::ostringstream param;

    param << RpgFormatWorldobject(player);
    param << " ";
    param << chat->FormatItem(item->GetTemplate());

    bool hasTraded = botAI->DoSpecificAction("trade", Event("rpg action", param.str().c_str()), true);

    if (hasTraded || bot->GetTradeData())
    {
        if (bot->GetTradeData() && bot->GetTradeData()->HasItem(item->GetGUID()))
        {
            if (bot->GetGroup() && bot->GetGroup()->IsMember(guidP) && botAI->HasRealPlayerMaster())
                botAI->TellMasterNoFacing(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "rpg_item_better_for_player",
                    "You can use this %item better than me, %player.",
                    {{"%item", chat->FormatItem(item->GetTemplate())}, {"%player", guidP.GetPlayer()->GetName()}}));
            else
                bot->Say(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                             "rpg_item_better_for_player",
                             "You can use this %item better than me, %player.",
                             {{"%item", chat->FormatItem(item->GetTemplate())}, {"%player", player->GetName()}}),
                         (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));

            if (!urand(0, 4) || items.size() < 2)
            {
                // bot->Say("End trade with" + RpgFormatWorldobject(player), (bot->GetTeamId() == TEAM_ALLIANCE ?
                // LANG_COMMON : LANG_ORCISH));
                WorldPacket p;
                uint32 status = TRADE_STATUS_ACCEPTED;  // 5.4.8: AC TRADE_STATUS_TRADE_ACCEPT; the handler ignores the payload
                p << status;
                bot->GetSession()->HandleAcceptTradeOpcode(p);
            }
        }
        else
            bot->Say(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                         "rpg_start_trade_with_player",
                         "Start trade with %player",
                         {{"%player", RpgFormatWorldobject(player)}}),
                     (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));

        botAI->SetNextCheckDelay(sPlayerbotAIConfig->rpgDelay);
        return true;
    }

    return false;
}

bool RpgDuelAction::isUseful()
{
    // do not offer duel in non pvp areas
    if (sPlayerbotAIConfig->IsInPvpProhibitedZone(bot->GetZoneId()))
        return false;

    // Players can only fight a duel with each other outside (=not inside dungeons and not in capital cities)
    AreaTableEntry const* casterAreaEntry = sAreaTableStore.LookupEntry(bot->GetAreaId());
    if (casterAreaEntry && !(casterAreaEntry->Flags & AREA_FLAG_ALLOW_DUELS))
    {
        // Dueling isn't allowed here
        return false;
    }

    return true;
}

bool RpgDuelAction::Execute(Event /*event*/)
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    if (!player)
        return false;

    return botAI->DoSpecificAction("cast custom spell", Event("rpg action", RpgFormatWorldobject(player) + " 7266"),
                                   true);
}

bool RpgMountAnimAction::isUseful()
{
    return AI_VALUE2(bool, "mounted", "self target") && !AI_VALUE2(bool, "moving", "self target");
}

bool RpgMountAnimAction::Execute(Event /*event*/)
{
    WorldPacket p;
    bot->GetSession()->HandleMountSpecialAnimOpcode(p);

    return true;
}
