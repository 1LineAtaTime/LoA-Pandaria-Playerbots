/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Action/NewRpgAction.cpp)
 * into the Legends of Azeroth Pandaria fork.
 */

#include "NewRpgAction.h"

#include "BotScenario.h"
#include "Creature.h"
#include "GameObject.h"
#include "Group.h"  // #201 (#612)
#include "LFGMgr.h"  // #201 (#612): sLFGMgr
#include "WaypointMovementGenerator.h"
#include "IVMapManager.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "NewRpgBaseAction.h"
#include "NewRpgDuelSpot.h"  // #201 (#612): sDuelSpotMgr
#include "BroadcastHelper.h"
#include "NewRpgStrategy.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "OutdoorPvP.h"
#include "OutdoorPvPMgr.h"
#include "PathGenerator.h"  // #399
#include "PlayerbotAI.h"
#include "PlayerbotTextMgr.h"  // #536
#include "Playerbots.h"
#include "Timer.h"
#include "WorldSession.h"
#include "NewRpgRoamTransport.h"  // #890 (beyond AC)

// #945 (BEYOND AC): town errands are indexed and picked in NewRpgBaseAction.cpp (namespace TownErrand); this file only
// seeds WANDER_NPC on the GO_CAMP arrival and does the work at the NPC.
namespace TownErrand
{
bool Enabled(Player* bot);
bool OnArrive(Player* bot, ObjectGuid& npc);
bool IsErrandNpc(Player* bot, ObjectGuid npc);
bool Expired(Player* bot);
uint32 MarkReached(Player* bot);
uint32 StayMs(Player* bot);
uint32 CurrentCat(Player* bot);
void Finish(Player* bot);
void Watch(Player* bot, NewRpgStatus status);
bool OnFlightLanded(Player* bot, WorldPosition& stand);  // #1004
}  // namespace TownErrand

// Arrival work at a vendor / repairer / profession trainer. Sell, buy and repair follow AC test-staging PR #2949
// (merged 2026-10-23, NewRpgWanderNpcAction::Execute): sell then buy when the NPC is a vendor and the bot has something
// to sell, repair at an armorer. The profession trainer teaches through the existing "trainer" action (TrainerAction).
// Runs for an errand target and for any WANDER_NPC target while RpgTownErrandEnable is on (0 = exactly AC).
static void TownErrandArrivalWork(PlayerbotAI* botAI, Player* bot, Creature* creature, bool errand)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    constexpr uint32 MIN_VENDOR_BALANCE = 1000;  // 10 silver (PR #2949)
    int64 const moneyBefore = bot->GetMoney();
    bool sold = false;
    bool repaired = false;
    bool trained = false;

    if (creature->HasNpcFlag(UNIT_NPC_FLAG_VENDOR) && context->GetValue<bool>("can sell")->Get())
    {
        sold = botAI->DoSpecificAction("sell", Event("sell", "vendor"), true);

        // Buying sorts the vendor list through the weight calculator; skip it for bots without money for cheap
        // consumables (PR #2949).
        if (bot->GetMoney() >= MIN_VENDOR_BALANCE)
            botAI->DoSpecificAction("buy", Event("buy", "vendor"), true);
    }

    if (creature->HasNpcFlag(UNIT_NPC_FLAG_REPAIR) && context->GetValue<bool>("can repair")->Get())
        repaired = botAI->DoSpecificAction("repair", Event("repair"), true);

    // Profession trainer: select it (TrainerAction::GetTarget reads the selection) and learn what the bot can afford.
    if (errand && (TownErrand::CurrentCat(bot) & 0x04) /* CAT_PROF */)
    {
        bot->SetSelection(creature->GetGUID());
        trained = botAI->DoSpecificAction("trainer", Event("trainer", "learn"), true);
    }

    bot->SetFacingToObject(creature);
    if (errand && urand(1, 100) <= 30)
        bot->HandleEmoteCommand(urand(0, 1) ? EMOTE_ONESHOT_TALK : EMOTE_ONESHOT_POINT);

    if (errand || sold || repaired)
        TC_LOG_DEBUG("playerbots", "[ERRAND] %s work at npc %u: sold=%d repaired=%d trainer=%d money %+lld c", bot->GetName().c_str(),
                     creature->GetEntry(), sold ? 1 : 0, repaired ? 1 : 0, trained ? 1 : 0,
                     (long long)(int64(bot->GetMoney()) - moneyBefore));
}

// #134 (flying bots): grind cells keep their z rounded to 50 yd (TravelMgr grind cells). A flying bot flies
// straight to that z and can end up under the terrain, where every later path fails and it freezes. So a
// flyer aims at the ground below the cell once it is within path range, and its arrival is measured in 2D.
// Latent upstream bug (AC has it too); fixed identically on both cores so 1:1 runs stay comparable.
static WorldPosition GrindMoveTarget(Player* bot, WorldPosition const& cell, float pathDis)
{
    if (!bot->IsFlying() || bot->GetExactDist2d(cell.GetPositionX(), cell.GetPositionY()) > pathDis)
        return cell;
    float const ground = bot->GetMapHeight(cell.GetPositionX(), cell.GetPositionY(), cell.GetPositionZ() + 25.0f, true, 50.0f);
    if (ground <= INVALID_HEIGHT)
        return cell;
    return WorldPosition(cell.GetMapId(), cell.GetPositionX(), cell.GetPositionY(), ground, cell.GetOrientation());
}

static float GrindArrivalDist(Player* bot, WorldPosition const& cell)
{
    return bot->IsFlying() ? bot->GetExactDist2d(cell.GetPositionX(), cell.GetPositionY()) : bot->GetExactDist(cell);
}

// MoP keeps quest progress per QuestObjective, but quest_poi.ObjectiveIndex keeps the 3.3.5 layout that AC's
// GetQuestPOIPosAndObjectiveIdx uses: creature/GO objectives are slots 0-3 and item objectives are
// QUEST_OBJECTIVES_COUNT (4) + item slot. StorageIndex counts within each type (a kill and an item objective can
// both be 0), so item objectives map to 4 + StorageIndex. Without this no item-objective POI ever matched and bots
// dropped every collect-item quest right after accepting it.
static int32 LegacyPoiIndex(QuestObjective const& objective)
{
    return objective.Type == QUEST_OBJECTIVE_ITEM ? 4 + int32(objective.StorageIndex) : int32(objective.StorageIndex);
}

// #536: AC Ai/World/Rpg/Action/NewRpgAction.cpp:58-260 ("rpg status" / "rpg do quest" chat commands), 1:1.
// 5.4.8 diffs: botAI->rpgInfo -> GetRpgInfo(); sPlayerbotAIConfig. -> ->; bot->Whisper takes (text, lang, receiver).
void TellRpgStatusAction::WhisperStatusChange(Player* owner, std::string const& statusName)
{
    std::string msg = PlayerbotTextMgr::instance().GetBotTextOrDefault(
        RPG_STATUS_CHANGED_KEY, RPG_STATUS_CHANGED_DEFAULT,
        {{"%status", statusName}});
    bot->Whisper(msg, LANG_UNIVERSAL, owner);
}

bool TellRpgStatusAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;

    std::string const text = event.getParam();
    if (text.empty())
    {
        std::string out = botAI->GetRpgInfo().ToString();
        bot->Whisper(out, LANG_UNIVERSAL, owner);
        return true;
    }

    Player* master = botAI->GetMaster();
    bool isMaster = master && master->GetGUID() == owner->GetGUID();
    bool isGM = owner->GetSession() && owner->GetSession()->GetSecurity() >= SEC_GAMEMASTER;
    if (!isMaster && !isGM)
    {
        std::string msg = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "rpg_debug_permission_error",
            "Only your master or a GM can change my rpg status.", {});
        bot->Whisper(msg, LANG_UNIVERSAL, owner);
        return false;
    }

    std::string name = text;
    uint32 questId = 0;
    static std::string const doQuestPrefix = "do quest ";
    size_t doQuestPos = text.find(doQuestPrefix);
    if (doQuestPos != std::string::npos)
    {
        name = "do quest";
        std::string idStr = text.substr(doQuestPos + doQuestPrefix.length());
        try
        {
            questId = static_cast<uint32>(std::stoul(idStr));
        }
        catch (std::exception const&)
        {
            questId = 0;
        }
    }

    NewRpgStatus status = NewRpgInfo::StatusFromString(name);
    NewRpgInfo& info = botAI->GetRpgInfo();

    if (status == RPG_IDLE)
    {
        info.ChangeToIdle();
        WhisperStatusChange(owner, "IDLE");
        return true;
    }
    else if (status == RPG_REST)
    {
        info.ChangeToRest();
        bot->SetStandState(UNIT_STAND_STATE_SIT);
        WhisperStatusChange(owner, "REST");
        return true;
    }
    else if (status == RPG_WANDER_RANDOM)
    {
        info.ChangeToWanderRandom();
        WhisperStatusChange(owner, "WANDER_RANDOM");
        return true;
    }
    else if (status == RPG_WANDER_NPC)
    {
        info.ChangeToWanderNpc();
        WhisperStatusChange(owner, "WANDER_NPC");
        return true;
    }
    else if (status == RPG_GO_GRIND)
    {
        WorldPosition pos = SelectRandomGrindPos(bot);
        if (pos == WorldPosition())
        {
            std::string msg = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "rpg_no_grind_pos_error", "No grind position available.", {});
            bot->Whisper(msg, LANG_UNIVERSAL, owner);
            return false;
        }
        info.ChangeToGoGrind(pos);
        WhisperStatusChange(owner, "GO_GRIND");
        return true;
    }
    else if (status == RPG_GO_CAMP)
    {
        WorldPosition pos = SelectRandomCampPos(bot);
        if (pos == WorldPosition())
        {
            std::string msg = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "rpg_no_camp_pos_error", "No camp position available.", {});
            bot->Whisper(msg, LANG_UNIVERSAL, owner);
            return false;
        }
        info.ChangeToGoCamp(pos);
        WhisperStatusChange(owner, "GO_CAMP");
        return true;
    }
    else if (status == RPG_TRAVEL_FLIGHT)
    {
        uint32 flightMasterEntry = 0;
        WorldPosition flightMasterPos;
        std::vector<uint32> path;
        if (!SelectRandomFlightTaxiNode(flightMasterEntry, flightMasterPos, path))
        {
            std::string msg = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "rpg_no_flight_path_error", "No flight path available.", {});
            bot->Whisper(msg, LANG_UNIVERSAL, owner);
            return false;
        }
        info.ChangeToTravelFlight(flightMasterEntry, flightMasterPos, std::move(path));
        WhisperStatusChange(owner, "TRAVEL_FLIGHT");
        return true;
    }
    else if (status == RPG_OUTDOOR_PVP)
    {
        info.ChangeToOutdoorPvp();
        WhisperStatusChange(owner, "OUTDOOR_PVP");
        return true;
    }
    else if (status == RPG_DUEL_SPOT)  // #201 (#612), as on the AC test tree (NewRpgAction.cpp:185-196)
    {
        int32 spotIdx = sDuelSpotMgr->SelectSpotFor(bot);
        if (spotIdx < 0)
        {
            bot->Whisper("No duel spot available.", LANG_UNIVERSAL, owner);
            return false;
        }
        info.ChangeToDuelSpot(static_cast<uint32>(spotIdx));
        WhisperStatusChange(owner, "DUEL_SPOT");
        return true;
    }
    else if (status == RPG_DO_QUEST)
    {
        if (!questId)
        {
            for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
            {
                uint32 qid = bot->GetQuestSlotQuestId(slot);
                if (!qid)
                    continue;
                std::vector<POIInfo> poi;
                if (GetQuestPOIPosAndObjectiveIdx(qid, poi, true))
                {
                    questId = qid;
                    break;
                }
            }
        }
        if (!questId)
        {
            std::string msg = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "rpg_no_quest_error", "No quest available; use 'do quest <id>'.", {});
            bot->Whisper(msg, LANG_UNIVERSAL, owner);
            return false;
        }
        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        QuestStatus questStatus = bot->GetQuestStatus(questId);
        if (!quest || (questStatus != QUEST_STATUS_INCOMPLETE && questStatus != QUEST_STATUS_COMPLETE))
        {
            std::string msg = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "rpg_invalid_quest_error", "Invalid quest %quest_id",
                {{"%quest_id", std::to_string(questId)}});
            bot->Whisper(msg, LANG_UNIVERSAL, owner);
            return false;
        }
        info.ChangeToDoQuest(questId, quest);
        WhisperStatusChange(owner, "DO_QUEST " + std::to_string(questId));
        return true;
    }

    std::string msg = PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "rpg_unknown_status_error",
        "Unknown rpg status. Options: idle, rest, wander random, wander npc, "
        "go grind, go camp, do quest [<id>], travel flight, outdoor pvp.", {});
    bot->Whisper(msg, LANG_UNIVERSAL, owner);
    return false;
}

bool StartRpgDoQuestAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;

    std::string const text = event.getParam();
    PlayerbotChatHandler ch(owner);
    uint32 questId = ch.extractQuestId(text);
    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (quest)
    {
        botAI->GetRpgInfo().ChangeToDoQuest(questId, quest);
        bot->Whisper("Start to do quest " + std::to_string(questId), LANG_UNIVERSAL, owner);
        return true;
    }
    bot->Whisper("Invalid quest " + text, LANG_UNIVERSAL, owner);
    return false;
}

bool NewRpgStatusUpdateAction::Execute(Event /*event*/)
{
    NewRpgInfo& info = botAI->GetRpgInfo();
    // #201 (#615): a seeded bot is handed to RPG_DUEL_SPOT first, before the scenario lock (SPEC 2).
    {
        uint32 pendingSpot = 0;
        if (sDuelSpotMgr->TakePending(bot->GetGUID(), pendingSpot))
        {
            info.ChangeToDuelSpot(pendingSpot, true);
            return true;
        }
    }
    // .botscenario pins the status it set until the scenario arrives or times out.
    if (BotScenario::IsStatusLocked(bot))
        return false;
    NewRpgStatus status = info.GetStatus();
    // #945: an errand whose status left GO_CAMP / WANDER_NPC ends here, with its reason logged.
    TownErrand::Watch(bot, status);
    switch (status)
    {
        case RPG_IDLE:
            return RandomChangeStatus({RPG_GO_CAMP, RPG_GO_GRIND, RPG_WANDER_RANDOM, RPG_WANDER_NPC, RPG_DO_QUEST,
                                       RPG_TRAVEL_FLIGHT, RPG_REST, RPG_OUTDOOR_PVP, RPG_DUEL_SPOT});

        case RPG_GO_GRIND:
        {
            auto* data = std::get_if<NewRpgInfo::GoGrind>(&info.data);
            if (!data)
                return false;
            // GO_GRIND -> WANDER_RANDOM
            if (GrindArrivalDist(bot, data->pos) < 10.0f)
            {
                info.ChangeToWanderRandom();
                return true;
            }
            break;
        }
        case RPG_GO_CAMP:
        {
            auto* data = std::get_if<NewRpgInfo::GoCamp>(&info.data);
            if (!data)
                return false;
            // GO_CAMP -> WANDER_NPC
            if (bot->GetExactDist(data->pos) < 10.0f)
            {
                info.ChangeToWanderNpc();
                // #945 (BEYOND AC): a town errand walked here; make its NPC the WANDER_NPC target so the bot interacts.
                ObjectGuid errandNpc;
                if (TownErrand::Enabled(bot) && TownErrand::OnArrive(bot, errandNpc))
                {
                    if (auto* wander = std::get_if<NewRpgInfo::WanderNpc>(&info.data))
                    {
                        wander->npcOrGo = errandNpc;
                        wander->lastReach = 0;
                    }
                }
                return true;
            }
            break;
        }
        case RPG_WANDER_RANDOM:
        {
            // WANDER_RANDOM -> IDLE
            if (info.HasStatusPersisted(statusWanderRandomDuration))
            {
                info.ChangeToIdle();
                return true;
            }
            break;
        }
        case RPG_WANDER_NPC:
        {
            if (info.HasStatusPersisted(statusWanderNpcDuration))
            {
                info.ChangeToIdle();
                return true;
            }
            break;
        }
        case RPG_DO_QUEST:
        {
            // DO_QUEST -> IDLE
            if (info.HasStatusPersisted(statusDoQuestDuration))
            {
                info.ChangeToIdle();
                return true;
            }
            break;
        }
        case RPG_TRAVEL_FLIGHT:
        {
            auto* data = std::get_if<NewRpgInfo::TravelFlight>(&info.data);
            if (!data)
                return false;

            if (data->inFlight && !bot->IsInFlight())
            {
                // flight arrival
                // #1004 (BEYOND AC): a town errand's flight continues as the GO_CAMP walk to its NPC
                WorldPosition errandStand;
                if (TownErrand::Enabled(bot) && TownErrand::OnFlightLanded(bot, errandStand))
                {
                    info.ChangeToGoCamp(errandStand);
                    return true;
                }
                info.ChangeToIdle();
                return true;
            }
            break;
        }
        case RPG_REST:
        {
            // REST -> IDLE
            if (info.HasStatusPersisted(statusRestDuration))
            {
                info.ChangeToIdle();
                return true;
            }
            break;
        }
        case RPG_OUTDOOR_PVP:
        {
            if (info.HasStatusPersisted(statusOutDoorPvPDuration))
            {
                info.ChangeToIdle();
                return true;
            }
            break;
        }
        case RPG_DUEL_SPOT:
        {
            // #201 (#612): DUEL_SPOT -> IDLE (AC NewRpgAction.cpp:343-350 outdoor-pvp pattern). A seeded bot stays until
            // the spot is disabled, it joins a BG / arena / queue, or it is grouped with a real player.
            auto* data = std::get_if<NewRpgInfo::DuelSpot>(&info.data);
            if (!data)
                return false;
            bool leave = (!data->seeded && info.HasStatusPersisted(statusDuelSpotDuration)) ||
                         data->spotIndex >= sPlayerbotAIConfig->duelSpots.size() || !sPlayerbotAIConfig->duelSpotEnable ||
                         bot->InBattleground() || bot->InArena() || bot->InBattlegroundQueue() ||
                         sLFGMgr->GetActiveState(bot->GetGUID()) != lfg::LFG_STATE_NONE;
            if (!leave)
            {
                if (Group* group = bot->GetGroup())
                {
                    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
                    {
                        Player* member = ref->GetSource();
                        if (!member || member == bot)
                            continue;
                        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
                        if (!memberAI || memberAI->IsRealPlayer())
                        {
                            leave = true;
                            break;
                        }
                    }
                }
            }
            if (leave)
            {
                // #201 (wave 30): why a seeded bot left its spot (t8 saw seeded bots back on RPG statuses minutes later)
                if (data->seeded)
                    TC_LOG_DEBUG("playerbots",
                                 "[DUELSPOT] leave %s spot=%u bg=%d arena=%d bgq=%d lfg=%d disabled=%d badidx=%d",
                                 bot->GetName().c_str(), data->spotIndex, bot->InBattleground() ? 1 : 0,
                                 bot->InArena() ? 1 : 0, bot->InBattlegroundQueue() ? 1 : 0,
                                 sLFGMgr->GetActiveState(bot->GetGUID()) != lfg::LFG_STATE_NONE ? 1 : 0,
                                 sPlayerbotAIConfig->duelSpotEnable ? 0 : 1,
                                 data->spotIndex >= sPlayerbotAIConfig->duelSpots.size() ? 1 : 0);
                info.ChangeToIdle();
                sDuelSpotMgr->Leave(bot->GetGUID());
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

bool NewRpgGoGrindAction::Execute(Event /*event*/)
{
    // #890 (BEYOND AC): no quest-giver detour while the bot's roam trip holds it for a transport leg - the detour to an NPC on
    // the Orgrimmar tower walked the waiting bot up and down the ramp and reset its dock wait (wait-again x12).
    if (!RoamTransport::HoldsForTransportLeg(bot) && SearchQuestGiverAndAcceptOrReward())
        return true;

    if (auto* data = std::get_if<NewRpgInfo::GoGrind>(&botAI->GetRpgInfo().data))
    {
        if (MoveFarTo(GrindMoveTarget(bot, data->pos, pathFinderDis)))
            return true;
        // Small nudge so the next tick's MoveFarTo starts from a
        // slightly different position. Kept small so it doesn't look
        // like the bot is abandoning its destination.
        return MoveRandomNear(10.0f);
    }

    return false;
}

bool NewRpgGoCampAction::Execute(Event /*event*/)
{
    // #890 (BEYOND AC): no quest-giver detour while the bot's roam trip holds it for a transport leg - the detour to an NPC on
    // the Orgrimmar tower walked the waiting bot up and down the ramp and reset its dock wait (wait-again x12).
    if (!RoamTransport::HoldsForTransportLeg(bot) && SearchQuestGiverAndAcceptOrReward())
        return true;

    if (auto* data = std::get_if<NewRpgInfo::GoCamp>(&botAI->GetRpgInfo().data))
    {
        if (MoveFarTo(data->pos))
            return true;
        return MoveRandomNear(10.0f);
    }

    return false;
}

namespace
{
// MoP 5.4.8 ships no src/server/game/Maps/AreaDefines.h, so AC's AREA_NAGRAND is not available
// by name; 3518 is the Nagrand zone id (Halaa, OutdoorPvPNA).
constexpr uint32 AREA_NAGRAND = 3518;
} // namespace

bool NewRpgOutdoorPvpAction::Execute(Event /*event*/)
{
    if (!bot->IsPvP())
    {
        botAI->GetRpgInfo().ChangeToIdle();
        return false;
    }
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL) || !bot->IsOutdoorPvPActive())
        return false;

    uint32 zoneId = bot->GetZoneId();
    OutdoorPvP* outdoorPvP = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(zoneId);
    if (!outdoorPvP || zoneId == AREA_NAGRAND)
    {
        botAI->GetRpgInfo().ChangeToIdle();
        return false;
    }

    OutdoorPvP::OPvPCapturePointMap const& capturePointMap = outdoorPvP->GetCapturePoints();

    NewRpgInfo& info = botAI->GetRpgInfo();
    auto* dataPtr = std::get_if<NewRpgInfo::OutdoorPvP>(&info.data);
    if (!dataPtr)
        return false;
    auto& data = *dataPtr;
    // Re-resolve stored lowguid from the capture point map each tick (avoids dangling pointers).
    // MoP keys OPvPCapturePointMap by that same lowguid, so the stored id is the map key itself.
    OPvPCapturePoint* objective = nullptr;
    if (data.capturePointSpawnId && !capturePointMap.empty())
    {
        auto it = capturePointMap.find(data.capturePointSpawnId);
        if (it != capturePointMap.end())
        {
            OPvPCapturePoint* capturePoint = it->second;
            if (capturePoint && capturePoint->m_capturePoint)
            {
                float threshold = capturePoint->GetMinValue();
                float slider = capturePoint->GetSlider();
                TeamId faction = bot->GetTeamId();
                TC_LOG_DEBUG("playerbots", "[New RPG] Bot %s with faction %u is evaluating existing RPG objective %s with threshold %f and slider value %f", bot->GetName().c_str(), uint32(faction), capturePoint->m_capturePoint->GetName().c_str(), threshold, slider);
                if ((faction == TEAM_HORDE && slider >= -threshold) ||
                    (faction == TEAM_ALLIANCE && slider <= threshold))
                    objective = capturePoint;
            }
        }
        if (!objective)
            data.capturePointSpawnId = 0;
    }

    if (!objective)
    {
        objective = SelectNewObjective(capturePointMap);
        if (!objective)
        {
            botAI->GetRpgInfo().ChangeToIdle();
            return true;
        }
        // AC reads objective->m_capturePointSpawnId here; MoP carries the same lowguid on
        // m_capturePointGUID, whose counter is what the map is keyed by.
        data.capturePointSpawnId = objective->m_capturePointGUID.GetCounter();
        TC_LOG_DEBUG("playerbots", "[New RPG] Bot %s selected outdoor PvP target capturePointSpawnId %u", bot->GetName().c_str(), uint32(data.capturePointSpawnId));
    }

    GameObject* objectiveGO = objective->m_capturePoint;
    if (!objectiveGO)
        return false;

    if (objectiveGO->GetGoType() != GAMEOBJECT_TYPE_CAPTURE_POINT)
        return false;

    float radius = objectiveGO->GetGOInfo()->capturePoint.radius / 2.0f;
    if (!objectiveGO->IsWithinDistInMap(bot, radius))
        return MoveFarTo(WorldPosition(objectiveGO));

    return PatrolCapturePoint(objectiveGO, radius);
}

OPvPCapturePoint* NewRpgOutdoorPvpAction::SelectNewObjective(OutdoorPvP::OPvPCapturePointMap const& capturePointMap)
{
    OPvPCapturePoint* objective = nullptr;
    TeamId faction = bot->GetTeamId();
    std::vector<OPvPCapturePoint*> candidateObjectives;

    if (capturePointMap.empty())
    {
        botAI->GetRpgInfo().ChangeToIdle();
        return objective;
    }
    for (auto const& [lowguid, point] : capturePointMap)
    {
        GameObject* capturePointObject = point->m_capturePoint;
        if (!capturePointObject)
            continue;

        float threshold = point->GetMinValue();
        float slider = point->GetSlider();
        if (faction == TEAM_HORDE && slider > -threshold)
            candidateObjectives.push_back(point);
        else if (faction == TEAM_ALLIANCE && slider < threshold)
            candidateObjectives.push_back(point);
    }
    if (candidateObjectives.empty())
    {
        TC_LOG_DEBUG("playerbots", "[New RPG] Bot %s found no valid outdoor PvP objectives to capture", bot->GetName().c_str());
        botAI->GetRpgInfo().ChangeToIdle();
        return objective;
    }
    int randomIndex = urand(0, candidateObjectives.size() - 1);
    objective = candidateObjectives[randomIndex];
    return objective;
}

bool NewRpgOutdoorPvpAction::PatrolCapturePoint(GameObject* objectiveGO, float radius)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL))
        return false;

    // Randomly pause at the current spot before picking a new patrol point
    if (urand(0, 2) == 0)
        return ForceToWait(urand(3000, 6000));

    float patrolRadius = radius * 0.8f;

    // #399: AC calls MoveRandomNear(patrolRadius, MOVEMENT_NORMAL, objectiveGO) (NewRpgOutdoorPvP.cpp:130), but its
    // MoveRandomNear never reads that centre (NewRpgBaseAction.cpp:243, unused since upstream #2217): it hops around
    // the bot and validates each 13-32 yd hop as ONE straight segment with Map::CanReachPositionAndGetValidCoords, whose
    // slope test (PathGenerator::IsWalkableClimb, Map.cpp:4629) refuses any segment that rises or falls more than about
    // the bot's collision height (~2 yd). On the hill of a Hellfire tower that refuses nearly every hop (the one hop
    // made rose 1.3 yd), so the bot waited for good (MoP, Stadium, 2026-10-03: 1 hop in 120 s).
    // Extending AC's intent (the centre it already passes): sample the hop around the capture point itself, within
    // patrolRadius, and validate the navmesh path the bot will actually walk segment by segment instead of one
    // straight line. Same guards as MoveRandomNear (path type, reachability, water, the #61 longest-short-hop
    // fallback); the destination always stays inside the capture radius.
    Map* map = bot->GetMap();
    if (!map)
        return ForceToWait(urand(3000, 6000));

    float const cx = objectiveGO->GetPositionX();
    float const cy = objectiveGO->GetPositionY();
    float const cz = objectiveGO->GetPositionZ();
    float const minHop = 0.25f * patrolRadius;
    float bestHop = 0.0f, bestX = 0.0f, bestY = 0.0f, bestZ = 0.0f;
    uint32 rejPath = 0, rejReach = 0, rejWater = 0, rejOut = 0;
    for (int attempt = 0; attempt < 8; ++attempt)
    {
        float distance, angle;
        // Deterministic scenarios (HARNESS-SPEC-botscenario-v2 G3.4) get the same golden-angle sweep as MoveRandomNear.
        if (!BotScenario::NextRandomNearAngle(bot, patrolRadius, angle, distance))
        {
            distance = static_cast<float>(rand_norm()) * patrolRadius;  // uniform in distance, like the duel-spot patrol
            angle = static_cast<float>(rand_norm()) * 2.0f * static_cast<float>(M_PI);
        }
        float dx = cx + distance * std::cos(angle);
        float dy = cy + distance * std::sin(angle);
        float dz = cz;
        float const h = map->GetHeight(bot->GetPhaseMask(), dx, dy, cz + 10.0f);
        if (h > INVALID_HEIGHT && std::fabs(h - cz) <= 30.0f)
            dz = h + 0.2f;

        PathGenerator path(bot);
        path.CalculatePath(dx, dy, dz);
        uint32 const typeOk = PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_FARFROMPOLY;
        if ((path.GetPathType() & ~typeOk) || path.GetPath().empty())
        {
            ++rejPath;
            continue;
        }

        // Walk the path's own ~4 yd smooth-path segments (SMOOTH_PATH_STEP_SIZE) through the same per-segment check
        // MoveRandomNear applies to its one straight segment; a refused segment ends the hop at the last one that passed.
        // Done here rather than with the Map.cpp:4547 path overload: that one also tests the degenerate first segment
        // (bot position -> first path point, same x/y), where getSlopeAngle divides by a zero floor distance and the
        // slope test fails.
        bool fullPath = true;
        float px = bot->GetPositionX(), py = bot->GetPositionY(), pz = bot->GetPositionZ();
        for (G3D::Vector3 const& point : path.GetPath())
        {
            float sx = point.x, sy = point.y, sz = point.z;
            if ((sx - px) * (sx - px) + (sy - py) * (sy - py) < 0.25f)
                continue;
            if (!map->CanReachPositionAndGetValidCoords(bot, px, py, pz, sx, sy, sz))
            {
                fullPath = false;
                break;
            }
            px = sx;
            py = sy;
            pz = sz;
        }
        dx = px;
        dy = py;
        dz = pz;
        if (map->IsInWater(bot->GetPhaseMask(), dx, dy, dz))
        {
            ++rejWater;
            continue;
        }
        if (objectiveGO->GetExactDist2d(dx, dy) > radius)
        {
            ++rejOut;
            continue;
        }

        float const hop = bot->GetExactDist2d(dx, dy);
        if (!fullPath || hop < minHop)
        {
            if (!fullPath)
                ++rejReach;
            if (hop >= 3.0f && hop > bestHop)
            {
                bestHop = hop;
                bestX = dx;
                bestY = dy;
                bestZ = dz;
            }
            continue;
        }

        if (MoveTo(bot->GetMapId(), dx, dy, dz, false, false, false, true, MovementPriority::MOVEMENT_NORMAL))
        {
            BotScenario::LogTarget(bot, "rnd", nullptr, dx, dy, dz, patrolRadius);
            return true;
        }
    }

    if (bestHop > 0.0f && MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true, MovementPriority::MOVEMENT_NORMAL))
    {
        BotScenario::LogTarget(bot, "rnd", nullptr, bestX, bestY, bestZ, patrolRadius);
        return true;
    }

    if (sPlayerbotAIConfig->debugBotNames.count(bot->GetName()) || BotScenario::WantsTrail(bot))
        TC_LOG_DEBUG("playerbots", "[OPVP] %s patrol: no hop around %s (%.1f,%.1f,%.1f) r=%.1f from (%.1f,%.1f,%.1f) rej path=%u reach=%u water=%u out=%u",
                     bot->GetName().c_str(), objectiveGO->GetName().c_str(), cx, cy, cz, patrolRadius, bot->GetPositionX(),
                     bot->GetPositionY(), bot->GetPositionZ(), rejPath, rejReach, rejWater, rejOut);

    return ForceToWait(urand(3000, 6000));
}

bool NewRpgWanderRandomAction::Execute(Event /*event*/)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;

    return MoveRandomNear();
}

bool NewRpgWanderNpcAction::Execute(Event /*event*/)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;

    NewRpgInfo& info = botAI->GetRpgInfo();
    auto* dataPtr = std::get_if<NewRpgInfo::WanderNpc>(&info.data);
    if (!dataPtr)
        return false;

    auto& data = *dataPtr;

    if (!data.npcOrGo)
    {
        ObjectGuid npcOrGo = ChooseNpcOrGameObjectToInteract();
        if (npcOrGo.IsEmpty())
        {
            // No npc can be found, switch to IDLE
            BotScenario::LogTarget(bot, "none", nullptr, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            info.ChangeToIdle();
            return true;
        }

        data.npcOrGo = npcOrGo;
        data.lastReach = 0;
        if (WorldObject* s = ObjectAccessor::GetWorldObject(*bot, npcOrGo))
            BotScenario::LogTarget(bot, "npc", s, s->GetPositionX(), s->GetPositionY(), s->GetPositionZ());
        return true;
    }

    bool const debugBot = !sPlayerbotAIConfig->debugBotNames.empty() && sPlayerbotAIConfig->debugBotNames.count(bot->GetName());

    WorldObject* object = ObjectAccessor::GetWorldObject(*bot, data.npcOrGo);
    bool const within = object && IsWithinInteractionDist(object);

    // #945: the bot never reached its errand NPC (blocked path, NPC gone): drop the errand and go idle. Tested only while
    // the NPC is out of reach - 2026-10-28: Alaindo and Nethol arrived 4.0 yd from their NPC, slept >180 s of throttled
    // ticks and the first tick after gave up "timeout (reach)" standing at it, because this ran before the reach test.
    if (!within && TownErrand::Expired(bot))
    {
        info.ChangeToIdle();
        return true;
    }

    if (debugBot)
        TC_LOG_DEBUG("playerbots", "[RPGNPC] %s npc=%s object=%d withinDist=%d waitingMove=%d dist=%.1f",
                     bot->GetName().c_str(), data.npcOrGo.ToString().c_str(), object ? 1 : 0, within ? 1 : 0,
                     IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL) ? 1 : 0,
                     object ? bot->GetExactDist(object) : -1.0f);

    if (within)
    {
        if (!data.lastReach)
        {
            data.lastReach = getMSTime();
            if (bot->CanInteractWithQuestGiver(object))
                InteractWithNpcOrGameObjectForQuest(data.npcOrGo);

            // #945 (BEYOND AC) + AC test-staging PR #2949: sell / buy / repair at a vendor or repairer, profession
            // trainer learn on an errand. Off (exactly AC) when RpgTownErrandEnable = 0 or in a deterministic scenario.
            if (TownErrand::Enabled(bot))
            {
                if (Creature* creature = object->ToCreature())
                {
                    bool const errandNpc = TownErrand::IsErrandNpc(bot, data.npcOrGo);
                    if (errandNpc)
                        TownErrand::MarkReached(bot);
                    TownErrandArrivalWork(botAI, bot, creature, errandNpc);
                }
                else if (object->GetTypeId() == TYPEID_GAMEOBJECT && TownErrand::IsErrandNpc(bot, data.npcOrGo))
                {
                    // mailbox errand (CAT_MAILBOX 0x100): a pure visit, stay StayMin..Max. AC's "check mail" action is not
                    // called: AC never schedules it (its trigger is commented out), and it bounces real players' mails to
                    // the bot (returns the items, deletes the mail) - a visible behaviour AC does not have.
                    TownErrand::MarkReached(bot);
                    bot->SetFacingToObject(object);
                    TC_LOG_DEBUG("playerbots", "[ERRAND] %s work at mailbox %u: mails=%u", bot->GetName().c_str(),
                                 object->GetEntry(), uint32(bot->GetMailSize()));
                }
            }
            return true;
        }

        // #945: an errand stays at its NPC for its own time (RpgTownErrandStayMin..Max for the pure visits), then the bot
        // goes on with AC's own WANDER_NPC rotation in that town (next NPC below) until the status's AC timer
        // (statusWanderNpcDuration, started at the GO_CAMP arrival) ends it. #947 (2026-10-28): going IDLE here cut AC's
        // town window short once errands finished in 1-2 min (capital bots 100 -> 68 in 1.5 h at 1000 bots).
        if (TownErrand::IsErrandNpc(bot, data.npcOrGo))
        {
            if (GetMSTimeDiffToNow(data.lastReach) < TownErrand::StayMs(bot))
                return false;
            TownErrand::Finish(bot);
            data.npcOrGo = ObjectGuid::Empty;
            data.lastReach = 0;
            return true;
        }

        if (GetMSTimeDiffToNow(data.lastReach) < npcStayTime)
            return false;

        // has stayed at this npc for more than `npcStayTime`, select the next target
        data.npcOrGo = ObjectGuid::Empty;
        data.lastReach = 0;
    }
    else
    {
        if (MoveWorldObjectTo(data.npcOrGo))
        {
            if (debugBot)
                TC_LOG_DEBUG("playerbots", "[RPGNPC] %s MoveWorldObjectTo OK", bot->GetName().c_str());
            return true;
        }

        // NPC pathing failed (random offset in a wall, mmap hiccup, etc).
        // Take a small random step so the next tick retries from a
        // different spot instead of staring at the NPC from afar.
        bool const moved = MoveRandomNear(15.0f);
        if (debugBot)
            TC_LOG_DEBUG("playerbots", "[RPGNPC] %s MoveWorldObjectTo FAILED, MoveRandomNear=%d", bot->GetName().c_str(), moved ? 1 : 0);
        return moved;
    }

    return true;
}

bool NewRpgDoQuestAction::Execute(Event /*event*/)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;

    NewRpgInfo& info = botAI->GetRpgInfo();
    auto* dataPtr = std::get_if<NewRpgInfo::DoQuest>(&info.data);
    if (!dataPtr)
        return false;

    switch (bot->GetQuestStatus(dataPtr->questId))
    {
        case QUEST_STATUS_INCOMPLETE:
            return DoIncompleteQuest(*dataPtr);
        case QUEST_STATUS_COMPLETE:
            return DoCompletedQuest(*dataPtr);
        default:
            break;
    }

    info.ChangeToIdle();
    return true;
}

bool NewRpgDoQuestAction::DoIncompleteQuest(NewRpgInfo::DoQuest& data)
{
    uint32 const questId = data.questId;
    Quest const* quest = data.quest ? data.quest : sObjectMgr->GetQuestTemplate(questId);
    if (!quest)
    {
        botAI->GetRpgInfo().ChangeToIdle();
        return true;
    }

    // On MoP, objective progress is per QuestObjective rather than the 3.3.5
    // CreatureOrGOCount/ItemCount arrays; match the POI's objective index against
    // QuestObjective::StorageIndex.
    auto objectiveComplete = [this, quest](int32 storageIndex) -> bool
    {
        for (QuestObjective const& objective : quest->GetObjectives())
            if (LegacyPoiIndex(objective) == storageIndex)
                return bot->IsQuestObjectiveComplete(quest, objective);

        return true;
    };

    auto objectiveHasProgress = [this, quest](int32 storageIndex) -> bool
    {
        for (QuestObjective const& objective : quest->GetObjectives())
            if (LegacyPoiIndex(objective) == storageIndex)
                return bot->GetQuestObjectiveCounter(objective.ID) != 0;

        return false;
    };

    if (data.pos != WorldPosition())
    {
        // the current objective is completed, clear it and pick a new one below
        if (objectiveComplete(data.objectiveIdx))
        {
            data.lastReachPOI = 0;
            data.pos = WorldPosition();
            data.objectiveIdx = 0;
        }
    }

    if (data.pos == WorldPosition())
    {
        std::vector<POIInfo> poiInfo;
        if (!GetQuestPOIPosAndObjectiveIdx(questId, poiInfo))
        {
            // can't find a poi pos to go, stop doing quest for now
            botAI->GetRpgInfo().ChangeToIdle();
            return true;
        }

        uint32 rndIdx = urand(0, poiInfo.size() - 1);
        float dx = poiInfo[rndIdx].pos.x;
        float dy = poiInfo[rndIdx].pos.y;

        // z = MAX_HEIGHT as we do not know the accurate z
        float dz = std::max(bot->GetMap()->GetHeight(bot->GetPhaseMask(), dx, dy, MAX_HEIGHT),
                            bot->GetMap()->GetWaterLevel(dx, dy));

        if (dz == INVALID_HEIGHT || dz == VMAP_INVALID_HEIGHT_VALUE)
            return false;

        data.lastReachPOI = 0;
        data.pos = WorldPosition(bot->GetMapId(), dx, dy, dz);
        data.objectiveIdx = poiInfo[rndIdx].objectiveIdx;
    }

    if (bot->GetDistance(data.pos) > 10.0f && !data.lastReachPOI)
    {
        if (MoveFarTo(data.pos))
            return true;

        // Long-range sampler couldn't land a candidate - nudge the bot a short distance so the
        // next tick retries from a different position instead of sitting idle.
        return MoveRandomNear(10.0f);
    }

    // Now we are near the quest objective. Killing mobs and looting quest items is done by the
    // grind strategy.
    if (!data.lastReachPOI)
    {
        data.lastReachPOI = getMSTime();
        return true;
    }

    if (GetMSTimeDiffToNow(data.lastReachPOI) >= poiStayTime)
    {
        if (!objectiveHasProgress(data.objectiveIdx))
        {
            // Sat at the POI for `poiStayTime` with no progress: probably not completable by a
            // bot, so stop picking this quest.
            botAI->GetLowPriorityQuests().insert(questId);
            botAI->GetRpgStatistic().questAbandoned++;
            TC_LOG_DEBUG("playerbots", "[New RPG] %s marked as abandoned quest %u", bot->GetName().c_str(), questId);
            botAI->GetRpgInfo().ChangeToIdle();
            return true;
        }

        // clear and select another poi later
        data.lastReachPOI = 0;
        data.pos = WorldPosition();
        data.objectiveIdx = 0;
        return true;
    }

    // At the POI: keep the bot actively placed but avoid large random 20yd hops that look like
    // pacing back and forth. A small ~8yd wander reads as the bot looking around while the
    // grind/loot strategies do their work.
    return MoveRandomNear(8.0f);
}

bool NewRpgDoQuestAction::DoCompletedQuest(NewRpgInfo::DoQuest& data)
{
    uint32 const questId = data.questId;

    if (data.objectiveIdx != -1)
    {
        // quest is complete: head back to the POI with objective index -1 to hand it in
        // #322 (AC NewRpgAction.cpp:608)
        if (Quest const* quest = data.quest ? data.quest : sObjectMgr->GetQuestTemplate(questId))
            BroadcastHelper::BroadcastQuestUpdateComplete(botAI, bot, quest);
        botAI->GetRpgStatistic().questCompleted++;

        std::vector<POIInfo> poiInfo;
        if (!GetQuestPOIPosAndObjectiveIdx(questId, poiInfo, true))
        {
            // can't find a poi pos to reward, stop doing quest for now
            botAI->GetRpgInfo().ChangeToIdle();
            return false;
        }

        float dx = poiInfo[0].pos.x;
        float dy = poiInfo[0].pos.y;
        float dz = std::max(bot->GetMap()->GetHeight(bot->GetPhaseMask(), dx, dy, MAX_HEIGHT),
                            bot->GetMap()->GetWaterLevel(dx, dy));

        if (dz == INVALID_HEIGHT || dz == VMAP_INVALID_HEIGHT_VALUE)
            return false;

        data.lastReachPOI = 0;
        data.pos = WorldPosition(bot->GetMapId(), dx, dy, dz);
        data.objectiveIdx = -1;
    }

    if (data.pos == WorldPosition())
        return false;

    if (bot->GetDistance(data.pos) > 10.0f && !data.lastReachPOI)
    {
        if (MoveFarTo(data.pos))
            return true;

        return MoveRandomNear(10.0f);
    }

    // Near the hand-in POI; the quest itself is turned in by SearchQuestGiverAndAcceptOrReward.
    if (!data.lastReachPOI)
    {
        data.lastReachPOI = getMSTime();
        return true;
    }

    if (GetMSTimeDiffToNow(data.lastReachPOI) >= poiStayTime)
    {
        // e.g. cannot hand a quest in to a game object
        botAI->GetLowPriorityQuests().insert(questId);
        botAI->GetRpgStatistic().questAbandoned++;
        TC_LOG_DEBUG("playerbots", "[New RPG] %s marked as abandoned quest %u", bot->GetName().c_str(), questId);
        botAI->GetRpgInfo().ChangeToIdle();
        return true;
    }

    return false;
}

bool NewRpgTravelFlightAction::Execute(Event /*event*/)
{
    NewRpgInfo& info = botAI->GetRpgInfo();
    auto* dataPtr = std::get_if<NewRpgInfo::TravelFlight>(&info.data);
    if (!dataPtr)
        return false;

    auto& data = *dataPtr;

    if (bot->IsInFlight())
    {
        data.inFlight = true;
        ContinueCrossMapTaxi();
        return false;
    }

    if (bot->GetDistance(data.flightMasterPos.GetPositionX(), data.flightMasterPos.GetPositionY(),
                         data.flightMasterPos.GetPositionZ()) > INTERACTION_DISTANCE)
        return MoveFarTo(data.flightMasterPos);

    Creature* flightMaster = bot->FindNearestCreature(data.flightMasterEntry, INTERACTION_DISTANCE * 3);
    if (!flightMaster || !flightMaster->IsAlive())
    {
        TC_LOG_DEBUG("playerbots",
                     "[New RPG] %s reached the flight master spot but found no live creature %u within %.1f yd "
                     "(bot at %.1f,%.1f,%.1f map %u) - giving up",
                     bot->GetName().c_str(), data.flightMasterEntry, INTERACTION_DISTANCE * 3, bot->GetPositionX(),
                     bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId());
        info.ChangeToIdle();
        return true;
    }

    if (bot->GetDistance(flightMaster) > INTERACTION_DISTANCE)
        return MoveFarTo(WorldPosition(flightMaster));

    std::vector<uint32> nodes = data.path;

    botAI->RemoveShapeshift();
    // #843 (MoP-only compensation, AC NewRpgAction.cpp:739-741 `bot->Dismount()`): on this core Unit::Dismount() only
    // clears the mount display and UNIT_FLAG_MOUNT and leaves the SPELL_AURA_MOUNTED aura (Unit.cpp Unit::Dismount;
    // #134), and ActivateTaxiPathTo with a flight master does not remove it (Player.cpp, "taximaster case"). AC's
    // Dismount() drops the aura. Dismount the way CheckMountStateAction does (CMSG_CANCEL_MOUNT_AURA), as on AC.
    if (bot->IsMounted())
    {
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    }

    // Bots have not discovered taxi nodes the way a player would, so learn them from the flight
    // master standing here first (this is what the client does on gossip).
    bot->GetSession()->SendLearnNewTaxiNode(flightMaster);

    if (!bot->ActivateTaxiPathTo(nodes, flightMaster, 0))
    {
        TC_LOG_DEBUG("playerbots", "[New RPG] %s activate taxi path %u (from %u to %u) failed", bot->GetName().c_str(),
                     flightMaster->GetEntry(), nodes.empty() ? 0 : nodes.front(), nodes.empty() ? 0 : nodes.back());
        info.ChangeToIdle();
        return true;
    }

    // Upstream logs only the failure; log the success too, so "bots actually take flights" is
    // observable in the server log rather than inferred from the absence of errors.
    TC_LOG_DEBUG("playerbots", "[New RPG] %s boarded taxi at %u: node %u -> %u (%u hops)",
                 bot->GetName().c_str(), flightMaster->GetEntry(), nodes.empty() ? 0 : nodes.front(),
                 nodes.empty() ? 0 : nodes.back(), uint32(nodes.size()));

    return true;
}

void NewRpgTravelFlightAction::ContinueCrossMapTaxi()
{
    if (bot->IsBeingTeleported())
        return;

    if (!bot->movespline->Finalized())
        return;

    MotionMaster* mm = bot->GetMotionMaster();
    if (!mm || mm->GetCurrentMovementGeneratorType() != FLIGHT_MOTION_TYPE)
        return;

    uint32 nextDest = bot->m_taxi.GetTaxiDestination();
    if (!nextDest)
        return;

    // Only act when the next node is on a different map - otherwise the flight generator handles
    // it on its own.
    TaxiNodesEntry const* nextNode = sTaxiNodesStore.LookupEntry(nextDest);
    if (!nextNode || nextNode->map_id == bot->GetMapId())
        return;

    FlightPathMovementGenerator* flight = dynamic_cast<FlightPathMovementGenerator*>(mm->top());
    if (!flight)
        return;

    TC_LOG_DEBUG("playerbots", "[New RPG] %s continuing taxi across map boundary (next node %u on map %u)",
                 bot->GetName().c_str(), nextDest, nextNode->map_id);

    flight->SetCurrentNodeAfterTeleport();

    if (flight->HasArrived())
        return;

    // LoA's TaxiPathNodeList is an indexed Path<> whose element type is TaxiPathNodeEntry const.
    TaxiPathNodeEntry const& node = flight->GetPath()[flight->GetCurrentNode()];
    flight->SkipCurrentNode();

    // #707 (Henri, owner log 2026-10-06: Rut'theran -> Exodar, path 2476 crosses map 1 -> 530 at node 6; the bot was left at
    // node 6 (-3050,-13147,60) in the Veiled Sea for 65 min). AC's worldport ack resumes the SAME flight generator
    // (azerothcore-wotlk Handlers/MovementHandler.cpp:218-224, top()->Initialize), so the flight continues from the node
    // after this one. This core's ack instead rebuilds the flight from PlayerTaxi (Player::ContinueTaxiFlight,
    // Player.cpp:23573-23597: start = m_taxi.GetLastNodeIndex() + 1), and the last node index still points at the map-1
    // leg. Point it at the node teleported to, so the rebuilt flight starts where AC's re-initialized one does.
    bot->m_taxi.SetLastNodeIndex(flight->GetCurrentNode() - 1);

    // Upstream passes TELE_TO_NOT_LEAVE_TAXI because AC's TeleportTo clears the taxi
    // destinations. LoA's does not touch taxi state, and its worldport ack resumes the flight
    // through Player::ContinueTaxiFlight, so no flag is needed here.
    bot->TeleportTo(nextNode->map_id, node.LocX, node.LocY, node.LocZ, bot->GetOrientation(),
                    TELE_TO_NOT_LEAVE_TAXI);
}
