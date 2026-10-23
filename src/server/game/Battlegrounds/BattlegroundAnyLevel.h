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
 * #702 phase 4 (#831): battlegrounds "any level" (beyond AC, beyond SPP, beyond Blizzard 5.4.8).
 *
 * Stock 5.4.8 files a battleground queue under the PvpDifficulty bracket of the player's level
 * (DBCManager::GetBattlegroundBracketByLevel). A level that is below every bracket of a map has no bracket, so
 * the join is dropped: Silvershard Mines, Temple of Kotmogu and Deepwind Gorge only have a level-90 bracket,
 * Twin Peaks and Battle for Gilneas start at 85. SPP (cmangos + mangosbot-bots) never lifts that rule (its
 * BGJoinAction::canJoinBg needs Player::GetBGAccessByLevel and the bot's own bracket), it fills the requester's
 * own bracket with bots of that bracket.
 *
 * With Battleground.AnyLevel.Enable = 1 a player of at least Battleground.AnyLevel.MinLevel who has NO bracket
 * on a battleground map gets a fallback bracket instead (Battleground.AnyLevel.Mode: 0 = the map's lowest
 * bracket, 1 = its highest). A level that already has a bracket keeps it, so every native queue is unchanged.
 * Arenas, the rated battleground and solo queue never get a fallback (arenas are #832).
 * The random bots that fill such a queue are picked around the requester's level by mod_playerbots
 * (AiPlayerbot.BgAnyLevel.*, modules/mod_playerbots/src/Manager/BgAnyLevelRequests.h).
 *
 * Battleground.AnyLevel.PvPScaling = 1 (needs the switch above, read at startup) equalises time-to-kill across
 * the level gap inside a battleground: damage and healing between two units of different levels, one of them
 * below the battleground's own minimum level, are multiplied by baseHP(target level) / baseHP(source level).
 * This is mod-modernWoW's DynamicScaling "Equal TTK" mode (conan513/mod-modernWoW@61632b32
 * src/Features/DynamicScaling/CreatureScalingScript.cpp:6-41, :244-258, :296-312), applied to unit -> unit hits
 * in a battleground instead of player -> creature hits in the open world (see BattlegroundAnyLevel.cpp).
 *
 * Off (the default) = every function below returns the stock answer: GetBracketByLevel returns exactly
 * sDBCManager.GetBattlegroundBracketByLevel(mapId, level), AllowsLevel returns false, ScaleHealing does nothing
 * and no damage script is registered.
 *
 * Kept out of the widely included headers (Battleground.h, World.h, DBCStores.h, Unit.h, Player.h): only the
 * .cpp files that call it include this file, and it needs nothing but forward declarations.
 */

#ifndef TRINITY_BATTLEGROUND_ANY_LEVEL_H
#define TRINITY_BATTLEGROUND_ANY_LEVEL_H

#include "Define.h"

class Battleground;
class Unit;
struct PvPDifficultyEntry;

namespace BattlegroundAnyLevel
{
    // World::LoadConfigSettings (startup and .reload config).
    void LoadConfig();

    // World::SetInitialWorldSettings, after sScriptMgr->Initialize(): registers the PvP damage-scaling UnitScript
    // only when Battleground.AnyLevel.Enable and Battleground.AnyLevel.PvPScaling are both on at startup.
    void RegisterScripts();

    bool Enabled();

    // The bracket a player of this level is filed under on this battleground (template or instance) map.
    // mapId is what the caller looked the bracket up with (normally bg->GetMapId()). Returns
    // sDBCManager.GetBattlegroundBracketByLevel(mapId, level) whenever that is not null or the switch is off;
    // otherwise, for an eligible battleground and a level >= MinLevel, the map's lowest (Mode 0) or highest
    // (Mode 1) bracket.
    PvPDifficultyEntry const* GetBracketByLevel(Battleground const* bg, uint32 mapId, uint32 level);

    // Player::GetBGAccessByLevel for a level BELOW the template's range: true only when the switch is on and
    // GetBracketByLevel would give this level a fallback bracket.
    bool AllowsLevel(Battleground const* bg, uint32 level);

    // Unit::ApplyPvPHealMods (direct heals, heal-over-time ticks, absorb shields): the Equal-TTK heal multiplier.
    void ScaleHealing(Unit const* healer, Unit const* target, uint32& amount);
}

#endif
