/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Trigger/PvpTriggers.cpp and
 * TimerBGTrigger from GenericTriggers.cpp) into the Legends of Azeroth Pandaria fork.
 */

#include "PvpTriggers.h"

// BattleGroundTactics.h documents its own include-order requirement (MovementActions.h, then
// DatabaseEnv.h, before any Battleground*.h) and satisfies it internally - pull it in before the
// DatabaseEnv.h block below so that still holds here. Needed for BGTactics::GetBotStrategyForTeam and
// AV_STRATEGY_BALANCED (AllianceNoSnowfallGY).
#include "BattleGroundTactics.h"

// DatabaseEnv.h must precede BattlegroundQueue.h: the latter pulls in RatedPvp.h, which uses
// CharacterDatabaseTransaction without declaring it.
#include "DatabaseEnv.h"

#include "Battleground.h"
#include "BattlegroundAV.h"
#include "BattlegroundDG.h"
#include "BattlegroundEY.h"
#include "BattlegroundMgr.h"
#include "BattlegroundQueue.h"
#include "BattlegroundTP.h"
#include "BattleGroundJoinAction.h"  // #702 P4 (#831): GetAssignedBgBracket for BgQueueDeadTrigger
#include "BgAnyLevelRequests.h"      // #702 P4 (#831): AiPlayerbot.BgAnyLevel (off by default)
#include "BattlegroundWS.h"
#include "GameObject.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"

// A hostile player is in range and selectable. The "enemy player target" value does the faction,
// PvP-flag and visibility filtering, so this only needs to ask whether it found anyone.
bool EnemyPlayerNear::IsActive() { return AI_VALUE(Unit*, "enemy player target"); }

bool BgWaitingTrigger::IsActive()
{
    if (!bot->InBattleground())
        return false;

    Battleground* bg = bot->GetBattleground();
    return bg && bg->GetStatus() == STATUS_WAIT_JOIN;
}

bool BgActiveTrigger::IsActive()
{
    if (!bot->InBattleground())
        return false;

    Battleground* bg = bot->GetBattleground();
    return bg && bg->GetStatus() == STATUS_IN_PROGRESS;
}

bool BgInviteActiveTrigger::IsActive()
{
    if (bot->InBattleground() || !bot->InBattlegroundQueue())
        return false;

    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        BattlegroundQueueTypeId queueTypeId = bot->GetBattlegroundQueueTypeId(i);
        if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
            continue;

        GroupQueueInfo ginfo;
        if (!sBattlegroundMgr->GetBattlegroundQueue(queueTypeId).GetPlayerGroupInfoData(bot->GetGUID(), &ginfo))
            continue;

        if (ginfo.IsInvitedToBGInstanceGUID && ginfo.RemoveInviteTime)
            return true;
    }

    return false;
}

bool BgQueueDeadTrigger::IsActive()
{
    if (!bot || bot->InBattleground() || !bot->InBattlegroundQueue())
        return false;

    for (uint8 queueSlot = 0; queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES; ++queueSlot)
    {
        BattlegroundQueueTypeId queueTypeId = bot->GetBattlegroundQueueTypeId(queueSlot);
        if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
            continue;

        // Arenas are match-made, not filled for a waiting real player, so the "queue cannot pop"
        // heuristic below does not apply to them. Since #437 IsSupportedBgQueue lets the 2v2/3v3/5v5
        // queues through and the census fills BattlegroundData for them, but an arena queue with no
        // running instance is a normal wait for an opponent team, not a dead queue - without this skip
        // every arena queue would drop itself (#389).
        if (BattlegroundMgr::BGArenaType(queueTypeId))
            continue;

        GroupQueueInfo ginfo;
        if (!sBattlegroundMgr->GetBattlegroundQueue(queueTypeId).GetPlayerGroupInfoData(bot->GetGUID(), &ginfo))
            continue;

        // An invitation is in flight - BGStatusAction is about to answer it.
        if (ginfo.IsInvitedToBGInstanceGUID)
            continue;

        BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
        Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
        if (!bg)
            continue;

        PvPDifficultyEntry const* pvpDiff = sDBCManager.GetBattlegroundBracketByLevel(bg->GetMapId(), bot->GetLevel());
        // #702 P4 (#831, off by default): a bot the level window put in another bracket (below its range, or a requester's
        // bracket above its own) waits in the bracket the queue assigned; judge that one, so it still leaves a dead queue.
        if (BgAnyLevel::Enabled())
            if (PvPDifficultyEntry const* assigned = GetAssignedBgBracket(bot, queueTypeId, bg->GetMapId()))
                pvpDiff = assigned;
        if (!pvpDiff)
            continue;

        BattlegroundInfo const info = sRandomPlayerbotMgr->GetBattlegroundInfo(queueTypeId, pvpDiff->GetBracketId());  // w24 hang

        // Nobody waiting and nothing running: this queue cannot pop.
        if (!info.activeBgQueue && !info.bgInstanceCount)
            return true;
    }

    return false;
}

bool PlayerHasFlag::IsActive() { return IsCapturingFlag(bot); }

bool PlayerHasFlag::IsCapturingFlag(Player* bot)
{
    if (!bot || !bot->InBattleground())
        return false;

    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    // Branch on the map, not on GetBattlegroundTypeId(): a battleground entered through the Random
    // Battleground queue keeps BATTLEGROUND_RB as its type id, so a type check would do nothing for
    // exactly the queue most players use.
    switch (bg->GetMapId())
    {
        case 489:  // Warsong Gulch
        case 726:  // Twin Peaks
        {
            bool const isTP = bg->GetMapId() == 726;
            uint32 const ownFlagObject = isTP ? uint32(BG_TP_OBJECT_A_FLAG) : uint32(BG_WS_OBJECT_A_FLAG);
            uint32 const enemyFlagObject = isTP ? uint32(BG_TP_OBJECT_H_FLAG) : uint32(BG_WS_OBJECT_H_FLAG);

            // GetFlagPickerGUID(team) is whoever is carrying THAT team's flag.
            for (uint8 carried = TEAM_ALLIANCE; carried <= TEAM_HORDE; ++carried)
            {
                if (bot->GetGUID() != bg->GetFlagPickerGUID(carried))
                    continue;

                TeamId const otherTeam = carried == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE;

                // The enemy is holding our flag too, so there is nothing to cap into yet. Only
                // report "capturing" once we are well away from our own base, so a carrier sitting
                // on the flag room can still turn and defend itself.
                if (bg->GetFlagPickerGUID(otherTeam))
                {
                    uint32 const ourFlagObject = carried == TEAM_ALLIANCE ? enemyFlagObject : ownFlagObject;
                    if (GameObject* go = bg->GetBGObject(ourFlagObject))
                        return bot->GetDistance(go) > 36.0f;
                }

                return true;  // enemy flag is home, we can run it in
            }

            return false;  // not carrying
        }
        case 1105:  // Deepwind Gorge (#798, no AC spec - the AC WSG branch above, Ai/Base/Trigger/PvpTriggers.cpp:130-162)
        {
            // Carrying a cart = being the picker of either team's cart (BattlegroundDG.h:319, the same per-team
            // picker table as WSG), or wearing a cart aura (BG_DG_ALLIANCE_MINE_CART / BG_DG_HORDE_MINE_CART, cast
            // in the same tick as the picker is set - BattlegroundDG.cpp:710-716/729-735).
            // Unlike WSG there is no "own flag is taken too" distance exception (AC :136-143): the core scores a
            // carried cart at the own cart stand whatever state the own cart is in (HandleAreaTrigger 9012/9013 test
            // m_flagState, which is always non-zero), so a carrier can run it in at once.
            if (bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_ALLIANCE) ||
                bot->GetGUID() == bg->GetFlagPickerGUID(TEAM_HORDE))
                return true;

            return bot->HasAura(BG_DG_ALLIANCE_MINE_CART) || bot->HasAura(BG_DG_HORDE_MINE_CART);
        }
        case 566:  // Eye of the Storm
        {
            if (bot->GetGUID() != bg->GetFlagPickerGUID())
                return false;

            BattlegroundEY const* ey = dynamic_cast<BattlegroundEY const*>(bg);
            if (!ey)
                return false;

            // The flag can only be capped at a base we hold; with none held the carrier is better
            // off fighting than running to nowhere.
            for (uint8 point = 0; point < EY_POINTS_MAX; ++point)
                if (ey->GetPointState(point) == EY_POINT_UNDER_CONTROL &&
                    ey->GetPointOwnedByTeam(point) == bot->GetBGTeam())
                    return true;

            return false;
        }
        default:
            return false;
    }
}

bool EnemyFlagCarrierNear::IsActive()
{
    Unit* carrier = AI_VALUE(Unit*, "enemy flag carrier");
    if (!carrier || bot->GetExactDist2d(carrier) > 100.0f)
        return false;

    // If an ordinary enemy is much closer than the carrier, deal with them instead of running past.
    if (Unit* nearbyEnemy = AI_VALUE(Unit*, "enemy player target"))
        if (bot->GetExactDist2d(nearbyEnemy) + 15.0f < bot->GetExactDist2d(carrier))
            return false;

    return true;
}

bool TeamFlagCarrierNear::IsActive()
{
    Battleground* bg = bot->GetBattleground();

    // With both flags off their bases nobody can cap, so escorting our carrier achieves nothing -
    // the flags have to be returned first.
    if (bg && (bg->GetMapId() == 489 || bg->GetMapId() == 726))
    {
        uint8 allianceState = 0;
        uint8 hordeState = 0;
        bool haveState = false;

        if (BattlegroundWS* ws = dynamic_cast<BattlegroundWS*>(bg))
        {
            allianceState = ws->GetFlagState(ALLIANCE);
            hordeState = ws->GetFlagState(HORDE);
            haveState = true;
        }
        else if (BattlegroundTP* tp = dynamic_cast<BattlegroundTP*>(bg))
        {
            allianceState = tp->GetFlagState(ALLIANCE);
            hordeState = tp->GetFlagState(HORDE);
            haveState = true;
        }

        if (haveState && allianceState != BG_WS_FLAG_STATE_ON_BASE && hordeState != BG_WS_FLAG_STATE_ON_BASE)
            return false;
    }

    Unit* carrier = AI_VALUE(Unit*, "team flag carrier");
    return carrier && bot->GetExactDist2d(carrier) <= 200.0f;
}

bool TimerBGTrigger::IsActive()
{
    time_t const now = time(nullptr);
    if (now - lastCheck < 60)
        return false;

    lastCheck = now;
    return true;
}

// #393, AC Ai/Base/Trigger/PvpTriggers.cpp:305-311 verbatim. "nearest vehicles" is registered on this fork
// (NearestVehiclesValue, used by VehicleActions.cpp).
bool VehicleNearTrigger::IsActive()
{
    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");
    return npcs.size();
}

bool InVehicleTrigger::IsActive() { return botAI->IsInVehicle(); }

bool AllianceNoSnowfallGY::IsActive()
{
    if (!bot)
        return false;

    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    // Branch on the map, not GetBattlegroundTypeId(): a battleground entered through the Random
    // Battleground queue keeps BATTLEGROUND_RB as its type id (same reasoning as PlayerHasFlag::IsCapturingFlag above).
    if (bg->GetMapId() != 30)  // Alterac Valley
        return false;

    // The BG team, not the race team (see BgTeamIdOf's comment in BattleGroundTactics.h): this core
    // assigns battleground sides independently of race, so a bot must be judged by which side it is
    // actually playing, not by Player::GetTeamId().
    if (bot->GetBGTeam() != ALLIANCE)
        return false;

    if (BGTactics::GetBotStrategyForTeam(bg, TEAM_ALLIANCE) != AV_STRATEGY_BALANCED)
        return false;

    if (bot->GetPositionX() <= -562.0f)
        return false;

    BattlegroundAV* av = dynamic_cast<BattlegroundAV*>(bg);
    if (!av)
        return false;

    BG_AV_NodeInfo const& snowfall = av->GetAVNodeInfo(BG_AV_NODES_SNOWFALL_GRAVE);
    // Active if the Snowfall Graveyard is NOT fully controlled by the Alliance. MoP's node owner is a
    // Team (ALLIANCE 469 / HORDE 67 / AV_NEUTRAL_TEAM 0), not AC's TeamId - the only owner-encoding
    // change from AC's trigger (Ai/Base/Trigger/PvpTriggers.cpp:313-336).
    return snowfall.Owner != ALLIANCE;
}
