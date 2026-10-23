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
 * #702 AutoBalance - port of azerothcore/mod-autobalance @ 1d707cb6 (see the .cpp header for the exact source lines and
 * every MoP difference). Phase 1 (#828): level scaling of instance creatures. Phase 2 (#829): player-count scaling
 * (InflectionPoint curve, MinPlayers, StatModifier.* and their per-instance/per-creature overrides), reward scaling
 * (XP, money), heal and crowd-control duration scaling, the .autobalance command, re-processing on .reload config /
 * level-up / players entering or leaving. Config keys are mod-autobalance's own (AutoBalance.*), read from
 * worldserver.conf; everything is off unless AutoBalance.Enable.Global = 1.
 *
 * Kept out of widely included headers: only Creature.cpp, Map.cpp, World.cpp, LootHandler.cpp and the playerbots
 * harness include this file.
 */

#ifndef TRINITY_AUTOBALANCE_LEVEL_SCALING_H
#define TRINITY_AUTOBALANCE_LEVEL_SCALING_H

#include "Define.h"
#include <atomic>
#include <string>

class Creature;
class Map;
class ObjectGuid;
class Player;
struct CreatureTemplate;
struct Loot;

namespace AutoBalance
{
    // AutoBalance.Enable.Global (#829: every AutoBalance feature hangs off it).
    extern std::atomic<bool> g_active;
    // AutoBalance.Enable.Global && AutoBalance.LevelScaling (what `abinfo active=` reports, as in phase 1).
    extern std::atomic<bool> g_levelScalingActive;
    // Some instance still has AutoBalance state (creatures may still carry scaled stats after a .reload config turned the
    // feature off; Creature::Update keeps calling OnCreatureUpdate until they are reverted).
    extern std::atomic<bool> g_hasInstances;

    inline bool IsActive() { return g_active.load(std::memory_order_relaxed); }
    inline bool IsLevelScalingActive() { return g_levelScalingActive.load(std::memory_order_relaxed); }
    inline bool NeedsCreatureUpdate() { return IsActive() || g_hasInstances.load(std::memory_order_relaxed); }

    // World::LoadConfigSettings (startup and .reload config). A reload re-processes every open instance (AB
    // globalConfigTime) and registers the hooks if the feature was turned on.
    void LoadConfig();

    // World::SetInitialWorldSettings, after sScriptMgr->Initialize(). Registers the unit/player/command scripts only when
    // AutoBalance.Enable.Global = 1 (so with it off the hooks run exactly as before); a later .reload config that turns it
    // on registers them then.
    void RegisterScripts();

    // Creature::SelectLevel, right after the level roll (where AC calls OnBeforeCreatureSelectLevel). Returns the level to
    // use. expansionColumn is set to the creature_classlevelstats column the scaled level must read (>= 0) when the level
    // was changed, else left at -1. Also computes the creature's player-count multipliers.
    uint8 OnCreatureSelectLevel(Creature* creature, CreatureTemplate const* cinfo, uint8 level, int32& expansionColumn);

    // Creature::SelectLevel, after the base health and mana of the (scaled) level are known: applies the player-count
    // health/mana multipliers and the armor of the scaled level x the armor multiplier (AB ModifyCreatureAttributes).
    void ModifyCreatureHealthMana(Creature* creature, uint32& health, uint32& mana);

    // Creature::UpdateEntry (and the idle re-scale): the creature's armor x its player-count armor multiplier.
    float ModifyCreatureArmor(Creature* creature, float armor);

    // Creature::Update: re-scales an alive, out-of-combat creature when the instance's players (count or highest level)
    // or the config changed since it was scaled; reverts it when AutoBalance no longer applies.
    void OnCreatureUpdate(Creature* creature);

    // InstanceMap::AddPlayerToMap / RemovePlayerFromMap / ~InstanceMap.
    void OnPlayerEnterInstance(Map* map, Player* player);
    void OnPlayerLeaveInstance(Map* map, Player* player);
    void OnInstanceDestroyed(uint32 instanceId);

    // WorldSession::HandleLootMoneyOpcode, before the money is shared (AC: PlayerScript::OnPlayerBeforeLootMoney).
    void OnPlayerBeforeLootMoney(Player* player, Loot* loot, ObjectGuid const& source);

    // creature_classlevelstats column for a level (the same choice Creature::SelectLevel makes for
    // battleground NPCs, #175: the TBC column has holes, the classic one is 0 above 85).
    uint32 ExpansionColumnForLevel(uint8 level);

    // Harness (.botscenario <player> abinfo): one-line state dumps. Never change anything.
    std::string DescribeMap(Map* map);
    std::string DescribeCreature(Creature* creature);
}

#endif
