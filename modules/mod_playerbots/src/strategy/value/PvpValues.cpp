/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/PvpValues.cpp:179-214)
 * into the Legends of Azeroth Pandaria fork, issue #272.
 */

#include "PvpValues.h"

// DatabaseEnv.h must precede BattlegroundMgr.h: it pulls in BattlegroundQueue.h -> RatedPvp.h, which uses
// CharacterDatabaseTransaction without declaring it (same note as strategy/triggers/PvpTriggers.cpp:17-18).
#include "DatabaseEnv.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "Creature.h"
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "TravelMgr.h"

namespace
{
    // AC RandomPlayerbotMgr::getBattleMastersCache() (Bot/RandomPlayerbotMgr.cpp:837-890, LoadBattleMastersCache)
    // is a map<TeamId, map<BattlegroundTypeId, vector<entry>>> built from `battlemaster_entry`. MoP's cache is
    // private and shaped differently (Manager/RandomPlayerbotMgr.cpp:1463-1520, spawn guids, no TEAM_NEUTRAL), so
    // the same membership is expressed here file-locally: "entry is in cache[TEAM_NEUTRAL][bgTypeId] or
    // cache[botTeam][bgTypeId]".
    //  - "entry is in battlemaster_entry for bgTypeId": BattlegroundMgr::GetBattleMasterBG(entry)
    //    (src/server/game/Battlegrounds/BattlegroundMgr.h:134, the core's own copy of that table) == bgTypeId.
    //    5.4.8 caveat: that getter returns BATTLEGROUND_WS for an entry that is NOT in the table, where AC's cache
    //    has no row at all; so the creature template must also carry UNIT_NPC_FLAG_BATTLEMASTER. A battlemaster-
    //    flagged template missing from battlemaster_entry therefore still counts as WS here (AC: never in cache).
    //  - bucket rules are AC's, line by line (RandomPlayerbotMgr.cpp:861-878): FactionTemplate -> faction id ->
    //    parent faction ("team" in AC's FactionEntry == ParentFactionID on 5.4.8, DBCStructure.h:1244):
    //    parent 891 or faction 189 -> Alliance, parent 892 or faction 66 -> Horde, else Neutral.
    //    AC dereferences the DBC lookups unchecked; here a missing row just means "not usable".
    bool IsInBattleMastersCache(uint32 entry, BattlegroundTypeId bgTypeId, TeamId botTeam)
    {
        CreatureTemplate const* bmaster = sObjectMgr->GetCreatureTemplate(entry);
        if (!bmaster || !(bmaster->npcflag & UNIT_NPC_FLAG_BATTLEMASTER))
            return false;

        if (sBattlegroundMgr->GetBattleMasterBG(entry) != bgTypeId)
            return false;

        FactionTemplateEntry const* bmFaction = sFactionTemplateStore.LookupEntry(bmaster->faction);
        if (!bmFaction)
            return false;

        uint32 bmFactionId = bmFaction->faction;
        FactionEntry const* bmParentFaction = sFactionStore.LookupEntry(bmFactionId);
        if (!bmParentFaction)
            return false;

        uint32 bmParentTeam = bmParentFaction->ParentFactionID;
        TeamId bmTeam = TEAM_NEUTRAL;
        if (bmParentTeam == 891)
            bmTeam = TEAM_ALLIANCE;

        if (bmFactionId == 189)
            bmTeam = TEAM_ALLIANCE;

        if (bmParentTeam == 892)
            bmTeam = TEAM_HORDE;

        if (bmFactionId == 66)
            bmTeam = TEAM_HORDE;

        return bmTeam == TEAM_NEUTRAL || bmTeam == botTeam;
    }
}

// AC PvpValues.cpp:179-214. 5.4.8 note: the loop range 1..MAX_BATTLEGROUND_QUEUE_TYPES-1 is AC's, but the enum
// differs (core SharedDefines.h:4318-4339: AV,WS,AB,EY,SA,IC,TP,BFG,TOK,SM,DG,RB,2v2,3v3,5v5,RATED_BG,SOLO = 17
// types vs AC's 10 up to 5v5 within 20). The extra MoP queue types resolve through BGTemplateId to MoP BGs
// (TP/BFG/TOK/SM/DG/RB) or to AA/RATED_10_VS_10, whose templates either do not exist (skipped by !bg) or have no
// battlemaster in the cache, so only bgs whose battlemasters exist can ever be returned.
BattlegroundTypeId RpgBgTypeValue::Calculate()
{
    GuidPosition guidPosition = AI_VALUE(GuidPosition, "rpg target");

    if (guidPosition)
        for (uint32 i = 1; i < MAX_BATTLEGROUND_QUEUE_TYPES; i++)
        {
            BattlegroundQueueTypeId queueTypeId = (BattlegroundQueueTypeId)i;

            BattlegroundTypeId bgTypeId = sBattlegroundMgr->BGTemplateId(queueTypeId);

            Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            if (!bg)
                continue;

            if (bot->GetLevel() < bg->GetMinLevel())
                continue;

            // check if already in queue
            if (bot->InBattlegroundQueueForBattlegroundQueueType(queueTypeId))
                continue;

            // AC: sRandomPlayerbotMgr.getBattleMastersCache() [TEAM_NEUTRAL][bgTypeId] and [bot->GetTeamId()][bgTypeId]
            if (IsInBattleMastersCache(guidPosition.GetEntry(), bgTypeId, bot->GetTeamId()))
                return bgTypeId;
        }

    return BATTLEGROUND_TYPE_NONE;
}
