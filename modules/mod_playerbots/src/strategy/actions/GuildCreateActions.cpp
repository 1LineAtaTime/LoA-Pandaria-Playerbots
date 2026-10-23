/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/GuildCreateActions.cpp (#611).
 *
 * 5.4.8 / fork adaptations vs AC (each also marked at the site):
 *  - The three petition packets are bit-packed on 5.4.8. AC's 3.3.5 CMSG_PETITION_BUY / CMSG_OFFER_PETITION /
 *    CMSG_TURN_IN_PETITION layouts are replaced by the layouts WorldSession::HandlePetitionBuyOpcode
 *    (Handlers/PetitionsHandler.cpp:52-80), HandleOfferPetitionOpcode (:693-736) and HandleTurnInPetitionOpcode
 *    (:862-886) read; the Handle*Opcode calls themselves are unchanged.
 *  - #611 item 4: RandomPlayerbotFactory::CreateRandomGuildName (AC RandomPlayerbotFactory.cpp:774-803) and
 *    PlayerbotAI::GetFixedBotNumber/GetGuilderType/GetGrouperType (AC PlayerbotAI.cpp:4673-4750) are now the real members;
 *    the file-local copies are gone. HasGameClientMaster() is HasRealPlayerMaster() on the fork (no selfbots).
 *  - chat->FormatQItem(id) -> std::to_string(id) (the fork's numeric qualifier, BudgetValues.cpp:212); IsRealPlayer(master)
 *    -> HasRealPlayerMaster(); sPlayerbotAIConfig. -> ->; ServerFacade::instance() -> sServerFacade;
 *    CharacterDatabase.Query("{}") -> PQuery("%u"); Field::Get<T>() -> GetUInt32()/GetString().
 *  - Guild emblem / rank: Guild::HandleSetEmblem(session, EmblemInfo) and HandleSetRankInfo(session, rank, name, rights,
 *    moneyPerDay, rightsAndSlots) take a session on 5.4.8 and EmblemInfo has no value constructor, so it is filled through
 *    EmblemInfo::ReadPacket (Guild.cpp:992-995). The rank keeps its default name and 0 money-per-day (the getters are private). The "give the bot the
 *    missing 10g" step is kept: Guild::HandleSetEmblem charges EMBLEM_PRICE exactly as the AC comment describes.
 *  - zone->flags -> zone->Flags (AreaTableEntry, DBCStructure.h:564).
 */

#include "GuildCreateActions.h"
#include "BudgetValues.h"
#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "Event.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Language.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "RandomPlayerbotFactory.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "TravelMgr.h"
#include "World.h"
#include "WorldSession.h"

bool BuyPetitionAction::Execute(Event /*event*/)
{
    GuidVector vendors = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (GuidVector::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_PETITIONER);
        if (!pCreature)
            continue;

        std::string const guildName = RandomPlayerbotFactory::CreateRandomGuildName();
        if (guildName.empty())
            continue;

        // 5.4.8 CMSG_PETITION_BUY: bit-packed npc guid + 7-bit name length, name, guid bytes
        // (WorldSession::HandlePetitionBuyOpcode, PetitionsHandler.cpp:52-80).
        ObjectGuid npcGuid = pCreature->GetGUID();
        WorldPacket data(CMSG_PETITION_BUY);

        data.WriteBit(npcGuid[5]);
        data.WriteBit(npcGuid[2]);
        data.WriteBit(npcGuid[3]);
        data.WriteBits(guildName.length(), 7);
        data.WriteBit(npcGuid[4]);
        data.WriteBit(npcGuid[1]);
        data.WriteBit(npcGuid[7]);
        data.WriteBit(npcGuid[0]);
        data.WriteBit(npcGuid[6]);
        data.FlushBits();

        data.WriteString(guildName);
        data.WriteByteSeq(npcGuid[1]);
        data.WriteByteSeq(npcGuid[7]);
        data.WriteByteSeq(npcGuid[4]);
        data.WriteByteSeq(npcGuid[6]);
        data.WriteByteSeq(npcGuid[0]);
        data.WriteByteSeq(npcGuid[5]);
        data.WriteByteSeq(npcGuid[2]);
        data.WriteByteSeq(npcGuid[3]);

        data.rpos(0);
        bot->GetSession()->HandlePetitionBuyOpcode(data);

        return true;
    }

    return false;
}

bool BuyPetitionAction::isUseful() { return canBuyPetition(bot); };

bool BuyPetitionAction::canBuyPetition(Player* bot)
{
    if (bot->GetGuildId())
        return false;

    if (bot->GetGuildIdInvited())
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (AI_VALUE2(uint32, "item count", std::to_string(5863)))
        return false;

    if (botAI->GetGuilderType() == GuilderType::SOLO)
        return false;

    if (botAI->GetGrouperType() == GrouperType::SOLO)
        return false;

    if (!botAI->HasStrategy("guild", BOT_STATE_NON_COMBAT))
        return false;

    uint32 cost = 1000;  // GUILD_CHARTER_COST;

    if (AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::guild)) < cost)
        return false;

    return true;
}

bool PetitionOfferAction::Execute(Event event)
{
    std::vector<Item*> petitions = AI_VALUE2(std::vector<Item*>, "inventory items", std::to_string(5863));

    if (petitions.empty())
        return false;

    ObjectGuid guid = event.getObject();

    Player* master = GetMaster();
    if (!master)
    {
        if (!guid)
            guid = bot->GetTarget();
    }
    else
    {
        if (!guid)
            guid = master->GetTarget();
    }

    if (!guid)
        return false;

    Player* player = ObjectAccessor::FindPlayer(guid);

    if (!player)
        return false;

    // 5.4.8 CMSG_OFFER_PETITION: uint32 skipped, then both bit-packed guids (WorldSession::HandleOfferPetitionOpcode,
    // PetitionsHandler.cpp:693-736); "playerGuid" is the offered player, "petitionGuid" the charter item.
    ObjectGuid petitionGuid = petitions.front()->GetGUID();
    ObjectGuid playerGuid = guid;

    WorldPacket data(CMSG_OFFER_PETITION);

    data << uint32(0);

    data.WriteBit(playerGuid[4]);
    data.WriteBit(playerGuid[1]);
    data.WriteBit(petitionGuid[2]);
    data.WriteBit(playerGuid[6]);
    data.WriteBit(petitionGuid[1]);
    data.WriteBit(playerGuid[2]);
    data.WriteBit(petitionGuid[4]);
    data.WriteBit(playerGuid[3]);
    data.WriteBit(playerGuid[7]);
    data.WriteBit(petitionGuid[0]);
    data.WriteBit(petitionGuid[6]);
    data.WriteBit(playerGuid[5]);
    data.WriteBit(playerGuid[0]);
    data.WriteBit(petitionGuid[3]);
    data.WriteBit(petitionGuid[5]);
    data.WriteBit(petitionGuid[7]);
    data.FlushBits();

    data.WriteByteSeq(playerGuid[7]);
    data.WriteByteSeq(petitionGuid[1]);
    data.WriteByteSeq(petitionGuid[4]);
    data.WriteByteSeq(petitionGuid[2]);
    data.WriteByteSeq(playerGuid[6]);
    data.WriteByteSeq(petitionGuid[3]);
    data.WriteByteSeq(petitionGuid[0]);
    data.WriteByteSeq(petitionGuid[5]);
    data.WriteByteSeq(playerGuid[0]);
    data.WriteByteSeq(playerGuid[2]);
    data.WriteByteSeq(playerGuid[5]);
    data.WriteByteSeq(playerGuid[3]);
    data.WriteByteSeq(playerGuid[4]);
    data.WriteByteSeq(petitionGuid[7]);
    data.WriteByteSeq(playerGuid[1]);
    data.WriteByteSeq(petitionGuid[6]);

    QueryResult result =
        CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE player_account = %u AND petitionguid = %u",
                                 player->GetSession()->GetAccountId(), petitions.front()->GetGUID().GetCounter());
    if (result)
    {
        return false;
    }

    data.rpos(0);
    bot->GetSession()->HandleOfferPetitionOpcode(data);

    result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = %u",
                                      petitions.front()->GetGUID().GetCounter());
    uint8 signs = result ? (uint8)result->GetRowCount() : 0;

    context->GetValue<uint8>("petition signs")->Set(signs);

    return true;
}

bool PetitionOfferAction::isUseful() { return !bot->GetGuildId(); }

bool PetitionOfferNearbyAction::Execute(Event /*event*/)
{
    uint32 found = 0;

    GuidVector nearGuids = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    for (auto& i : nearGuids)
    {
        Player* player = ObjectAccessor::FindPlayer(i);

        if (!player)
            continue;

        if (player->GetGuildId())
            continue;

        if (player->GetGuildIdInvited())
            continue;

        PlayerbotAI* playerAI = GET_PLAYERBOT_AI(player);

        if (playerAI)
        {
            /*
            if (botAI->GetGrouperType() == SOLO && !botAI->HasGameClientMaster())   // Do not invite solo players.
                continue;

            */
            // AC: IsRealPlayer(botAI->GetMaster())   // Do not invite bots that belong to an active player.
            if (playerAI->HasRealPlayerMaster())
                continue;
        }
        else
        {
            if (!sPlayerbotAIConfig->randomBotGroupNearby)
                return false;
        }

        if (sServerFacade->GetDistance2d(bot, player) > sPlayerbotAIConfig->sightDistance)
            continue;

        // Parse rpg target to quest action.
        WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
        p << i;
        p.rpos(0);

        if (PetitionOfferAction::Execute(Event("petition offer nearby", p)))
            found++;
    }

    return found > 0;
}

bool PetitionOfferNearbyAction::isUseful()
{
    return !bot->GetGuildId() && AI_VALUE2(uint32, "item count", std::to_string(5863)) &&
           AI_VALUE(uint8, "petition signs") < sWorld->getIntConfig(CONFIG_MIN_PETITION_SIGNS);
}

bool PetitionTurnInAction::Execute(Event /*event*/)
{
    GuidVector vendors = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    std::vector<Item*> petitions = AI_VALUE2(std::vector<Item*>, "inventory items", std::to_string(5863));

    if (petitions.empty())
        return false;

    for (GuidVector::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_PETITIONER);
        if (!pCreature)
            continue;

        Item* petition = petitions.front();

        if (!petition)
            return false;

        // 5.4.8 CMSG_TURN_IN_PETITION: bit-packed petition guid (WorldSession::HandleTurnInPetitionOpcode,
        // PetitionsHandler.cpp:862-886).
        ObjectGuid petitionGuid = petition->GetGUID();
        WorldPacket data(CMSG_TURN_IN_PETITION, 9);

        data.WriteBit(petitionGuid[1]);
        data.WriteBit(petitionGuid[2]);
        data.WriteBit(petitionGuid[3]);
        data.WriteBit(petitionGuid[0]);
        data.WriteBit(petitionGuid[5]);
        data.WriteBit(petitionGuid[7]);
        data.WriteBit(petitionGuid[4]);
        data.WriteBit(petitionGuid[6]);
        data.FlushBits();

        data.WriteByteSeq(petitionGuid[2]);
        data.WriteByteSeq(petitionGuid[1]);
        data.WriteByteSeq(petitionGuid[4]);
        data.WriteByteSeq(petitionGuid[6]);
        data.WriteByteSeq(petitionGuid[0]);
        data.WriteByteSeq(petitionGuid[7]);
        data.WriteByteSeq(petitionGuid[5]);
        data.WriteByteSeq(petitionGuid[3]);

        data.rpos(0);
        bot->GetSession()->HandleTurnInPetitionOpcode(data);

        if (bot->GetGuildId())
        {
            // Ensure that bot has at least 10g for HandleSetEmblem can be managed core side
            // (EMBLEM_PRICE = 10 * GOLD in core)
            static constexpr uint32 REQUIRED = 10 * 10000;  // AC: 10 * GOLD (copper)
            uint32 have = bot->GetMoney();                  // actual money earned by bot in copper
            if (have < REQUIRED)
            {
                bot->ModifyMoney(int32(REQUIRED - have));  // add only the missing amount to bot to reach 10g
            }

            Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());

            uint32 st, cl, br, bc, bg;
            bg = urand(0, 51);
            bc = urand(0, 17);
            cl = urand(0, 17);
            br = urand(0, 7);
            st = urand(0, 180);

            // AC: EmblemInfo emblemInfo(st, cl, br, bc, bg); 5.4.8 has no such constructor, ReadPacket order is
            // borderStyle, backgroundColor, borderColor, color, style (Guild.cpp:994).
            WorldPacket emblemData;
            emblemData << br << bg << bc << cl << st;
            EmblemInfo emblemInfo;
            emblemInfo.ReadPacket(emblemData);

            guild->HandleSetEmblem(bot->GetSession(), emblemInfo);  // official core handling

            // LANG_GUILD_VETERAN -> can invite (AC: HandleSetRankInfo(2, rights)). 5.4.8 also takes the session, the rank
            // name and bank money-per-day; Guild::GetRankName/GetRankBankMoneyPerDay are private (Guild.h:1054-1056), so the
            // rank keeps the name Guild::CreateDefaultGuildRanks gave it (Guild.cpp:3510, in the creating session's locale)
            // and the default 0 money-per-day.
            guild->HandleSetRankInfo(bot->GetSession(), 2,
                                     sObjectMgr->GetTrinityString(LANG_GUILD_VETERAN,
                                                                  bot->GetSession()->GetSessionDbLocaleIndex()),
                                     GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK | GR_RIGHT_INVITE, 0,
                                     GuildBankRightsAndSlotsVec());
        }

        return true;
    }

    TravelTarget* oldTarget = context->GetValue<TravelTarget*>("travel target")->Get();

    // Select a new target to travel to.
    TravelTarget newTarget = TravelTarget(botAI);

    bool foundTarget = SetNpcFlagTarget(&newTarget, {UNIT_NPC_FLAG_PETITIONER});

    if (!foundTarget || !newTarget.isActive())
        return false;

    newTarget.setRadius(INTERACTION_DISTANCE);

    setNewTarget(&newTarget, oldTarget);

    return true;
}

bool PetitionTurnInAction::isUseful()
{
    bool inCity = false;
    if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(bot->GetZoneId()))
    {
        if (zone->Flags & AREA_FLAG_CAPITAL)
            inCity = true;
    }

    return inCity && !bot->GetGuildId() && AI_VALUE2(uint32, "item count", std::to_string(5863)) &&
           AI_VALUE(uint8, "petition signs") >= sWorld->getIntConfig(CONFIG_MIN_PETITION_SIGNS) &&
           !context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling();
}

bool BuyTabardAction::Execute(Event /*event*/)
{
    bool canBuy = botAI->DoSpecificAction("buy", Event("buy tabard", "Hitem:5976:"), true);
    if (canBuy && AI_VALUE2(uint32, "item count", std::to_string(5976)))
        return true;

    TravelTarget* oldTarget = context->GetValue<TravelTarget*>("travel target")->Get();

    // Select a new target to travel to.
    TravelTarget newTarget = TravelTarget(botAI);

    bool foundTarget = SetNpcFlagTarget(&newTarget, {UNIT_NPC_FLAG_TABARDDESIGNER}, "Tabard Vendor", {5976});

    if (!foundTarget || !newTarget.isActive())
        return false;

    newTarget.setRadius(INTERACTION_DISTANCE);

    setNewTarget(&newTarget, oldTarget);

    return true;
};

bool BuyTabardAction::isUseful()
{
    bool inCity = false;
    if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(bot->GetZoneId()))
    {
        if (zone->Flags & AREA_FLAG_CAPITAL)
            inCity = true;
    }

    return inCity && bot->GetGuildId() && !AI_VALUE2(uint32, "item count", std::to_string(5976)) &&
           AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::guild)) >= 10000 &&
           !context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling();
}
