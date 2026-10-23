/*
 * #900 (beyond AC: AC has no battlefield bot play, Script/Playerbots.cpp:607-612 is an empty hook): random bots take part in
 * the outdoor battlefields (Wintergrasp battle id 1, Tol Barad battle id 21 once BattlefieldTB is registered).
 *
 *  1. Always on: a random bot standing in a battlefield zone during the war (or when the queue opens) accepts through the
 *     core API (Battlefield::PlayerAcceptInviteToWar / PlayerAcceptInviteToQueue). Bot sessions never answer the
 *     SMSG_BATTLEFIELD_MGR_ENTRY_INVITE, so they were kicked when the 20 s accept timer ran out.
 *  2. Opt-in census, AiPlayerbot.RandomBotJoinBattlefield = 1 (default 0): up to AiPlayerbot.RandomBotJoinBattlefieldCount
 *     eligible random bots PER FACTION queue for an upcoming (<= 15 min) or running battlefield; when the war runs they are
 *     teleported to a graveyard their team controls (the core's own entry point) and accept; at battle end they leave the
 *     battlefield raid and return to roaming.
 *
 * Called from RandomPlayerbotMgr::UpdateAIInternal (world thread, like CheckBgQueue). Self-throttled to one pass per 2 s.
 */
#ifndef _PLAYERBOT_BATTLEFIELDBOTS_H
#define _PLAYERBOT_BATTLEFIELDBOTS_H

#include <vector>

class Player;

namespace BattlefieldBots
{
    bool Due();                                        // true at most once per 2 s
    void Update(std::vector<Player*> const& randomBots);
}

#endif
