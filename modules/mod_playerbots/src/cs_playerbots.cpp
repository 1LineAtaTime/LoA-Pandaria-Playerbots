/*
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

#include "BotScenario.h"
#include "Chat.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PlayerbotMgr.h"
#include "Timer.h"
#include "PerformanceMonitor.h"
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"

#include <sstream>

bool HandleRandomPlayerbotConsoleCommand(ChatHandler* handler, char const* args);   // Manager/RandomPlayerbotMgr.cpp (#808)

// #934 (public #9): AC registers ".playerbots bot" as SEC_PLAYER (Script/PlayerbotCommandScript.cpp), so any player can
// add, remove and control the characters of their own account. Permission 1346 is linked to role 199 (Player Commands) by
// sql/updates/master/auth/2026_10_26_00_playerbots_bot_player_command.sql; a local id keeps RBAC.h (included everywhere)
// untouched. pmon / rndbot / botscenario / botprobe keep their GM permissions. Who may be added stays the job of
// PlayerbotHolder::AddPlayerBot (same account, same guild or a random-bot account).
static rbac::RBACPermissions const RBAC_PERM_COMMAND_PLAYERBOTS_BOT = static_cast<rbac::RBACPermissions>(1346);

class playerbots_commandscript : public CommandScript
{
public:
    playerbots_commandscript() : CommandScript("playerbots_commandscript") {}

    std::vector<ChatCommand> GetCommands() const override
    {
        // #650: AC's ".playerbots ..." table (AC Script/PlayerbotCommandScript.cpp:42-49) - "bot", "pmon" and (#808)
        // "rndbot" exist on this fork; gtask / debug / account have no counterpart here yet. The older top-level
        // ".npcbot" and ".pmon" stay as aliases so existing harness scripts keep working.
        static std::vector<ChatCommand> playerbotsCommandTable =
        {
            { "bot",  &HandlePlayerbotCommand, RBAC_PERM_COMMAND_PLAYERBOTS_BOT, Trinity::ChatCommands::Console::Yes },
            { "pmon", &HandlePerfMonCommand,   rbac::RBAC_PERM_COMMAND_PMON,   Trinity::ChatCommands::Console::Yes },
            // #808: AC SEC_GAMEMASTER + Console::Yes (PlayerbotCommandScript.cpp:45); console/SOAP only needs Console::Yes.
            { "rndbot", &HandleRandomPlayerbotCommand, rbac::RBAC_PERM_COMMAND_PMON, Trinity::ChatCommands::Console::Yes },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "playerbots", playerbotsCommandTable, RBAC_PERM_COMMAND_PLAYERBOTS_BOT, Trinity::ChatCommands::Console::Yes },
            { "npcbot", &HandlePlayerbotCommand, RBAC_PERM_COMMAND_PLAYERBOTS_BOT, Trinity::ChatCommands::Console::Yes },
            { "pmon",   &HandlePerfMonCommand,   rbac::RBAC_PERM_COMMAND_PMON,   Trinity::ChatCommands::Console::Yes },
            { "botscenario", &HandleBotScenarioCommand, rbac::RBAC_PERM_COMMAND_PMON, Trinity::ChatCommands::Console::Yes },
            { "botprobe", &HandleBotProbeCommand, rbac::RBAC_PERM_COMMAND_PMON, Trinity::ChatCommands::Console::Yes },
        };
        return commandTable;
    }

    static bool HandlePlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, args);
    }

    // #808: AC Script/PlayerbotCommandScript.cpp:60-70 HandleRandomPlayerbotCommand -> RandomPlayerbotMgr::
    // HandlePlayerbotConsoleCommand; defined in Manager/RandomPlayerbotMgr.cpp (free function, header untouched).
    static bool HandleRandomPlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return HandleRandomPlayerbotConsoleCommand(handler, args);
    }

    static bool HandlePerfMonCommand(ChatHandler* handler, char const* args)
    {
        if (!strcmp(args, "reset"))
        {
            sPerformanceMonitor->Reset();
            return true;
        }

        if (!strcmp(args, "tick"))
        {
            sPerformanceMonitor->PrintStats(true, false);
            sPerformanceMonitor->DumpJson(true);  // AC PlayerbotCommandScript.cpp:83
            return true;
        }

        if (!strcmp(args, "stack"))
        {
            sPerformanceMonitor->PrintStats(false, true);
            sPerformanceMonitor->DumpJson(false);  // AC PlayerbotCommandScript.cpp:90
            return true;
        }

        if (!strcmp(args, "toggle"))
        {
            sPlayerbotAIConfig->perfMonEnabled = !sPlayerbotAIConfig->perfMonEnabled;
            if (sPlayerbotAIConfig->perfMonEnabled)
                TC_LOG_INFO("playerbots", "Performance monitor enabled");
            else
                TC_LOG_INFO("playerbots", "Performance monitor disabled");
            return true;
        }

        sPerformanceMonitor->PrintStats();
        sPerformanceMonitor->DumpJson(false);  // AC PlayerbotCommandScript.cpp:104
        return true;
    }

    // .botscenario <botname> goto|wander|attack|stop ... - all parsing/state lives in
    // Scenario/BotScenario.cpp (HARNESS-SPEC-botscenario-v3-module); this is a one-line forward so
    // the harness can change without recompiling this file.
    static bool HandleBotScenarioCommand(ChatHandler* handler, char const* args)
    {
        return BotScenario::HandleScenarioCommand(handler, args);
    }

    // .botprobe <botname> on [intervalMs]|off, or .botprobe list - see BotScenario.h.
    static bool HandleBotProbeCommand(ChatHandler* handler, char const* args)
    {
        return BotScenario::HandleProbeCommand(handler, args);
    }
};

void AddSC_playerbots_commandscript() { new playerbots_commandscript(); }
