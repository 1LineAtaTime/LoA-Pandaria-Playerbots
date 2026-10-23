/*
 * #218 / #528 (owner request 2026-09-29, BEYOND AC): roaming random bots take boats, zeppelins, elevators, trams and
 * flight paths on their own.
 *
 * AC drives none of this. AC's NewRpg destinations never leave the bot's map (SelectRandomGrindPos /
 * SelectRandomCampPos filter on the map and zone, AC Ai/World/Rpg/Action/NewRpgBaseAction.cpp:1051-1171), and
 * AC's route-following code (TravelNodePathType::transport / flightPath / portal) is commented out (AC
 * Ai/Base/Actions/MovementActions.cpp:380-560). On the same-map side, AC teleports a bot that has made no progress
 * for 90 s (AC NewRpgBaseAction.cpp:95-117), so a bot that needs an elevator is teleported.
 *
 * The extension keeps AC's NewRpg status machine, weights and MoveFarTo untouched and adds three hooks, all in
 * NewRpgBaseAction.cpp:
 *  1. SelectRandomCampPos: with AiPlayerbot.RoamTransportCampChance percent, the camp hub may be a level-matched
 *     innkeeper / flight-master hub on ANOTHER continent that the travel graph can reach (RoamTransport::
 *     SelectCrossMapHub). The status stays AC's RPG_GO_CAMP.
 *  2. MoveFarTo, before AC's code: a destination on another map, or one with a trip already running, is driven by
 *     NewRpgBaseAction::RoamTransportMove over a TravelNodeMap::getFullPath route.
 *  3. MoveFarTo's stuck branch: after 30 s with no progress, a graph route is built once. If it has a transport,
 *     flight-path or portal leg the bot follows it; otherwise AC's 90 s stuck teleport runs unchanged.
 *
 * Config (read here, no PlayerbotAIConfig.h change; read once at first use):
 *   AiPlayerbot.RoamTransports = 1            # 0 = AC behaviour exactly (all three hooks off)
 *   AiPlayerbot.RoamTransportCampChance = 10  # percent of GO_CAMP picks that try a cross-continent hub
 *
 * Log lines (playerbots debug category): "[ROAMTRIP] <bot> start|wait|board|ride|leave|taxi|portal|arrive|abandon ...".
 */

#ifndef _PLAYERBOT_NEWRPGROAMTRANSPORT_H
#define _PLAYERBOT_NEWRPGROAMTRANSPORT_H

#include "Define.h"
#include "WorldPosition.h"

class Player;
class Transport;

namespace RoamTransport
{
    bool Enabled();
    uint32 CampChancePct();

    /// A level-matched faction hub (TravelMgr::GetTravelHubs) on another continent the travel graph has a map
    /// transfer to, or an empty WorldPosition.
    WorldPosition SelectCrossMapHub(Player* bot);

    /// Drop any trip state kept for this bot (logout / reset safety; bounded by the bot roster anyway).
    void Forget(Player* bot);

    /// #218 wave 33: true while the bot's trip is at a stop that must be caught in seconds (dock wait, boarding,
    /// riding, stepping off). PlayerbotAI::GetReactDelay then returns the base react delay, the way AC already
    /// does for a bot with a real-player master, instead of the 2-20 s a resting (city) bot gets.
    /// Wave 34 (MoP divergence, owner decision 2026-10-12): the Walk phase of an active trip too.
    /// Wave 41: called once per AI tick, it also ends (arrive / abandon status-<new status>) a trip whose bot reached
    /// the destination or whose RPG status moved on between two MoveFarTo calls.
    bool WantsFastReact(Player* bot);

    /// #218 wave 33: a boat / zeppelin leg that changes map (or a portal leg) far-teleports the bot, and
    /// PlayerbotAI::Reset(true) recreates its NewRpgInfo, which drops the status that drives the trip. Returns the
    /// status to re-apply (grind = RPG_GO_GRIND, else RPG_GO_CAMP) and its destination for a trip that is mid-leg.
    bool ResumeAfterReset(Player* bot, WorldPosition& dest, bool& grind);
    /// #218: true while the bot's trip is on a transport leg; PlayerbotAI::AllowActive keeps it active (beyond AC).
    bool OnTransportLeg(Player* bot);
    /// #889 (beyond AC): true while a static (type-11) platform is between two landings - moving, or leaving within
    /// 250 ms (the TransportAnimation test DockWait / Board / Leave use). PlayerbotAI's ambient check keeps a carried
    /// rider on such a platform instead of dropping it into the shaft.
    bool StaticPlatformMoving(Transport* t);
    /// #890 (beyond AC): true while the bot's roam trip is at a transport leg or walking its last 45 yd to one. The
    /// GO_GRIND / GO_CAMP quest-giver detour (SearchQuestGiverAndAcceptOrReward) waits meanwhile. Own map thread only.
    bool HoldsForTransportLeg(Player* bot);
}

#endif
