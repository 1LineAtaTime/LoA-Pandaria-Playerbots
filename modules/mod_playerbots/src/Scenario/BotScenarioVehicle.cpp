/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Issue #207 (Kezan quest 14071 "Rolling with my Homies"): a playerbot-driven proof that the ported
 * vehicle primitives (MovementAction::MoveTo's vehicle branch, MovementActions.cpp:365-399; and
 * VehicleActions.cpp's Enter/LeaveVehicleAction, #179) work end to end through a real quest, using
 * the exact code path AC's own bots use for Isle of Conquest vehicles - see that file's header
 * comment for the AC-side citations. No new movement system is added here: everything below either
 * calls the bot's existing "normal" actions (SearchQuestGiverAndAcceptOrReward via NewRpgInfo's
 * DoQuest status, UseItemAction, LeaveVehicleAction) or drives through MovementAction::MoveTo via a
 * thin public wrapper (MoveTo itself is protected on MovementAction).
 *
 * Two scenario kinds are registered here, independently of BotScenario.cpp (BotScenarioInternal.h's
 * registry is a free function anyone can call - see its header comment):
 *  - "drive": a generic, quest-agnostic multi-waypoint vehicle-driving scenario. Reusable as-is for
 *    #204 (Isle of Conquest) and #205 (Strand of the Ancients) vehicle parity tests later - it only
 *    assumes the bot is already a controlling vehicle passenger when it starts.
 *  - "hotrod": the actual #207 end-to-end proof (accept -> use the key -> drive to Izzy/Ace/Gobber ->
 *    drive back -> leave the vehicle -> turn in), hardcoded to quest 14071's ids/coordinates because
 *    it proves one specific quest, not a generic quest driver (see the "quest" scenario in
 *    BotScenario.cpp for that).
 *
 * Coordinates/ids below are verified against the test DB `world` schema
 * not the quest_poi blobs: the blobs sit 5-9.5 yd off
 * the real creature spawns, which is not tight enough to reliably land the Hot Rod inside each
 * homie's 10 yd MoveInLineOfSight trigger radius (zone_kezan.cpp's npc_homies) on the first pass.
 */

#include "BotScenario.h"
#include "BotScenarioInternal.h"

#include "Bag.h"
#include "Chat.h"
#include "Creature.h"
#include "MovementActions.h"
#include "NewRpgInfo.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "Timer.h"
#include "UseItemAction.h"
#include "Unit.h"
#include "Vehicle.h"
#include "VehicleActions.h"

#include <sstream>
#include <string>
#include <vector>

namespace BotScenario
{
namespace
{
    // --- shared vehicle-driving primitive -----------------------------------------------------------

    // Minimal public wrapper around the protected MovementAction::MoveTo, so this file's free
    // functions (not Action subclasses ticked by the engine) can drive a boarded vehicle through the
    // exact AC-parity path MovementActions.cpp:365-399 uses: when bot->GetVehicle() has a
    // CanControl() seat, MoveTo issues MotionMaster::MovePoint on the vehicle's own Unit
    // (vehicleBase->GetMotionMaster()), not the bot. This is the same primitive #204/#205 are
    // expected to reuse - do not add a second movement system next to it.
    class VehicleDriveAction : public MovementAction
    {
    public:
        explicit VehicleDriveAction(PlayerbotAI* botAI) : MovementAction(botAI, "vehicle drive") {}

        // Returns false when the bot is not currently a controlling vehicle passenger, or when
        // MoveTo itself declines (duplicate move, still waiting on the last one).
        bool DriveTo(uint32 mapId, float x, float y, float z) { return MoveTo(mapId, x, y, z); }
    };

    // #216 diagnostic helper: a thin public wrapper around the protected
    // EnterVehicleAction::EnterVehicle, matching the VehicleDriveAction pattern above, so the
    // harness can call the same spellclick re-board path VehicleActions.cpp:51-64 uses
    // (HandleSpellClick, spellclick 34840->46598) on a specific vehicle directly.
    class HotRodBoardAction : public EnterVehicleAction
    {
    public:
        explicit HotRodBoardAction(PlayerbotAI* botAI) : EnterVehicleAction(botAI, "hotrod board") {}
        bool TryBoard(Unit* vehicleBase) { return EnterVehicle(vehicleBase, true); }
    };

    // --- "drive": generic multi-waypoint vehicle scenario -------------------------------------------

    constexpr float DRIVE_DEFAULT_ARRIVE_YD = 5.0f;
    constexpr uint32 DRIVE_TIMEOUT_MS = 900000;    // 15 min overall cap
    constexpr uint32 DRIVE_LEG_TIMEOUT_MS = 300000; // 5 min/leg - ends a stuck run sooner than the overall cap

    bool ParseDrive(ChatHandler* handler, Player* bot, PlayerbotAI* botAI, std::istringstream& in, ScenarioState& state)
    {
        uint32 mapId = 0;
        if (!(in >> mapId))
        {
            handler->SendSysMessage("Usage: .botscenario <botname> drive <map> <x1> <y1> <z1> [<x2> <y2> <z2> ...] [arrive=<yd>]");
            return false;
        }

        std::vector<float> waypoints;
        float arriveDist = DRIVE_DEFAULT_ARRIVE_YD;
        std::string tok;
        while (in >> tok)
        {
            if (tok.rfind("arrive=", 0) == 0)
            {
                try { arriveDist = std::stof(tok.substr(7)); }
                catch (std::exception const&) { /* malformed arrive= on this debug-only command - keep the default */ }
                continue;
            }
            try { waypoints.push_back(std::stof(tok)); }
            catch (std::exception const&)
            {
                handler->SendSysMessage("botscenario: drive: malformed coordinate");
                return false;
            }
        }

        if (waypoints.empty() || waypoints.size() % 3 != 0)
        {
            handler->SendSysMessage("Usage: .botscenario <botname> drive <map> <x1> <y1> <z1> [<x2> <y2> <z2> ...] [arrive=<yd>]");
            return false;
        }

        Vehicle* vehicle = bot->GetVehicle();
        VehicleSeatEntry const* seat = vehicle ? vehicle->GetSeatForPassenger(bot) : nullptr;
        if (!vehicle || !seat || !seat->CanControl())
        {
            handler->SendSysMessage("botscenario: drive: bot is not currently a controlling vehicle passenger");
            return false;
        }

        // Movement is driven entirely from this file's Update function below (via VehicleDriveAction),
        // not through the "new rpg" status machine - park it at Idle+lock so nothing else fights for
        // control of the vehicle (the same pattern the "attack"/"die" scenarios use).
        botAI->GetRpgInfo().ChangeToIdle();
        botAI->ChangeEngine(BOT_STATE_NON_COMBAT);

        state.kind = ScenarioKind::Drive;
        state.destMapId = mapId;
        state.lock = true;
        state.startT = getMSTime();
        state.lastTraceT = 0;
        state.drive = DriveTrack{};
        state.drive.waypoints = std::move(waypoints);
        state.drive.arriveDist = arriveDist;

        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s start drive map=%u legs=%zu arrive=%.1f",
            bot->GetName().c_str(), mapId, state.drive.waypoints.size() / 3, arriveDist);
        return true;
    }

    bool DriveUpdate(Player* bot, PlayerbotAI* botAI, ScenarioState& state, uint32 /*now*/, uint32 t)
    {
        DriveTrack& d = state.drive;
        uint32 const legCount = uint32(d.waypoints.size() / 3);

        Vehicle* vehicle = bot->GetVehicle();
        Unit* vehicleBase = vehicle ? vehicle->GetBase() : nullptr;
        if (!vehicleBase)
        {
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s drive lost-vehicle t=%u leg=%u/%u",
                bot->GetName().c_str(), t, d.legIdx, legCount);
            return true;
        }

        if (t >= DRIVE_TIMEOUT_MS)
        {
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s drive timeout t=%u leg=%u/%u x=%.1f y=%.1f z=%.1f",
                bot->GetName().c_str(), t, d.legIdx, legCount,
                vehicleBase->GetPositionX(), vehicleBase->GetPositionY(), vehicleBase->GetPositionZ());
            return true;
        }

        if (t - d.legStartT >= DRIVE_LEG_TIMEOUT_MS)
        {
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s drive stuck t=%u leg=%u/%u x=%.1f y=%.1f z=%.1f",
                bot->GetName().c_str(), t, d.legIdx, legCount,
                vehicleBase->GetPositionX(), vehicleBase->GetPositionY(), vehicleBase->GetPositionZ());
            return true;
        }

        float const lx = d.waypoints[d.legIdx * 3 + 0];
        float const ly = d.waypoints[d.legIdx * 3 + 1];
        float const lz = d.waypoints[d.legIdx * 3 + 2];
        float const dist = vehicleBase->GetExactDist(lx, ly, lz);

        if (dist <= d.arriveDist)
        {
            TC_LOG_DEBUG("playerbots", "[SCENARIO] %s drive leg-arrived t=%u leg=%u/%u dist=%.1f",
                bot->GetName().c_str(), t, d.legIdx, legCount, dist);
            ++d.legIdx;
            d.legStartT = t;
            if (d.legIdx >= legCount)
            {
                TC_LOG_DEBUG("playerbots", "[SCENARIO] %s drive done t=%u legs=%u", bot->GetName().c_str(), t, legCount);
                return true;
            }
            return false;
        }

        VehicleDriveAction(botAI).DriveTo(state.destMapId, lx, ly, lz);
        return false;
    }

    void RearmDrive(Player* /*bot*/, PlayerbotAI* botAI, ScenarioState& state)
    {
        // Nothing in NewRpgInfo needs to be reapplied (unlike goto/quest) - just keep new-rpg parked.
        botAI->GetRpgInfo().ChangeToIdle();
        state.lock = true;
    }

    // --- "hotrod": quest 14071 end-to-end -------------------------------------------------------

    constexpr uint32 HOTROD_MAP = 648;               // Kezan
    constexpr uint32 HOTROD_QUEST_ID = 14071;        // "Rolling with my Homies"
    constexpr uint32 HOTROD_VEHICLE_ENTRY = 34840;   // Hot Rod
    constexpr uint32 HOTROD_KEY_ITEM_ID = 46856;     // Keys to the Hot Rod (spell 66393 on use)
    // Megs Dreadshredder (creature 34874) is both the accept and turn-in giver (creature_queststarter/
    // creature_questender), found by the bot's own SearchQuestGiverAndAcceptOrReward - no entry lookup
    // needed here.

    // Fresh-goblin spawn point (playercreateinfo, race 9) - a few yards from Megs.
    constexpr float HOTROD_START_X = -8423.81f, HOTROD_START_Y = 1361.3f, HOTROD_START_Z = 104.671f, HOTROD_START_O = 1.55428f;
    // Megs Dreadshredder 34874 (creature.guid 228866): drive-back target and turn-in point.
    constexpr float HOTROD_MEGS_X = -8419.26f, HOTROD_MEGS_Y = 1341.82f, HOTROD_MEGS_Z = 103.0f;

    struct Homie
    {
        char const* name;
        uint32 objectiveId; // quest_objective.id, for Player::GetQuestObjectiveCounter
        uint32 summonEntry; // SUMMON-flagged copy (34957-34959) that takes the credit and boards
        int8 seatId;       // its preferred rear seat, per zone_kezan.cpp's seatPerHomie
        float x, y, z;
    };

    // Live `creature` rows (world DB), not the quest_poi blobs - see the file header comment.
    Homie const HOMIES[3] =
    {
        { "Izzy",   265516, 34959, 3, -8507.65f, 1342.85f, 101.78f  },
        { "Ace",    265517, 34957, 1, -8068.13f, 1482.03f, 9.01469f },
        { "Gobber", 265518, 34958, 2, -8179.48f, 1321.38f, 27.6826f },
    };

    constexpr uint32 HOTROD_ACCEPT_TIMEOUT_MS = 60000;
    constexpr uint32 HOTROD_BOARD_TIMEOUT_MS = 60000;
    constexpr uint32 HOTROD_LEG_TIMEOUT_MS = 300000;  // 5 min/leg - Ace/Gobber are ~1300 yd of vertical terrain away
    constexpr uint32 HOTROD_RETURN_TIMEOUT_MS = 300000;
    constexpr uint32 HOTROD_LEAVE_TIMEOUT_MS = 30000;
    constexpr uint32 HOTROD_TURNIN_TIMEOUT_MS = 180000;
    constexpr float HOTROD_ARRIVE_YD = 5.0f;
    constexpr float HOTROD_RETURN_ARRIVE_YD = 15.0f; // close enough to hop off and walk the rest
    constexpr uint32 HOTROD_ACTION_RETRY_MS = 2000;  // throttle UseItemAction/LeaveVehicleAction retries
    constexpr uint32 HOTROD_LOG_INTERVAL_MS = 2000;  // [HOTROD] evidence line cadence
    constexpr uint32 HOTROD_KEEPALIVE_MS = 1500;      // re-assert DoQuest status at this cadence (see below)

    Quest const* HotRodQuest() { return sObjectMgr->GetQuestTemplate(HOTROD_QUEST_ID); }

    // #207: the "picked-up" criterion used to be proximity alone (dist <= HOTROD_ARRIVE_YD),
    // which printed a false pass the moment the bot merely drove past a homie - nobody in the
    // car, objective not credited. It is now a real seat query: the homie's SUMMON-flagged copy
    // (zone_kezan.cpp's npc_homies_summon) sitting in one of the Hot Rod's three rear seats.
    // Any rear seat counts, not only Homie::seatId, because npc_homies_summon falls back to
    // seatId -1 (the next empty seat) when its preferred one is taken. Returns the seat, -1
    // if the homie is not aboard.
    int8 HomieRearSeat(Vehicle* veh, Homie const& homie)
    {
        if (!veh)
            return -1;

        for (int8 seat = 1; seat <= 3; ++seat)
            if (Unit* passenger = veh->GetPassenger(seat))
                if (passenger->GetEntry() == homie.summonEntry)
                    return seat;

        return -1;
    }

    // Issue #212-adjacent finding (kept local to this file - not filed as its own issue, worked
    // around here instead): NewRpgDoQuestAction::Execute (strategy/RPG/NewRpgAction.cpp:282-304)
    // calls SearchQuestGiverAndAcceptOrReward() first every tick, and if that returns false while
    // the quest is still QUEST_STATUS_NONE (i.e. ChooseNpcOrGameObjectToInteract found nothing in the
    // "possible new rpg targets"/"...game objects" AI values - e.g. the very first tick right after a
    // teleport, before those caches repopulate), it falls through to `info.ChangeToIdle(); return
    // true;` - abandoning DoQuest entirely, permanently, with no retry. One unlucky tick is enough to
    // silently kill the whole accept/turn-in attempt (reproduced twice while testing #207). Re-assert
    // DoQuest here on a short cadence while our own phase still needs it, so a single bad tick just
    // costs a beat instead of the run.
    void KeepDoQuestAlive(PlayerbotAI* botAI, HotRodTrack& h, uint32 t)
    {
        if (h.lastKeepaliveT && t - h.lastKeepaliveT < HOTROD_KEEPALIVE_MS)
            return;
        h.lastKeepaliveT = t;
        if (botAI->GetRpgInfo().GetStatus() != RPG_DO_QUEST)
            botAI->GetRpgInfo().ChangeToDoQuest(HOTROD_QUEST_ID, HotRodQuest());
    }

    // Root cause of every "noaccept" timeout while diagnosing #207: quest_template.StartItem for
    // 14071 is 46856 ("Keys to the Hot Rod"), and Player::CanAddQuest (Player.cpp:16038-16059)
    // refuses the WHOLE quest - not just the item grant - if it can't CanStoreNewItem() a fresh
    // stack of the StartItem (confirmed via AiPlayerbot.DebugBotNames' [QUEST] trace:
    // "quest=14071 ... take=1 add=0 worth=1 capable=1" - CanTakeQuest passed, CanAddQuest alone
    // failed). This test's character is an existing random-bot pool member reused for a
    // deterministic level-3 goblin (there is no in-game way to create a brand new character without
    // the headless client), so its bags are very likely still full of loot from its previous life at
    // a much higher level. Clear the backpack and the 4 equipped bags (not worn gear) so there is
    // always room.
    void FreeBagsForQuestStart(Player* bot)
    {
        std::vector<Item*> toDestroy;
        for (uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
            if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                toDestroy.push_back(item);
        for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
            if (Bag* bag = bot->GetBagByPos(i))
                for (uint32 j = 0; j < bag->GetBagSize(); ++j)
                    if (Item* item = bag->GetItemByPos(j))
                        toDestroy.push_back(item);

        for (Item* item : toDestroy)
            bot->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
    }

    bool ParseHotRod(ChatHandler* handler, Player* bot, PlayerbotAI* botAI, std::istringstream& in, ScenarioState& state)
    {
        uint32 secs = 0;
        std::string tok;
        while (in >> tok)
        {
            try { secs = uint32(std::stoul(tok)); }
            catch (std::exception const&) { /* malformed seconds on this debug-only command - keep the default */ }
        }
        if (!secs)
            secs = 2400;

        Quest const* quest = HotRodQuest();
        if (!quest)
        {
            handler->SendSysMessage("botscenario: hotrod: quest 14071 not found in this world DB");
            return false;
        }

        // Deterministic rerun: drop a leftover 14071 (an earlier life of this random bot, or a
        // previous run of this scenario) so accept always starts from QUEST_STATUS_NONE. The
        // prerequisite chain (25473/28349) is expected to already be rewarded out of band, per the
        // #207 test plan's "deterministic setup" - not reset here.
        if (bot->GetQuestStatus(HOTROD_QUEST_ID) != QUEST_STATUS_NONE)
        {
            for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
            {
                if (bot->GetQuestSlotQuestId(i) != HOTROD_QUEST_ID)
                    continue;
                bot->DestroyQuestItems(HOTROD_QUEST_ID);
                bot->RemoveActiveQuest(HOTROD_QUEST_ID);
                bot->SetQuestSlot(i, 0);
                break;
            }
            bot->RemoveRewardedQuest(HOTROD_QUEST_ID);
        }
        bot->DestroyItemCount(HOTROD_KEY_ITEM_ID, 9999, true);
        FreeBagsForQuestStart(bot);

        bot->TeleportTo(HOTROD_MAP, HOTROD_START_X, HOTROD_START_Y, HOTROD_START_Z, HOTROD_START_O);
        bot->AttackStop();
        botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
        botAI->ChangeEngine(BOT_STATE_NON_COMBAT);
        botAI->GetRpgInfo().ChangeToIdle();

        state.kind = ScenarioKind::HotRod;
        state.destMapId = HOTROD_MAP;
        state.wanderMs = secs * IN_MILLISECONDS;
        state.lock = true;
        state.startT = getMSTime();
        state.lastTraceT = 0;
        // Deterministic accept/turn-in target: ChooseNpcOrGameObjectToInteract's pickNearest branch
        // (NewRpgBaseAction.cpp) picks the closest offerable quest-giver instead of the first one in
        // iteration order, which - a few yards from the goblin spawn - is Megs (34874) in practice.
        state.pickNearest = true;
        state.hotrod = HotRodTrack{};

        TC_LOG_DEBUG("playerbots", "[SCENARIO] %s start hotrod quest=%u secs=%u", bot->GetName().c_str(), HOTROD_QUEST_ID, secs);
        return true;
    }

    void LogHotRodEvidence(Player* bot, HotRodTrack& h, uint32 t)
    {
        if (h.lastLogT && t - h.lastLogT < HOTROD_LOG_INTERVAL_MS)
            return;
        h.lastLogT = t;

        Vehicle* veh = bot->GetVehicle();
        Unit* vehicleBase = veh ? veh->GetBase() : nullptr;
        Unit* p1 = veh ? veh->GetPassenger(1) : nullptr;
        Unit* p2 = veh ? veh->GetPassenger(2) : nullptr;
        Unit* p3 = veh ? veh->GetPassenger(3) : nullptr;

        TC_LOG_DEBUG("playerbots",
            "[HOTROD] %s t=%u phase=%u vx=%.1f vy=%.1f vz=%.1f izzy=%u ace=%u gobber=%u seat1=%u seat2=%u seat3=%u",
            bot->GetName().c_str(), t, h.phase,
            vehicleBase ? vehicleBase->GetPositionX() : 0.0f,
            vehicleBase ? vehicleBase->GetPositionY() : 0.0f,
            vehicleBase ? vehicleBase->GetPositionZ() : 0.0f,
            bot->GetQuestObjectiveCounter(HOMIES[0].objectiveId),
            bot->GetQuestObjectiveCounter(HOMIES[1].objectiveId),
            bot->GetQuestObjectiveCounter(HOMIES[2].objectiveId),
            p1 ? p1->GetEntry() : 0, p2 ? p2->GetEntry() : 0, p3 ? p3->GetEntry() : 0);
    }

    bool HotRodUpdate(Player* bot, PlayerbotAI* botAI, ScenarioState& state, uint32 /*now*/, uint32 t)
    {
        HotRodTrack& h = state.hotrod;
        char const* name = bot->GetName().c_str();

        if (t >= state.wanderMs)
        {
            TC_LOG_DEBUG("playerbots", "[HOTROD] %s timeout t=%u phase=%u", name, t, h.phase);
            return true;
        }

        LogHotRodEvidence(bot, h, t);

        switch (h.phase)
        {
            case 0: // arm: settle for a moment, then let DoQuest talk to Megs
            {
                if (!h.readyT)
                {
                    h.readyT = t;
                    return false;
                }
                if (t - h.readyT < 1000)
                    return false;

                botAI->GetRpgInfo().ChangeToDoQuest(HOTROD_QUEST_ID, HotRodQuest());
                h.phase = 1;
                h.phaseT = t;
                return false;
            }

            case 1: // accept: the bot's own SearchQuestGiverAndAcceptOrReward (NewRpgDoQuestAction)
            {
                if (bot->GetQuestStatus(HOTROD_QUEST_ID) == QUEST_STATUS_INCOMPLETE)
                {
                    h.acceptT = t;
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s accepted t=%u", name, t);
                    // Take movement back from the RPG subsystem for the pickup phase: DoIncompleteQuest
                    // would otherwise pick a random incomplete objective's POI - including the far
                    // Izzy/Ace/Gobber blobs - before the key is even used.
                    botAI->GetRpgInfo().ChangeToIdle();
                    h.phase = 2;
                    h.phaseT = t;
                    h.lastActionT = 0;
                    return false;
                }
                if (t - h.phaseT >= HOTROD_ACCEPT_TIMEOUT_MS)
                {
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s noaccept t=%u", name, t);
                    return true;
                }
                KeepDoQuestAlive(botAI, h, t);
                return false;
            }

            case 2: // use the key, then wait for the Hot Rod to appear with the bot in the driver seat
            {
                Vehicle* veh = bot->GetVehicle();
                if (veh && veh->GetBase()->GetEntry() == HOTROD_VEHICLE_ENTRY)
                {
                    h.boardT = t;
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s boarded t=%u vehicleGuid=%u",
                        name, t, veh->GetBase()->GetGUID().GetCounter());
                    h.phase = 3;
                    h.phaseT = t;
                    h.homieIdx = 0;
                    return false;
                }

                if (t - h.phaseT >= HOTROD_BOARD_TIMEOUT_MS)
                {
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s noboard t=%u haskey=%d", name, t,
                        bot->HasItemCount(HOTROD_KEY_ITEM_ID, 1, false) ? 1 : 0);
                    return true;
                }

                Creature* nearbyHotRod = bot->FindNearestCreature(HOTROD_VEHICLE_ENTRY, 30.0f, false);
                if (!h.lastLogT || t - h.lastLogT >= HOTROD_LOG_INTERVAL_MS)
                    TC_LOG_DEBUG("playerbots", "[HOTROD216] %s t=%u nearbyHotRod=%d alive=%d",
                        name, t, nearbyHotRod ? 1 : 0, nearbyHotRod && nearbyHotRod->IsAlive() ? 1 : 0);

                if (!h.keyUsedOnce && (!h.lastActionT || t - h.lastActionT >= HOTROD_ACTION_RETRY_MS) &&
                    bot->HasItemCount(HOTROD_KEY_ITEM_ID, 1, false))
                {
                    h.lastActionT = t;
                    // The ported UseItemAction path (UseItemAction.cpp): finds item 46856 by id via
                    // "inventory items"/InventoryAction::Find and casts its on-use spell (66393). The
                    // core's own SUMMON_CATEGORY_VEHICLE effect handling (SpellEffects.cpp) then
                    // summons the Hot Rod and casts the ride-vehicle spell on it for us - no
                    // module-side boarding call needed for this particular item.
                    bool const used = UseItemAction(botAI, std::to_string(HOTROD_KEY_ITEM_ID)).Execute(Event());
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s use-key t=%u result=%d", name, t, used ? 1 : 0);
                    if (used)
                    {
                        h.keyUsedT = t;
                        h.keyUsedOnce = true;
                        h.lastActionT = 0;
                    }
                }
                else if (h.keyUsedOnce && nearbyHotRod && nearbyHotRod->IsAlive() &&
                    (!h.lastActionT || t - h.lastActionT >= HOTROD_ACTION_RETRY_MS))
                {
                    h.lastActionT = t;
                    bool const clicked = HotRodBoardAction(botAI).TryBoard(nearbyHotRod);
                    TC_LOG_DEBUG("playerbots", "[HOTROD216] %s spellclick-attempt t=%u result=%d",
                        name, t, clicked ? 1 : 0);
                }
                return false;
            }

            case 3: // drive to Izzy, Ace, Gobber in turn - VehicleDriveAction, same as the generic
                    // "drive" scenario above
            {
                Vehicle* veh = bot->GetVehicle();
                Unit* vehicleBase = veh ? veh->GetBase() : nullptr;
                if (!vehicleBase || vehicleBase->GetEntry() != HOTROD_VEHICLE_ENTRY)
                {
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s vehiclelost t=%u homie=%u", name, t, h.homieIdx);
                    return true;
                }

                Homie const& target = HOMIES[h.homieIdx];
                float const dist = vehicleBase->GetExactDist(target.x, target.y, target.z);
                bool const credited = bot->GetQuestObjectiveCounter(target.objectiveId) > 0;
                int8 const seat = HomieRearSeat(veh, target);

                if (seat >= 0)
                {
                    h.homieT[h.homieIdx] = t;
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s picked-up t=%u homie=%s seat=%d dist=%.1f credited=%d",
                        name, t, target.name, int32(seat), dist, credited ? 1 : 0);
                    ++h.homieIdx;
                    h.phaseT = t;
                    if (h.homieIdx >= 3)
                    {
                        h.phase = 4; // drive back to Megs before dismounting
                    }
                    return false;
                }

                if (t - h.phaseT >= HOTROD_LEG_TIMEOUT_MS)
                {
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s stuck t=%u homie=%s seat=%d dist=%.1f "
                        "arrive<=%.1f credited=%d", name, t, target.name, int32(seat), dist,
                        HOTROD_ARRIVE_YD, credited ? 1 : 0);
                    return true;
                }

                VehicleDriveAction(botAI).DriveTo(HOTROD_MAP, target.x, target.y, target.z);
                return false;
            }

            case 4: // drive back near Megs before leaving the vehicle - much faster than walking back
                    // from Gobber's spot (~1300 yd of vertical terrain away)
            {
                Vehicle* veh = bot->GetVehicle();
                Unit* vehicleBase = veh ? veh->GetBase() : nullptr;
                if (!vehicleBase || vehicleBase->GetEntry() != HOTROD_VEHICLE_ENTRY)
                {
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s vehiclelost t=%u phase=return", name, t);
                    return true;
                }

                float const dist = vehicleBase->GetExactDist(HOTROD_MEGS_X, HOTROD_MEGS_Y, HOTROD_MEGS_Z);
                if (dist <= HOTROD_RETURN_ARRIVE_YD)
                {
                    h.returnT = t;
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s returned t=%u dist=%.1f", name, t, dist);
                    h.phase = 5;
                    h.phaseT = t;
                    h.lastActionT = 0;
                    return false;
                }

                if (t - h.phaseT >= HOTROD_RETURN_TIMEOUT_MS)
                {
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s stuck-return t=%u dist=%.1f", name, t, dist);
                    return true;
                }

                VehicleDriveAction(botAI).DriveTo(HOTROD_MAP, HOTROD_MEGS_X, HOTROD_MEGS_Y, HOTROD_MEGS_Z);
                return false;
            }

            case 5: // leave the vehicle - VehicleActions.cpp's LeaveVehicleAction (the #179 primitive)
            {
                if (!bot->GetVehicle())
                {
                    h.leftVehicleT = t;
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s left-vehicle t=%u", name, t);
                    botAI->GetRpgInfo().ChangeToDoQuest(HOTROD_QUEST_ID, HotRodQuest());
                    h.phase = 6;
                    h.phaseT = t;
                    return false;
                }

                if (t - h.phaseT >= HOTROD_LEAVE_TIMEOUT_MS)
                {
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s noleave t=%u", name, t);
                    return true;
                }

                if (!h.lastActionT || t - h.lastActionT >= HOTROD_ACTION_RETRY_MS)
                {
                    h.lastActionT = t;
                    bool const left = LeaveVehicleAction(botAI).Execute(Event());
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s leave-attempt t=%u result=%d", name, t, left ? 1 : 0);
                }
                return false;
            }

            case 6: // turn in: the bot's own SearchQuestGiverAndAcceptOrReward/DoCompletedQuest (TurnInQuest)
            {
                if (bot->GetQuestRewardStatus(HOTROD_QUEST_ID))
                {
                    h.rewardT = t;
                    TC_LOG_DEBUG("playerbots",
                        "[HOTROD] %s done t=%u accept=%u keyused=%u board=%u izzy=%u ace=%u gobber=%u "
                        "return=%u left=%u reward=%u",
                        name, t, h.acceptT, h.keyUsedT, h.boardT, h.homieT[0], h.homieT[1], h.homieT[2],
                        h.returnT, h.leftVehicleT, h.rewardT);
                    return true;
                }

                if (t - h.phaseT >= HOTROD_TURNIN_TIMEOUT_MS)
                {
                    TC_LOG_DEBUG("playerbots", "[HOTROD] %s noreward t=%u status=%d", name, t,
                        int(bot->GetQuestStatus(HOTROD_QUEST_ID)));
                    return true;
                }
                KeepDoQuestAlive(botAI, h, t);
                return false;
            }

            default:
                return true;
        }
    }

    void RearmHotRod(Player* /*bot*/, PlayerbotAI* botAI, ScenarioState& state)
    {
        // Re-apply whatever RPG status the current phase needs after PlayerbotAI::Reset(full)
        // recreates NewRpgInfo (mirrors RearmQuest/RearmGoto). Phases 1 and 6 need DoQuest active for
        // SearchQuestGiverAndAcceptOrReward to run; every other phase owns movement directly (Idle).
        HotRodTrack const& h = state.hotrod;
        if (h.phase == 1 || h.phase == 6)
            botAI->GetRpgInfo().ChangeToDoQuest(HOTROD_QUEST_ID, HotRodQuest());
        else
            botAI->GetRpgInfo().ChangeToIdle();
        state.lock = true;
    }

} // anonymous namespace

// --- registration ------------------------------------------------------------------------------
// RegisterScenarioHandler is a free function (BotScenarioInternal.h) backed by a Meyer's-singleton
// registry, so registration order relative to other translation units' static initializers does not
// matter. It is NOT self-registered from a static initializer in this file though: this module links
// BotScenario*.cpp out of a static library (modules.lib), and a .obj that exports no symbol anyone
// else references gets dropped by the linker before any file-static constructor in it would run
// (confirmed: the "drive"/"hotrod" strings were absent from the built worldserver.exe until this
// function got an explicit caller). BotScenario.cpp's own HandlerRegistrar calls this once, which is
// enough to force the .obj in - see BotScenarioInternal.h's comment on the declaration.
void RegisterVehicleScenarioHandlers()
{
    RegisterScenarioHandler(ScenarioKind::Drive, {"drive", &ParseDrive, &DriveUpdate, &RearmDrive});
    RegisterScenarioHandler(ScenarioKind::HotRod, {"hotrod", &ParseHotRod, &HotRodUpdate, &RearmHotRod});
}

} // namespace BotScenario
