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
 * #702 phase 3 (#830): LFG.AnyLevel settings and the .lfgjoin command. See LFGAnyLevel.h for the keys and their call sites.
 *
 * Why a command (read 2026-10-15, tekkub/wow-ui-source tag 5.4.8, the 5.4.8 FrameXML):
 *   - Specific dungeons: FrameXML/LFGFrame.lua:1772-1816 LFGList_DefaultFilterFunction shows a specific dungeon the
 *     server sent no lock for ("If the server tells us we can join, we won't argue", :1786-1789) before any level test,
 *     so the any-level 5-mans of phase 1 appear in the Dungeon Finder's specific list (LFGLockList comes from
 *     GetLFDChoiceLockedState, :1092 / :127, i.e. the lock block of SMSG_LFG_PLAYER_INFO). No command needed there.
 *   - Random dungeons: FrameXML/LFDFrame.lua:280-281 (and ScenarioFinder.lua:258-260 for random scenarios) list a random
 *     entry only when LFG_IsRandomDungeonDisplayable (LFGFrame.lua:2131-2135) passes: player level inside the entry's
 *     own DBC min/max level. A server-side unlock cannot make an out-of-range random entry selectable.
 *   - Scenarios and Raid Finder: PVEFrame.lua:116-127 disable the Scenarios and Raid Finder buttons below
 *     SCENARIOS_SHOW_LEVEL / RAID_FINDER_SHOW_LEVEL (client constants), and ScenarioFinder.lua:32-73 covers the frame
 *     when no scenario's level range holds the player.
 * So `.lfgjoin <id> [roles]` hands the dungeon finder exactly what CMSG_LFG_JOIN would (WorldSession::HandleLfgJoinOpcode,
 * LFGHandler.cpp:26-101, the same checks first): the queue, role check, proposal and teleport are the stock ones, and
 * the 5.4.8 client shows the queue status, proposal and role check as for a UI join. `.lfgjoin list [kind]` prints the
 * entries this character may queue now. Players have the command through RBAC_PERM_JOIN_DUNGEON_FINDER (the permission
 * the dungeon finder itself needs). It exists only while LFG.AnyLevel.Enable is on (registered at startup or by the
 * .reload config that turns the switch on; with the switch off at startup the command table is the stock one).
 */

#include "LFGAnyLevel.h"
#include "Chat.h"
#include "Config.h"
#include "DBCStores.h"
#include "Group.h"
#include "LFGMgr.h"
#include "Log.h"
#include "Player.h"
#include "RBAC.h"
#include "ScriptMgr.h"
#include "WorldSession.h"
#include <cctype>
#include <sstream>
#include <string>

namespace
{
    using namespace lfg;

    bool g_scriptMgrReady = false;     // world thread only
    bool g_scriptsRegistered = false;

    // "random" (any random entry), "dungeon" (5-man normal/heroic), "raid" (LFR), "scenario", "flex", "other"
    char const* KindOf(LFGDungeonData const& dungeon)
    {
        if (dungeon.type == LFG_TYPE_RANDOM)
            return "random";
        if (dungeon.type == LFG_TYPE_RAID)
            return "other";
        switch (dungeon.difficulty)
        {
            case DUNGEON_DIFFICULTY_NORMAL:
            case DUNGEON_DIFFICULTY_HEROIC:
                return "dungeon";
            case RAID_DIFFICULTY_25MAN_LFR:
                return "raid";
            case SCENARIO_DIFFICULTY_NORMAL:
            case SCENARIO_DIFFICULTY_HEROIC:
                return "scenario";
            case RAID_DIFFICULTY_1025MAN_FLEX:
                return "flex";
            default:
                return "other";
        }
    }

    // "tank", "healer"/"heal", "dps"/"damage", comma-separated, or a mask (0x2 tank, 0x4 healer, 0x8 damage). 0 = invalid.
    uint8 ParseRoles(std::string text)
    {
        if (text.empty())
            return PLAYER_ROLE_DAMAGE;

        for (char& c : text)
            c = char(std::tolower(static_cast<unsigned char>(c)));

        if (std::isdigit(static_cast<unsigned char>(text[0])))
        {
            unsigned long mask = 0;
            try { mask = std::stoul(text, nullptr, 0); }
            catch (...) { return 0; }
            mask &= (PLAYER_ROLE_TANK | PLAYER_ROLE_HEALER | PLAYER_ROLE_DAMAGE);
            return uint8(mask);
        }

        uint8 roles = 0;
        std::istringstream parts(text);
        std::string part;
        while (std::getline(parts, part, ','))
        {
            if (part == "tank")
                roles |= PLAYER_ROLE_TANK;
            else if (part == "healer" || part == "heal")
                roles |= PLAYER_ROLE_HEALER;
            else if (part == "dps" || part == "damage")
                roles |= PLAYER_ROLE_DAMAGE;
            else
                return 0;
        }
        return roles;
    }

    void ListDungeons(ChatHandler* handler, Player* player, std::string const& kind)
    {
        LfgLockMap const& locks = sLFGMgr->GetLockedDungeons(player->GetGUID());
        uint32 total = 0, shown = 0;
        for (uint32 i = 0; i < sLFGDungeonStore.GetNumRows(); ++i)
        {
            LFGDungeonEntry const* entry = sLFGDungeonStore.LookupEntry(i);
            if (!entry)
                continue;

            LFGDungeonData const* dungeon = sLFGMgr->GetLFGDungeon(entry->ID);
            if (!dungeon)
                continue;

            std::string const k = KindOf(*dungeon);
            if (k == "other" || k == "flex" || (!kind.empty() && kind != k))
                continue;

            if (locks.find(dungeon->Entry()) != locks.end())
                continue;

            ++total;
            if (shown >= 40)
                continue;

            ++shown;
            handler->PSendSysMessage("  %u  %s (%s, levels %u-%u)", dungeon->id, dungeon->name.c_str(), k.c_str(),
                uint32(dungeon->minlevel), uint32(dungeon->maxlevel));
        }

        if (!total)
            handler->SendSysMessage("No dungeon finder entry of that kind is open to this character.");
        else if (total > shown)
            handler->PSendSysMessage("... and %u more (narrow it: .lfgjoin list random|dungeon|raid|scenario)", total - shown);
    }

    class LfgAnyLevelCommandScript : public CommandScript
    {
    public:
        LfgAnyLevelCommandScript() : CommandScript("LFG_AnyLevel_CommandScript") { }

        ChatCommandTable GetCommands() const override
        {
            static ChatCommandTable commandTable =
            {
                { "lfgjoin", &HandleLfgJoin, rbac::RBAC_PERM_JOIN_DUNGEON_FINDER, Trinity::ChatCommands::Console::No,
                  std::string("Syntax: .lfgjoin <dungeon finder id> [tank|healer|dps|<mask>]  or  .lfgjoin list [random|dungeon|raid|scenario]\n"
                              "Queues the dungeon finder for that entry (any-level server: also random dungeons, scenarios and Raid Finder the UI hides).") },
            };
            return commandTable;
        }

        static bool Usage(ChatHandler* handler)
        {
            handler->SendSysMessage(".lfgjoin <dungeon finder id> [tank|healer|dps|<mask>]   (default dps)");
            handler->SendSysMessage(".lfgjoin list [random|dungeon|raid|scenario]   (the entries this character may queue)");
            handler->SetSentErrorMessage(true);
            return false;
        }

        static bool Refuse(ChatHandler* handler, char const* why)
        {
            handler->SendSysMessage(why);
            handler->SetSentErrorMessage(true);
            return false;
        }

        static bool HandleLfgJoin(ChatHandler* handler, char const* args)
        {
            Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
            if (!player)
                return false;

            if (!AnyLevel::Enabled())
                return Refuse(handler, "LFG.AnyLevel is off on this server: use the Dungeon Finder.");

            std::istringstream in(args ? args : "");
            std::string first, second;
            in >> first >> second;
            if (first.empty())
                return Usage(handler);

            if (first == "list")
            {
                ListDungeons(handler, player, second);
                return true;
            }

            uint32 dungeonId = 0;
            try { dungeonId = uint32(std::stoul(first)); }
            catch (...) { dungeonId = 0; }

            LFGDungeonData const* dungeon = dungeonId ? sLFGMgr->GetLFGDungeon(dungeonId) : nullptr;
            if (!dungeon)
                return Refuse(handler, "Unknown dungeon finder id (see .lfgjoin list).");

            uint8 roles = ParseRoles(second);
            if (!roles)
                return Usage(handler);

            // WorldSession::HandleLfgJoinOpcode (LFGHandler.cpp:28-69): the same gates as a client join
            if (!sLFGMgr->isOptionEnabled(LFG_OPTION_ENABLE_DUNGEON_FINDER | LFG_OPTION_ENABLE_RAID_BROWSER))
                return Refuse(handler, "The dungeon finder is disabled.");

            Group* group = player->GetGroup();
            if (group)
            {
                if (group->GetMembersCount() > GetGroupSize(group->isRaidGroup()))
                    return Refuse(handler, "Your group is too large for the dungeon finder.");

                if (group->isLFGGroup())
                {
                    LfgState const groupState = sLFGMgr->GetActiveState(group->GetGUID());
                    bool const allowed = groupState == LFG_STATE_FINISHED_DUNGEON ||
                        (groupState == LFG_STATE_DUNGEON && group->GetMembersCount() < GetGroupSize(group->isRaidGroup()));
                    if (!allowed)
                        return Refuse(handler, "Your dungeon finder group cannot queue now.");
                }
                else if (player->GetGUID() != group->GetLeaderGUID())
                    return Refuse(handler, "Only the group leader can queue the group.");

                roles |= PLAYER_ROLE_LEADER;   // the client sends the leader bit for the group leader
            }

            LfgDungeonSet dungeons;
            dungeons.insert(dungeonId);
            sLFGMgr->JoinLfg(player, LfgRoles(roles), dungeons, "");

            ObjectGuid const queueGuid = group ? group->GetGUID() : player->GetGUID();
            LfgState const after = sLFGMgr->GetActiveState(queueGuid);
            if (after == LFG_STATE_NONE)
                return Refuse(handler, "Not queued: the dungeon finder refused the join (its message shows why).");

            handler->PSendSysMessage("Queued for %u '%s' (%s, levels %u-%u), roles 0x%X, state %u.", dungeon->id, dungeon->name.c_str(),
                KindOf(*dungeon), uint32(dungeon->minlevel), uint32(dungeon->maxlevel), uint32(roles), uint32(after));
            return true;
        }
    };

    // Script registration grows ScriptMgr's registries: only from the world thread while the map threads are parked
    // (startup, or a .reload config), the same contract as AutoBalance's scripts (AutoBalanceLevelScaling.cpp).
    void RegisterScriptsIfActive()
    {
        if (!g_scriptMgrReady || g_scriptsRegistered || !lfg::AnyLevel::Enabled())
            return;

        g_scriptsRegistered = true;
        new LfgAnyLevelCommandScript();
        ChatHandler::SetLoadCommandTable(true);         // the .lfgjoin table is picked up at the next command
        TC_LOG_INFO("server.loading", "LFG.AnyLevel: .lfgjoin command registered");
    }
}

void lfg::AnyLevel::LoadConfig()
{
    Settings& settings = GetSettings();
    settings.enabled = sConfigMgr->GetBoolDefault("LFG.AnyLevel.Enable", false);
    settings.minLevel = uint32(std::max(1, sConfigMgr->GetIntDefault("LFG.AnyLevel.MinLevel", 10)));
    settings.ignoreItemLevel = sConfigMgr->GetBoolDefault("LFG.AnyLevel.IgnoreItemLevel", true);

    // #830
    settings.random = sConfigMgr->GetBoolDefault("LFG.AnyLevel.Random", false);
    settings.raids = sConfigMgr->GetBoolDefault("LFG.AnyLevel.Raids", false);
    settings.scenarios = sConfigMgr->GetBoolDefault("LFG.AnyLevel.Scenarios", false);
    int32 const loot = sConfigMgr->GetIntDefault("LFG.AnyLevel.Loot", 0);
    settings.loot = loot == int32(LOOT_LEVEL_RANDOM_REWARD) ? uint32(LOOT_LEVEL_RANDOM_REWARD) : uint32(LOOT_NATIVE);

    if (settings.enabled)
    {
        if (loot != int32(LOOT_NATIVE) && loot != int32(LOOT_LEVEL_RANDOM_REWARD))
            TC_LOG_ERROR("server.loading", "LFG.AnyLevel.Loot = %d is not implemented (0 native, 1 level-appropriate random reward); using 0", loot);
        TC_LOG_INFO("server.loading", "LFG.AnyLevel: ON (min level %u, ignore item level %u, random %u, raids %u, scenarios %u, loot %u)",
            settings.minLevel.load(), uint32(settings.ignoreItemLevel.load()), uint32(settings.random.load()), uint32(settings.raids.load()),
            uint32(settings.scenarios.load()), settings.loot.load());
    }

    // a .reload config that turns the switch on registers the command now (no-op at startup: ScriptMgr is not ready yet)
    RegisterScriptsIfActive();
}

void lfg::AnyLevel::RegisterScripts()
{
    g_scriptMgrReady = true;
    RegisterScriptsIfActive();
}
