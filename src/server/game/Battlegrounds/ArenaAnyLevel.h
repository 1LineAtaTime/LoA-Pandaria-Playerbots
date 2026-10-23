/*
* This file is part of the Pandaria 5.4.8 Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * #702 phase 5 (#832): arenas "any level" - arena skirmishes below the maximum level (beyond Blizzard 5.4.8, SPP-style).
 *
 * 5.4.8 has no arena skirmish (Wowpedia "Arena", section Skirmishes: removed by Cataclysm, back in patch 6.0.2), and
 * this core queues every CMSG_BATTLEMASTER_JOIN_ARENA as a RATED match of an exact-size party
 * (WorldSession::HandleBattlemasterJoinArena -> BattlegroundQueue::AddGroup(..., isRated = true)); a player without a
 * party is dropped silently. The queue itself still has the TrinityCore 3.3.5 unrated-arena pass (BattlegroundQueue::
 * BattlegroundQueueUpdate !isRated -> CheckNormalMatch / CheckSkirmishForSameFaction), which the wave-29 harness arena
 * legs already play (BotScenario.cpp QueueFactionGroupForBg, #544; #774 fixed its dampening crash).
 *
 * With Arena.AnyLevel.Enable = 1 (worldserver.conf, default 0):
 *   - an arena join from a player BELOW CONFIG_MAX_PLAYER_LEVEL is an unrated skirmish: alone, or as the leader of a
 *     party of at most the bracket size, filed under the "All Arenas" bracket of the player's level (every level 10-90
 *     has one: PvpDifficulty map 562, 10-14 .. 90) - AC's solo / group skirmish join (azerothcore-wotlk@7f12e89e
 *     src/server/game/Handlers/BattleGroundHandler.cpp:709-711, :792-830). A max-level join stays the stock rated
 *     join, so rated arena stays max-level and is never changed;
 *   - the "All Arenas" draw of an UNRATED match may also pick Tol'viron (map 980) and the Tiger's Peak (map 1134), which
 *     only have a level-90 PvpDifficulty row (#437 kept them out of every lower match);
 *   - an unrated arena instance on such a map gives its players the bracket the instance was created for when their
 *     level has none there (HandleBattleFieldPortOpcode would otherwise drop the accept, BattleGroundHandler.cpp:401-403),
 *     and the map is created with the map's own PvpDifficulty row (the spawn mode a level-90 match there gets).
 * Random bots that fill a skirmish are picked around the requester's level by AiPlayerbot.ArenaAnyLevel.* in
 * playerbots.conf (modules/mod_playerbots/src/Manager/ArenaAnyLevelRequests.h).
 *
 * No Mode key (unlike Battleground.AnyLevel.Mode): the skirmish queue is the "All Arenas" map, which has a native
 * bracket for every level 10-90, so there is never a choice of bracket to make. No PvP scaling key: the bot window keeps
 * every participant within AiPlayerbot.ArenaAnyLevel.LevelWindow of the requester and inside the bracket's MaxLevel,
 * and Battleground.AnyLevel.PvPScaling is inert in arenas by construction (its pair test needs Map::IsBattleground(),
 * MAP_BATTLEGROUND only, Map.h:442 / DBCStructure.h:1832).
 *
 * Off (the default) = every function below returns false / nullptr, and each call site then runs its stock code.
 * Kept out of the widely included headers: only BattlegroundAnyLevel.cpp, BattlegroundMgr.cpp, BattleGroundHandler.cpp,
 * and MapInstanced.cpp include this file (mod_playerbots has its own ArenaAnyLevelBots census, ArenaAnyLevelRequests.h).
 */

#ifndef TRINITY_ARENA_ANY_LEVEL_H
#define TRINITY_ARENA_ANY_LEVEL_H

#include "Define.h"

class Battleground;
struct PvPDifficultyEntry;

namespace ArenaAnyLevel
{
    // BattlegroundAnyLevel::LoadConfig (World::LoadConfigSettings: startup and .reload config).
    void LoadConfig();

    bool Enabled();

    // WorldSession::HandleBattlemasterJoinArena: true when an arena join of a player of this level is an unrated skirmish
    // (switch on, level below CONFIG_MAX_PLAYER_LEVEL). False with the switch off.
    bool IsSkirmishJoin(uint32 level);

    // BattlegroundMgr::CreateNewBattleground, "All Arenas" draw: may an arena without a bracket for the match level be
    // drawn? Only for an unrated match with the switch on.
    bool AnyArenaMap(bool isRated);

    // BattlegroundAnyLevel::GetBracketByLevel, after the native lookup found nothing: for a live UNRATED arena instance,
    // the "All Arenas" bracket the instance was created for (Battleground::GetBracketId); nullptr otherwise.
    PvPDifficultyEntry const* InstanceBracket(Battleground const* bg);

    // MapInstanced::CreateBattleground, after the native lookup by the instance's MinLevel found nothing: for an
    // UNRATED arena, the arena map's own lowest PvpDifficulty row (its spawn mode); nullptr otherwise.
    PvPDifficultyEntry const* MapBracket(Battleground const* bg);
}

#endif
