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
 * #702 phase 1: dungeon finder "any level" switch (beyond AC and beyond Blizzard 5.4.8).
 *
 * With LFG.AnyLevel.Enable = 1 a player of at least LFG.AnyLevel.MinLevel may queue any 5-man dungeon
 * (normal or heroic) whatever its level range: LFGMgr::InitializeLockedDungeons leaves out the
 * TOO_LOW_LEVEL / TOO_HIGH_LEVEL locks (and, with LFG.AnyLevel.IgnoreItemLevel = 1, the item-level lock),
 * and Player::Satisfy lets a dungeon-finder group into the 5-man instance whatever the access_requirement
 * level. This is the dungeon-finder counterpart of SPP's (cmangos) Instance.IgnoreLevel; the content is
 * scaled to the group by AutoBalance (Maps/AutoBalanceLevelScaling.h), the random bots that fill the
 * group are picked around the requester's level by mod_playerbots (AiPlayerbot.LfgAnyLevel.*).
 *
 * #702 phase 3 (#830), every key needs LFG.AnyLevel.Enable and is off by default:
 *  - LFG.AnyLevel.Random: a player outside a random 5-man entry's own level range may queue it; the pick is
 *    made from that entry's whole dungeon group (one expansion, like Blizzard's per-expansion Timewalking
 *    queues). Off, a random queue keeps the stock pool (each member's native level range), also with the
 *    switch on (LFGMgr::JoinLfg, AnyLevelRandomPool).
 *  - LFG.AnyLevel.Raids: Raid Finder (LFR, RAID_DIFFICULTY_25MAN_LFR) wings any level; flex is never unlocked.
 *  - LFG.AnyLevel.Scenarios: scenarios (normal and heroic, specific and random) any level.
 *  - LFG.AnyLevel.Loot: 0 = native rewards; 1 = a random-queue / LFR completion by a player outside the
 *    entry's level range is rewarded with the random 5-man reward of the player's own level
 *    (LFGMgr::GetRandomDungeonReward). 2 (modernWoW-style drop boost) is reserved, read as 0.
 *  - .lfgjoin <id> [roles]: the 5.4.8 client hides out-of-range random dungeons (FrameXML LFGFrame.lua
 *    LFG_IsRandomDungeonDisplayable) and disables the Scenario / Raid Finder tabs below their level
 *    (PVEFrame.lua), so the command is the way to queue those (LFGAnyLevel.cpp). Registered only while
 *    LFG.AnyLevel.Enable is on.
 *
 * Off (the default) = the checks are exactly the stock ones.
 *
 * The settings are atomics read through inline accessors; LoadConfig / RegisterScripts live in LFGAnyLevel.cpp.
 * Read by LFGMgr.cpp, Player.cpp, World.cpp and LFGAnyLevel.cpp only, and kept out of World.h's config enum on
 * purpose (no rebuild of every file that includes World.h).
 */

#ifndef TRINITY_LFG_ANY_LEVEL_H
#define TRINITY_LFG_ANY_LEVEL_H

#include "Config.h"
#include "Define.h"
#include <algorithm>
#include <atomic>

namespace lfg
{
namespace AnyLevel
{
    enum LootMode : uint32
    {
        LOOT_NATIVE                 = 0,    // stock rewards
        LOOT_LEVEL_RANDOM_REWARD    = 1     // the random 5-man reward of the player's own level
    };

    struct Settings
    {
        std::atomic<bool> enabled{ false };
        std::atomic<uint32> minLevel{ 10 };
        std::atomic<bool> ignoreItemLevel{ true };
        std::atomic<bool> random{ false };      // #830
        std::atomic<bool> raids{ false };       // #830
        std::atomic<bool> scenarios{ false };   // #830
        std::atomic<uint32> loot{ LOOT_NATIVE };// #830
    };

    inline Settings& GetSettings()
    {
        static Settings settings;
        return settings;
    }

    // World::LoadConfigSettings (startup and .reload config). A player's cached lock list is rebuilt at
    // login / level-up / equipment change, so a reload reaches an online player at its next rebuild.
    // A reload that turns LFG.AnyLevel.Enable on also registers the .lfgjoin command (LFGAnyLevel.cpp).
    void LoadConfig();

    // World::SetInitialWorldSettings, right after ScriptMgr::Initialize: registers the .lfgjoin command when
    // LFG.AnyLevel.Enable is on (and remembers that later reloads may register it).
    void RegisterScripts();

    inline bool Enabled() { return GetSettings().enabled.load(std::memory_order_relaxed); }
    inline uint32 MinLevel() { return GetSettings().minLevel.load(std::memory_order_relaxed); }
    inline bool IgnoreItemLevel() { return GetSettings().ignoreItemLevel.load(std::memory_order_relaxed); }

    // True when the switch is on and a character of this level may use it.
    inline bool AppliesToLevel(uint32 level) { return Enabled() && level >= MinLevel(); }

    // #830: the per-content keys; each one is false unless LFG.AnyLevel.Enable is on too.
    inline bool Random() { return Enabled() && GetSettings().random.load(std::memory_order_relaxed); }
    inline bool Raids() { return Enabled() && GetSettings().raids.load(std::memory_order_relaxed); }
    inline bool Scenarios() { return Enabled() && GetSettings().scenarios.load(std::memory_order_relaxed); }
    inline uint32 Loot() { return Enabled() ? GetSettings().loot.load(std::memory_order_relaxed) : uint32(LOOT_NATIVE); }
}
}

#endif
