/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
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

#include "PlayerbotAI.h"
#include "RBAC.h"
#include "Timer.h"
#include <map>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>

#include "AiFactory.h"
#include "BotScenario.h"
#include "ChannelMgr.h"
#include "CharacterCache.h"
#include "CreatureAIImpl.h"
#include "Engine.h"
#include "ExternalEventHelper.h"
#include "GuildMgr.h"
#include "Helper.h"
#include "LastMovementValue.h"
#include "LastSpellCastValue.h"
#include "LFGMgr.h"
#include "LootObjectStack.h"
#include "MapManager.h"
#include "NewRpgRoamTransport.h"  // #218 (beyond AC)
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "Player.h"
#include "PositionValue.h"
#include "PointMovementGenerator.h"
#include "Playerbots.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotSpec.h"
#include "PerformanceMonitor.h"
#include "RandomPlayerbotMgr.h"
#include "ServerFacade.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SocialMgr.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "Transport.h"
#include "GameObjectModel.h"
#include "TradeData.h"
#include "Unit.h"
#include "Vehicle.h"
#include "SpellHistory.h"  // #581: Unit::GetSpellHistory()->HasCooldown in CanCastVehicleSpell
#include "BotArc.h"
#include "CheckMountStateAction.h"  // #871: CompleteDismount (SMSG_DISMOUNT)
#include "UpdateFields.h"
#include "Config.h"

// #528: AC botAI->HasCheat(BotCheatMask::taxi) = per-bot mask OR configured AiPlayerbot.BotCheats
// (AC PlayerbotAIConfig.cpp:545-563, default "food,taxi,raid"). Parsed locally (no PlayerbotAIConfig.h change).
static bool HasConfiguredTaxiCheat()
{
    static bool const value = []
    {
        std::stringstream ss(sConfigMgr->GetStringDefault("AiPlayerbot.BotCheats", "food,taxi,raid"));
        std::string token;
        while (std::getline(ss, token, ','))
        {
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            if (token == "taxi")
                return true;
        }
        return false;
    }();
    return value;
}

PlayerbotChatHandler::PlayerbotChatHandler(Player* pMasterPlayer) : ChatHandler(pMasterPlayer->GetSession())
{
}

uint32 PlayerbotChatHandler::extractQuestId(std::string const str)
{
    char* source = (char*)str.c_str();
    char* cId = extractKeyFromLink(source, "Hquest");
    return cId ? atol(cId) : 0;
}

void PacketHandlingHelper::AddHandler(uint16 opcode, std::string const handler)
{
    _handlers[opcode] = handler;
}

// w26 (both cores): AddPacket runs on whichever thread sends the bot a packet (world thread, another map's worker)
// while Handle drains the queue on the bot's map thread; the unguarded std::stack crashed AC at queue.top()
// (Crashes/7f12e89ee5f4_worldserver.exe_[9-10_13-42-58]). One file-static lock (no header change): AddPacket pushes
// under it, Handle swaps the pending packets out under it and handles them unlocked, so a handler that makes the bot
// send another packet only queues it for the next pass.
static std::mutex sPacketQueueLock;

void PacketHandlingHelper::Handle(ExternalEventHelper& helper)
{
    std::stack<WorldPacket> pending;
    {
        std::lock_guard<std::mutex> guard(sPacketQueueLock);
        pending.swap(_queue);
    }
    while (!pending.empty())
    {
        WorldPacket packet = pending.top();
        pending.pop();

        helper.HandlePacket(_handlers, packet);
    }
}

void PacketHandlingHelper::AddPacket(WorldPacket const& packet)
{
    if (packet.empty())
        return;
    // assert(handlers);
    // assert(packet);
    // assert(packet.GetOpcode());
    if (_handlers.find(packet.GetOpcode()) != _handlers.end())
    {
        std::lock_guard<std::mutex> guard(sPacketQueueLock);
        _queue.push(WorldPacket(packet));
    }
}

void AiFactoryEvaluateHealerDps(Player* bot, PlayerbotAI* botAI);   // #817, Factory/AiFactory.cpp

PlayerbotAI::PlayerbotAI()
    : PlayerbotAIBase(true),
    forceRebuff(nullptr),
    _isBotInitializing{ true },
    bot(nullptr),
    accountId(0),
    master(nullptr),
    _aiObjectContext{ nullptr },
    _currentEngine{ nullptr },
    _engines{ nullptr },
    _currentState(BOT_STATE_NON_COMBAT),
    chatHelper(this),
    chatFilter(this),
    security(nullptr)
{
    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
        _engines[i] = nullptr;

    for (uint8 i = 0; i < MAX_ACTIVITY_TYPE; i++)
    {
        _allowActiveCheckTimer[i] = 0;  // #72: AC PlayerbotAI.cpp:231/250 (AllowActivity is ms-based now)
        _allowActive[i] = false;
    }
}

PlayerbotAI::PlayerbotAI(Player* bot)
    : PlayerbotAIBase(true),
    forceRebuff(bot),
    _isBotInitializing{ true },
    bot(bot),
    master(nullptr),
    _aiObjectContext{ nullptr },
    _currentEngine{ nullptr },
    _engines{ nullptr },
    chatHelper(this),
    chatFilter(this),
    security(bot)
{
    // AC PlayerbotAI.cpp:240-241
    if (!bot->isTaxiCheater() && HasConfiguredTaxiCheat())
        bot->SetTaxiCheater(true);

    for (uint8 i = 0; i < MAX_ACTIVITY_TYPE; i++)
    {
        _allowActiveCheckTimer[i] = 0;  // #72: AC PlayerbotAI.cpp:231/250 (AllowActivity is ms-based now)
        _allowActive[i] = false;
    }

    accountId = bot->GetSession()->GetAccountId();
    _aiObjectContext = AiFactory::createAiObjectContext(bot, this);
    _engines[BOT_STATE_COMBAT] = AiFactory::createCombatEngine(bot, this, _aiObjectContext);
    _engines[BOT_STATE_NON_COMBAT] = AiFactory::createNonCombatEngine(bot, this, _aiObjectContext);
    _engines[BOT_STATE_DEAD] = AiFactory::createDeadEngine(bot, this, _aiObjectContext);

    // #545 (AC Bot/PlayerbotAI.cpp:256-257)
    if (sPlayerbotAIConfig->applyInstanceStrategies)
        ApplyInstanceStrategies(bot->GetMapId());

    _currentEngine = _engines[BOT_STATE_NON_COMBAT];
    _currentState = BOT_STATE_NON_COMBAT;
    _rpgInfo.Reset();

    // #178: no CMSG_GROUP_DISBAND handler, as in AC (Bot/PlayerbotAI.cpp:292-293 maps only the uninvite opcodes). 5.4.8 sends DISBAND
    // when the master leaves the party; mapped to "uninvite" it made every bot leave before "party command" (SMSG_PARTY_COMMAND_RESULT) ran.
    masterIncomingPacketHandlers.AddHandler(CMSG_GROUP_UNINVITE_GUID, "uninvite guid");
    masterIncomingPacketHandlers.AddHandler(CMSG_REPOP_REQUEST, "release spirit");
    masterIncomingPacketHandlers.AddHandler(CMSG_RECLAIM_CORPSE, "revive from corpse");
    // #179 (AC Bot/PlayerbotAI.cpp:171-177, spec section 3 "following the master"): use game
    // object (meeting stone summon click), taxi with the master and area trigger / instance
    // portal follow. 5.4.8 renames CMSG_ACTIVATETAXI(EXPRESS) to CMSG_ACTIVATE_TAXI(_EXPRESS); there
    // is no 5.4.8 CMSG_TAXICLEARALLNODES/CLEARNODE equivalent (opcode-mapping research), so the AC
    // "taxi done" handler has nothing to bind to and is left out.
    masterIncomingPacketHandlers.AddHandler(CMSG_GAMEOBJ_USE, "use game object");
    masterIncomingPacketHandlers.AddHandler(CMSG_ACTIVATE_TAXI, "activate taxi");
    masterIncomingPacketHandlers.AddHandler(CMSG_ACTIVATE_TAXI_EXPRESS, "activate taxi");
    masterIncomingPacketHandlers.AddHandler(CMSG_AREATRIGGER, "area trigger");
    // #210 S10 G3 (AC Bot/PlayerbotAI.cpp:180): copy the master's LFG teleport in/out. 5.4.8
    // CMSG_LFG_TELEPORT is a single bit (LfgActions.cpp has the 5.4.8 packet notes).
    masterIncomingPacketHandlers.AddHandler(CMSG_LFG_TELEPORT, "lfg teleport");
    // #648 (AC Bot/PlayerbotAI.cpp:295): the master's spell casts -> "see spell" (SeeSpellAction: RTSC move, fish with
    // master). The 5.4.8 CMSG_CAST_SPELL is bit-packed; SeeSpellAction.cpp reads it like WorldSession::HandleCastSpellOpcode.
    masterIncomingPacketHandlers.AddHandler(CMSG_CAST_SPELL, "see spell");

    botOutgoingPacketHandlers.AddHandler(SMSG_LEVELUP_INFO, "levelup");
    botOutgoingPacketHandlers.AddHandler(SMSG_GROUP_INVITE, "group invite");
    botOutgoingPacketHandlers.AddHandler(SMSG_GROUP_DESTROYED, "group destroyed");
    botOutgoingPacketHandlers.AddHandler(SMSG_PARTY_COMMAND_RESULT, "party command");  // #178: AC Bot/PlayerbotAI.cpp:313
    // #149: the "resurrect request" WorldPacketTrigger (WorldPacketTriggerContext.h) was registered in
    // creators[] but never wired to an opcode, so it could never fire; DeadStrategy schedules "accept
    // resurrect" on it.
    botOutgoingPacketHandlers.AddHandler(SMSG_RESURRECT_REQUEST, "resurrect request");
    // #163 S8 (AC parity, PlayerbotAI.cpp:203): wires the "duel requested" WorldPacketTrigger to the
    // opcode DuelStrategy needs (schedules "accept duel").
    botOutgoingPacketHandlers.AddHandler(SMSG_DUEL_REQUESTED, "duel requested");
    // #163 S8 (AC parity, PlayerbotAI.cpp:208-209): the receiving side of EmoteStrategy.
    botOutgoingPacketHandlers.AddHandler(SMSG_TEXT_EMOTE, "receive text emote");
    botOutgoingPacketHandlers.AddHandler(SMSG_EMOTE, "receive emote");
    // D4 (death-revive-parity.md F4): AC XpGainAction.cpp:16 resets "death count" to 0 on every XP
    // gain (WorldPacketHandlerStrategy.cpp:56, PlayerbotAI.cpp:201). Invisible at the level cap, where
    // AC's own test bots run; matters below it.
    botOutgoingPacketHandlers.AddHandler(SMSG_LOG_XPGAIN, "xpgain");
    // #151 (AC Bot/PlayerbotAI.cpp:196): SMSG_LOOT_RESPONSE -> "loot response" (StoreLootAction).
    botOutgoingPacketHandlers.AddHandler(SMSG_LOOT_RESPONSE, "loot response");
    // #188 (S6 of epic #163, AC Bot/PlayerbotAI.cpp): loot rolls and auto-equip-on-loot. See
    // LootRollAction.cpp/EquipAction.cpp for the 5.4.8 packet-parse notes on each of these.
    botOutgoingPacketHandlers.AddHandler(SMSG_LOOT_START_ROLL, "loot roll");
    botOutgoingPacketHandlers.AddHandler(SMSG_ITEM_PUSH_RESULT, "item push result");
    botOutgoingPacketHandlers.AddHandler(SMSG_LOOT_ROLL_WON, "loot roll won");
    // AC: SMSG_BATTLEFIELD_STATUS -> "bg status" lets a bot accept a BG invite in combat; 5.4.8 sends
    // the invite (and its 20 s reminder) as the NEEDCONFIRMATION status packet
    botOutgoingPacketHandlers.AddHandler(SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION, "bg status");
    // #632 (AC Bot/PlayerbotAI.cpp:304 routes the in-queue SMSG_BATTLEFIELD_STATUS too): STATUS_WAIT_QUEUE is
    // SMSG_BATTLEFIELD_STATUS_QUEUED here; BGStatusAction handles the arena "waited too long" leave.
    botOutgoingPacketHandlers.AddHandler(SMSG_BATTLEFIELD_STATUS_QUEUED, "bg status");
    // #210 S10 G6 (AC parity, Battleground.cpp:1196 EndBattleground -> STATUS_IN_PROGRESS, which this
    // core also sends as SMSG_BATTLEFIELD_STATUS_ACTIVE): lets BGStatusAction notice the match end and
    // leave on maps with no BGTactics case.
    botOutgoingPacketHandlers.AddHandler(SMSG_BATTLEFIELD_STATUS_ACTIVE, "bg status");

    // #210 S10 G1/G2 (AC Bot/PlayerbotAI.cpp:206-207): LFG role check and proposal updates.
    botOutgoingPacketHandlers.AddHandler(SMSG_LFG_ROLE_CHECK_UPDATE, "lfg role check");
    botOutgoingPacketHandlers.AddHandler(SMSG_LFG_PROPOSAL_UPDATE, "lfg proposal");

    // #193 (S7 of epic #163, AC Bot/PlayerbotAI.cpp:194-195): SMSG_TRADE_STATUS/SMSG_TRADE_STATUS_EXTENDED
    // -> "accept trade"/"trade status extended" (TradeStatusAction/TradeStatusExtendedAction).
    // Both are bit-packed on this core - see each action's file header for the 5.4.8 parse.
    botOutgoingPacketHandlers.AddHandler(SMSG_TRADE_STATUS, "trade status");
    botOutgoingPacketHandlers.AddHandler(SMSG_TRADE_STATUS_EXTENDED, "trade status extended");

    // #179 (AC Bot/PlayerbotAI.cpp:191, #134): re-check mount state the instant a nearby unit's run
    // speed changes (mount/dismount included) - the observing bot's own session receives this as a
    // broadcast movement packet, so it fires for the master's speed changes too, not just the
    // bot's own. 5.4.8 renames SMSG_FORCE_RUN_SPEED_CHANGE to SMSG_MOVE_SET_RUN_SPEED.
    botOutgoingPacketHandlers.AddHandler(SMSG_MOVE_SET_RUN_SPEED, "check mount state");

    // #178/#179 (AC Bot/PlayerbotAI.cpp:203): react to the master leaving the party without
    // waiting for the next periodic master-reassignment pass (PartyCommandAction, LeaveGroupAction.*).
    // SMSG_PARTY_COMMAND_RESULT is sent to the party member who issued the command (here: the
    // master), so this is a masterOutgoingPacketHandlers entry, not botOutgoingPacketHandlers.
    masterOutgoingPacketHandlers.AddHandler(SMSG_PARTY_COMMAND_RESULT, "party command");

    // #210 S10 G4 (AC Bot/PlayerbotAI.cpp:216-217): the leader's ready check broadcast and its
    // completion, observed the same way "party command" is - sent to the master, not the bot itself.
    masterOutgoingPacketHandlers.AddHandler(SMSG_RAID_READY_CHECK, "ready check");
    masterOutgoingPacketHandlers.AddHandler(SMSG_RAID_READY_CHECK_COMPLETED, "ready check finished");

    // #257 (AC Bot/PlayerbotAI.cpp:264-265,315-319): the master-commanded quest layer. The master
    // opens a questgiver's menu, accepts and completes quests, pushes a quest into the party log and
    // answers the bot's "do you want this quest?" prompt - the bot observes all of it as packets.
    masterIncomingPacketHandlers.AddHandler(CMSG_GOSSIP_HELLO, "gossip hello");
    masterIncomingPacketHandlers.AddHandler(CMSG_QUESTGIVER_HELLO, "gossip hello");
    masterIncomingPacketHandlers.AddHandler(CMSG_QUESTGIVER_ACCEPT_QUEST, "accept quest");
    masterIncomingPacketHandlers.AddHandler(CMSG_QUESTGIVER_COMPLETE_QUEST, "complete quest");
    masterIncomingPacketHandlers.AddHandler(CMSG_QUEST_CONFIRM_ACCEPT, "confirm quest");
    masterIncomingPacketHandlers.AddHandler(CMSG_PUSHQUESTTOPARTY, "quest share");

    // #257 (AC Bot/PlayerbotAI.cpp:311): the quest turn-in prompt the master is answering.
    masterOutgoingPacketHandlers.AddHandler(SMSG_QUESTGIVER_OFFER_REWARD, "questgiver quest details");

    // #257 (AC Bot/PlayerbotAI.cpp:320-323): the quest progress packets the bot receives itself.
    // 5.4.8 renamed SMSG_QUESTUPDATE_ADD_KILL to SMSG_QUESTUPDATE_ADD_CREDIT and rewrote its layout
    // (Player::SendQuestUpdateAddCredit, core Player.cpp:18302 - it still logs "Sent
    // SMSG_QUESTUPDATE_ADD_KILL"), so QuestUpdateAddKillAction's parser needs the 5.4.8 layout.
    // AC Bot/PlayerbotAI.cpp:320: SMSG_QUESTUPDATE_COMPLETE -> "quest update complete". 5.4.8 does have it,
    // renamed SMSG_QUEST_UPDATE_COMPLETE (Opcodes.h:997, 0x0776), body = uint32 questId
    // (WorldPackets::Quest::QuestUpdateComplete::Write, QuestPackets.cpp:56), sent by Player::SendQuestComplete
    // (Player.cpp:18160/18170) when the last objective of a quest is met.
    botOutgoingPacketHandlers.AddHandler(SMSG_QUEST_UPDATE_COMPLETE, "quest update complete");
    botOutgoingPacketHandlers.AddHandler(SMSG_QUESTUPDATE_ADD_CREDIT, "quest update add kill");
    botOutgoingPacketHandlers.AddHandler(SMSG_QUEST_CONFIRM_ACCEPT, "confirm quest");
    // #258 (AC PlayerbotAI.cpp:299, :301). AC's two BUY_ERR_* "not enough money/reputation" bindings are enum values used as opcodes
    // (AC :302-303) and never fire; they are left out here the same way.
    botOutgoingPacketHandlers.AddHandler(SMSG_GUILD_INVITE, "guild invite");
    botOutgoingPacketHandlers.AddHandler(SMSG_PETITION_SHOW_SIGNATURES, "petition offer");
}

std::set<std::string> PlayerbotAI::unsecuredCommands;

PlayerbotAI::~PlayerbotAI()
{
    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
    {
        if (_engines[i])
            delete _engines[i];
    }

    if (_aiObjectContext)
        delete _aiObjectContext;
}

// #173. Ported from AC Bot/PlayerbotAI.cpp:945-968 (IsAllowedCommand) and :577-788 (HandleCommand),
// trimmed to what this fork's chat layer supports so far:
//  - the "debug " remote-command path (AC HandleRemoteCommand) is dropped - nothing ports it;
//  - the "logout"/"logout cancel"/"wait " branches are dropped - AC itself ships them fully
//    commented out ("TODO: missing implementation to port"), so this matches AC's actual behaviour;
//  - AC's IsAllowedCommand() call before the security check also runs before the chat prefix/
//    separator/filter pipeline (AC PlayerbotAI.cpp:623-624); this fork keeps that ordering.
bool PlayerbotAI::IsAllowedCommand(std::string const text)
{
    if (unsecuredCommands.empty())
    {
        unsecuredCommands.insert("who");
        unsecuredCommands.insert("wts");
        unsecuredCommands.insert("sendmail");
        unsecuredCommands.insert("invite");
        unsecuredCommands.insert("leave");
        unsecuredCommands.insert("lfg");
        unsecuredCommands.insert("pvp stats");
        unsecuredCommands.insert("rpg status");
    }

    for (std::set<std::string>::iterator i = unsecuredCommands.begin(); i != unsecuredCommands.end(); ++i)
    {
        if (text.find(*i) == 0)
            return true;
    }

    return false;
}

namespace
{
    // No core Util::trim() exists on this fork (AC's HandleCommand relies on one); a tiny local
    // helper stands in for it.
    std::string TrimChatCommand(std::string s)
    {
        size_t begin = s.find_first_not_of(" \t");
        if (begin == std::string::npos)
            return "";

        size_t end = s.find_last_not_of(" \t");
        return s.substr(begin, end - begin + 1);
    }
}

void PlayerbotAI::HandleCommand(uint32 type, std::string const text, Player* fromPlayer)
{
    if (!bot || !fromPlayer)
        return;

    std::string filtered = text;

    if (!IsAllowedCommand(filtered) &&
        !GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, type != CHAT_MSG_WHISPER, fromPlayer))
        return;

    if (type == CHAT_MSG_ADDON || type == CHAT_MSG_SYSTEM)
        return;

    if (filtered.find(sPlayerbotAIConfig->commandSeparator) != std::string::npos)
    {
        std::vector<std::string> commands;
        split(commands, filtered, sPlayerbotAIConfig->commandSeparator.c_str());
        for (std::vector<std::string>::iterator i = commands.begin(); i != commands.end(); ++i)
            HandleCommand(type, *i, fromPlayer);

        return;
    }

    if (!sPlayerbotAIConfig->commandPrefix.empty())
    {
        if (filtered.find(sPlayerbotAIConfig->commandPrefix) != 0)
            return;

        filtered = filtered.substr(sPlayerbotAIConfig->commandPrefix.size());
    }

    static std::map<std::string, ChatMsg> chatMap;
    if (chatMap.empty())
    {
        chatMap["#w "] = CHAT_MSG_WHISPER;
        chatMap["#p "] = CHAT_MSG_PARTY;
        chatMap["#r "] = CHAT_MSG_RAID;
        chatMap["#a "] = CHAT_MSG_ADDON;
        chatMap["#g "] = CHAT_MSG_GUILD;
    }

    currentChat = std::pair<ChatMsg, time_t>(CHAT_MSG_WHISPER, 0);
    for (std::map<std::string, ChatMsg>::iterator i = chatMap.begin(); i != chatMap.end(); ++i)
    {
        if (filtered.find(i->first) == 0)
        {
            filtered = filtered.substr(3);
            currentChat = std::pair<ChatMsg, time_t>(i->second, time(nullptr) + 2);
            break;
        }
    }

    filtered = TrimChatCommand(filtered);
    filtered = chatFilter.Filter(filtered);
    if (filtered.empty())
        return;

    if (!IsAllowedCommand(filtered) &&
        !GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_ALLOW_ALL, type != CHAT_MSG_WHISPER, fromPlayer))
        return;

    if (type == CHAT_MSG_RAID_WARNING && filtered.find(bot->GetName()) != std::string::npos &&
        filtered.find("award") == std::string::npos)
    {
        chatCommands.push_back(BotChatCommand("warning", fromPlayer, type));
        return;
    }

    if ((filtered.size() > 2 && filtered.substr(0, 2) == "d ") ||
        (filtered.size() > 3 && filtered.substr(0, 3) == "do "))
    {
        Event event("do", "", fromPlayer);
        std::string action = filtered.substr(filtered.find(' ') + 1);
        DoSpecificAction(action, event);
    }

    if (ChatHelper::parseValue("command", filtered).substr(0, 3) == "do ")
    {
        Event event("do", "", fromPlayer);
        std::string action = ChatHelper::parseValue("command", filtered);
        action = action.substr(3);
        DoSpecificAction(action, event);
    }
    else if (type != CHAT_MSG_WHISPER && filtered.size() > 6 && filtered.substr(0, 6) == "queue ")
    {
        std::string remaining = filtered.substr(filtered.find(' ') + 1);
        int index = 1;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                if (ref->GetSource() == master)
                    continue;

                if (ref->GetSource() == bot)
                    break;

                index++;
            }
        }

        chatCommands.push_back(BotChatCommand(remaining, fromPlayer, type, time(nullptr) + index));
    }
    else if (filtered == "reset")
    {
        Reset(true);
    }
    else
    {
        chatCommands.push_back(BotChatCommand(filtered, fromPlayer, type));
    }
}

// #173. Ported from AC Bot/PlayerbotAI.cpp:577-613; drains the queue HandleCommand() fills, one
// ExternalEventHelper::ParseChatCommand() per queued entry, once its per-entry delay (used by the
// "queue " raid stagger above) has elapsed.
void PlayerbotAI::HandleCommands()
{
    ExternalEventHelper helper(_aiObjectContext);

    for (auto it = chatCommands.begin(); it != chatCommands.end();)
    {
        time_t& checkTime = it->GetTime();
        if (checkTime && time(nullptr) < checkTime)
        {
            ++it;
            continue;
        }

        Player* owner = it->GetOwner();
        if (!owner || it->GetCommand().empty())
        {
            it = chatCommands.erase(it);
            continue;
        }

        helper.ParseChatCommand(it->GetCommand(), owner);
        it = chatCommands.erase(it);
    }
}

// Ported from AC Bot/PlayerbotAI.cpp:2145-2169 (#173). Reserved for the taxi stagger (spec section
// 3.3 / plan batch S5); unused until then.
int32 PlayerbotAI::GetGroupSlotIndex(Player* player)
{
    Group* group = bot->GetGroup();
    if (!group)
        return -1;

    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;

        if (player == member)
            return counter;

        counter++;
    }

    return 0;
}

// AC Bot/PlayerbotAI.cpp:1783-1940 (#545, epic #569). The switch is AC's, every map; a strategy name that
// is not registered yet (raids, Phase 6 dungeons not ported) is skipped by the guard below so the master is
// never told "Added X" for a strategy the engine silently refused (Engine::addStrategy ignores unknown
// names, strategy/Engine.cpp:357-372). AC never hits that case - all its names are registered.
void PlayerbotAI::ApplyInstanceStrategies(uint32 mapId, bool tellMaster)
{
    static const std::vector<std::string> allInstanceStrategies =
    {
        "aq20", "blacktemple", "bwl", "gruulslair", "hyjal", "icc", "karazhan", "magtheridon",
        "moltencore", "naxx", "onyxia", "rs", "ssc", "tbc-ac", "tbc-mech", "tbc-mgt", "tbc-ramp",
        "tbc-seth", "tbc-ub", "tempestkeep", "ulduar", "voa", "wotlk-an", "wotlk-cos", "wotlk-dtk",
        "wotlk-eoe", "wotlk-fos", "wotlk-gd", "wotlk-hol", "wotlk-hos", "wotlk-nex", "wotlk-occ",
        "wotlk-ok", "wotlk-os", "wotlk-pos", "wotlk-toc", "wotlk-uk", "wotlk-up", "wotlk-vh",
        "zulaman",
        // epic #373 (MoP-only dungeons)
        "cata-bc", "cata-dm", "cata-et", "cata-gb", "cata-hoo", "cata-hot", "cata-lct", "cata-sc",
        "cata-sfk", "cata-tott", "cata-vp", "cata-woe", "cata-za", "cata-zg", "mop-gss", "mop-msp",
        "mop-scholo", "mop-sh", "mop-sm", "mop-snt", "mop-spm", "mop-ssb", "mop-tjs"
    };

    for (std::string const& strat : allInstanceStrategies)
    {
        _engines[BOT_STATE_COMBAT]->removeStrategy(strat);
        _engines[BOT_STATE_NON_COMBAT]->removeStrategy(strat);
    }

    std::string strategyName;
    switch (mapId)
    {
        case 33: strategyName = "cata-sfk"; break;       // Shadowfang Keep (revamped)
        case 36: strategyName = "cata-dm"; break;        // Deadmines (revamped)
        case 249: strategyName = "onyxia"; break;        // Onyxia's Lair
        case 409: strategyName = "moltencore"; break;    // Molten Core
        case 469: strategyName = "bwl"; break;           // Blackwing Lair
        case 509: strategyName = "aq20"; break;          // Ruins of Ahn'Qiraj
        case 532: strategyName = "karazhan"; break;      // Karazhan
        case 533: strategyName = "naxx"; break;          // Naxxramas
        case 534: strategyName = "hyjal"; break;         // The Battle for Mount Hyjal (Hyjal Summit)
        case 543: strategyName = "tbc-ramp"; break;      // Hellfire Citadel: Hellfire Ramparts
        case 544: strategyName = "magtheridon"; break;   // Magtheridon's Lair
        case 546: strategyName = "tbc-ub"; break;        // Coilfang Reservoir: The Underbog
        case 548: strategyName = "ssc"; break;           // Serpentshrine Cavern
        case 550: strategyName = "tempestkeep"; break;   // Tempest Keep: The Eye
        case 556: strategyName = "tbc-seth"; break;      // Auchindoun: Sethekk Halls
        case 554: strategyName = "tbc-mech"; break;      // Tempest Keep: The Mechanar
        case 558: strategyName = "tbc-ac"; break;        // Auchindoun: Auchenai Crypts
        case 564: strategyName = "blacktemple"; break;   // Black Temple
        case 565: strategyName = "gruulslair"; break;    // Gruul's Lair
        case 568: strategyName = "cata-za"; break;       // Zul'Aman (Cataclysm 5-man; AC's TBC raid "zulaman" is a different instance)
        case 574: strategyName = "wotlk-uk"; break;      // Utgarde Keep
        case 575: strategyName = "wotlk-up"; break;      // Utgarde Pinnacle
        case 576: strategyName = "wotlk-nex"; break;     // The Nexus
        case 578: strategyName = "wotlk-occ"; break;     // The Oculus
        case 585: strategyName = "tbc-mgt"; break;       // Magisters' Terrace
        case 595: strategyName = "wotlk-cos"; break;     // The Culling of Stratholme
        case 599: strategyName = "wotlk-hos"; break;     // Halls of Stone
        case 600: strategyName = "wotlk-dtk"; break;     // Drak'Tharon Keep
        case 601: strategyName = "wotlk-an"; break;      // Azjol-Nerub
        case 602: strategyName = "wotlk-hol"; break;     // Halls of Lightning
        case 603: strategyName = "ulduar"; break;        // Ulduar
        case 604: strategyName = "wotlk-gd"; break;      // Gundrak
        case 608: strategyName = "wotlk-vh"; break;      // Violet Hold
        case 615: strategyName = "wotlk-os"; break;      // Obsidian Sanctum
        case 616: strategyName = "wotlk-eoe"; break;     // Eye Of Eternity
        case 619: strategyName = "wotlk-ok"; break;      // Ahn'kahet: The Old Kingdom
        case 624: strategyName = "voa"; break;           // Vault of Archavon
        case 631: strategyName = "icc"; break;           // Icecrown Citadel
        case 632: strategyName = "wotlk-fos"; break;     // The Forge of Souls
        case 643: strategyName = "cata-tott"; break;     // Throne of the Tides
        case 644: strategyName = "cata-hoo"; break;      // Halls of Origination
        case 645: strategyName = "cata-bc"; break;       // Blackrock Caverns
        case 650: strategyName = "wotlk-toc"; break;     // Trial of the Champion
        case 657: strategyName = "cata-vp"; break;       // The Vortex Pinnacle
        case 658: strategyName = "wotlk-pos"; break;     // Pit of Saron
        case 670: strategyName = "cata-gb"; break;       // Grim Batol
        case 724: strategyName = "rs"; break;            // Ruby Sanctum
        case 725: strategyName = "cata-sc"; break;       // The Stonecore
        case 755: strategyName = "cata-lct"; break;      // Lost City of the Tol'vir
        case 859: strategyName = "cata-zg"; break;       // Zul'Gurub (Cataclysm 5-man)
        case 938: strategyName = "cata-et"; break;       // Caverns of Time: End Time
        case 939: strategyName = "cata-woe"; break;      // Caverns of Time: Well of Eternity
        case 940: strategyName = "cata-hot"; break;      // Caverns of Time: Hour of Twilight
        case 959: strategyName = "mop-spm"; break;       // Shado-Pan Monastery
        case 960: strategyName = "mop-tjs"; break;       // Temple of the Jade Serpent
        case 961: strategyName = "mop-ssb"; break;       // Stormstout Brewery
        case 962: strategyName = "mop-gss"; break;       // Gate of the Setting Sun
        case 994: strategyName = "mop-msp"; break;       // Mogu'shan Palace
        case 1001: strategyName = "mop-sh"; break;       // Scarlet Halls
        case 1004: strategyName = "mop-sm"; break;       // Scarlet Monastery
        case 1007: strategyName = "mop-scholo"; break;   // Scholomance
        case 1011: strategyName = "mop-snt"; break;      // Siege of Niuzao Temple
        default:
            break;
    }

    if (strategyName.empty())
        return;

    // fork-only guard (see the header comment)
    if (!_aiObjectContext->GetSupportedStrategies().count(strategyName))
        return;

    _engines[BOT_STATE_COMBAT]->addStrategy(strategyName);
    _engines[BOT_STATE_NON_COMBAT]->addStrategy(strategyName);

    if (tellMaster && !strategyName.empty())
    {
        std::ostringstream out;
        out << "Added " << strategyName << " instance strategy";
        TellMasterNoFacing(out.str());
    }
}

uint32 PlayerbotAI::GetReactDelay()
{
    uint32 base = sPlayerbotAIConfig->reactDelay;  // Default 100(ms)

    // #218 (BEYOND AC, MoP only - NewRpgRoamTransport.h): a bot waiting for, boarding, riding or leaving a transport
    // on its own trip reacts like AC's bot with a real-player master. A resting bot otherwise ticks every 2-20 s and
    // misses an elevator's 5 s stop (wave 33, Thunder Bluff leg E).
    if (RoamTransport::WantsFastReact(bot))
        return base;

    // If dynamic react delay is disabled, use a static calculation
    if (!sPlayerbotAIConfig->dynamicReactDelay)
    {
        if (HasRealPlayerMaster())
            return base;

        bool inBG = bot->InBattleground() || bot->InArena();
        if (sPlayerbotAIConfig->fastReactInBG && inBG)
            return base;

        bool inCombat = bot->IsInCombat();

        if (!inCombat)
            return base * 10.0f;

        else if (inCombat)
            return base * 2.5f;

        return base;
    }

    // Dynamic react delay calculation:

    if (HasRealPlayerMaster())
        return base;

    float multiplier = 1.0f;
    bool inBG = bot->InBattleground() || bot->InArena();

    if (inBG)
    {
        if (bot->IsInCombat() || _currentState == BOT_STATE_COMBAT)
        {
            multiplier = sPlayerbotAIConfig->fastReactInBG ? 2.5f : 5.0f;
            return base * multiplier;
        }
        else
        {
            multiplier = sPlayerbotAIConfig->fastReactInBG ? 1.0f : 10.0f;
            return base * multiplier;
        }
    }

    // When in combat, return 5 times the base
    if (bot->IsInCombat() || _currentState == BOT_STATE_COMBAT)
    {
        multiplier = 5.0f;
        return base * multiplier;
    }

    // When not resting, return 10-30 times the base
    if (!bot->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
    {
        multiplier = urand(10, 30);
        return base * multiplier;
    }

    // In other cases, return 20-200 times the base
    multiplier = urand(20, 200);
    return base * multiplier;
}

namespace
{
    // Issue #141 item 1: AC ports the transport boarding/leaving check from PlayerbotAI::UpdateAI
    // (AC Bot/PlayerbotAI.cpp:388-412). Its timer state (AC's PlayerbotAI member nextTransportCheck)
    // is kept here instead of in PlayerbotAI.h, which is a widely included header, per one bot's
    // GUID. The bot roster is fixed-size (the bot pool), so this map does not grow unbounded.
    std::unordered_map<ObjectGuid::LowType, uint32> g_nextTransportCheck;
    // #141 F6: g_nextTransportCheck above is read/written from PlayerbotAI::UpdateAI, which runs on
    // whichever map-update thread owns the bot's map. AC's equivalent state is a per-bot PlayerbotAI
    // member (AC PlayerbotAI.h:655), so it never races; this map is only safe as-is while
    // MapUpdate.Threads = 1 (the MoP test conf). Guard it so a future multi-threaded map update does
    // not corrupt it.
    std::mutex g_nextTransportCheckMutex;

    // #141 F5 (AC parity): AC's StopMovingOnCurrentPos (AC Unit.cpp:12636-12649) always launches a
    // zero-length spline in the bot's current frame (transport-relative when ONTRANSPORT + a
    // transport guid are set), so a board/leave shows up to observers even when the old spline was
    // already finalized. bot->StopMoving() (Unit.cpp:14862-14876) is a no-op in exactly that case -
    // the common one for an idle bot - so it never sends the transport-attached packet. Unit::
    // DisableSpline() is protected (Unit.h:2681), so this stands in for it via a fresh
    // MoveSplineInit, which already falls back to the transport offset when the frame differs
    // (MoveSplineInit.cpp:64-86).
    void StopOnCurrentPos(Player* bot)
    {
        bot->ClearUnitState(UNIT_STATE_MOVING);
        if (!bot->IsInWorld())
            return;

        Movement::MoveSplineInit init(bot);
        init.MoveTo(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), false, true);
        init.SetFacing(bot->GetOrientation());
        init.Launch();
    }

    // #206/#209 F3 (owner requirement, beyond AC): a static transport (elevator/tram) only counts as
    // under a unit when its floor is within ~2 yd below the unit's feet (a 4 yd ray). The core's
    // GetTransportForPos ray reaches ~30 yd down, which attached a bot standing in an empty shaft with
    // the gap as its transport offset (s9-elevator-boat-rootcause.md). Same helper as FollowActions.cpp.
    bool IsUnderStaticTransportDeck(Transport* t, float x, float y, float z)
    {
        if (!t || !t->m_model)
            return false;
        G3D::Vector3 const v(x, y, z + 2.0f);
        G3D::Ray const r(v, G3D::Vector3(0.0f, 0.0f, -1.0f));
        float dist = 4.0f;
        return t->m_model->intersectRay(r, dist, false, t->GetPhaseMask(), VMAP::ModelIgnoreFlags::Nothing);
    }

    // #223 (AC parity, AC Bot/PlayerbotAI.cpp ambient transport check): the core probe used there is
    // a single exact-Z GetTransportForPos ray, so on a moving deck it can miss for one 1 s sample and
    // the rider is dropped for good (by the next check the deck has moved away). Same 4-offset
    // tolerant probe FollowAction already uses for boarding (FollowActions.cpp,
    // GetTransportForPosTolerant); duplicated file-locally because that one is in another TU.
    Transport* GetTransportForPosTolerant(Map* map, WorldObject* ref, uint32 phaseMask, float x, float y, float z)
    {
        if (!map || !ref)
            return nullptr;

        float const probes[4] = { z, z + 0.5f, z + 1.5f, z - 0.5f };
        for (float const pz : probes)
        {
            Transport* t = map->GetTransportForPos(phaseMask, x, y, pz, ref);
            // #141 F3 parity (crash hazard) and #206/#209 F3: same two guards as the block below.
            if (t && !t->GetGUID().IsMOTransport())
                t = nullptr;
            if (t && t->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT && !IsUnderStaticTransportDeck(t, x, y, pz))
                t = nullptr;
            if (t)
                return t;
        }

        return nullptr;
    }

    // #223/#224 evidence helpers. Only ever reached from inside a TC_LOG_DEBUG argument list, which
    // Log.h evaluates only when the playerbots log category is enabled, so they cost nothing when it
    // is off. All of them are null-safe: a missing bot/map/transport can never crash a log line.
    std::string TransportText(Transport* t)
    {
        return t ? t->GetGUID().ToString() : std::string("none");
    }

    Transport* ProbeTransportExact(Player* bot)
    {
        if (!bot || !bot->IsInWorld() || !bot->GetMap())
            return nullptr;
        return bot->GetMap()->GetTransportForPos(bot->GetPhaseMask(), bot->GetPositionX(),
            bot->GetPositionY(), bot->GetPositionZ(), bot);
    }

    // #223/#224 P3: deck anchoring. Transport::AddPassenger (Transport.cpp:551-593) stores
    // CalculatePassengerOffset(wherever the bot happens to be standing); after a .summon that is the
    // summoner's spot at the edge of the deck, which can be off-model. When the master is a genuine
    // passenger of that same transport its own stored offset came from a real client, so it is
    // deck-valid: copy it, then re-derive the world position with CalculatePassengerPosition, so
    // Transport::UpdatePassengerPositions (Transport.cpp:1091-1092) carries the bot on the deck from
    // the first tick instead of at a poisoned local (0,0,0). Same recipe as AC's own transport
    // harness (AC Scenario/BotScenario.cpp:4000-4021). Returns true only when a snap happened.
    bool SnapToDeckOffsetFromMaster(Player* bot, Player* master, Transport* transport)
    {
        if (!bot || !master || !transport || master == bot)
            return false;
        if (!bot->IsInWorld() || !master->IsInWorld() || !transport->IsInWorld())
            return false;
        if (bot->GetMap() != transport->GetMap())
            return false;
        if (master->GetTransport() != transport && master->m_movementInfo.transport.guid != transport->GetGUID())
            return false;

        float lx = master->m_movementInfo.transport.pos.GetPositionX();
        float ly = master->m_movementInfo.transport.pos.GetPositionY();
        float lz = master->m_movementInfo.transport.pos.GetPositionZ();
        float lo = master->m_movementInfo.transport.pos.GetOrientation();
        float wx = lx, wy = ly, wz = lz;
        transport->CalculatePassengerPosition(wx, wy, wz, nullptr);
        bot->m_movementInfo.transport.pos.Relocate(lx, ly, lz, lo);
        bot->Relocate(wx, wy, wz, lo);
        return true;
    }

    // #316: fallback deck anchoring for a rider that the master-based snap cannot rescue. When the bot
    // is de-seated mid-ride (already below the hull - z < 0 on a boat) both transport probes miss, and
    // if the master is not itself a passenger of oldTransport (it already left the deck, or it never
    // boarded) SnapToDeckOffsetFromMaster bails out at its master check - which is the case that dropped
    // the rider into the sea and left it swimming. The bot's own m_movementInfo.transport.pos is still
    // the offset Transport::AddPassenger stored when it boarded, so re-seating from it is enough to keep
    // the attachment: re-derive the world position with CalculatePassengerPosition exactly as the master
    // path does, but leave the stored offset alone. That last part is the deliberate difference - a
    // master offset is deck-valid by construction, a self offset is only the last known good one, so it
    // must never be written back over the stored offset. Guards match the master helper minus its master
    // checks. Returns true only when a snap happened.
    bool SnapToOwnDeckOffset(Player* bot, Transport* transport)
    {
        if (!bot || !transport)
            return false;
        if (!bot->IsInWorld() || !transport->IsInWorld())
            return false;
        if (bot->GetMap() != transport->GetMap())
            return false;

        float lx = bot->m_movementInfo.transport.pos.GetPositionX();
        float ly = bot->m_movementInfo.transport.pos.GetPositionY();
        float lz = bot->m_movementInfo.transport.pos.GetPositionZ();
        float lo = bot->m_movementInfo.transport.pos.GetOrientation();

        // Usability: a never-written offset is the poisoned local (0,0,0) that #223 P3 warns about -
        // snapping to it would land the bot on the transport's own origin, not on its deck.
        if (lx == 0.0f && ly == 0.0f && lz == 0.0f)
            return false;

        float wx = lx, wy = ly, wz = lz;
        transport->CalculatePassengerPosition(wx, wy, wz, nullptr);
        // #879: re-seat only onto the deck. While a passenger walks, Unit::UpdateSplinePosition writes its
        // transport-local spline position into this offset, so a masterless rider that walked off onto a pier
        // carries an offset that projects off the hull (w69 Stallian stored 168.2/44.0/14.5, Zolan 72.2/11.0/5.3):
        // re-seating there kept the anchor and the ship carried the bot back and forth. Same 4 yd deck ray as
        // FollowActions.cpp / RoamTransport DeckUnder; no deck there -> no snap -> the removal below (AC).
        if (!IsUnderStaticTransportDeck(transport, wx, wy, wz))
            return false;
        bot->Relocate(wx, wy, wz, lo);
        return true;
    }

    // #889 (MoP extension of #206/#209 F3, beyond AC - AC ignores static transports): is this rider still being
    // carried by its static (type-11) platform? Its world position is the projection of its stored transport offset
    // (Transport::UpdatePassengerPositions relocates it there every platform tick; 2 yd covers one tick of platform
    // travel), and that position lies inside the platform model's world bounds widened by 1.5 yd horizontally, 1 yd
    // down and 4 yd up. Unlike the deck rays this does not need the deck right under the bot's feet: Orgrimmar elevator
    // 206610 (Orgrimmar_Elevator_02.m2, collision box -5.67..5.67 x -5.67..5.67 x -0.38..0.40) carried Cellietta at the
    // offset (-0.70,-6.13,1.94), 0.46 yd past its edge, so every ray missed (w88 Z4 "exact=none tol=none").
    bool StaticRiderCarried(Player* bot, Transport* t)
    {
        if (!bot || !t || !t->m_model || !t->IsInWorld() || t->GetGoType() != GAMEOBJECT_TYPE_TRANSPORT)
            return false;
        float wx = bot->m_movementInfo.transport.pos.GetPositionX();
        float wy = bot->m_movementInfo.transport.pos.GetPositionY();
        float wz = bot->m_movementInfo.transport.pos.GetPositionZ();
        if (wx == 0.0f && wy == 0.0f && wz == 0.0f)
            return false;  // never-written offset (#223 P3)
        t->CalculatePassengerPosition(wx, wy, wz, nullptr);
        if (bot->GetExactDist(wx, wy, wz) > 2.0f)
            return false;
        G3D::AABox const& b = t->m_model->getBounds();
        float const x = bot->GetPositionX(), y = bot->GetPositionY(), z = bot->GetPositionZ();
        return x >= b.low().x - 1.5f && x <= b.high().x + 1.5f && y >= b.low().y - 1.5f && y <= b.high().y + 1.5f &&
               z >= b.low().z - 1.0f && z <= b.high().z + 4.0f;
    }

    // #586 (owner requirement, beyond AC): is this MO transport (boat/zeppelin) stopped at a dock right
    // now? Same keyframe test as FollowActions.cpp IsDockedNow (duplicated file-locally: other TU). Only an
    // MO transport has keyframes (#587: a static transport's GetKeyFrames() dereferences nullptr).
    bool IsMoTransportDockedNow(Transport* t)
    {
        if (!t || t->GetGoType() != GAMEOBJECT_TYPE_MO_TRANSPORT || !t->GetTransportTemplate() ||
            !t->GetTransportPeriod())
            return false;
        uint32 const tm = t->GetTimer() % t->GetTransportPeriod();
        for (KeyFrame const& k : t->GetKeyFrames())
            if (tm >= k.ArriveTime && tm < k.DepartureTime)
                return true;
        return false;
    }

    // #586: the rider walked off a STOPPED platform after a master who has already left it. Then the
    // #316 own-offset re-seat must not run: while a bot walks as a passenger its stored offset follows
    // its feet (Unit::UpdateSplinePosition writes the transport-local spline position), so re-deriving
    // the world position from that offset lands it exactly where it already stands - off the deck - and
    // only keeps the attachment, which then carries it away when the platform leaves (s9-walkon zep w9:
    // [TRN][AMB] act=reanchor botpos == projected stored offset, mdist 0.0 until departure, then 60-95 yd).
    // All four must hold, so the #316 rescue is untouched wherever it can actually do something:
    //  - the master is in world on the bot's map and is neither seated on nor standing on this platform;
    //  - the platform is stopped: an MO transport inside a keyframe stop window, or a static
    //    (elevator/tram) platform whose rider is level with the master (it is at the master's landing);
    //  - the bot's own offset already resolves to its current position (re-seating would not move it,
    //    so it cannot put it back on the deck; a de-seated rider of #316 is > 2 yd from its offset).
    // AC removes such a rider unconditionally (AC Bot/PlayerbotAI.cpp ambient check); this restores
    // AC's removal for exactly this case.
    // #586/#206: the master is in world on the bot's map and is neither seated on nor standing on this platform.
    bool MasterOffPlatform(Player* bot, Player* master, Transport* transport)
    {
        if (!bot || !master || !transport || master == bot)
            return false;
        if (!bot->IsInWorld() || !master->IsInWorld() || !transport->IsInWorld())
            return false;
        if (master->GetMap() != bot->GetMap() || transport->GetMap() != bot->GetMap())
            return false;
        if (master->GetTransport() == transport)
            return false;
        if (GetTransportForPosTolerant(master->GetMap(), master, master->GetPhaseMask(),
                master->GetPositionX(), master->GetPositionY(), master->GetPositionZ()) == transport)
            return false;
        return true;
    }

    // #206 (#586 follow-up, owner requirement beyond AC): the bot stands right beside a master who is off this
    // platform - within 5 yd (2D) and level with it (|dz| <= 2) - so it is where its master is, on the dock /
    // landing, and has nothing to ride. The core probe cannot tell: Map::GetTransportForPos (Map.cpp:2514-2526)
    // reports an MO transport for any point whose 30 yd DOWNWARD ray meets the hull, and at Vengeance Landing
    // the zeppelin's hull sits under the tower edge (s9wo-mop-zep-w11-1512): Aleneya walked off to mdist 0.0
    // at (1976.4,-6092.4,66.9) and the exact probe still hit 181689, so she was never removed; Carius was
    // removed (act=remove 1791228229) and then RE-attached by the departing hull passing under him (ambient
    // "is on a transport" with no act line, 1791228270, stored z -18.45 = tower level). #316's own-offset
    // re-seat then carried both to Undercity. A rider of the #316 rescue is never in this state: its master is
    // still a passenger, or it is far from the master.
    bool BotBesideMasterOffPlatform(Player* bot, Player* master, Transport* transport)
    {
        if (!MasterOffPlatform(bot, master, transport))
            return false;
        return bot->GetExactDist2d(master) <= 5.0f && std::fabs(bot->GetPositionZ() - master->GetPositionZ()) <= 2.0f;
    }

    // #586: the rule described in the comment above MasterOffPlatform.
    bool RiderWalkedOffStoppedPlatform(Player* bot, Player* master, Transport* transport)
    {
        if (!MasterOffPlatform(bot, master, transport))
            return false;
        // #206: a rider beside its off-platform master is removed even while the platform moves (see above).
        if (BotBesideMasterOffPlatform(bot, master, transport))
            return true;

        bool stopped = false;
        if (transport->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT)
            stopped = IsMoTransportDockedNow(transport);
        else if (transport->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT)
            stopped = std::fabs(bot->GetPositionZ() - master->GetPositionZ()) <= 4.0f;
        if (!stopped)
            return false;

        float wx = bot->m_movementInfo.transport.pos.GetPositionX();
        float wy = bot->m_movementInfo.transport.pos.GetPositionY();
        float wz = bot->m_movementInfo.transport.pos.GetPositionZ();
        transport->CalculatePassengerPosition(wx, wy, wz, nullptr);
        return bot->GetExactDist(wx, wy, wz) <= 2.0f;
    }

    // #223/#224: the single seat used by both teleport branches.
    // P1 far branch, P2 stop-before-board, P3 deck snap.
    void SeatBotOnMasterTransport(Player* bot, Player* master)
    {
        if (!bot || !master || !bot->IsInWorld() || !master->IsInWorld())
            return;
        Transport* transport = master->GetTransport();
        if (!transport || bot->GetTransport() || !transport->IsInWorld())
            return;
        if (bot->GetMap() != transport->GetMap())
            return;
        // #584: only a teleport TO the master may seat the bot (#224 summon/follow lands within contact range).
        // Any other teleport (harness, quest, a stale bot master riding a zeppelin far away) is left to the
        // ambient transport check, as upstream AC does - seating it here put it on a deck ~95 yd away.
        if (bot->GetExactDist(master) > 10.0f)
            return;

        // P2: stop BEFORE boarding. Unit::StopMoving -> MoveSplineInit::Stop() builds its args with
        // TransformForTransport = unit->GetTransGUID() (MoveSplineInit.cpp:125-148), so stopping after
        // AddPassenger seals a world-frame spline onto a transport-local frame, and
        // Unit::UpdateSplinePosition (Unit.cpp:549-568) then writes that WORLD position into
        // m_movementInfo.transport.pos every tick - a guaranteed garbage offset. Stopping first also
        // finalizes the spline so that code cannot run at all.
        if (bot->GetMotionMaster())
            bot->GetMotionMaster()->Clear(true);
        bot->StopMoving();

        transport->AddPassenger(bot, true);

        // P3: replace the offset AddPassenger just stored (the summoner's spot on the deck edge) with
        // the master's deck-valid one.
        bool const snapped = SnapToDeckOffsetFromMaster(bot, master, transport);

        // Evidence line: never dereferences null (every pointer is guarded above / by the helpers).
        // Cheap: TC_LOG_DEBUG only evaluates its arguments when the playerbots category is enabled
        // (Log.h TC_LOG_MESSAGE_BODY), so the two raycast probes cost nothing when it is off.
        TC_LOG_DEBUG("playerbots",
            "[TRN][SEAT] bot=%s master=%s tr=%s type=%u botpos=%.2f/%.2f/%.2f stored=%.2f/%.2f/%.2f/%.3f onTr=%d snapped=%d exact=%s tol=%s",
            bot->GetName().c_str(), master->GetName().c_str(), TransportText(transport),
            (uint32)transport->GetGoType(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            bot->m_movementInfo.transport.pos.GetPositionX(), bot->m_movementInfo.transport.pos.GetPositionY(),
            bot->m_movementInfo.transport.pos.GetPositionZ(), bot->m_movementInfo.transport.pos.GetOrientation(),
            bot->GetTransport() ? 1 : 0, snapped ? 1 : 0, TransportText(ProbeTransportExact(bot)),
            TransportText(GetTransportForPosTolerant(bot->GetMap(), bot, bot->GetPhaseMask(),
                bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())));
    }

    // #225 (AC parity): AC flies a multi-leg taxi as ONE spline and advances the route inside the
    // movement generator (AC WaypointMovementGenerator.cpp:579 LoadPath merges every leg, :720-759
    // DoUpdate calls NextTaxiDestination at each _pointsForPathSwitch boundary). The MoP core flies
    // exactly one leg per FlightPathMovementGenerator (MotionMaster.cpp:953-961) and only starts the
    // next leg when the session receives the CLIENT's CMSG_MOVE_SPLINE_DONE (TaxiHandler.cpp:339-418
    // HandleMoveSplineDoneOpcode). A bot has no client, so it stops at the first connection point and
    // the rest of the route stays unconsumed in m_taxi. Replay that handler's same-map branch here.
    void ContinueTaxiIfLanded(Player* bot)
    {
        if (!bot || !bot->IsInWorld() || bot->IsBeingTeleported())
            return;
        if (bot->IsInFlight() || !bot->GetMotionMaster() ||
            bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE)
            return;                                   // a leg is still running
        if (bot->m_taxi.empty())
            return;

        uint32 current = bot->m_taxi.GetTaxiDestination();
        if (current)
        {
            // Only continue once the leg that was flying actually reached its node.
            TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(current);
            if (!node || node->map_id != bot->GetMapId() ||
                bot->GetExactDist2d(node->x, node->y) > 60.0f)
                return;
        }

        // Handlers/TaxiHandler.cpp:391-424, same-map branch.
        uint32 destinationnode = bot->m_taxi.NextTaxiDestination();
        if (destinationnode)
        {
            uint32 sourcenode = bot->m_taxi.GetTaxiSource();
            uint32 mountId = sObjectMgr->GetTaxiMountDisplayId(sourcenode, bot->GetTeam());
            uint32 path = 0, cost = 0;
            sObjectMgr->GetTaxiPath(sourcenode, destinationnode, path, cost);
            if (path && mountId)
            {
                TC_LOG_DEBUG("playerbots", "[TAXI] %s continues taxi from node %u to %u (path %u)",
                    bot->GetName().c_str(), sourcenode, destinationnode, path);
                bot->GetSession()->SendDoFlight(mountId, path, 1);   // skip start fly node
            }
            else
                bot->m_taxi.ClearTaxiDestinations();
            return;
        }

        bot->CleanupAfterTaxiFlight();
        bot->SetFallInformation(0, bot->GetPositionZ());
    }

}

void PlayerbotAI::UpdateAI(uint32 elapsed, bool minimal)
{
    // Handle the AI check delay
    if (nextAICheckDelay > elapsed)
        nextAICheckDelay -= elapsed;
    else
        nextAICheckDelay = 0;

    // Early return if bot is in invalid state
    if (!bot || !bot->IsInWorld() || !bot->GetSession() || bot->GetSession()->isLogingOut() ||
        bot->IsDuringRemoveFromWorld())
        return;

    // Debug scenario (.botscenario) / probe (.botprobe): see Scenario/BotScenario.cpp. AnyActive()
    // is a single atomic load, so this costs nothing for the common case (no scenario, no probe).
    BotScenario::OnUpdate(this);

    // AC Bot/PlayerbotAI.cpp:378-381: bots send no movement opcodes, so the fall state the client would reset stays frozen and
    // blocks LFG teleports. AC's Player::IsFalling() is a Z test against m_lastFallZ (reset below as AC does); this core's is
    // Unit::IsFalling(), the movement-flag test, and MotionMaster::MoveFall sets MOVEMENTFLAG_FALLING on a player for the client's
    // landing opcode to clear. A bot never sends it, so after any server-side fall (CompleteDismount, the duel fall) the flag stayed
    // and LFGMgr::TeleportPlayer refused the bot with LFG_TELEPORTERROR_FALLING (#876: w84b "Catheri NOT teleported to map 36:
    // error 2 (map 0, falling 1)"). Same intent: once no fall spline runs, the bot has landed. (A finished fall spline's own flag is
    // handled in Unit::IsFalling, as AC does.)
    if (!bot->IsBeingTeleported() && bot->movespline->Finalized())
    {
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR);
        bot->SetFallInformation(0, bot->GetPositionZ());
    }

    // #521 w14 (combat rogue, no finisher): AC evaluates AllowActivity() only after its early return for
    // bot->IsBeingTeleported() (AC Bot/PlayerbotAI.cpp:355-357 then :382), so a value cached during a near teleport
    // never exists there. This fork's UpdateAI has no such return, and AllowActive() answers false for any bot that
    // IsBeingTeleported() (PlayerbotAI.cpp AllowActive, first check) - so a Killing Spree / Shadowstep / blink
    // teleport cached allowActive=false for ~5 s (AllowActivity cache) even while in combat, and DoNextAction ran
    // the engine minimal (only relevance >= 100 triggers) and then slept passiveDelay (10 s): t521 w14 Drianon, in combat
    // at 100 energy, had one "T:often" tick at t=20.1 s after Killing Spree and the next at 30.8 s, and never reached 5
    // combo points. Same gate as AC.
    if (!bot->IsBeingTeleported())
        AllowActivity();

    // #754 (BEYOND AC - AC Bot/PlayerbotAI.cpp:346-385 has the same gap; measured on both cores, IoC: 12 of 15 AC and
    // 9 of 15 MoP capture casts that damage interrupted left the bot asleep >= 2 s, 8 AC / 5 MoP bots died asleep).
    // A long self-imposed wait - WaitForSpellCast of a capture/use cast (cast time + react delay, which nothing shortens
    // when damage interrupts the cast: AC's SpellInterrupted only interrupts, this fork's SMSG_SPELL_FAILURE case just
    // returns), the up-to-12 s eat/drink sit (NonCombatActions.cpp:75, AC NonCombatActions.cpp:58-65) or a no-target
    // WaitForReach - ends in a battleground as soon as the bot is fought, unless a cast is still running.
    // #754 w20/w21 leftover: an attacker on the bot also counts while the combat flag lapses between hits (a bot that sat
    // down in such a gap stayed seated until the flag came back).
    if (nextAICheckDelay > sPlayerbotAIConfig->globalCoolDown && bot->InBattleground() && bot->IsAlive() &&
        !bot->IsNonMeleeSpellCasted(false, false, true) &&
        (!bot->getAttackers().empty() || (bot->IsInCombat() && !bot->GetCombatManager().GetPvPCombatRefs().empty())))
    {
        if (!bot->IsStandState())
            bot->SetStandState(UNIT_STAND_STATE_STAND);  // leave the food/drink sit

        nextAICheckDelay = 0;
    }

    if (!CanUpdateAI())
        return;

    // AC parity (#61): AC paces every tick with YieldThread(bot, GetReactDelay()) - 1-3 s idle,
    // 2-20 s resting, 0.5 s in combat, 0.1/0.25 s in BGs - plus a deterministic 0-200 ms per-bot
    // offset to stagger updates (AC PlayerbotAIBase::YieldThread). Done here rather than in
    // PlayerbotAIBase so the widely included PlayerbotAIBase.h does not change.
    auto const yieldReactDelay = [this]()
    {
        uint32 const delay = GetReactDelay();
        if (nextAICheckDelay < delay)
            nextAICheckDelay = delay + bot->GetGUID().GetCounter() % 201;
    };

    // Handle the current spell
    Spell* currentSpell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell)
        currentSpell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (currentSpell)
    {
        const SpellInfo* spellInfo = currentSpell->GetSpellInfo();
        if (spellInfo && currentSpell->getState() == SPELL_STATE_PREPARING)
        {
            Unit* spellTarget = currentSpell->m_targets.GetUnitTarget();
            // Interrupt if target is dead or spell can't target dead units
            if (spellTarget && !spellTarget->IsAlive() && !spellInfo->IsAllowingDeadTarget())
            {
                InterruptSpell();
                yieldReactDelay();
                return;
            }

            // AC parity (#141 item 2): interrupt a cast whose GameObject target has despawned
            // (AC Bot/PlayerbotAI.cpp:318-325).
            GameObject* goSpellTarget = currentSpell->m_targets.GetGOTarget();
            if (goSpellTarget && !goSpellTarget->isSpawned())
            {
                InterruptSpell();
                yieldReactDelay();
                return;
            }

            bool isHeal = false;
            bool isSingleTarget = true;

            for (uint8 i = 0; i < 3; ++i)
            {
                if (!spellInfo->Effects[i].Effect)
                    continue;

                // Check if spell is a heal
                if (spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL ||
                    spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL_MAX_HEALTH ||
                    spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL_MECHANICAL)
                    isHeal = true;

                // Check if spell is single-target
                if ((spellInfo->Effects[i].TargetA.GetTarget() &&
                    spellInfo->Effects[i].TargetA.GetTarget() != TARGET_UNIT_TARGET_ALLY) ||
                    (spellInfo->Effects[i].TargetB.GetTarget() &&
                        spellInfo->Effects[i].TargetB.GetTarget() != TARGET_UNIT_TARGET_ALLY))
                {
                    isSingleTarget = false;
                }
            }

            // Interrupt if target ally has full health (heal by other member)
            if (isHeal && isSingleTarget && spellTarget && spellTarget->IsFullHealth())
            {
                InterruptSpell();
                yieldReactDelay();
                return;
            }

            // Ensure bot is facing target if necessary
            if (spellTarget && !bot->HasInArc(CAST_ANGLE_IN_FRONT, spellTarget) &&
                (spellInfo->FacingCasterFlags & SPELL_FACING_FLAG_INFRONT))
            {
                sServerFacade->SetFacingTo(bot, spellTarget);
            }

            // Wait for spell cast
            yieldReactDelay();
            return;
        }
    }

    // AC parity (#141 item 1): once a second, board or leave the transport (boat, zeppelin) the
    // bot is currently standing on (AC Bot/PlayerbotAI.cpp:388-412). This is AC's only boarding
    // path for a random bot that is not following a master (issue #61 research).
    // #141 F4 (AC parity): AC returns from the whole UpdateAI while IsBeingTeleported() (AC :256),
    // which also protects a same-map transport teleport's NOT_LEAVE_TRANSPORT window from being
    // undone by a check that runs before the teleport ack. Gating just this block (rather than
    // widening the early return above) keeps the spell-in-progress block's own behaviour unchanged.
    if (!bot->IsBeingTeleported())
    {
        // #206/#209 F1 safety net: a bot that lost its transport/vehicle by any other path but still
        // carries IGNORE_PATHFINDING would otherwise path in straight lines forever.
        if (!bot->GetTransport() && !bot->GetVehicle() && bot->HasUnitState(UNIT_STATE_IGNORE_PATHFINDING))
            bot->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);

        // .botscenario transport tcheck= gate (research\transport-stage-design.md section 3.2):
        // makes the once-a-minutes-scale check timing deterministic for the harness on both cores.
        // No-op (false) for every bot without a running transport scenario.
        bool const forceTransportCheck = BotScenario::TransportCheckDue(bot);

        // #141 F6 / #650: the lock guards only the shared timer map. It used to be held across the whole
        // probe/board/leave block below, which with MapUpdate.Threads > 1 serialised every bot's once-a-second
        // transport check on all map threads behind one global mutex. The block itself only touches this bot,
        // its own map and its transport (whose passenger set has its own lock, #231).
        bool transportCheckDue = false;
        {
            std::lock_guard<std::mutex> transportCheckLock(g_nextTransportCheckMutex);
            uint32& nextTransportCheck = g_nextTransportCheck[bot->GetGUID().GetCounter()];

            if (forceTransportCheck)
                nextTransportCheck = 0;

            if (nextTransportCheck > elapsed)
                nextTransportCheck -= elapsed;
            else
                nextTransportCheck = 0;

            if (!nextTransportCheck)
            {
                nextTransportCheck = 1000;
                transportCheckDue = true;
            }
        }

        if (transportCheckDue)
        {
            Transport* newTransport = bot->GetMap()->GetTransportForPos(bot->GetPhaseMask(), bot->GetPositionX(),
                bot->GetPositionY(), bot->GetPositionZ(), bot);

            // #141 F3 (crash hazard): loop 2 of GetTransportForPos can return the nearest type-11
            // GameObject reinterpret_cast to Transport (GameObject::ToTransport, GameObject.h:345)
            // even when it is a plain GameObject, never added to the map as a real Transport (entry
            // 216666, GameObject.cpp:230). Every real MoP Transport - MO or local type-11 - is
            // guided Mo_Transport (Transport.cpp:73/211), so reject anything else before touching it.
            if (newTransport && !newTransport->GetGUID().IsMOTransport())
                newTransport = nullptr;

            // #206/#209 F3: attach to an elevator/tram only when its floor is right under the bot's feet.
            if (newTransport && newTransport->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT &&
                !IsUnderStaticTransportDeck(newTransport, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()))
                newTransport = nullptr;

            // #223: the core probe above is a single exact-Z ray, so on a boat/zeppelin deck it can
            // miss for one 1 s sample and the rider is removed for good - by the next check the deck
            // has moved away. Re-probe at the tolerant offsets Follow uses before dropping an MO
            // transport passenger; a bot that really is off the deck still probes null and is
            // removed as before, and a bot that is already on the deck stays attached.
            // P5: this gate used to be MO-transport only, so a type-11 rider (elevator/tram/any type-11
            // boat) got no tolerance at all and was dropped on a single exact-Z miss - and the deck
            // had moved by the next check. Widening it to every transport the bot is really riding is
            // safe: GetTransportForPosTolerant applies the same IsUnderStaticTransportDeck test to a
            // type-11 hit that the exact probe above already applies, so this adds tolerance, not a
            // new way to attach, and a genuinely off-deck bot still probes null and is removed.
            if (!newTransport)
                if (Transport* cur = bot->GetTransport())
                    if (cur->GetGUID().IsMOTransport() || cur->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT)
                        newTransport = GetTransportForPosTolerant(bot->GetMap(), bot, bot->GetPhaseMask(),
                            bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

            // #889 (beyond AC; w88 Z4: "[TRN][AMB] act=remove bot=Cellietta ... type=11 botpos=1752.41/-4391.16/84.70
            // stored=-0.70/-6.13/1.94 ... exact=none tol=none", then "[ROAMTRIP] Cellietta abandon ride-lost-transport" in
            // the middle of the 206610 shaft): nobody steps off a static platform between two landings, so a rider it is
            // still carrying (StaticRiderCarried) keeps it while it moves (RoamTransport::StaticPlatformMoving). At a
            // landing (paused platform) the probes above decide as before, so a rider that walked off is still removed
            // there (#586 / #206 below).
            if (!newTransport)
                if (Transport* cur = bot->GetTransport())
                    if (cur->GetGUID().IsMOTransport() && cur->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT &&
                        RoamTransport::StaticPlatformMoving(cur) && StaticRiderCarried(bot, cur))
                    {
                        newTransport = cur;
                        TC_LOG_DEBUG("playerbots",
                            "[TRN][AMB] act=keep-moving bot=%s tr=%s type=%u botpos=%.2f/%.2f/%.2f stored=%.2f/%.2f/%.2f",
                            bot->GetName().c_str(), TransportText(cur), (uint32)cur->GetGoType(), bot->GetPositionX(),
                            bot->GetPositionY(), bot->GetPositionZ(), bot->m_movementInfo.transport.pos.GetPositionX(),
                            bot->m_movementInfo.transport.pos.GetPositionY(), bot->m_movementInfo.transport.pos.GetPositionZ());
                    }

            // #206 (#586 follow-up): a probe hit under a bot that stands beside its off-platform master is the
            // core's 30 yd hull ray, not a deck (BotBesideMasterOffPlatform above). Drop it, so such a bot is
            // never attached and a rider in that state falls through to the removal below
            // (RiderWalkedOffStoppedPlatform). Bots without a master (random bots) are unaffected (AC parity).
            if (newTransport)
            {
                Player* const besideMaster = GetMaster();
                if (BotBesideMasterOffPlatform(bot, besideMaster, newTransport))
                {
                    if (newTransport != bot->GetTransport())
                        TC_LOG_DEBUG("playerbots",
                            "[TRN][AMB] act=veto bot=%s master=%s tr=%s type=%u botpos=%.2f/%.2f/%.2f",
                            bot->GetName().c_str(), besideMaster->GetName().c_str(), TransportText(newTransport),
                            (uint32)newTransport->GetGoType(), bot->GetPositionX(), bot->GetPositionY(),
                            bot->GetPositionZ());
                    newTransport = nullptr;
                }
            }

            // #675: a bot in a jump arc (EFFECT_MOTION_TYPE: MoveJump - the IoC gunship drop, a catapult launch,
            // a knockback) is in the air, not on a deck: never attach it to a transport mid-flight.
            if (newTransport && !bot->GetTransport() &&
                bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE)
                newTransport = nullptr;

            // #879 (MoP extension of #586, beyond AC): attach to (or switch to) a boat/zeppelin only when its deck is
            // right under the bot's feet - the 4 yd deck ray static transports already need above (#206/#209 F3) and
            // RoamTransport's DeckUnder uses. The core's 30 yd hull ray (Map.cpp:2578-2587) also hits the hull under a
            // bot swimming at the docked ship's waterline (w69 Stallian, z 0.6) or standing on a pier above it. A rider
            // of this same transport (newTransport == GetTransport()) keeps the tolerant #223 rule.
            if (newTransport && newTransport != bot->GetTransport() &&
                newTransport->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT &&
                !IsUnderStaticTransportDeck(newTransport, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()))
                newTransport = nullptr;

            if (newTransport != bot->GetTransport())
            {
                Player* const master = GetMaster();
                Transport* const oldTransport = bot->GetTransport();

                // P4: both probes missed, but the master the bot follows is still a passenger of that
                // same transport. Removal is only correct when nobody the bot follows is on the deck:
                // here the rider is simply off-deck (drifted or spline-poisoned offset) and must be
                // re-seated, not dropped into the water. Re-anchor it onto the master's deck-valid
                // offset and keep the attachment. This is the only one of these patches that rescues
                // a standing bot, which has no live spline left for P2 to clean up.
                // #316: SnapToDeckOffsetFromMaster needs the master to be a passenger of that same
                // transport. When it is not - the master already stepped off, or never boarded - it
                // returns false, and the rider was then removed while bot->GetTransport() was still
                // non-null, i.e. while it was still seated. Try the master's deck-valid offset first
                // (preferred: it lands the bot at the master's deck spot), then fall back to the bot's
                // own stored offset, which needs no master. Removal now only happens when neither
                // offset can produce a snap.
                bool reanchored = oldTransport && !newTransport &&
                    SnapToDeckOffsetFromMaster(bot, master, oldTransport);
                // #586: ...but never re-seat a rider that walked off a stopped platform after its master
                // (RiderWalkedOffStoppedPlatform above): fall through to the removal below, as AC does.
                if (!reanchored && oldTransport && !newTransport &&
                    !RiderWalkedOffStoppedPlatform(bot, master, oldTransport))
                    reanchored = SnapToOwnDeckOffset(bot, oldTransport);
                if (reanchored)
                    newTransport = oldTransport;

                TC_LOG_DEBUG("playerbots", "Bot %s is on a transport", bot->GetName().c_str());

                if (reanchored)
                {
                    // Evidence line for the re-anchor half of this block (the removal half is
                    // covered by the "is on a transport" line above plus act=remove below).
                    TC_LOG_DEBUG("playerbots",
                        "[TRN][AMB] act=reanchor bot=%s master=%s tr=%s type=%u botpos=%.2f/%.2f/%.2f stored=%.2f/%.2f/%.2f/%.3f onTr=%d exact=%s tol=%s",
                        bot->GetName().c_str(), master ? master->GetName().c_str() : "none",
                        TransportText(oldTransport), (uint32)oldTransport->GetGoType(),
                        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                        bot->m_movementInfo.transport.pos.GetPositionX(),
                        bot->m_movementInfo.transport.pos.GetPositionY(),
                        bot->m_movementInfo.transport.pos.GetPositionZ(),
                        bot->m_movementInfo.transport.pos.GetOrientation(),
                        bot->GetTransport() ? 1 : 0, TransportText(ProbeTransportExact(bot)),
                        TransportText(GetTransportForPosTolerant(bot->GetMap(), bot, bot->GetPhaseMask(),
                            bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())));
                }
                else
                {
                    if (oldTransport)
                    {
                        TC_LOG_DEBUG("playerbots",
                            "[TRN][AMB] act=remove bot=%s master=%s tr=%s type=%u botpos=%.2f/%.2f/%.2f stored=%.2f/%.2f/%.2f/%.3f onTr=%d exact=%s tol=%s",
                            bot->GetName().c_str(), master ? master->GetName().c_str() : "none",
                            TransportText(oldTransport), (uint32)oldTransport->GetGoType(),
                            bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                            bot->m_movementInfo.transport.pos.GetPositionX(),
                            bot->m_movementInfo.transport.pos.GetPositionY(),
                            bot->m_movementInfo.transport.pos.GetPositionZ(),
                            bot->m_movementInfo.transport.pos.GetOrientation(),
                            bot->GetTransport() ? 1 : 0, TransportText(ProbeTransportExact(bot)),
                            TransportText(GetTransportForPosTolerant(bot->GetMap(), bot, bot->GetPhaseMask(),
                                bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())));
                        // #141 F1 (must): AC's MotionTransport::RemovePassenger(withAll=true) clears
                        // IGNORE_PATHFINDING (AC Transport.cpp:396-399); MoP's RemovePassenger only does
                        // that for type-11 transports (Transport.cpp:623-625), so a bot that walks off a
                        // boat/zeppelin keeps the flag and moves in straight lines (no navmesh) forever.
                        bool const wasMo = oldTransport->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT;
                        oldTransport->RemovePassenger(bot, true);
                        if (wasMo)
                            bot->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                    }
                }

                if (newTransport)
                {
                    newTransport->AddPassenger(bot, true);

                    // #206/#209 F1 (owner requirement, beyond AC): keep IGNORE_PATHFINDING (set by the core's
                    // AddPassenger) while riding an elevator/tram too; navmesh pathing from inside a shaft
                    // snapped the bot onto an unreachable navmesh island (s9-elevator-boat-rootcause.md).
                    // The core's RemovePassenger clears it for type-11 transports (Transport.cpp:623-625).
                }

                // #141 F5: see StopOnCurrentPos above.
                StopOnCurrentPos(bot);
            }
        }
    }

    // #225: continue a multi-leg taxi at each stopover (see ContinueTaxiIfLanded above). Runs
    // before UpdateAIInternal -> DoNextAction so the RPG/follow action cannot walk the bot back.
    ContinueTaxiIfLanded(bot);

    // #639 (AC Bot/PlayerbotAI.cpp:540-544): update the bot's group status (stale-master release,
    // new-master search) before UpdateAIInternal turns the queued packets into trigger events and
    // runs the engine. AC returns from the whole UpdateAI while the bot is being teleported
    // (AC :355-357); this fork's UpdateAI does not, so the same gate is applied here.
    if (!bot->IsBeingTeleported())
        UpdateAIGroupMaster();

    // Update internal AI
    UpdateAIInternal(elapsed, minimal);
    yieldReactDelay();
}

// Defined in strategy/actions/MovementActions.cpp (#590).
void RefreshBotLiquidMovementFlags(Player* bot);

void PlayerbotAI::UpdateAIInternal([[maybe_unused]] uint32 elapsed, bool minimal)
{
    if (bot->IsBeingTeleported() || !bot->IsInWorld())
        return;

    // Diagnostic (issue #61): dump the roaming/AI state of AiPlayerbot.DebugBotNames every 10 s.
    // This runs on every map thread, so the last-log table is thread_local (no shared state).
    if (!sPlayerbotAIConfig->debugBotNames.empty() && sPlayerbotAIConfig->debugBotNames.count(bot->GetName()))
    {
        static thread_local std::map<ObjectGuid, uint32> s_debugLastLog;
        uint32 const now = getMSTime();
        uint32& last = s_debugLastLog[bot->GetGUID()];
        if (!last || getMSTimeDiff(last, now) >= 10000)
        {
            last = now;
            NewRpgInfo& info = GetRpgInfo();
            Unit* target = bot->GetVictim();
            Unit* grindTarget = _aiObjectContext->GetValue<Unit*>("grind target")->Get();
            Unit* curTarget = _aiObjectContext->GetValue<Unit*>("current target")->Get();
            TC_LOG_DEBUG("playerbots",
                "[RPGSTAT] %s status=%s engine=%s inCombat=%d moving=%d map=%u pos=(%.1f, %.1f, %.1f) "
                "victim=%s curTarget=%s grindTarget=%s gValid=%d gDist=%.0f hostile=%d friendly=%d valid=%d mounted=%d "
                "quests=%u/%u/%u/%u statusAge=%us stuckAttempts=%u",
                bot->GetName().c_str(), info.ToString().c_str(),
                GetState() == BOT_STATE_COMBAT ? "combat" : (GetState() == BOT_STATE_DEAD ? "dead" : "noncombat"),
                bot->IsInCombat() ? 1 : 0, bot->isMoving() ? 1 : 0,
                bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                target ? target->GetName().c_str() : "(none)",
                curTarget ? curTarget->GetName().c_str() : "(none)",
                grindTarget ? grindTarget->GetName().c_str() : "(none)",
                grindTarget && bot->IsValidAttackTarget(grindTarget) ? 1 : 0,
                grindTarget ? bot->GetExactDist(grindTarget) : -1.0f,
                target && bot->IsHostileTo(target) ? 1 : 0, target && bot->IsFriendlyTo(target) ? 1 : 0,
                target && bot->IsValidAttackTarget(target) ? 1 : 0, bot->IsMounted() ? 1 : 0,
                GetRpgStatistic().questAccepted, GetRpgStatistic().questCompleted, GetRpgStatistic().questRewarded,
                GetRpgStatistic().questDropped,
                uint32(GetMSTimeDiffToNow(info.startT) / 1000), info.stuckAttempts);
        }
    }

    // #590: keep the swim / water-walk flags in step with the real liquid state (rate-limited).
    RefreshBotLiquidMovementFlags(bot);

    std::string const mapString = WorldPosition(bot).isOverworld() ? std::to_string(bot->GetMapId()) : "I";
    PerformanceMonitorOperation* pmo =
            sPerformanceMonitor->start(PERF_MON_TOTAL, "PlayerbotAI::UpdateAIInternal " + mapString);

    ExternalEventHelper helper(_aiObjectContext);

    HandleCommands();

    // logout if logout timer is ready or if instant logout is possible
    if (bot->GetSession()->isLogingOut())
    {
        WorldSession* botWorldSessionPtr = bot->GetSession();
        bool logout = botWorldSessionPtr->ShouldLogOut(time(nullptr));
        if (!master || !master->GetSession()->GetPlayer())
            logout = true;

        if (bot->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || bot->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
            botWorldSessionPtr->GetSecurity() >= AccountTypes::SEC_PLAYER)
        {
            logout = true;
        }

        if (master &&
            (master->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || master->HasUnitState(UNIT_STATE_IN_FLIGHT) ||
                (master->GetSession() &&
                    master->GetSession()->GetSecurity() >= AccountTypes::SEC_PLAYER)))
        {
            logout = true;
        }

        if (logout)
        {
        }

        SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        return;
    }

    botOutgoingPacketHandlers.Handle(helper);
    masterIncomingPacketHandlers.Handle(helper);
    masterOutgoingPacketHandlers.Handle(helper);

    DoNextAction(minimal);

    if (pmo)
        pmo->finish();
}

bool PlayerbotAI::DoSpecificAction(std::string const name, Event event, bool silent, std::string const qualifier)
{
    std::ostringstream out;

    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
    {
        ActionResult res = _engines[i]->ExecuteAction(name, event, qualifier);
        switch (res)
        {
        case ACTION_RESULT_UNKNOWN:
            continue;
        case ACTION_RESULT_OK:
            if (!silent)
            {
                //PlaySound(TEXT_EMOTE_NOD);
            }
            return true;
        case ACTION_RESULT_IMPOSSIBLE:
            out << name << ": impossible";
            if (!silent)
            {
                //TellError(out.str());
                //PlaySound(TEXT_EMOTE_NO);
            }
            return false;
        case ACTION_RESULT_USELESS:
            out << name << ": useless";
            if (!silent)
            {
                //TellError(out.str());
                //PlaySound(TEXT_EMOTE_NO);
            }
            return false;
        case ACTION_RESULT_FAILED:
            if (!silent)
            {
                //out << name << ": failed";
                //TellError(out.str());
            }
            return false;
        }
    }

    if (!silent)
    {
        //out << name << ": unknown action";
        //TellError(out.str());
    }

    return false;
}

// #72: AC's activity world-diff source. AC AutoScaleActivity reads sWorldUpdateTime.GetMaxUpdateTimeOfCurrentTable()
// (AC core Time/UpdateTime.cpp:67-70 and 102-125: the max of the current and the previous 500-diff table). This core
// has no UpdateTime, so the module keeps the same two maxima, fed with the World::Update diff from the world thread
// (mod_playerbots.cpp OnPlayerbotUpdate) and read by the map threads.
namespace
{
constexpr uint32 PLAYERBOT_AVG_DIFF_COUNT = 500;  // AC UpdateTime.h:26 AVG_DIFF_COUNT
std::atomic<uint32> s_maxUpdateTimeOfCurrentTable{0};
std::atomic<uint32> s_maxUpdateTimeOfLastTable{0};
uint32 s_updateTimeTableIndex = 0;  // world thread only

uint32 GetMaxUpdateTimeOfCurrentTable()
{
    return std::max(s_maxUpdateTimeOfCurrentTable.load(std::memory_order_relaxed),
                    s_maxUpdateTimeOfLastTable.load(std::memory_order_relaxed));
}

// AC Bot/PlayerbotAI.cpp:5010-5034 (a member there; file-local here so PlayerbotAI.h does not change)
uint32 AutoScaleActivity(uint32 mod)
{
    // Current max server update time (ms), and the configured floor/ceiling values for bot scaling
    uint32 maxDiff = GetMaxUpdateTimeOfCurrentTable();
    uint32 diffLimitFloor = sPlayerbotAIConfig->botActiveAloneSmartScaleDiffLimitfloor;
    uint32 diffLimitCeiling = sPlayerbotAIConfig->botActiveAloneSmartScaleDiffLimitCeiling;

    if (diffLimitCeiling <= diffLimitFloor)
    {
        // Perfrom binary decision if ceiling <= floor: Either all bots are active or none are
        return (maxDiff > diffLimitCeiling) ? 0 : mod;
    }

    if (maxDiff > diffLimitCeiling)
        return 0;

    if (maxDiff <= diffLimitFloor)
        return mod;

    // Calculate lag progress from floor to ceiling (0 to 1)
    double lagProgress = (maxDiff - diffLimitFloor) / (double)(diffLimitCeiling - diffLimitFloor);

    // Apply the percentage of active bots (the complement of lag progress) to the mod value
    return static_cast<uint32>(mod * (1 - lagProgress));
}
}  // namespace

// AC core UpdateTime::UpdateWithDiff (UpdateTime.cpp:102-119), max tracking only. World thread.
void PlayerbotRecordWorldUpdateDiff(uint32 diff)
{
    if (diff > s_maxUpdateTimeOfCurrentTable.load(std::memory_order_relaxed))
        s_maxUpdateTimeOfCurrentTable.store(diff, std::memory_order_relaxed);

    if (++s_updateTimeTableIndex >= PLAYERBOT_AVG_DIFF_COUNT)
    {
        s_updateTimeTableIndex = 0;
        s_maxUpdateTimeOfLastTable.store(s_maxUpdateTimeOfCurrentTable.load(std::memory_order_relaxed),
                                         std::memory_order_relaxed);
        s_maxUpdateTimeOfCurrentTable.store(0, std::memory_order_relaxed);
    }
}

// #72: AC Bot/PlayerbotAI.cpp:4809-4988, 1:1. This fork had AllowActive() stubbed to `return true` (LoA upstream), so
// BotActiveAlone / smart scale / the ForceWhen* rules did nothing and every bot was always fully active (AC's prod
// default is 10%). Fork substitutions: IsSelfBot(p) -> the AI of p IsRealPlayer() (master == self, the fork's selfbot
// test, see HasGameClientMaster below); sLFGMgr->GetState(guid) -> GetActiveState(guid) (5.4.8 queues per id).
bool PlayerbotAI::AllowActive(ActivityType activityType)
{
    // bot is in an invalid state, not safe to process
    if (!bot || !bot->GetSession() || !bot->IsInWorld() || bot->IsBeingTeleported() ||
        bot->GetSession()->isLogingOut() || bot->IsDuringRemoveFromWorld())
        return false;

    // always allow packet handling (e.g. group invites, trade, loot, friend requests etc)
    if (activityType == PACKET_ACTIVITY)
        return true;

    // harness (.botscenario, test-only): a bot the harness is driving acts like a played character
    if (BotScenario::Driving(bot))
        return true;

    // #218 (BEYOND AC): a bot on its own roam trip's transport leg stays active - minimal ticks slept through the 60 s zeppelin stop.
    if (RoamTransport::OnTransportLeg(bot))
        return true;

    // all bots forced active, no rotation or scaling needed
    if (sPlayerbotAIConfig->botActiveAlone >= 100 && !sPlayerbotAIConfig->botActiveAloneSmartScale)
        return true;

    // bot is in combat, always defend yourself
    if (activityType != OUT_OF_PARTY_ACTIVITY && activityType != PACKET_ACTIVITY)
    {
        if (bot->IsInCombat())
            return true;

        // #868 (BEYOND AC - AC PlayerbotAI.cpp AllowActive has no duel rule): a bot in a duel (counting down or
        // fighting) fights like a bot in combat. A duel sets no combat flag until the first hit, so with no real player
        // in range both bots ran minimal ticks (relevance >= 100 only) and slept PassiveDelay (10 s) in between: w73
        // t613 Philias/Alaine stood 60 s at 4 yd, the opponent targeted, with no swing; AC Doririala/Nelanderon 329 s.
        // The accepting bot lost ~7 s of every duel the same way. MoP's duel spots keep duelling bots fighting
        // (the "in duel" activity flag is on while Player::duel is set).
        if (bot->duel && bot->duel->opponent)
            return true;
    }

    // bot is inside a BG, dungeon, or raid - always active
    // #720: the MoP continents (870 Pandaria etc.) are now part of WorldPosition::isOverworld() itself (one list for
    // every caller), so this is AC's line again (AC PlayerbotAI.cpp:4832).
    if (!WorldPosition(bot).isOverworld())
        return true;

    // #900 (beyond AC: AC bots take no part in outdoor battlefields): a bot in a Wintergrasp / Tol Barad war raid is
    // in a PvP raid on an overworld map - keep it active like a BG bot so it fights instead of idling on minimal ticks.
    if (Group* bfGroup = bot->GetGroup())
        if (bfGroup->isBFGroup())
            return true;

    // bot is waiting in a BG queue - stay active to speed up join
    if (bot->InBattlegroundQueue())
        return true;

    // bot is in a guild that contains a real player
    if (sPlayerbotAIConfig->botActiveAloneForceWhenInGuild)
    {
        if (IsInRealGuild())  // checks cache list
            return true;
    }

    // a real player is in the same zone (e.g. Elwynn Forest), same continent or within configured yard radius
    // combined into a single loop to multiple iterations since this function is called so often
    bool checkMap = sPlayerbotAIConfig->botActiveAloneForceWhenInMap;
    bool checkZone = sPlayerbotAIConfig->botActiveAloneForceWhenInZone;
    bool checkRadius = sPlayerbotAIConfig->botActiveAloneForceWhenInRadius > 0;
    if (checkMap || checkZone || checkRadius)
    {
        uint32 botMapId = bot->GetMapId();
        uint32 botZoneId = checkZone ? bot->GetZoneId() : 0;
        float sqRange = 0.0f;
        WorldPosition botPos(bot);
        if (checkRadius)
        {
            float range = static_cast<float>(sPlayerbotAIConfig->botActiveAloneForceWhenInRadius);
            sqRange = range * range;
        }

        for (auto& player : sRandomPlayerbotMgr->GetPlayers())
        {
            if (!player || player->GetMapId() != botMapId)
                continue;

            bool isGM = player->IsGameMaster();

            // map check
            if (checkMap && !(isGM && !player->IsVisible()))
                return true;

            // zone check
            if (checkZone && !(isGM && !player->IsVisible()) && player->GetZoneId() == botZoneId)
                return true;

            // radius check
            if (checkRadius && (!isGM || player->isGMVisible()))
            {
                if (botPos.sqDistance(WorldPosition(player)) < sqRange)
                    return true;

                WorldObject* viewObj = player->GetViewpoint();
                if (viewObj && viewObj != player && botPos.sqDistance(WorldPosition(viewObj)) < sqRange)
                    return true;
            }
        }
    }

    // bot has a real player master (not another bot)
    if (GetMaster())
    {
        PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(GetMaster());
        if (!masterBotAI || masterBotAI->IsRealPlayer())
            return true;
    }

    // bot is grouped with a real player (or a bot owned by one)
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (!member || !member->IsInWorld() || member->GetMapId() != bot->GetMapId())
                continue;

            if (member == bot)
                continue;

            PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);

            // group member is a real player or owned by one - stay active
            if (!memberBotAI || memberBotAI->HasGameClientMaster())
                return true;

            // if group leader (bot) is inactive, follow suit
            if (group->IsLeader(member->GetGUID()))
            {
                if (!memberBotAI->AllowActivity(PARTY_ACTIVITY))
                    return false;
            }
        }
    }

    // bot is in LFG queue - stay active
    bool isLFG = false;
    if (group)
    {
        if (sLFGMgr->GetActiveState(group->GetGUID()) != lfg::LFG_STATE_NONE)
            isLFG = true;
    }
    if (sLFGMgr->GetActiveState(bot->GetGUID()) != lfg::LFG_STATE_NONE)
        isLFG = true;

    if (isLFG)
        return true;

    // a real player has this bot on their friends list
    if (sPlayerbotAIConfig->botActiveAloneForceWhenIsFriend)
    {
        // shouldnt be needed analyse in future
        if (!bot->GetGUID())
            return false;

        for (auto& player : sRandomPlayerbotMgr->GetPlayers())
        {
            if (!player || !player->GetSession() || !player->IsInWorld() || player->IsDuringRemoveFromWorld() ||
                player->GetSession()->isLogingOut())
                continue;

            PlayerbotAI* playerAI = GET_PLAYERBOT_AI(player);
            if (!playerAI || !playerAI->IsRealPlayer())
                continue;

            PlayerSocial* social = player->GetSocial();
            if (social && social->HasFriend(bot->GetGUID()))
                return true;
        }
    }

    // pathfinding only runs for bots forced active by the rules above -
    // skip it for bots that would only be active via random rotation
    if (activityType == DETAILED_MOVE_ACTIVITY)
        return false;

    // #######################################################################################
    // Acitivity throttling logic
    // #######################################################################################
    if (sPlayerbotAIConfig->botActiveAlone <= 0)
        return false;

    // base threshold capped at 100
    uint32 mod = sPlayerbotAIConfig->botActiveAlone > 100 ? 100 : sPlayerbotAIConfig->botActiveAlone;

    // reduce threshold based on server tick time when SmartScale is enabled
    if (sPlayerbotAIConfig->botActiveAloneSmartScale &&
        bot->GetLevel() >= sPlayerbotAIConfig->botActiveAloneSmartScaleWhenMinLevel &&
        bot->GetLevel() <= sPlayerbotAIConfig->botActiveAloneSmartScaleWhenMaxLevel)
    {
        mod = AutoScaleActivity(mod);
    }

    // deterministic rotation - bot is active if its hash falls below the threshold
    uint32 ActivityNumber = GetFixedBotNumber(100);
    return ActivityNumber < mod;
}

// AC Bot/PlayerbotAI.cpp:4990-5008 (ms timer with a per-bot 0-499 ms stagger instead of a whole-second time(nullptr))
bool PlayerbotAI::AllowActivity(ActivityType activityType, bool checkNow)
{
    const int activityIndex = static_cast<int>(activityType);

    if (!_allowActiveCheckTimer[activityIndex])
        _allowActiveCheckTimer[activityIndex] = getMSTime();

    // 4500ms base + 0-499ms per-bot offset = 4500-4999ms, capping at just under 5 seconds
    uint32 offset = bot->GetGUID().GetCounter() % 500;

    if (!checkNow && getMSTime() < (_allowActiveCheckTimer[activityIndex] + 4500 + offset))
        return _allowActive[activityIndex];

    const bool allowed = AllowActive(activityType);
    _allowActive[activityIndex] = allowed;
    _allowActiveCheckTimer[activityIndex] = getMSTime();

    return allowed;
}

void PlayerbotAI::Reset(bool full)
{
    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return;

    WorldSession* botWorldSessionPtr = bot->GetSession();
    bool logout = botWorldSessionPtr->ShouldLogOut(time(nullptr));

    // cancel logout
    if (!logout && bot->GetSession()->isLogingOut())
    {
        //WorldPackets::Character::LogoutCancel data = WorldPacket(CMSG_LOGOUT_CANCEL);
        //bot->GetSession()->HandleLogoutCancelOpcode(data);
        //TellMaster("Logout cancelled!");
    }

    _currentEngine = _engines[BOT_STATE_NON_COMBAT];
    _currentState = BOT_STATE_NON_COMBAT;
    nextAICheckDelay = 0;
    whispers.clear();

    _aiObjectContext->GetValue<Unit*>("old target")->Set(nullptr);
    _aiObjectContext->GetValue<Unit*>("current target")->Set(nullptr);
    _aiObjectContext->GetValue<GuidVector>("prioritized targets")->Reset();
    _aiObjectContext->GetValue<ObjectGuid>("pull target")->Set(ObjectGuid::Empty);
    //_aiObjectContext->GetValue<GuidPosition>("rpg target")->Set(GuidPosition());
    // #151 (AC Bot/PlayerbotAI.cpp:892): re-enabled now that "loot target" is a real value.
    _aiObjectContext->GetValue<LootObject>("loot target")->Set(LootObject());
    // #210 S10 (AC Bot/PlayerbotAI.cpp Reset()): clear the dormant "lfg proposal" value now that
    // LfgAcceptAction (strategy/actions/LfgActions.cpp) actually reads/writes it.
    _aiObjectContext->GetValue<uint32>("lfg proposal")->Set(0);
    bot->SetTarget(ObjectGuid::Empty);

    LastSpellCast& lastSpell = _aiObjectContext->GetValue<LastSpellCast&>("last spell cast")->Get();
    lastSpell.Reset();

    if (full)
    {
        _aiObjectContext->GetValue<LastMovement&>("last movement")->Get().Set(nullptr);
        //_aiObjectContext->GetValue<LastMovement&>("last area trigger")->Get().Set(nullptr);
        //_aiObjectContext->GetValue<LastMovement&>("last taxi")->Get().Set(nullptr);
        //_aiObjectContext->GetValue<TravelTarget*>("travel target")->Get()->setTarget(sTravelMgr->nullTravelDestination, sTravelMgr->nullWorldPosition, true);
        //_aiObjectContext->GetValue<TravelTarget*>("travel target")->Get()->setStatus(TRAVEL_STATUS_EXPIRED);
        //_aiObjectContext->GetValue<TravelTarget*>("travel target")->Get()->setExpireIn(1000);
        _rpgInfo = NewRpgInfo();
        // A running .botscenario survives the full reset a far teleport does: BotScenario::OnReset
        // re-arms it (re-applies the RPG status it needs, restarts its clock) from its own side
        // table, which was untouched by the _rpgInfo recreation above.
        BotScenario::OnReset(this);
        // #218 (BEYOND AC, MoP only): the same for a roam trip whose boat / zeppelin / portal leg changed map - the
        // status that drives the trip is re-applied, so the bot steps off at the far dock and walks on.
        WorldPosition tripDest;
        bool tripGrind = false;
        if (_rpgInfo.GetStatus() == RPG_IDLE && RoamTransport::ResumeAfterReset(bot, tripDest, tripGrind))
        {
            if (tripGrind)
                _rpgInfo.ChangeToGoGrind(tripDest);
            else
                _rpgInfo.ChangeToGoCamp(tripDest);
        }
    }

    //_aiObjectContext->GetValue<GuidSet&>("ignore rpg target")->Get().clear();

    bot->GetMotionMaster()->Clear();

    InterruptSpell();

    if (full)
    {
        for (uint8 i = 0; i < BOT_STATE_MAX; i++)
        {
            _engines[i]->Init();
        }
    }
}

void PlayerbotAI::ResetStrategies()
{
    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
        _engines[i]->removeAllStrategies();

    AiFactory::AddDefaultCombatStrategies(bot, this, _engines[BOT_STATE_COMBAT]);
    AiFactory::AddDefaultNonCombatStrategies(bot, this, _engines[BOT_STATE_NON_COMBAT]);
    AiFactory::AddDefaultDeadStrategies(bot, this, _engines[BOT_STATE_DEAD]);
    // #545 (AC Bot/PlayerbotAI.cpp:2055-2056)
    if (sPlayerbotAIConfig->applyInstanceStrategies)
        ApplyInstanceStrategies(bot->GetMapId());

    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
        _engines[i]->Init();
}

void PlayerbotAI::ReInitCurrentEngine()
{
    // InterruptSpell();
    _currentEngine->Init();
}

void PlayerbotAI::ChangeEngine(BotState type)
{
    Engine* engine = _engines[type];

    if (_currentEngine != engine)
    {
        _currentEngine = engine;
        _currentState = type;
        ReInitCurrentEngine();

        switch (type)
        {
        case BOT_STATE_COMBAT:
            //TC_LOG_DEBUG("playerbots",  "=== %s COMBAT ===", bot->GetName().c_str());
            // AC parity (#141 item 5, AC ChangeEngineOnCombat, PlayerbotAI.cpp:1474-1484): start
            // the "wait for attack" clock the moment the bot switches into the combat engine.
            // No-op for every bot without that opt-in strategy active.
            if (HasStrategy("wait for attack", BOT_STATE_COMBAT))
                _aiObjectContext->GetValue<time_t>("combat start time")->Set(time(nullptr));
            break;
        case BOT_STATE_NON_COMBAT:
            //TC_LOG_DEBUG("playerbots",  "=== %s NON-COMBAT ===", bot->GetName().c_str());
            // AC parity (ChangeEngineOnNonCombat, PlayerbotAI.cpp:1486-1496): clear the clock on
            // leaving combat.
            if (HasStrategy("wait for attack", BOT_STATE_COMBAT))
                _aiObjectContext->GetValue<time_t>("combat start time")->Set(0);
            break;
        case BOT_STATE_DEAD:
            //TC_LOG_DEBUG("playerbots",  "=== %s DEAD ===", bot->GetName().c_str());
            break;
        default:
            break;
        }
    }
}
void PlayerbotAI::ChangeStrategy(std::string const names, BotState type)
{
    Engine* e = _engines[type];
    if (!e)
        return;

    e->ChangeStrategy(names);
}

void PlayerbotAI::ClearStrategies(BotState type)
{
    Engine* e = _engines[type];
    if (!e)
        return;

    e->removeAllStrategies();
}

std::vector<std::string> PlayerbotAI::GetStrategies(BotState type)
{
    Engine* e = _engines[type];
    if (!e)
        return std::vector<std::string>();

    return e->GetStrategies();
}

// #639 (AC parity, AC Bot/PlayerbotAI.cpp:541, 549-608): AC keeps the whole group-master upkeep in
// UpdateAIGroupMaster(), which UpdateAI() calls once per tick BEFORE UpdateAIInternal() - i.e. before
// the queued packets are turned into trigger events and before Engine::DoNextAction(). This fork ran
// the same code at the END of DoNextAction(), after the engine tick (see #52/#53, #141 item 4). That
// ordering lost group invites: on the tick where a bot whose group had just disbanded got
// SMSG_GROUP_INVITE, the engine pushed "accept invitation" (100) but executed another relevance-100
// action first (one action per tick), and the stale-master release that followed called
// Reset(true) + ResetStrategies() -> Engine::Init() -> Engine::Reset(), which deletes every queued
// action. The one-shot "group invite" trigger had already been consumed, so the invite was never
// accepted. Running the release first, as AC does, leaves the queued "accept invitation" alive.
void PlayerbotAI::UpdateAIGroupMaster()
{
    if (!bot)
        return;

    Group* group = bot->GetGroup();

    // AC :561-570. If the bot is not in a group, a random bot drops its master and resets, as a
    // freshly spawned bot would ("exclude alt/addclass bots, they rely on their current (real
    // player) master, security-wise" - this fork has no alt/addclass bot concept, but random bots
    // are exactly the bots this applies to).
    if (!group)
    {
        if (master && sRandomPlayerbotMgr->IsRandomBot(bot))
        {
            // Ungrouped random bot still holding a master (its group disbanded, or a stray
            // reference survived a teleport): drop it and reset, as a freshly spawned bot would.
            TC_LOG_DEBUG("playerbots", "[MASTER] %s: releasing stale master %s (ungrouped random bot)",
                         bot->GetName().c_str(), master->GetName().c_str());
            SetMaster(nullptr);
            Reset(true);
            ResetStrategies();
        }
        return;
    }

    // AC :572-575. #52/#53: bot->GetGroup() returns the BG raid, so a real player in the match is a
    // group member of every bot on their own team with no invite at all. If that accepted master
    // has since left the raid (left the BG, logged out, ported out), release it here so the search
    // below picks a new one instead of keeping a stale one.
    // #900: a battlefield raid (Wintergrasp / Tol Barad) is the same trap as a BG raid.
    bool const inPvpRaid = bot->InBattleground() || group->isBFGroup();
    if (inPvpRaid && sRandomPlayerbotMgr->IsRandomBot(bot) && master && !master->GetGroup())
    {
        TC_LOG_DEBUG("playerbots", "[MASTER] %s: releasing BG master %s (left the raid group)",
                     bot->GetName().c_str(), master->GetName().c_str());
        SetMaster(nullptr);
    }

    PlayerbotAI* masterBotAI = nullptr;
    if (master)
        masterBotAI = GET_PLAYERBOT_AI(master);

    // Test BG master set
    if ((!master || (masterBotAI && !masterBotAI->IsRealPlayer())) && group)
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
        {
            return;
        }

        // Ideally we want to have the leader as master.
        Player* newMaster = botAI->GetGroupMaster();
        Player* playerMaster = nullptr;

        // Are there any non-bot players in the group?
        if (!newMaster || GET_PLAYERBOT_AI(newMaster))
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();
                if (!member || member == bot || member == newMaster || !member->IsInWorld() ||
                    !member->IsInSameRaidWith(bot))
                    continue;

                PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);
                if (memberBotAI)
                {
                    if (memberBotAI->IsRealPlayer() && !inPvpRaid)
                        playerMaster = member;

                    continue;
                }

                // AV real-player master exception (AC-identical:
                // Bot/PlayerbotAI.cpp:4499-4513). Re-enabled now that AV has ported tactics (step 4):
                // a bot in the same BG match and subgroup may still follow a real player with more
                // honor. Only API-name and honor-as-a-currency changed - 5.4.8 has no
                // Player::GetHonorPoints(); honor is CURRENCY_TYPE_HONOR_POINTS (DBCEnums.h).
                if (bot->InBattleground() && bot->GetBattleground() &&
                    bot->GetBattleground()->GetTypeID(true) == BATTLEGROUND_AV && !GET_PLAYERBOT_AI(member) &&
                    member->InBattleground() && bot->GetMapId() == member->GetMapId())
                {
                    // Skip if same BG but same subgroup or lower level
                    if (!group->SameSubGroup(bot, member) || member->GetLevel() < bot->GetLevel())
                        continue;

                    // Follow real player only if higher honor points
                    uint32 const honorpts = member->GetCurrency(CURRENCY_TYPE_HONOR_POINTS, false);
                    uint32 const botHonorpts = bot->GetCurrency(CURRENCY_TYPE_HONOR_POINTS, false);
                    if (botHonorpts && honorpts < botHonorpts)
                        continue;

                    playerMaster = member;
                    continue;
                }

                // A real player in the group is NOT a master while the bot is inside a
                // battleground: the bot has to play the objective, not trail the player around.
                // Upstream AC expresses this as the `!bot->InBattleground()` half of
                // `if ((!memberBotAI || IsSelfBot(member)) && !bot->InBattleground()) return member;`
                // in PlayerbotAI::FindNewMaster. The guard was commented out in this port, which is
                // what let a grouped bot acquire a master mid-match. See issue #53.
                if (inPvpRaid)
                    continue;

                newMaster = member;
                break;
            }
        }

        if (!newMaster && playerMaster)
            newMaster = playerMaster;

        if (newMaster && (!master || master != newMaster) && bot != newMaster)
        {
            master = newMaster;
            botAI->SetMaster(newMaster);
            botAI->ResetStrategies();

            // ResetStrategies() rebuilds from AiFactory, which deliberately omits "follow" while the
            // bot is in a battleground. Re-adding it unconditionally here defeated that: the bot
            // alternated between walking to its objective and running back to its master. Upstream
            // AC splits the two cases; this port had only the first half. See issue #53.
            if (!inPvpRaid)
            {
                botAI->ChangeStrategy("+follow", BOT_STATE_NON_COMBAT);

                if (botAI->GetMaster() == botAI->GetGroupMaster())
                    botAI->TellMaster("Hello, I follow you!");
                else
                    botAI->TellMaster(!urand(0, 2) ? "Hello!" : "Hi!");
            }
            else
            {
                // In a match: stay with the pack and focus on the objective.
                botAI->ChangeStrategy("-follow", BOT_STATE_NON_COMBAT);
            }
        }
    }
}

void PlayerbotAI::DoNextAction(bool min)
{
    if (!bot->IsInWorld() || bot->IsBeingTeleported() || (GetMaster() && GetMaster()->IsBeingTeleported()))
    {
        SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
        return;
    }

    // taxi-quest-stage-design.md G-T1 / AC parity (AC PlayerbotAI.cpp:1497ff has no in-flight early
    // return at all): the issue #120 fix above used to early-return here for the whole flight and set
    // SetNextCheckDelay(passiveDelay) (10 s on both confs), so the engine slept in up-to-10s chunks
    // while airborne and NewRpgTravelFlightAction::Execute (NewRpgAction.cpp) - the only other place
    // that set TravelFlight.inFlight, and the only caller of ContinueCrossMapTaxi - never ran during
    // the flight. AC just lets the engine run normally in flight: NewRpgTravelFlightAction::Execute
    // sets inFlight itself and calls ContinueCrossMapTaxi() every tick once bot->IsInFlight() is true
    // (NewRpgAction.cpp:478-483), so removing this block is enough - no manual flag-set needed here
    // any more. Since #72 AllowActive() is AC's, so a flying bot follows the same activity rotation as on AC.

    // Change engine if just died
    bool isBotAlive = bot->IsAlive();
    if (_currentEngine != _engines[BOT_STATE_DEAD] && !isBotAlive)
    {
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
        bot->GetMotionMaster()->MoveIdle();

        // Death Count to prevent skeleton piles
        Player* master = GetMaster();  // warning here - whipowill
        if (!HasActivePlayerMaster() && !bot->InBattleground())
        {
            uint32 dCount = _aiObjectContext->GetValue<uint32>("death count")->Get();
            _aiObjectContext->GetValue<uint32>("death count")->Set(++dCount);
        }

        _aiObjectContext->GetValue<Unit*>("current target")->Set(nullptr);
        //_aiObjectContext->GetValue<Unit*>("enemy player target")->Set(nullptr);
        //_aiObjectContext->GetValue<ObjectGuid>("pull target")->Set(ObjectGuid::Empty);
        // #151 (AC Bot/PlayerbotAI.cpp:1521): re-enabled now that "loot target" is a real value.
        _aiObjectContext->GetValue<LootObject>("loot target")->Set(LootObject());

        ChangeEngine(BOT_STATE_DEAD);
        return;
    }

    // Change engine if just ressed
    if (_currentEngine == _engines[BOT_STATE_DEAD] && isBotAlive)
    {
        ChangeEngine(BOT_STATE_NON_COMBAT);
        return;
    }

    // AC parity (#129): the MoP-only combat-engine "safety net" (drop to the non-combat engine 5 s after
    // entering the combat engine without IsInCombat) was removed - AC has none, and it aborted every
    // approach to a target more than ~35 yd away. Non-attackable targets are rejected by Attack() (#61).

    // Clear targets if in combat but sticking with old data
    if (_currentEngine == _engines[BOT_STATE_NON_COMBAT] && bot->IsInCombat())
    {
        Unit* currentTarget = _aiObjectContext->GetValue<Unit*>("current target")->Get();
        if (currentTarget != nullptr)
        {
            _aiObjectContext->GetValue<Unit*>("current target")->Set(nullptr);
        }
    }

    bool minimal = !AllowActivity();
    _currentEngine->DoNextAction(nullptr, 0, (minimal || min));

    if (minimal)
    {
        // #900: never flag a battlefield war bot AFK - Battlefield::KickAfkPlayers (1/s) teleports AFK players to the
        // KickPosition (WG: BattlefieldWG.cpp:76), which emptied Wintergrasp of every accepted bot in wave 95.
        bool const inBfRaid = bot->GetGroup() && bot->GetGroup()->isBFGroup();
        if (!bot->isAFK() && !bot->InBattleground() && !inBfRaid && !HasRealPlayerMaster())
            bot->ToggleAFK();

        SetNextCheckDelay(sPlayerbotAIConfig->passiveDelay);
        return;
    }
    else if (bot->isAFK())
        bot->ToggleAFK();

    if (master && master->IsInWorld())
    {
        float distance = sServerFacade->GetDistance2d(bot, master);
        if (master->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING) && distance < 20.0f)
            bot->m_movementInfo.AddMovementFlag(MOVEMENTFLAG_WALKING);
        else
            bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_WALKING);

        if (master->IsSitState() && nextAICheckDelay < 1000)
        {
            if (!bot->isMoving() && distance < 10.0f)
                bot->SetStandState(UNIT_STAND_STATE_SIT);
        }
        else if (nextAICheckDelay < 1000)
            bot->SetStandState(UNIT_STAND_STATE_STAND);
    }
    else if (bot->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING))
        bot->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_WALKING);
    else if ((nextAICheckDelay < 1000) && bot->IsSitState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);


    bool hasMountAura = bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED) ||
        bot->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED);
    if (hasMountAura && !bot->IsMounted())
    {
        bot->RemoveAurasByType(SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED);
        bot->RemoveAurasByType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED);
    }
}

// #401: defined in strategy/actions/CheckMountStateAction.cpp (AC ClearStaleFlightFlags port).
void ClearStaleFlightFlags(Player* bot);

void PlayerbotAI::HandleTeleportAck()
{
    if (IsRealPlayer())
        return;

    bot->GetMotionMaster()->Clear(true);
    bot->StopMoving();
    bool const wasFar = bot->IsBeingTeleportedFar();
    bool const wasNear = bot->IsBeingTeleportedNear();
    if (bot->IsBeingTeleportedNear())
    {
        // Temporary fix for instance can not enter
        if (!bot->IsInWorld())
        {
            bot->GetMap()->AddPlayerToMap(bot);
        }
        while (bot->IsInWorld() && bot->IsBeingTeleportedNear())
        {
            Player* plMover = bot->m_mover->ToPlayer();
            if (!plMover)
                return;

            ObjectGuid guid = plMover->GetGUID();
            WorldPacket p = WorldPacket(CMSG_MOVE_TELEPORT_ACK, 20);
            p << (uint32)0;  // supposed to be flags? not used currently
            p << (uint32)0;  // time - not currently used
            
            p.WriteBit(guid[0]);
            p.WriteBit(guid[7]);
            p.WriteBit(guid[3]);
            p.WriteBit(guid[5]);
            p.WriteBit(guid[4]);
            p.WriteBit(guid[6]);
            p.WriteBit(guid[1]);
            p.WriteBit(guid[2]);

            p.WriteByteSeq(guid[4]);
            p.WriteByteSeq(guid[1]);
            p.WriteByteSeq(guid[6]);
            p.WriteByteSeq(guid[7]);
            p.WriteByteSeq(guid[0]);
            p.WriteByteSeq(guid[2]);
            p.WriteByteSeq(guid[5]);
            p.WriteByteSeq(guid[3]);
            
            WorldPackets::Movement::MoveTeleportAck ack(std::move(p));
            ack.Read();
            bot->GetSession()->HandleMoveTeleportAck(ack);

            // #224: a bot teleported next to a master who is riding a boat/zeppelin arrives at the
            // summoner's world coordinates as a non-passenger (Player::TeleportTo only ever removes a
            // passenger), so the deck moves out from under it and the bot is left in the air. Seat it
            // now, while its world position is known - AddPassenger stores the transport-local offset
            // and Transport::UpdatePassengerPositions then carries it. This also makes FollowAction's
            // #206 B1 dock-hold condition (bot->GetTransport() != transport) false, so the bot is not
            // then told to hold still in mid-air. P2 (stop before board) and P3 (deck snap) are inside
            // SeatBotOnMasterTransport; see the comments there.
            SeatBotOnMasterTransport(bot, GetMaster());
        };
    }
    if (bot->IsBeingTeleportedFar())
    {
        while (bot->IsBeingTeleportedFar())
        {
            bot->GetSession()->HandleMoveWorldportAck();
        }
        // AC's core recomputes position data (outdoors flag, zone/area) on a far teleport
        // (AC MovementHandler.cpp:106); this core only does it on relocation, so a bot that left from
        // indoors arrives flagged indoors and never mounts until it moves
        bot->UpdatePositionData();
        // SetNextCheckDelay(urand(2000, 5000));
        // #545 (AC Bot/PlayerbotAI.cpp:930-932): apply instance strategies after the map attach
        if (sPlayerbotAIConfig->applyInstanceStrategies)
            ApplyInstanceStrategies(bot->GetMapId(), true);
        // #817 (AC Bot/PlayerbotAI.cpp:954-955 EvaluateHealerDpsStrategy): re-evaluate "healer dps" for the new map
        {
            static bool const restrictHealerDps = sConfigMgr->GetBoolDefault("AiPlayerbot.HealerDPSMapRestriction", true);
            if (restrictHealerDps)
                AiFactoryEvaluateHealerDps(bot, this);
        }
        Reset(true);

        // P1: the same hole as the near branch, and on a zeppelin leg it is the branch that actually
        // runs - Cloudkisser 181689 changes map 0 -> 571. Player::TeleportTo only ever REMOVES a
        // passenger, so a bot left behind on the previous map lands at the summoner's world
        // coordinates as a non-passenger and the deck moves out from under it, for good. Seat it
        // here, after HandleMoveWorldportAck + UpdatePositionData, while it is in world on the
        // destination map.
        SeatBotOnMasterTransport(bot, GetMaster());
    }
    // #401 (beyond AC; AC has the same window): Player::TeleportTo keeps DISABLE_GRAVITY across a
    // teleport (MOVEMENTFLAG_MASK_HAS_PLAYER_STATUS_OPCODE) and removing a flying mount never clears it,
    // so a bot that rode a flyer arrives with IsFlying()==true and CastSpell refuses every cast until a
    // movement action runs; in a BG prep room CheckMountStateAction::isUseful is false (>30 s to start),
    // so its own ClearStaleFlightFlags never runs. Clear it here, with the same gates as isUseful.
    if ((wasFar || wasNear) && bot->IsInWorld() && !IsInVehicle() && !bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        ClearStaleFlightFlags(bot);
    // AC parity: simulated teleport latency (AC PlayerbotAI.cpp:827 far, :859 near)
    if (wasFar)
        SetNextCheckDelay(urand(2000, 5000));
    else if (wasNear)
        SetNextCheckDelay(urand(1000, 2000));
    else
        SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
}

void PlayerbotAI::HandleBotOutgoingPacket(WorldPacket const& packet)
{
    if (packet.empty())
        return;
    if (!bot || !bot->IsInWorld() || bot->IsDuringRemoveFromWorld())
    {
        return;
    }

    //TC_LOG_INFO("playerbots", "Player: %s Received packet %s", bot->GetName().c_str(), GetOpcodeNameForLogging((OpcodeServer)packet->GetOpcode()).c_str());

    switch (packet.GetOpcode())
    {
    case SMSG_SPELL_FAILURE:
    {
        return;
    }
    case SMSG_SPELL_DELAYED:
    {
        return;
    }
    case SMSG_EMOTE:  // do not react to NPC emotes
    {
        // #163 S8 (AC parity, Bot/PlayerbotAI.cpp:1176-1186): forward a player's SMSG_EMOTE to the
        // handler dispatch (research s8-pet-control-and-class-spells.md B.9 - the packet layout is
        // unchanged: uint32 emoteId, uint64 sourceGuid), so "receive emote" can fire on it.
        WorldPacket p(packet);
        ObjectGuid source;
        uint32 emoteId;
        p.rpos(0);
        p >> emoteId >> source;
        if (source.IsPlayer())
            botOutgoingPacketHandlers.AddPacket(packet);

        return;
    }
    case SMSG_MESSAGECHAT:  // do not react to self or if not ready to reply
    {
        return;
    }
    case SMSG_MOVE_KNOCK_BACK:  // handle knockbacks
    {
        // #574: AC Bot/PlayerbotAI.cpp:1460-1480. The packet layout is 5.4.8's: Unit::SendMoveKnockBack
        // (src/server/game/Entities/Unit/Unit.cpp:17422-17452) writes speedXY, vsin, speedZ (already negated by
        // Unit::KnockbackFrom, :17480), uint32 counter, vcos, then the bit-packed mover guid. AC reads its guid
        // but never uses it, so the trailing guid is left unread here.
        WorldPacket p(packet);
        p.rpos(0);

        uint32 counter;
        float vcos, vsin, horizontalSpeed, verticalSpeed = 0.f;

        p >> horizontalSpeed >> vsin >> verticalSpeed >> counter >> vcos;
        if (horizontalSpeed <= 0.1f)
            horizontalSpeed = 0.11f;
        verticalSpeed = -verticalSpeed;

        bot->CastStop();
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();

        // MotionMaster::MoveKnockbackFromForPlayer exists on this core with AC's signature
        // (src/server/game/Movement/MotionMaster.cpp:1046-1065).
        bot->GetMotionMaster()->MoveKnockbackFromForPlayer(bot->GetPositionX() - vcos, bot->GetPositionY() - vsin,
                                                           horizontalSpeed, verticalSpeed);
        return;
    }
    case SMSG_DISMOUNT:
    {
        // #871 (AC Bot/PlayerbotAI.cpp:1549-1558): the server forced a dismount (Unit::Dismount,
        // Unit.cpp:11635-11648, sends it to the set incl. self); land the bot. 5.4.8 layout
        // (WorldPackets::Misc::Dismount::Write, MiscPackets.cpp:188-206): 8 guid mask bits in the order
        // 6,3,0,7,1,2,5,4 then the 8 guid bytes in the order 3,6,7,5,1,4,2,0 (no mask gating).
        WorldPacket p(packet);
        p.rpos(0);
        p.ResetBitPos();
        static uint8 const bitOrder[8] = {6, 3, 0, 7, 1, 2, 5, 4};
        static uint8 const byteOrder[8] = {3, 6, 7, 5, 1, 4, 2, 0};
        ObjectGuid guid;
        // ReadBit sets the byte slot to 1 (present); ReadByteSeq xors it with the wire byte (b ^ 1 on write).
        for (uint8 i = 0; i < 8; ++i)
            guid[bitOrder[i]] = p.ReadBit() ? 1 : 0;
        for (uint8 i = 0; i < 8; ++i)
            p.ReadByteSeq(guid[byteOrder[i]]);
        if (guid != bot->GetGUID())
            return;
        CheckMountStateAction::CompleteDismount(bot);
        return;
    }
    case SMSG_TIME_SYNC_REQ:
    {
        // #503: a selfbot has a real client that answers this itself (same rule as HandleTeleportAck, AC
        // PlayerbotAI.cpp:931 "Skip acknowledgment for selfbots. The player's client handles that.").
        if (IsRealPlayer())
            break;

        WorldPacket p = packet;
        uint32 counter;
        p >> counter;
        uint32 clientTicks = time(NULL);
        WorldPacket packet(CMSG_TIME_SYNC_RESP);
        packet.rpos(0);
        packet << counter << clientTicks;

        bot->GetSession()->HandleTimeSyncResp(packet);
        break;
    }
    default:
        botOutgoingPacketHandlers.AddPacket(packet);
        return;
    }
}

void PlayerbotAI::HandleMasterIncomingPacket(WorldPacket const& packet)
{
    masterIncomingPacketHandlers.AddPacket(packet);
}

void PlayerbotAI::HandleMasterOutgoingPacket(WorldPacket const& packet)
{
    masterOutgoingPacketHandlers.AddPacket(packet);
}

bool PlayerbotAI::HasRealPlayerMaster()
{
    if (master)
    {
        PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(master);
        return !masterBotAI || masterBotAI->IsRealPlayer();
    }

    return false;
}

// #611 item 4. AC Bot/PlayerbotAI.cpp:4661: IsRealPlayer(master) || IsSelfBot(master); the fork has no selfbots, so
// this is HasRealPlayerMaster() (as BudgetValues.h:17 and AiFactory.cpp:260 already substitute).
bool PlayerbotAI::HasGameClientMaster() { return HasRealPlayerMaster(); }

// AC Bot/PlayerbotAI.cpp:4673-4695
uint32 PlayerbotAI::GetFixedBotNumber(uint32 maxNum)
{
    if (maxNum == 0)
        return 0;

    // Deterministic pseudo-random hash based on the bot GUID evenly distributed across active slots
    uint32 id = bot->GetGUID().GetCounter();
    uint32 h = id;
    h ^= h >> 16;
    h *= 0x7feb352d;
    h ^= h >> 15;
    h *= 0x846ca68b;
    h ^= h >> 16;

    // Current time slot
    uint32 timeSlot = (getMSTime() / 1000) / sPlayerbotAIConfig->botActiveAloneDurationSeconds;

    // Mix timeSlot into the hash to reshuffle every rotation window
    uint32 mixed = h ^ (timeSlot * 0x9e3779b9);  // with multiplicative constant

    return mixed % maxNum;
}

// AC Bot/PlayerbotAI.cpp:4708-4750
GrouperType PlayerbotAI::GetGrouperType()
{
    uint32 grouperNumber = GetFixedBotNumber(100);

    if (grouperNumber < 20 && !HasGameClientMaster())
        return GrouperType::SOLO;

    if (grouperNumber < 80)
        return GrouperType::MEMBER;

    if (grouperNumber < 85)
        return GrouperType::LEADER_2;

    if (grouperNumber < 90)
        return GrouperType::LEADER_3;

    if (grouperNumber < 95)
        return GrouperType::LEADER_4;

    return GrouperType::LEADER_5;
}

GuilderType PlayerbotAI::GetGuilderType()
{
    uint32 grouperNumber = GetFixedBotNumber(100);

    if (grouperNumber < 20 && !HasGameClientMaster())
        return GuilderType::SOLO;

    if (grouperNumber < 30)
        return GuilderType::TINY;

    if (grouperNumber < 40)
        return GuilderType::SMALL;

    if (grouperNumber < 60)
        return GuilderType::MEDIUM;

    if (grouperNumber < 80)
        return GuilderType::LARGE;

    return GuilderType::VERY_LARGE;
}

// AC Bot/PlayerbotAI.cpp:5570
bool PlayerbotAI::HasSkill(SkillType skill) { return bot->HasSkill(skill) && bot->GetSkillValue(skill) > 0; }

// AC Bot/PlayerbotAI.cpp:6235 -> PlayerbotGuildMgr::IsRealGuild (PlayerbotGuildMgr.cpp:297): a real guild is one whose
// leader's account is not in the bot accounts list. The fork has no PlayerbotGuildMgr cache, so the same test is
// evaluated directly against the guild leader's account.
bool PlayerbotAI::IsInRealGuild()
{
    if (!bot->GetGuildId())
        return false;

    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    // #72: CharacterCache, not ObjectMgr::GetPlayerAccountIdByGUID - that runs a synchronous character DB query
    // when the leader is offline, and AllowActive() now calls this from the map threads (AC reads a cache list).
    uint32 const leaderAccount = sCharacterCache->GetCharacterAccountIdByGuid(guild->GetLeaderGUID());
    return leaderAccount && !sPlayerbotAIConfig->IsInRandomAccountList(leaderAccount);
}

bool PlayerbotAI::HasActivePlayerMaster()
{
    return master && !GET_PLAYERBOT_AI(master);
}

bool PlayerbotAI::HasPlayerNearby(WorldPosition* pos, float range)
{
    float sqRange = range * range;
    bool nearPlayer = false;
    for (auto& player : sRandomPlayerbotMgr->GetPlayers())
    {
        if (!player->IsGameMaster() || player->isGMVisible())
        {
            if (player->GetMapId() != bot->GetMapId())
                continue;

            if (pos->sqDistance(WorldPosition(player)) < sqRange)
                nearPlayer = true;

            // if player is far check farsight/cinematic camera
            WorldObject* viewObj = player->GetViewpoint();
            if (viewObj && viewObj != player)
            {
                if (pos->sqDistance(WorldPosition(viewObj)) < sqRange)
                    nearPlayer = true;
            }
        }
    }

    return nearPlayer;
}

bool PlayerbotAI::HasPlayerNearby(float range)
{
    WorldPosition botPos(bot);
    return HasPlayerNearby(&botPos, range);
};

bool PlayerbotAI::HasManyPlayersNearby(uint32 trigerrValue, float range)
{
    float sqRange = range * range;
    uint32 found = 0;

    for (auto& player : sRandomPlayerbotMgr->GetPlayers())
    {
        if ((!player->IsGameMaster() || player->isGMVisible()) && sServerFacade->GetDistance2d(player, bot) < sqRange)
        {
            found++;

            if (found >= trigerrValue)
                return true;
        }
    }

    return false;
}

bool PlayerbotAI::IsAlt()
{
    return HasRealPlayerMaster() && !sRandomPlayerbotMgr->IsRandomBot(bot);
}

bool PlayerbotAI::IsInVehicle(bool canControl, bool canCast, bool canAttack, bool canTurn, bool fixed)
{
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicle)
        return false;

    // get vehicle
    Unit* vehicleBase = vehicle->GetBase();
    if (!vehicleBase || !vehicleBase->IsAlive())
        return false;

    if (!vehicle->GetVehicleInfo())
        return false;

    // get seat
    VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
    if (!seat)
        return false;

    if (!(canControl || canCast || canAttack || canTurn || fixed))
        return true;

    if (canControl)
        return seat->CanControl() && !(vehicle->GetVehicleInfo()->m_flags & VEHICLE_FLAG_FIXED_POSITION);

    if (canCast)
        return (seat->m_flags & VEHICLE_SEAT_FLAG_CAN_CAST) != 0;

    if (canAttack)
        return (seat->m_flags & VEHICLE_SEAT_FLAG_CAN_ATTACK) != 0;

    if (canTurn)
        return (seat->m_flags & VEHICLE_SEAT_FLAG_ALLOW_TURNING) != 0;

    if (fixed)
        return (vehicle->GetVehicleInfo()->m_flags & VEHICLE_FLAG_FIXED_POSITION) != 0;

    return false;
}
bool PlayerbotAI::IsOpposing(Player* player)
{
    return IsOpposing(player->GetRace(), bot->GetRace());
}

bool PlayerbotAI::IsOpposing(uint8 race1, uint8 race2)
{
    return (IsAlliance(race1) && !IsAlliance(race2)) || (!IsAlliance(race1) && IsAlliance(race2));
}
// AC PlayerbotAI.cpp:1948-1950 (#413)
bool PlayerbotAI::HasTargetExclusions() const
{
    return _engines[BOT_STATE_COMBAT] && _engines[BOT_STATE_COMBAT]->HasTargetExclusions();
}

Strategy* PlayerbotAI::GetStrategy(std::string const name, BotState type)
{
    return _engines[type] ? _engines[type]->GetStrategy(name) : nullptr;
}

bool PlayerbotAI::HasStrategy(std::string const name, BotState type)
{
    if (_engines[type])
        return _engines[type]->HasStrategy(name);
    return false;
}

bool PlayerbotAI::ContainsStrategy(StrategyType type)
{
    for (uint8 i = 0; i < BOT_STATE_MAX; i++)
    {
        if (_engines[i]->HasStrategyType(type))
            return true;
    }

    return false;
}

bool PlayerbotAI::CanMove()
{
    // do not allow if not vehicle driver
    //if (IsInVehicle() && !IsInVehicle(true))
        //return false;

    if (bot->isFrozen() || bot->IsPolymorphed() || (bot->isDead() && !bot->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GHOST)) ||
        bot->IsBeingTeleported() /* || bot->HasRootAura() || bot->HasSpiritOfRedemptionAura() || bot->HasConfuseAura()*/ ||
        bot->IsCharmed() /* || bot->HasStunAura()*/ || bot->IsInFlight() || bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
        return false;

    return bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FLIGHT_MOTION_TYPE;
}

Unit* PlayerbotAI::GetUnit(ObjectGuid guid)
{
    if (!guid)
        return nullptr;

    return ObjectAccessor::GetUnit(*bot, guid);
}

Player* PlayerbotAI::GetPlayer(ObjectGuid guid)
{
    Unit* unit = GetUnit(guid);
    return unit ? unit->ToPlayer() : nullptr;
}

Player* PlayerbotAI::GetGroupMaster()
{
    if (!bot->InBattleground())
        if (Group* group = bot->GetGroup())
            if (Player* player = ObjectAccessor::FindPlayer(group->GetLeaderGUID()))
                return player;

    return master;
}

uint32 GetCreatureIdForCreatureTemplateId(uint32 creatureTemplateId)
{
    QueryResult results = WorldDatabase.PQuery("SELECT guid FROM `creature` WHERE id = %u LIMIT 1;", creatureTemplateId);
    if (results)
    {
        Field* fields = results->Fetch();
        return fields[0].GetUInt32();
    }
    return 0;
}

Unit* PlayerbotAI::GetUnit(CreatureData const* creatureData)
{
    if (!creatureData)
        return nullptr;

    Map* map = sMapMgr->FindMap(creatureData->mapId, 0);
    if (!map)
        return nullptr;

    uint32 spawnId = creatureData->spawnId;
    if (!spawnId)  // workaround for CreatureData with missing spawnId (this just uses first matching creatureId in DB,
        // but thats ok this method is only used for battlemasters and theres only 1 of each type)
        spawnId = GetCreatureIdForCreatureTemplateId(creatureData->id);
    auto creatureBounds = map->GetCreatureBySpawnIdStore().equal_range(spawnId);
    if (creatureBounds.first == creatureBounds.second)
        return nullptr;

    return creatureBounds.first->second;
}

Creature* PlayerbotAI::GetCreature(ObjectGuid guid)
{
    if (!guid)
        return nullptr;

    return ObjectAccessor::GetCreature(*bot, guid);
}

GameObject* PlayerbotAI::GetGameObject(ObjectGuid guid)
{
    if (!guid)
        return nullptr;

    return ObjectAccessor::GetGameObject(*bot, guid);
}
WorldObject* PlayerbotAI::GetWorldObject(ObjectGuid guid)
{
    if (!guid)
        return nullptr;

    return ObjectAccessor::GetWorldObject(*bot, guid);
}

bool PlayerbotAI::SayToParty(const std::string& msg)
{
    if (!bot->GetGroup())
        return false;

    /*WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_PARTY, msg.c_str(), LANG_UNIVERSAL, CHAT_TAG_NONE, bot->GetGUID(),
        bot->GetName());

    for (auto reciever : GetPlayersInGroup())
    {
        sServerFacade->SendPacket(reciever, &data);
    }*/

    return true;
}

bool PlayerbotAI::SayToRaid(const std::string& msg)
{
    if (!bot->GetGroup() || bot->GetGroup()->isRaidGroup())
        return false;

    /*WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_RAID, msg.c_str(), LANG_UNIVERSAL, CHAT_TAG_NONE, bot->GetGUID(),
        bot->GetName());

    for (auto reciever : GetPlayersInGroup())
    {
        sServerFacade->SendPacket(reciever, &data);
    }*/

    return true;
}

bool PlayerbotAI::Yell(const std::string& msg)
{
    if (bot->GetTeamId() == TeamId::TEAM_ALLIANCE)
    {
        bot->Yell(msg, LANG_COMMON);
    }
    else
    {
        bot->Yell(msg, LANG_ORCISH);
    }

    return true;
}

bool PlayerbotAI::Say(const std::string& msg)
{
    if (bot->GetTeamId() == TeamId::TEAM_ALLIANCE)
    {
        bot->Say(msg, LANG_COMMON);
    }
    else
    {
        bot->Say(msg, LANG_ORCISH);
    }

    return true;
}

bool PlayerbotAI::Whisper(const std::string& msg, const std::string& receiverName)
{
    const auto receiver = ObjectAccessor::FindPlayerByName(receiverName);
    if (!receiver)
    {
        return false;
    }

    if (bot->GetTeamId() == TeamId::TEAM_ALLIANCE)
    {
        bot->Whisper(msg, LANG_COMMON, receiver);
    }
    else
    {
        bot->Whisper(msg, LANG_ORCISH, receiver);
    }

    return true;
}

// #173: AC Bot/PlayerbotAI.cpp:3255-3283 (securityLevel parameter; no master -> TellMasterNoFacing when
// randomBotSayWithoutMaster, which Says it, as AC).
bool PlayerbotAI::TellMaster(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel)
{
    return TellMaster(stream.str(), securityLevel);
}

bool PlayerbotAI::TellMaster(std::string const text, PlayerbotSecurityLevel securityLevel)
{
    if (!master)
    {
        if (sPlayerbotAIConfig->randomBotSayWithoutMaster)
            return TellMasterNoFacing(text, securityLevel);

        return false;
    }
    if (!TellMasterNoFacing(text, securityLevel))
        return false;

    if (!bot->isMoving() && !bot->IsInCombat() && bot->GetMapId() == master->GetMapId() &&
        !bot->HasUnitState(UNIT_STATE_IN_FLIGHT) && !bot->IsFlying())
    {
        if (!BotHasInArcAC(bot, EMOTE_ANGLE_IN_FRONT, master, sPlayerbotAIConfig->sightDistance))
            bot->SetFacingToObject(master);

        bot->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
    }

    return true;
}

// #173: AC Bot/PlayerbotAI.cpp:3184-3223.
bool PlayerbotAI::TellMasterNoFacing(std::ostringstream& stream, PlayerbotSecurityLevel securityLevel)
{
    return TellMasterNoFacing(stream.str(), securityLevel);
}

bool PlayerbotAI::TellMasterNoFacing(std::string const text, PlayerbotSecurityLevel securityLevel)
{
    Player* master = GetMaster();
    PlayerbotAI* masterBotAI = nullptr;
    if (master)
        masterBotAI = GET_PLAYERBOT_AI(master);

    if ((!master || (masterBotAI && !masterBotAI->IsRealPlayer())) &&
        (sPlayerbotAIConfig->randomBotSayWithoutMaster || HasStrategy("debug", BOT_STATE_NON_COMBAT)))
    {
        bot->Say(text, (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
        return true;
    }

    if (!IsTellAllowed(securityLevel))
        return false;

    time_t lastSaid = whispers[text];

    if (!lastSaid || (time(nullptr) - lastSaid) >= sPlayerbotAIConfig->repeatDelay / 1000)
    {
        whispers[text] = time(nullptr);

        ChatMsg type = CHAT_MSG_WHISPER;
        if (currentChat.second - time(nullptr) >= 1)
            type = currentChat.first;

        WorldPacket data;
        ChatHandler::BuildChatPacket(data, type == CHAT_MSG_ADDON ? CHAT_MSG_PARTY : type,
            type == CHAT_MSG_ADDON ? LANG_ADDON : LANG_UNIVERSAL, bot, nullptr, text.c_str());
        master->SendDirectMessage(&data);
    }

    return true;
}

// #173. Ported from AC Bot/PlayerbotAI.cpp:3237-3253 (IsTellAllowed(securityLevel)).
bool PlayerbotAI::IsTellAllowed(PlayerbotSecurityLevel securityLevel)
{
    Player* master = GetMaster();
    if (!master || master->IsBeingTeleported())
        return false;

    if (!GetSecurity()->CheckLevelFor(securityLevel, true, master))
        return false;

    if (sPlayerbotAIConfig->whisperDistance && !bot->GetGroup() && sRandomPlayerbotMgr->IsRandomBot(bot) &&
        !master->GetSession()->HasPermission(rbac::RBAC_PERM_COMMAND_GM) && // AC CanBeGameMaster()
        (bot->GetMapId() != master->GetMapId() ||
         bot->GetDistance2d(master) > sPlayerbotAIConfig->whisperDistance))
        return false;

    return true;
}
// #173: AC Bot/PlayerbotAI.cpp:3225-3235 (the IsTellAllowed gate first, as AC).
bool PlayerbotAI::TellError(std::string const text, PlayerbotSecurityLevel securityLevel)
{
    Player* master = GetMaster();
    if (!IsTellAllowed(securityLevel) || !master || GET_PLAYERBOT_AI(master))
        return false;

    if (PlayerbotMgr* mgr = GET_PLAYERBOT_MGR(master))
        mgr->TellError(bot->GetName(), text);

    return false;
}

int32 PlayerbotAI::GetNearGroupMemberCount(float dis)
{
    int count = 1;  // yourself
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (member == bot)  // calculated
                continue;

            if (!member || !member->IsInWorld())
                continue;

            if (member->GetMapId() != bot->GetMapId())
                continue;

            if (member->GetExactDist(bot) > dis)
                continue;

            count++;
        }
    }
    return count;
}

float PlayerbotAI::GetRange(std::string const type)
{
    float val = 0;
    if (_aiObjectContext)
        val = _aiObjectContext->GetValue<float>("range", type)->Get();

    if (abs(val) >= 0.1f)
        return val;

    if (type == "spell")
        return sPlayerbotAIConfig->spellDistance;

    if (type == "shoot")
        return sPlayerbotAIConfig->shootDistance;

    if (type == "flee")
        return sPlayerbotAIConfig->fleeDistance;

    if (type == "heal")
        return sPlayerbotAIConfig->healDistance;

    if (type == "melee")
        return sPlayerbotAIConfig->meleeDistance;

    return 0;
}

bool IsRealAura(Player* bot, AuraEffect const* aurEff, Unit const* unit)
{
    if (!aurEff)
        return false;

    if (!unit->IsHostileTo(bot))
        return true;

    SpellInfo const* spellInfo = aurEff->GetSpellInfo();

    uint32 stacks = aurEff->GetBase()->GetStackAmount();
    if (stacks >= spellInfo->StackAmount)
        return true;

    if (aurEff->GetCaster() == bot || spellInfo->IsPositive() ||
        spellInfo->Effects[aurEff->GetEffIndex()].IsAreaAuraEffect())
        return true;

    return false;
}

bool PlayerbotAI::canDispel(SpellInfo const* spellInfo, uint32 dispelType)
{
    static std::vector<std::string> dispel_whitelist =
    {
        "mutating injection",
        "frostbolt",
    };

    if (spellInfo->Dispel != dispelType)
        return false;

    if (!spellInfo->SpellName)
    {
        return true;
    }

    for (std::string& wl : dispel_whitelist)
    {
        // #388: caseInsensitiveEqual returns true on a match (AC strcmpi returns 0), so AC PlayerbotAI.cpp:4618
        // "strcmpi(...) == 0" is "caseInsensitiveEqual(...)" here. The "== 0" port rejected every named aura.
        if (caseInsensitiveEqual(spellInfo->SpellName, wl))
        {
            return false;
        }
    }

    // #388: AC PlayerbotAI.cpp:4624-4629 "strcmpi(name, x) &&" (= name differs from x) is "!caseInsensitiveEqual(name, x)".
    return !spellInfo->SpellName || (!caseInsensitiveEqual(spellInfo->SpellName, "demon skin") &&
        !caseInsensitiveEqual(spellInfo->SpellName, "mage armor") &&
        !caseInsensitiveEqual(spellInfo->SpellName, "frost armor") &&
        !caseInsensitiveEqual(spellInfo->SpellName, "wavering will") &&
        !caseInsensitiveEqual(spellInfo->SpellName, "chilled") &&
        !caseInsensitiveEqual(spellInfo->SpellName, "mana tap") &&
        !caseInsensitiveEqual(spellInfo->SpellName, "ice armor"));
}

bool PlayerbotAI::HasAuraToDispel(Unit* target, uint32 dispelType)
{
    if (!target->IsInWorld())
    {
        return false;
    }
    bool isFriend = bot->IsFriendlyTo(target);
    Unit::VisibleAuraMap const* visibleAuras = target->GetVisibleAuras();
    for (Unit::VisibleAuraMap::const_iterator itr = visibleAuras->begin(); itr != visibleAuras->end(); ++itr)
    {
        Aura* aura = itr->second->GetBase();

        if (aura->IsPassive())
            continue;

        if (sPlayerbotAIConfig->dispelAuraDuration && aura->GetDuration() &&
            aura->GetDuration() < (int32)sPlayerbotAIConfig->dispelAuraDuration)
            continue;

        SpellInfo const* spellInfo = aura->GetSpellInfo();

        bool isPositiveSpell = spellInfo->IsPositive();
        if (isPositiveSpell && isFriend)
            continue;

        if (!isPositiveSpell && !isFriend)
            continue;

        if (canDispel(spellInfo, dispelType))
            return true;
    }
    return false;
}

bool PlayerbotAI::HasAura(std::string const name, Unit* unit, bool maxStack, bool checkIsOwner, int maxAuraAmount,
    bool checkDuration)
{
    if (!unit)
        return false;

    std::wstring wnamepart;
    if (!Utf8toWStr(name, wnamepart))
        return false;

    wstrToLower(wnamepart);

    int auraAmount = 0;

    // Iterate through all aura types
    for (uint32 auraType = SPELL_AURA_BIND_SIGHT; auraType < TOTAL_AURAS; auraType++)
    {
        Unit::AuraEffectList const& auras = unit->GetAuraEffectsByType((AuraType)auraType);
        if (auras.empty())
            continue;

        // Iterate through each aura effect
        for (AuraEffect const* aurEff : auras)
        {
            if (!aurEff)
                continue;

            SpellInfo const* spellInfo = aurEff->GetSpellInfo();
            if (!spellInfo)
                continue;

            // Check if the aura name matches
            std::string_view const auraName = spellInfo->SpellName;
            if (auraName.empty() || auraName.length() != wnamepart.length() || !Utf8FitTo(std::string(auraName), wnamepart))
                continue;

            // Check if this is a valid aura for the bot
            if (IsRealAura(bot, aurEff, unit))
            {
                // Check caster if necessary
                if (checkIsOwner && aurEff->GetCasterGUID() != bot->GetGUID())
                    continue;

                // Check aura duration if necessary
                if (checkDuration && aurEff->GetBase()->GetDuration() == -1)
                    continue;

                // Count stacks and charges
                uint32 maxStackAmount = spellInfo->StackAmount;
                uint32 maxProcCharges = spellInfo->ProcCharges;

                // Count the aura based on max stack and proc charges
                if (maxStack)
                {
                    if (maxStackAmount && aurEff->GetBase()->GetStackAmount() >= maxStackAmount)
                        auraAmount++;

                    if (maxProcCharges && aurEff->GetBase()->GetCharges() >= maxProcCharges)
                        auraAmount++;
                }
                else
                {
                    auraAmount++;
                }

                // Early exit if maxAuraAmount is reached
                if (maxAuraAmount < 0 && auraAmount > 0)
                    return true;
            }
        }
    }

    // Return based on the maximum aura amount conditions
    if (maxAuraAmount >= 0)
    {
        return auraAmount == maxAuraAmount || (auraAmount > 0 && auraAmount <= maxAuraAmount);
    }

    return false;
}

bool PlayerbotAI::HasAura(uint32 spellId, Unit const* unit)
{
    if (!spellId || !unit)
        return false;

    return unit->HasAura(spellId);
}

void PlayerbotAI::RemoveShapeshift()
{
    RemoveAura("bear form");
    RemoveAura("dire bear form");
    RemoveAura("moonkin form");
    RemoveAura("travel form");
    RemoveAura("cat form");
    RemoveAura("flight form");
    RemoveAura("swift flight form");
    RemoveAura("aquatic form");
    RemoveAura("ghost wolf");
    // RemoveAura("tree of life");
}

void PlayerbotAI::WaitForSpellCast(Spell* spell)
{
    if (!spell)
        return;

    SpellInfo const* spellInfo = spell->GetSpellInfo();
    uint32 castTime = spell->GetCastTime();

    if (spellInfo && spellInfo->IsChanneled())
    {
        int32 duration = spellInfo->GetDuration();
        bot->ApplySpellMod(spellInfo->Id, SPELLMOD_DURATION, duration);
        if (duration > 0)
            castTime += duration;
    }

    SetNextCheckDelay(castTime + sPlayerbotAIConfig->reactDelay);
}

void PlayerbotAI::RemoveAura(std::string const name)
{
    uint32 spellid = _aiObjectContext->GetValue<uint32>("spell id", name)->Get();
    if (spellid && HasAura(spellid, bot))
        bot->RemoveAurasDueToSpell(spellid);
}

Aura* PlayerbotAI::GetAura(std::string const name, Unit* unit, bool checkIsOwner, bool checkDuration, int checkStack)
{
    if (!unit)
        return nullptr;

    std::wstring wnamepart;
    if (!Utf8toWStr(name, wnamepart))
        return nullptr;

    wstrToLower(wnamepart);

    for (uint32 auraType = SPELL_AURA_BIND_SIGHT; auraType < TOTAL_AURAS; ++auraType)
    {
        Unit::AuraEffectList const& auras = unit->GetAuraEffectsByType((AuraType)auraType);
        if (auras.empty())
            continue;

        for (AuraEffect const* aurEff : auras)
        {
            SpellInfo const* spellInfo = aurEff->GetSpellInfo();
            if (!spellInfo)  // AC PlayerbotAI.cpp GetAura
                continue;

            // #72 perf: SpellName is a char const* in this core - binding it to std::string const& built a temporary
            // string (a heap allocation for names over 15 bytes) for every aura effect scanned. Same test as
            // HasAura above: length first, the string only for a candidate (AC compares SpellName[0], a std::string).
            std::string_view const auraName = spellInfo->SpellName ? spellInfo->SpellName : "";

            // Directly skip if name mismatch (both length and content)
            if (auraName.empty() || auraName.length() != wnamepart.length() || !Utf8FitTo(std::string(auraName), wnamepart))
                continue;

            if (!IsRealAura(bot, aurEff, unit))
                continue;

            // Check owner if necessary
            if (checkIsOwner && aurEff->GetCasterGUID() != bot->GetGUID())
                continue;

            // Check duration if necessary
            if (checkDuration && aurEff->GetBase()->GetDuration() == -1)
                continue;

            // Check stack if necessary
            if (checkStack != -1 && aurEff->GetBase()->GetStackAmount() < checkStack)
                continue;

            return aurEff->GetBase();
        }
    }

    return nullptr;
}

bool PlayerbotAI::HasAnyAuraOf(Unit* player, ...)
{
    if (!player)
        return false;

    va_list vl;
    va_start(vl, player);

    const char* cur;
    while ((cur = va_arg(vl, const char*)) != nullptr)
    {
        if (HasAura(cur, player))
        {
            va_end(vl);
            return true;
        }
    }

    va_end(vl);
    return false;
}

bool PlayerbotAI::IsInterruptableSpellCasting(Unit* target, std::string const spell)
{
    uint32 spellid = _aiObjectContext->GetValue<uint32>("spell id", spell)->Get();
    if (!spellid || !target->IsNonMeleeSpellCasted(true))
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return false;

    for (uint8 i = EFFECT_0; i <= EFFECT_2; i++)
    {
        if ((spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT) &&
            spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
            return true;

        if (spellInfo->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST &&
            !target->IsImmunedToSpellEffect(spellInfo, i))
            return true;

        if ((spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA) &&
            spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOD_SILENCE)
            return true;
    }

    return false;
}

void PlayerbotAI::SpellInterrupted(uint32 spellid)
{
    for (uint8 type = CURRENT_MELEE_SPELL; type <= CURRENT_CHANNELED_SPELL; type++)
    {
        Spell* spell = bot->GetCurrentSpell((CurrentSpellTypes)type);
        if (!spell)
            continue;
        if (spell->GetSpellInfo()->Id == spellid)
            bot->InterruptSpell((CurrentSpellTypes)type);
    }
}

void PlayerbotAI::InterruptSpell()
{
    for (uint8 type = CURRENT_MELEE_SPELL; type <= CURRENT_CHANNELED_SPELL; type++)
    {
        Spell* spell = bot->GetCurrentSpell((CurrentSpellTypes)type);
        if (!spell)
            continue;

        bot->InterruptSpell((CurrentSpellTypes)type);

        WorldPacket data(SMSG_SPELL_FAILURE, 8 + 1 + 4 + 1);
        data << bot->GetPackGUID();
        data << uint8(1);
        data << uint32(spell->m_spellInfo->Id);
        data << uint8(0);
        bot->SendMessageToSet(&data, true);

        data.Initialize(SMSG_SPELL_FAILED_OTHER, 8 + 1 + 4 + 1);
        data << bot->GetPackGUID();
        data << uint8(1);
        data << uint32(spell->m_spellInfo->Id);
        data << uint8(0);
        bot->SendMessageToSet(&data, true);

        SpellInterrupted(spell->m_spellInfo->Id);
    }
}

// #597 (MoP-only, no AC counterpart - WotLK has no override spells): a real client sends the BASE spell id and the
// server swaps it in WorldSession::HandleCastSpellOpcode (src/server/game/Handlers/SpellHandler.cpp:1091-1117):
// first SpellInfo::OverrideSpellList (talent ReplacesSpell, SpecializationSpells.RemovesSpellId, SpellMgr's
// AddSpellOverride list) when the player knows the override, then any SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS(_2)
// aura effect that affects the spell (amount = replacement id). The bot cast path skipped both, so e.g. a cat-form
// druid cast base Mangle 33917 (no combo point) instead of 33876 (Cat Form (Passive) 48629). Same order as the client.
static SpellInfo const* BotOverrideSpell(Player* bot, SpellInfo const* spellInfo)
{
    if (!bot || !spellInfo)
        return spellInfo;

    for (uint32 overrideId : spellInfo->OverrideSpellList)
    {
        if (bot->HasSpell(overrideId))
        {
            if (SpellInfo const* overrideInfo = sSpellMgr->GetSpellInfo(overrideId))
                spellInfo = overrideInfo;
            break;
        }
    }

    Unit::AuraEffectList swaps = bot->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS);
    Unit::AuraEffectList const& swaps2 = bot->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS_2);
    if (!swaps2.empty())
        swaps.insert(swaps.end(), swaps2.begin(), swaps2.end());

    for (AuraEffect const* eff : swaps)
    {
        if (eff->IsAffectingSpell(spellInfo))
        {
            if (SpellInfo const* newInfo = sSpellMgr->GetSpellInfo(eff->GetAmount()))
                return newInfo;
        }
    }

    return spellInfo;
}

bool PlayerbotAI::CanCastSpell(std::string const name, Unit* target, Item* itemTarget)
{
    return CanCastSpell(_aiObjectContext->GetValue<uint32>("spell id", name)->Get(), target, true, itemTarget);
}

bool PlayerbotAI::CanCastSpell(uint32 spellid, Unit* target, bool checkHasSpell, Item* itemTarget, Item* castItem)
{
    // Phase 1.1 diagnostic (issue #59). Every rejection used to be silent (the TC_LOG_DEBUG calls
    // were commented out), so "bots only auto-attack" could not be told apart: did the name fail to
    // resolve to a spell id, is the spell simply not in the spellbook, is it on cooldown, or did
    // CheckCast reject it? One greppable [CAST] line per rejection answers all four.
    // AC PlayerbotAI.cpp:3479/3490/3511/3523/3535/3548/3560/3574/3591/3605/3653 (#530 item 6): same gate on every one.
    auto logCast = [&]() { return !sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster()); };
    auto castFailed = [&](char const* why)
    {
        if (logCast())
            TC_LOG_DEBUG("playerbots", "[CAST] %s spell=%u has=%u why=%s", bot->GetName().c_str(), spellid,
                         uint32(bot->HasSpell(spellid)), why);
        return false;
    };

    if (!spellid)
        return castFailed("no-spellid");

    // #854 (MoP "while stunned/feared/confused" rows; beyond AC, which refuses every cast here): a spell flagged
    // usable in the bot's current loss-of-control state (Berserker Rage, Every Man for Himself, Blink, Dispersion...) may
    // still be cast. Possessed / jumping / charging and any state the spell is not flagged for keep AC's refusal.
    if (bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
    {
        bool usable = !bot->HasUnitState(UNIT_STATE_LOST_CONTROL & ~UNIT_STATE_CONTROLLED);
        if (usable)
        {
            SpellInfo const* ccInfo = sSpellMgr->GetSpellInfo(spellid);
            usable = ccInfo &&
                     (!bot->HasUnitState(UNIT_STATE_STUNNED) || ccInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_STUNNED)) &&
                     (!bot->HasUnitState(UNIT_STATE_FLEEING) || ccInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_FEARED)) &&
                     (!bot->HasUnitState(UNIT_STATE_CONFUSED) || ccInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_CONFUSED));
        }
        if (!usable)
            return castFailed("lost-control");
    }

    if (!target)
        target = bot;

    if (Pet* pet = bot->GetPet())
        if (pet->HasSpell(spellid))
            return true;

    if (checkHasSpell && !bot->HasSpell(spellid))
        return castFailed("not-known");

    // #597: the spellbook check stays on the base id (as the client); cooldown, power and CheckCast below test the
    // spell the bot will really cast (BotOverrideSpell, same swap as SpellHandler.cpp:1091-1117).
    if (SpellInfo const* baseInfo = sSpellMgr->GetSpellInfo(spellid))
        spellid = BotOverrideSpell(bot, baseInfo)->Id;

    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
        return castFailed("channeling");

    if (bot->HasSpellCooldown(spellid))
        return castFailed("cooldown");

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return castFailed("no-spellinfo");

    uint32 CastingTime = !spellInfo->IsChanneled() ? spellInfo->CalcCastTime(bot->GetLevel()) : spellInfo->GetDuration();
    if ((CastingTime || spellInfo->IsAutoRepeatRangedSpell()) && bot->isMoving())
        return castFailed("moving");

    if (!itemTarget)
    {
        if (target->IsImmunedToSpell(spellInfo, spellInfo->GetAllEffectsMechanicMask()))
            return castFailed("immune");

        if (bot != target && sServerFacade->GetDistance2d(bot, target) > sPlayerbotAIConfig->sightDistance)
            return castFailed("range");
    }

    Unit* oldSel = bot->GetSelectedUnit();
    // TRIGGERED_IGNORE_POWER_AND_REAGENT_COST flag for not calling CheckPower in check
    // which avoids buff charge to be ineffectively reduced (e.g. dk freezing fog for howling blast)
    /// @TODO: Fix all calls to ApplySpellMod

    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_IGNORE_POWER_AND_REAGENT_COST);
    spell->m_targets.SetUnitTarget(target);
    spell->m_CastItem = castItem;
    spell->m_targets.SetItemTarget(itemTarget);
    SpellCastResult result = spell->CheckCast(true);
    delete spell;

    if (oldSel)
        bot->SetSelection(oldSel->GetGUID());

    switch (result)
    {
    case SPELL_FAILED_NOT_INFRONT:
    case SPELL_FAILED_NOT_STANDING:
    case SPELL_FAILED_UNIT_NOT_INFRONT:
    case SPELL_FAILED_MOVING:
    case SPELL_FAILED_TRY_AGAIN:
    case SPELL_CAST_OK:
    case SPELL_FAILED_NOT_SHAPESHIFT:
    case SPELL_FAILED_OUT_OF_RANGE:
        return true;
    default:
        // w76 #868 (Helice: every GCD spell NOT_READY for ~10 min): on NOT_READY also log which lock it is (spell history vs GCD).
        if (logCast())
            TC_LOG_DEBUG("playerbots", "[CAST] %s spell=%u has=%u why=checkcast result=%u ready=%d gcd=%d", bot->GetName().c_str(),
                         spellid, uint32(bot->HasSpell(spellid)), uint32(result),
                         result == SPELL_FAILED_NOT_READY ? int32(bot->GetSpellHistory()->IsReady(spellInfo, 0, false)) : -1,
                         result == SPELL_FAILED_NOT_READY ? int32(bot->GetGlobalCooldownMgr().HasGlobalCooldown(spellInfo)) : -1);
        return false;
    }
}

bool PlayerbotAI::CanCastSpell(uint32 spellid, GameObject* goTarget, uint8 effectMask, bool checkHasSpell)
{
    if (!spellid)
        return false;

    if (bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
        return false;

    Pet* pet = bot->GetPet();
    if (pet && pet->HasSpell(spellid))
        return true;

    if (checkHasSpell && !bot->HasSpell(spellid))
        return false;

    if (bot->HasSpellCooldown(spellid))
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return false;

    int32 CastingTime = !spellInfo->IsChanneled() ? spellInfo->CalcCastTime(bot->GetLevel()) : spellInfo->GetDuration();
    if (CastingTime > 0 && bot->isMoving())
        return false;

    bool damage = false;
    for (int32 i = EFFECT_0; i <= EFFECT_2; i++)
    {
        if (spellInfo->Effects[i].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
        {
            damage = true;
            break;
        }
    }

    if (sServerFacade->GetDistance2d(bot, goTarget) > sPlayerbotAIConfig->sightDistance)
        return false;

    // ObjectGuid oldSel = bot->GetTarget();
    // bot->SetTarget(goTarget->GetGUID());
    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);

    spell->m_targets.SetGOTarget(goTarget);
    //spell->m_CastItem = aiObjectContext->GetValue<Item*>("item for spell", spellid)->Get();
    //spell->m_targets.SetItemTarget(spell->m_CastItem);

    SpellCastResult result = spell->CheckCast(true);
    delete spell;
    // if (oldSel)
    //     bot->SetTarget(oldSel);

    switch (result)
    {
    case SPELL_FAILED_NOT_INFRONT:
    case SPELL_FAILED_NOT_STANDING:
    case SPELL_FAILED_UNIT_NOT_INFRONT:
    case SPELL_FAILED_MOVING:
    case SPELL_FAILED_TRY_AGAIN:
    case SPELL_CAST_OK:
        return true;
    default:
        break;
    }

    return false;
}

bool PlayerbotAI::CanCastSpell(uint32 spellid, float x, float y, float z, uint8 effectMask, bool checkHasSpell,
    Item* itemTarget)
{
    if (!spellid)
        return false;

    Pet* pet = bot->GetPet();
    if (pet && pet->HasSpell(spellid))
        return true;

    if (checkHasSpell && !bot->HasSpell(spellid))
        return false;

    if (bot->HasSpellCooldown(spellid))
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return false;

    if (!itemTarget)
    {
        if (bot->GetDistance(x, y, z) > sPlayerbotAIConfig->sightDistance)
            return false;
    }

    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);

    spell->m_targets.SetDst(x, y, z, 0.f);
    //spell->m_CastItem = itemTarget ? itemTarget : aiObjectContext->GetValue<Item*>("item for spell", spellid)->Get();
    //spell->m_targets.SetItemTarget(spell->m_CastItem);

    SpellCastResult result = spell->CheckCast(true);
    delete spell;

    switch (result)
    {
    case SPELL_FAILED_NOT_INFRONT:
    case SPELL_FAILED_NOT_STANDING:
    case SPELL_FAILED_UNIT_NOT_INFRONT:
    case SPELL_FAILED_MOVING:
    case SPELL_FAILED_TRY_AGAIN:
    case SPELL_CAST_OK:
        return true;
    default:
        return false;
    }
}

bool PlayerbotAI::CastSpell(std::string const name, Unit* target, Item* itemTarget)
{
    TC_LOG_DEBUG("playerbots", "%s cast: %s", bot->GetName().c_str(), name.c_str());
    bool result = CastSpell(_aiObjectContext->GetValue<uint32>("spell id", name)->Get(), target, itemTarget);
    //const std::string res = result ? "success" : "failed";
    //TC_LOG_DEBUG("playerbots", "%s cast: %s => %s", bot->GetName().c_str(), name.c_str(), res.c_str());
    if (result)
    {
        _aiObjectContext->GetValue<time_t>("last spell cast time", name)->Set(time(nullptr));
    }

    return result;
}

bool PlayerbotAI::CastSpell(uint32 spellId, Unit* target, Item* itemTarget)
{
    if (!spellId)
    {
        return false;
    }

    if (!target)
        target = bot;

    Pet* pet = bot->GetPet();
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (pet && pet->HasSpell(spellId))
    {
        bool autocast = false;
        for (unsigned int& m_autospell : pet->m_autospells)
        {
            if (m_autospell == spellId)
            {
                autocast = true;
                break;
            }
        }

        pet->ToggleAutocast(spellInfo, !autocast);
        //std::ostringstream out;
        //out << (autocast ? "|cffff0000|Disabling" : "|cFF00ff00|Enabling") << " pet auto-cast for ";
        //out << chatHelper.FormatSpell(spellInfo);
        //TellMaster(out);
        return true;
    }

    // #597: cast the override the client would cast (SpellHandler.cpp:1091-1117); spellId follows so "last spell
    // cast" and the scenario/probe ids name the real spell.
    if (spellInfo)
    {
        spellInfo = BotOverrideSpell(bot, spellInfo);
        spellId = spellInfo->Id;
    }

    if (bot->IsFlying() || bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
    {
        return false;
    }

    bool failWithDelay = false;
    if (!bot->IsStandState())
    {
        bot->SetStandState(UNIT_STAND_STATE_STAND);
        failWithDelay = true;
    }

    ObjectGuid oldSel = bot->GetSelectedUnit() ? bot->GetSelectedUnit()->GetGUID() : ObjectGuid();
    bot->SetSelection(target->GetGUID());
    WorldObject* faceTo = target;
    if (!bot->HasInArc(CAST_ANGLE_IN_FRONT, faceTo) && (spellInfo->FacingCasterFlags & SPELL_FACING_FLAG_INFRONT))
    {
        sServerFacade->SetFacingTo(bot, faceTo);
        // failWithDelay = true;
    }

    if (failWithDelay)
    {
        SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        return false;
    }

    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);
    spell->m_cast_count += 1;
    SpellCastTargets targets;
    // #193 Fix 2a (AC parity, Bot/PlayerbotAI.cpp:3725-3741): a real item target (e.g. Pick Lock cast
    // on a trade-slot item) must reach the OPEN_LOCK branch below instead of being swallowed here -
    // see UnlockTradedItemAction.cpp and s8-pet-control-and-class-spells.md Part C.1.
    if (spellInfo->Effects[0].Effect != SPELL_EFFECT_OPEN_LOCK &&
        (spellInfo->Targets & TARGET_FLAG_ITEM || spellInfo->Targets & TARGET_FLAG_GAMEOBJECT_ITEM))
    {
        Item* item = itemTarget ? itemTarget : _aiObjectContext->GetValue<Item*>("item for spell", spellId)->Get();
        targets.SetItemTarget(item);

        if (bot->GetTradeData())
        {
            bot->GetTradeData()->SetSpell(spellId);
            delete spell;
            return true;
        }
    }
    else if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
    {
        // WorldLocation aoe = aiObjectContext->GetValue<WorldLocation>("aoe position")->Get();
        // targets.SetDst(aoe);
        targets.SetDst(*target);
    }
    else if (spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
    {
        targets.SetDst(*bot);
    }
    else
    {
        targets.SetUnitTarget(target);
    }

    if (spellInfo->Effects[0].Effect == SPELL_EFFECT_OPEN_LOCK || spellInfo->Effects[0].Effect == SPELL_EFFECT_SKINNING)
    {
        // #151 (AC Bot/PlayerbotAI.cpp:3751-3787): re-enabled now that "loot target" is a real
        // value. AC's aiObjectContext member is this fork's _aiObjectContext. The chest opens
        // through Spell::EffectOpenLock -> Player::SendLoot once the spell targets the GO
        // (core:Spells/SpellEffects.cpp:2244-2334); GameObject::Use has no CHEST case on this core,
        // so the CMSG_GAMEOBJ_USE call below is largely a no-op for chests, but SetGOTarget is what
        // matters (loot-stack-port.md section 2 step 5).
        LootObject loot = _aiObjectContext->GetValue<LootObject>("loot target")->Get();
        GameObject* go = GetGameObject(loot.guid);
        if (go && go->isSpawned())
        {
            // Bit-packed CMSG_GAMEOBJ_USE (core:Handlers/SpellHandler.cpp:596-618): guid mask
            // 6,1,3,4,0,5,7,2, then bytes 0,1,6,2,3,4,5,7.
            WorldPacket packetgouse(CMSG_GAMEOBJ_USE, 8);
            packetgouse.WriteBit(loot.guid[6]);
            packetgouse.WriteBit(loot.guid[1]);
            packetgouse.WriteBit(loot.guid[3]);
            packetgouse.WriteBit(loot.guid[4]);
            packetgouse.WriteBit(loot.guid[0]);
            packetgouse.WriteBit(loot.guid[5]);
            packetgouse.WriteBit(loot.guid[7]);
            packetgouse.WriteBit(loot.guid[2]);
            packetgouse.FlushBits();
            packetgouse.WriteByteSeq(loot.guid[0]);
            packetgouse.WriteByteSeq(loot.guid[1]);
            packetgouse.WriteByteSeq(loot.guid[6]);
            packetgouse.WriteByteSeq(loot.guid[2]);
            packetgouse.WriteByteSeq(loot.guid[3]);
            packetgouse.WriteByteSeq(loot.guid[4]);
            packetgouse.WriteByteSeq(loot.guid[5]);
            packetgouse.WriteByteSeq(loot.guid[7]);
            bot->GetSession()->HandleGameObjectUseOpcode(packetgouse);
            targets.SetGOTarget(go);
            faceTo = go;
        }
        // #193 Fix 2b (AC parity, Bot/PlayerbotAI.cpp:3769-3782): a trade-slot Pick Lock has no loot
        // target GameObject - the item itself is the lock target. The core fully supports this:
        // SpellCastTargets::Update (Spell.cpp:500-503), CheckCast (Spell.cpp:6789-6800, 7108-7126) and
        // "remember the spell in the trade data until the trade completes" (Spell.cpp:3755-3775).
        else if (itemTarget)
        {
            if (Player* trader = bot->GetTrader())
            {
                targets.SetTradeItemTarget(bot);
                targets.SetUnitTarget(bot);
                faceTo = trader;
            }
            else
                targets.SetItemTarget(itemTarget);
        }
        else
        {
            if (Unit* creature = GetUnit(loot.guid))
            {
                targets.SetUnitTarget(creature);
                faceTo = creature;
            }
        }
    }

    if (bot->isMoving() && spell->GetCastTime())
    {
        // bot->StopMoving();
        SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        spell->cancel();
        delete spell;
        return false;
    }

    if (bot->IsNonMeleeSpellCasted(false, true, true, spellId == 75))
    {
        TC_LOG_DEBUG("playerbots", "Spell cast failed. - target name: %s, spellid: %u, bot name: %s, result: %u", target->GetName().c_str(), spellId, bot->GetName().c_str(), SPELL_FAILED_SPELL_IN_PROGRESS);
        delete spell;
        return false;
    }

    // #870: the core refuses a form spell from another form (Bear Form 5487 has SPELL_ATTR0_NOT_SHAPESHIFT and Cat
    // Form acts as shifted -> SpellInfo::CheckShapeshift = SPELL_FAILED_NOT_SHAPESHIFT, same in AC and MoP); a
    // real client leaves the old form first (autoUnshift). AC bots get that from the "dire bear form" node's
    // "caster form" prerequisite (AC BearDruidStrategy.cpp:35-42), which 5.4.8 never reaches (no Dire Bear Form).
    if (spellInfo->HasAura(SPELL_AURA_MOD_SHAPESHIFT) && bot->GetShapeshiftForm() != FORM_NONE && !bot->HasAura(spellId) &&
        spellInfo->CheckShapeshift(bot->GetShapeshiftForm()) == SPELL_FAILED_NOT_SHAPESHIFT &&
        !bot->HasSpellCooldown(spellId) && !bot->GetGlobalCooldownMgr().HasGlobalCooldown(spellInfo))
        bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

    spell->prepare(&targets);
    SpellCastResult result = spell->GetCastResult();
    BotScenario::OnCast(bot, spellId, int(result));
    if (result != SPELL_CAST_OK)
    {
        // if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && HasRealPlayerMaster())) {
        TC_LOG_DEBUG("playerbots", "Spell cast failed. - target name: %s, spellid: %u, bot name: %s, result: %u", target->GetName().c_str(), spellId, bot->GetName().c_str(), result);
        // }
        return false;
    }

    _aiObjectContext->GetValue<LastSpellCast&>("last spell cast")->Get().Set(spellId, target->GetGUID(), time(nullptr));
    _aiObjectContext->GetValue<PositionMap&>("position")->Get()["random"].Reset();

    if (oldSel)
        bot->SetSelection(oldSel);

    /*if (HasStrategy("debug spell", BOT_STATE_NON_COMBAT))
    {
        std::ostringstream out;
        out << "Casting " << ChatHelper::FormatSpell(spellInfo);
        TellMasterNoFacing(out);
    }*/

    forceRebuff.NoteCast(spellInfo);  // AC PlayerbotAI.cpp:4044 (#628)

    return true;
}

bool PlayerbotAI::CastSpell(uint32 spellId, float x, float y, float z, Item* itemTarget)
{
    if (!spellId)
        return false;

    Pet* pet = bot->GetPet();
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (pet && pet->HasSpell(spellId))
    {
        bool autocast = false;
        for (unsigned int& m_autospell : pet->m_autospells)
        {
            if (m_autospell == spellId)
            {
                autocast = true;
                break;
            }
        }

        pet->ToggleAutocast(spellInfo, !autocast);
        return true;
    }

    // #597: same override swap as the unit-target CastSpell above.
    if (spellInfo)
    {
        spellInfo = BotOverrideSpell(bot, spellInfo);
        spellId = spellInfo->Id;
    }

    MotionMaster& mm = *bot->GetMotionMaster();

    if (bot->IsFlying() || bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;

    bool failWithDelay = false;
    if (!bot->IsStandState())
    {
        bot->SetStandState(UNIT_STAND_STATE_STAND);
        failWithDelay = true;
    }

    ObjectGuid oldSel = bot->GetSelectedUnit() ? bot->GetSelectedUnit()->GetGUID() : ObjectGuid();

    if (!bot->isMoving())
        bot->SetFacingTo(bot->GetAngle(x, y));

    if (failWithDelay)
    {
        SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
        return false;
    }

    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);

    SpellCastTargets targets;
    if (spellInfo->Targets & TARGET_FLAG_ITEM)
    {
        /*spell->m_CastItem =
            itemTarget ? itemTarget : aiObjectContext->GetValue<Item*>("item for spell", spellId)->Get();
        targets.SetItemTarget(spell->m_CastItem);

        if (bot->GetTradeData())
        {
            bot->GetTradeData()->SetSpell(spellId);
            delete spell;
            return true;
        }*/
    }
    else if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
    {
        //WorldLocation aoe = aiObjectContext->GetValue<WorldLocation>("aoe position")->Get();
        targets.SetDst(x, y, z, 0.f);
    }
    else if (spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
    {
        targets.SetDst(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), 0.f);
    }
    else
    {
        return false;
    }

    if (spellInfo->Effects[0].Effect == SPELL_EFFECT_OPEN_LOCK || spellInfo->Effects[0].Effect == SPELL_EFFECT_SKINNING)
    {
        return false;
    }

    spell->prepare(&targets);

    if (bot->isMoving() && spell->GetCastTime())
    {
        // bot->StopMoving();
        SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        spell->cancel();
        delete spell;
        return false;
    }

    if (spellInfo->Effects[0].Effect == SPELL_EFFECT_OPEN_LOCK || spellInfo->Effects[0].Effect == SPELL_EFFECT_SKINNING)
    {
        // #151 (AC Bot/PlayerbotAI.cpp:4010-4019): re-enabled now that "loot target" is a real
        // value - cancel a mining/herbalism-node cast whose target despawned or was taken by
        // another bot mid-approach.
        LootObject loot = _aiObjectContext->GetValue<LootObject>("loot target")->Get();
        if (!loot.IsLootPossible(bot))
        {
            spell->cancel();
            delete spell;
            return false;
        }
    }

    _aiObjectContext->GetValue<LastSpellCast&>("last spell cast")->Get().Set(spellId, bot->GetGUID(), time(nullptr));
    _aiObjectContext->GetValue<PositionMap&>("position")->Get()["random"].Reset();

    if (oldSel)
        bot->SetSelection(oldSel);

    /*if (HasStrategy("debug spell", BOT_STATE_NON_COMBAT))
    {
        std::ostringstream out;
        out << "Casting " << ChatHelper::FormatSpell(spellInfo);
        TellMasterNoFacing(out);
    }*/

    return true;
}

// #393, AC PR mod-playerbots#2856 (Bot/PlayerbotAI.cpp IsSiegeShot): implemented for IoC (extend for SotA if
// needed): a building-damaging missile at the siege position, or Ram at the gate.
static bool IsSiegeShot(Player* bot, AiObjectContext* context, uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    PositionInfo siege = context->GetValue<PositionMap&>("position")->Get()["bg siege"];
    if (!bot->GetVehicle() || !info || !siege.isSet())
        return false;

    if (info->Targets & TARGET_FLAG_DEST_LOCATION)
    {
        // a gate shot launches a missile that damages buildings (Boulder, Glaive, Cannon; not Napalm or rockets)
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (info->Effects[i].Effect == SPELL_EFFECT_TRIGGER_MISSILE)
                if (SpellInfo const* missile = sSpellMgr->GetSpellInfo(info->Effects[i].TriggerSpell))
                    if (missile->HasEffect(SPELL_EFFECT_GAMEOBJECT_DAMAGE))
                        return true;
        return false;
    }

    Unit* base = bot->GetVehicleBase();
    return !info->Targets && info->HasEffect(SPELL_EFFECT_GAMEOBJECT_DAMAGE) && base &&
           base->GetExactDist2d(siege.x, siege.y) < 15.0f;
}

// #736: the lobbed siege shots have spell range "Anywhere" (SpellRange 13) and the core trusts the destination of a
// DEST_TRAJ cast, so nothing but the bot limited where the shot lands - AC's 120 yd 2D cap ignores that the gate
// may stand far above the vehicle, and AC's speed 30 / pow((d+10)/120, 2) elevation drew a missile that does not
// fly to the gate. Launch speed and gravity from the 5.4.8 SpellMissile.dbc rows these spells use; the vehicles'
// aim pitch reaches 60 deg (Vehicle.dbc 158/509/447), above the 45 deg of the longest throw.
struct SiegeMissile
{
    uint32 spellId;
    float speed;
    float gravity;
};

static SiegeMissile const SIEGE_MISSILES[] = {
    {52338, 60.0f, 30.0f},   // Hurl Boulder, SotA Battleground Demolisher (SpellMissile 642)
    {67440, 60.0f, 30.0f},   // Hurl Boulder, IoC Demolisher (SpellMissile 2022)
    {66456, 65.0f, 19.29f},  // Glaive Throw, IoC Glaive Thrower (SpellMissile 1863)
    {67034, 65.0f, 19.29f},  // Glaive Throw, IoC Glaive Thrower (SpellMissile 1962)
};

// Low-arc pitch that lands the shot dist2d yards away and dz yards above the launcher; false when that point is out
// of the missile's reach (or the spell is not a lobbed siege shot).
static bool SiegeShotArc(uint32 spellId, float dist2d, float dz, float& speed, float& pitch)
{
    for (SiegeMissile const& missile : SIEGE_MISSILES)
    {
        if (missile.spellId != spellId)
            continue;

        float const v2 = missile.speed * missile.speed;
        float const disc = v2 * v2 - missile.gravity * (missile.gravity * dist2d * dist2d + 2.0f * dz * v2);
        if (dist2d < 1.0f || disc < 0.0f)
            return false;

        speed = missile.speed;
        pitch = std::atan((v2 - std::sqrt(disc)) / (missile.gravity * dist2d));
        return true;
    }

    return false;
}

static bool IsLobbedSiegeShot(uint32 spellId)
{
    for (SiegeMissile const& missile : SIEGE_MISSILES)
        if (missile.spellId == spellId)
            return true;

    return false;
}

// #581, AC Bot/PlayerbotAI.cpp:4176-4259. Adapters: AC IsValidUnit() -> IsInWorld() (MoP has no
// IsDuringRemoveFromWorld); Unit::HasSpellCooldown -> GetSpellHistory()->HasCooldown(spellInfo) (checked after
// the SpellInfo lookup); CalcCastTime(caster) -> CalcCastTime(level); WorldLocation dest -> SetDst(x,y,z,o,map).
bool PlayerbotAI::CanCastVehicleSpell(uint32 spellId, Unit* target)
{
    if (!spellId)
        return false;

    // AC PR mod-playerbots#2856: a siege shot goes at the gate (the siege position), not at the current target
    if (IsSiegeShot(bot, _aiObjectContext, spellId))
        target = nullptr;
    else if (!target || !target->IsInWorld())
        return false;

    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicle)
        return false;

    // do not allow if no spells
    VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
    if (!seat || !(seat->m_flags & VEHICLE_SEAT_FLAG_CAN_CAST))
        return false;

    Unit* vehicleBase = vehicle->GetBase();
    Unit* spellTarget = target;

    if (!spellTarget)
        spellTarget = vehicleBase;

    if (!spellTarget || !spellTarget->IsInWorld())
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    if (vehicleBase->GetSpellHistory() && !vehicleBase->GetSpellHistory()->IsReady(spellInfo, 0, false)) // HasCooldown is private on this core
        return false;

    // check BG siege position set in BG Tactics
    PositionInfo siegePos = GetAiObjectContext()->GetValue<PositionMap&>("position")->Get()["bg siege"];

    // do not cast spell on self if spell is location based
    if (!(siegePos.isSet() || spellTarget != vehicleBase) && spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
        return false;

    uint32 CastingTime = !spellInfo->IsChanneled() ? spellInfo->CalcCastTime(vehicleBase->GetLevel()) : spellInfo->GetDuration();
    if (CastingTime && vehicleBase->isMoving())
        return false;

    if (vehicleBase != spellTarget && sServerFacade->GetDistance2d(vehicleBase, spellTarget) > 120.0f)
        return false;

    if (!target && siegePos.isSet())
    {
        if (sServerFacade->GetDistance2d(vehicleBase, siegePos.x, siegePos.y) > 120.0f)
            return false;

        // #736: and only where the missile's arc can land (a gate on a hill is out of reach well inside 120 yd)
        float speed, pitch;
        if (IsLobbedSiegeShot(spellId) &&
            !SiegeShotArc(spellId, vehicleBase->GetExactDist2d(siegePos.x, siegePos.y),
                          siegePos.z - vehicleBase->GetPositionZ(), speed, pitch))
            return false;
    }

    Spell* spell = new Spell(vehicleBase, spellInfo, TRIGGERED_NONE);

    if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
    {
        if (siegePos.isSet())
            spell->m_targets.SetDst(siegePos.x, siegePos.y, siegePos.z, 0.0f, bot->GetMapId());
        else if (spellTarget != vehicleBase)
            spell->m_targets.SetDst(*spellTarget);
    }
    else if (spellTarget != vehicleBase)
        spell->m_targets.SetUnitTarget(spellTarget);

    SpellCastResult result = spell->CheckCast(true);
    delete spell;

    switch (result)
    {
        case SPELL_FAILED_NOT_INFRONT:
        case SPELL_FAILED_NOT_STANDING:
        case SPELL_FAILED_UNIT_NOT_INFRONT:
        case SPELL_FAILED_MOVING:
        case SPELL_FAILED_TRY_AGAIN:
        case SPELL_CAST_OK:
            return true;
        default:
            return false;
    }
}

// #581, AC Bot/PlayerbotAI.cpp:4261-4350 (same adapters as above; AC's commented-out lines and the
// "debug spell" tell are dropped - they have no effect).
bool PlayerbotAI::CastVehicleSpell(uint32 spellId, Unit* target)
{
    if (!spellId)
        return false;

    // AC PR mod-playerbots#2856: a siege shot goes at the gate (the siege position), not at the current target
    if (IsSiegeShot(bot, _aiObjectContext, spellId))
        target = nullptr;
    else if (!target || !target->IsInWorld())
        return false;

    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicle)
        return false;

    // do not allow if no spells
    VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
    if (!seat || !(seat->m_flags & VEHICLE_SEAT_FLAG_CAN_CAST))
        return false;

    Unit* vehicleBase = vehicle->GetBase();
    Unit* spellTarget = target;

    if (!spellTarget)
        spellTarget = vehicleBase;

    if (!spellTarget || !spellTarget->IsInWorld())
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    // check BG siege position set in BG Tactics
    PositionInfo siegePos = GetAiObjectContext()->GetValue<PositionMap&>("position")->Get()["bg siege"];
    if (!target && siegePos.isSet())
    {
        if (sServerFacade->GetDistance2d(vehicleBase, siegePos.x, siegePos.y) > 120.0f)
            return false;
    }

    // do not cast spell on self if spell is location based
    if (!(siegePos.isSet() || spellTarget != vehicleBase) && (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION) != 0)
        return false;

    // turn vehicle if target is not in front
    bool failWithDelay = false;
    if (spellTarget != vehicleBase && (seat->CanControl() || (seat->m_flags & VEHICLE_SEAT_FLAG_ALLOW_TURNING)))
    {
        if (!BotHasInArcAC(vehicleBase, CAST_ANGLE_IN_FRONT, spellTarget, 100.0f))  // #515: AC 3rd arg = target radius (AC PlayerbotAI.cpp:4350)
        {
            vehicleBase->SetFacingToObject(spellTarget);
            failWithDelay = true;
        }
    }

    if (siegePos.isSet() && (seat->CanControl() || (seat->m_flags & VEHICLE_SEAT_FLAG_ALLOW_TURNING)))
        vehicleBase->SetFacingTo(vehicleBase->GetAngle(siegePos.x, siegePos.y));

    if (failWithDelay)
    {
        SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
        return false;
    }

    Spell* spell = new Spell(vehicleBase, spellInfo, TRIGGERED_NONE);

    SpellCastTargets targets;
    if ((spellTarget != vehicleBase || siegePos.isSet()) && (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION))
    {
        float dx, dy, dz;
        if (spellTarget != vehicleBase)
        {
            dx = spellTarget->GetPositionX(); dy = spellTarget->GetPositionY(); dz = spellTarget->GetPositionZ();
        }
        else
        {
            // AC PR mod-playerbots#2856: +-2 yd (the Glaive's building damage reaches only 5 yd from the impact)
            dx = siegePos.x + frand(-2.0f, 2.0f); dy = siegePos.y + frand(-2.0f, 2.0f); dz = siegePos.z;
        }

        targets.SetDst(dx, dy, dz, 0.0f, bot->GetMapId());
        // #736: a lobbed siege shot flies its real arc (DBC speed, solved pitch), so the missile lands on the gate it
        // damages; out of reach it is not fired at all
        float speed, pitch;
        if (IsLobbedSiegeShot(spellId))
        {
            if (!SiegeShotArc(spellId, vehicleBase->GetExactDist2d(dx, dy), dz - vehicleBase->GetPositionZ(), speed,
                              pitch))
            {
                delete spell;
                return false;
            }

            targets.SetSpeed(speed);
            targets.SetElevation(pitch);
        }
        else
        {
            targets.SetSpeed(30.0f);
            float dist = vehicleBase->GetExactDist(dx, dy, dz);
            // very much an approximation of the real projectile arc (AC comment)
            float elev = dist >= 110.0f ? 1.0f : pow(((dist + 10.0f) / 120.0f), 2.0f);
            targets.SetElevation(elev);
        }
    }

    if (spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
        targets.SetSrc(vehicleBase->GetPositionX(), vehicleBase->GetPositionY(), vehicleBase->GetPositionZ());

    if (target && !(spellInfo->Targets & TARGET_FLAG_DEST_LOCATION))
        targets.SetUnitTarget(spellTarget);

    spell->prepare(&targets);

    if (seat->CanControl() && vehicleBase->isMoving() && spell->GetCastTime())
    {
        vehicleBase->StopMoving();
        SetNextCheckDelay(sPlayerbotAIConfig->globalCoolDown);
        spell->cancel();
        return false;
    }

    return true;
}

float PlayerbotAI::GetItemScoreMultiplier(ItemQualities quality)
{
    switch (quality)
    {
        // each quality increase 1.1x
        case ITEM_QUALITY_POOR:
            return 1.0f;
            break;
        case ITEM_QUALITY_NORMAL:
            return 1.1f;
            break;
        case ITEM_QUALITY_UNCOMMON:
            return 1.21f;
            break;
        case ITEM_QUALITY_RARE:
            return 1.331f;
            break;
        case ITEM_QUALITY_EPIC:
            return 1.4641f;
            break;
        case ITEM_QUALITY_LEGENDARY:
            return 1.61051f;
            break;
        default:
            break;
    }
    return 1.0f;
}

void PlayerbotAI::_fillGearScoreData(Player* player, Item* item, std::vector<uint32>* gearScore, uint32& twoHandScore,
    bool mixed)
{
    if (!item)
        return;

    ItemTemplate const* proto = item->GetTemplate();
    if (player->CanUseItem(proto) != EQUIP_ERR_OK)
        return;

    uint8 type = proto->InventoryType;
    uint32 level = mixed ? proto->ItemLevel * PlayerbotAI::GetItemScoreMultiplier(ItemQualities(proto->Quality))
        : proto->ItemLevel;

    switch (type)
    {
    case INVTYPE_2HWEAPON:
        twoHandScore = std::max(twoHandScore, level);
        break;
    case INVTYPE_WEAPON:
    case INVTYPE_WEAPONMAINHAND:
        (*gearScore)[SLOT_MAIN_HAND] = std::max((*gearScore)[SLOT_MAIN_HAND], level);
        break;
    case INVTYPE_SHIELD:
    case INVTYPE_WEAPONOFFHAND:
        (*gearScore)[EQUIPMENT_SLOT_OFFHAND] = std::max((*gearScore)[EQUIPMENT_SLOT_OFFHAND], level);
        break;
    case INVTYPE_THROWN:
    case INVTYPE_RANGEDRIGHT:
    case INVTYPE_RANGED:
    case INVTYPE_QUIVER:
    case INVTYPE_RELIC:
        (*gearScore)[EQUIPMENT_SLOT_RANGED] = std::max((*gearScore)[EQUIPMENT_SLOT_RANGED], level);
        break;
    case INVTYPE_HEAD:
        (*gearScore)[EQUIPMENT_SLOT_HEAD] = std::max((*gearScore)[EQUIPMENT_SLOT_HEAD], level);
        break;
    case INVTYPE_NECK:
        (*gearScore)[EQUIPMENT_SLOT_NECK] = std::max((*gearScore)[EQUIPMENT_SLOT_NECK], level);
        break;
    case INVTYPE_SHOULDERS:
        (*gearScore)[EQUIPMENT_SLOT_SHOULDERS] = std::max((*gearScore)[EQUIPMENT_SLOT_SHOULDERS], level);
        break;
    case INVTYPE_BODY:
        (*gearScore)[EQUIPMENT_SLOT_BODY] = std::max((*gearScore)[EQUIPMENT_SLOT_BODY], level);
        break;
    case INVTYPE_CHEST:
        (*gearScore)[EQUIPMENT_SLOT_CHEST] = std::max((*gearScore)[EQUIPMENT_SLOT_CHEST], level);
        break;
    case INVTYPE_WAIST:
        (*gearScore)[EQUIPMENT_SLOT_WAIST] = std::max((*gearScore)[EQUIPMENT_SLOT_WAIST], level);
        break;
    case INVTYPE_LEGS:
        (*gearScore)[EQUIPMENT_SLOT_LEGS] = std::max((*gearScore)[EQUIPMENT_SLOT_LEGS], level);
        break;
    case INVTYPE_FEET:
        (*gearScore)[EQUIPMENT_SLOT_FEET] = std::max((*gearScore)[EQUIPMENT_SLOT_FEET], level);
        break;
    case INVTYPE_WRISTS:
        (*gearScore)[EQUIPMENT_SLOT_WRISTS] = std::max((*gearScore)[EQUIPMENT_SLOT_WRISTS], level);
        break;
    case INVTYPE_HANDS:
        (*gearScore)[EQUIPMENT_SLOT_HEAD] = std::max((*gearScore)[EQUIPMENT_SLOT_HEAD], level);
        break;
        // equipped gear score check uses both rings and trinkets for calculation, assume that for bags/banks it is the
        // same with keeping second highest score at second slot
    case INVTYPE_FINGER:
    {
        if ((*gearScore)[EQUIPMENT_SLOT_FINGER1] < level)
        {
            (*gearScore)[EQUIPMENT_SLOT_FINGER2] = (*gearScore)[EQUIPMENT_SLOT_FINGER1];
            (*gearScore)[EQUIPMENT_SLOT_FINGER1] = level;
        }
        else if ((*gearScore)[EQUIPMENT_SLOT_FINGER2] < level)
            (*gearScore)[EQUIPMENT_SLOT_FINGER2] = level;
        break;
    }
    case INVTYPE_TRINKET:
    {
        if ((*gearScore)[EQUIPMENT_SLOT_TRINKET1] < level)
        {
            (*gearScore)[EQUIPMENT_SLOT_TRINKET2] = (*gearScore)[EQUIPMENT_SLOT_TRINKET1];
            (*gearScore)[EQUIPMENT_SLOT_TRINKET1] = level;
        }
        else if ((*gearScore)[EQUIPMENT_SLOT_TRINKET2] < level)
            (*gearScore)[EQUIPMENT_SLOT_TRINKET2] = level;
        break;
    }
    case INVTYPE_CLOAK:
        (*gearScore)[EQUIPMENT_SLOT_BACK] = std::max((*gearScore)[EQUIPMENT_SLOT_BACK], level);
        break;
    default:
        break;
    }
}

bool PlayerbotAI::IsSafe(Player* player)
{
    return player && player->GetMapId() == bot->GetMapId() && player->GetInstanceId() == bot->GetInstanceId() &&
        !player->IsBeingTeleported();
}
bool PlayerbotAI::IsSafe(WorldObject* obj)
{
    return obj && obj->GetMapId() == bot->GetMapId() && obj->GetInstanceId() == bot->GetInstanceId() &&
        (!obj->IsPlayer() || !((Player*)obj)->IsBeingTeleported());
}

uint32 PlayerbotAI::GetMixedGearScore(Player* player, bool withBags, bool withBank, uint32 topN)
{
    std::vector<uint32> gearScore(EQUIPMENT_SLOT_END);
    uint32 twoHandScore = 0;

    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            _fillGearScoreData(player, item, &gearScore, twoHandScore, true);
    }

    if (withBags)
    {
        // check inventory
        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                _fillGearScoreData(player, item, &gearScore, twoHandScore, true);
        }

        // check bags
        for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        {
            if (Bag* pBag = (Bag*)player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                {
                    if (Item* item2 = pBag->GetItemByPos(j))
                        _fillGearScoreData(player, item2, &gearScore, twoHandScore, true);
                }
            }
        }
    }

    if (withBank)
    {
        for (uint8 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                _fillGearScoreData(player, item, &gearScore, twoHandScore, true);
        }

        for (uint8 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                if (item->IsBag())
                {
                    Bag* bag = (Bag*)item;
                    for (uint8 j = 0; j < bag->GetBagSize(); ++j)
                    {
                        if (Item* item2 = bag->GetItemByPos(j))
                            _fillGearScoreData(player, item2, &gearScore, twoHandScore, true);
                    }
                }
            }
        }
    }
    if (!topN)
    {
        uint8 count = EQUIPMENT_SLOT_END - 2;  // ignore body and tabard slots
        uint32 sum = 0;

        // check if 2h hand is higher level than main hand + off hand
        if (gearScore[EQUIPMENT_SLOT_MAINHAND] + gearScore[EQUIPMENT_SLOT_OFFHAND] < twoHandScore * 2)
        {
            gearScore[EQUIPMENT_SLOT_OFFHAND] = 0;  // off hand is ignored in calculations if 2h weapon has higher score
            --count;
            gearScore[EQUIPMENT_SLOT_MAINHAND] = twoHandScore;
        }

        for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        {
            sum += gearScore[i];
        }

        if (count)
        {
            uint32 res = uint32(sum / count);
            return res;
        }

        return 0;
    }
    // topN != 0
    if (gearScore[EQUIPMENT_SLOT_MAINHAND] + gearScore[EQUIPMENT_SLOT_OFFHAND] < twoHandScore * 2)
    {
        gearScore[EQUIPMENT_SLOT_OFFHAND] = twoHandScore;
        gearScore[EQUIPMENT_SLOT_MAINHAND] = twoHandScore;
    }
    std::vector<uint32> topGearScore;
    for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        topGearScore.push_back(gearScore[i]);
    }
    std::sort(topGearScore.begin(), topGearScore.end(), [&](const uint32 lhs, const uint32 rhs) { return lhs > rhs; });
    uint32 sum = 0;
    for (int i = 0; i < std::min((uint32)topGearScore.size(), topN); i++)
    {
        sum += topGearScore[i];
    }
    return sum / topN;
}

// A custom CanEquipItem (remove AutoUnequipOffhand in FindEquipSlot to prevent unequip on `item usage` calculation)
InventoryResult PlayerbotAI::CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap, bool not_loading) const
{
    dest = 0;
    if (pItem)
    {
        //TC_LOG_DEBUG("playerbots", "STORAGE: CanEquipItem slot = %u, item = %u, count = %u", slot, pItem->GetEntry(), pItem->GetCount());
        ItemTemplate const* pProto = pItem->GetTemplate();
        if (pProto)
        {
            // item used
            if (pItem->m_lootGenerated)
                return InventoryResult::EQUIP_ERR_LOOT_GONE;

            if (pItem->IsBindedNotWith(bot))
                return InventoryResult::EQUIP_ERR_NOT_OWNER;

            InventoryResult res = bot->CanTakeMoreSimilarItems(pItem);
            if (res != EQUIP_ERR_OK)
                return res;

            ScalingStatDistributionEntry const* ssd =
                pProto->ScalingStatDistribution
                ? sScalingStatDistributionStore.LookupEntry(pProto->ScalingStatDistribution)
                : 0;
            // check allowed level (extend range to upper values if MaxLevel more or equal max player level, this let GM
            // set high level with 1...max range items)
            if (ssd && ssd->MaxLevel < DEFAULT_MAX_LEVEL && ssd->MaxLevel < bot->GetLevel())
                return InventoryResult::EQUIP_ERR_CANT_EQUIP_EVER;

            uint8 eslot = FindEquipSlot(pProto, slot, swap);
            if (eslot == NULL_SLOT)
                return InventoryResult::EQUIP_ERR_CANT_EQUIP_EVER;

            // Xinef: dont allow to equip items on disarmed slot
            if (!bot->CanUseAttackType(bot->GetAttackBySlot(eslot)))
                return EQUIP_ERR_NOT_WHILE_DISARMED;

            res = bot->CanUseItem(pItem, not_loading);
            if (res != EQUIP_ERR_OK)
                return res;

            if (!swap && bot->GetItemByPos(INVENTORY_SLOT_BAG_0, eslot))
                return InventoryResult::EQUIP_ERR_NO_SLOT_AVAILABLE;

            // if we are swapping 2 equiped items, CanEquipUniqueItem check
            // should ignore the item we are trying to swap, and not the
            // destination item. CanEquipUniqueItem should ignore destination
            // item only when we are swapping weapon from bag
            uint8 ignore = uint8(NULL_SLOT);
            switch (eslot)
            {
            case EQUIPMENT_SLOT_MAINHAND:
                ignore = EQUIPMENT_SLOT_OFFHAND;
                break;
            case EQUIPMENT_SLOT_OFFHAND:
                ignore = EQUIPMENT_SLOT_MAINHAND;
                break;
            case EQUIPMENT_SLOT_FINGER1:
                ignore = EQUIPMENT_SLOT_FINGER2;
                break;
            case EQUIPMENT_SLOT_FINGER2:
                ignore = EQUIPMENT_SLOT_FINGER1;
                break;
            case EQUIPMENT_SLOT_TRINKET1:
                ignore = EQUIPMENT_SLOT_TRINKET2;
                break;
            case EQUIPMENT_SLOT_TRINKET2:
                ignore = EQUIPMENT_SLOT_TRINKET1;
                break;
            }

            if (ignore == uint8(NULL_SLOT) || pItem != bot->GetItemByPos(INVENTORY_SLOT_BAG_0, ignore))
                ignore = eslot;

            InventoryResult res2 = bot->CanEquipUniqueItem(pItem, swap ? ignore : uint8(NULL_SLOT));
            if (res2 != EQUIP_ERR_OK)
                return res2;

            // check unique-equipped special item classes
            if (pProto->Class == ITEM_CLASS_QUIVER)
                for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
                    if (Item* pBag = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                        if (pBag != pItem)
                            if (ItemTemplate const* pBagProto = pBag->GetTemplate())
                                if (pBagProto->Class == pProto->Class && (!swap || pBag->GetSlot() != eslot))
                                    return (pBagProto->SubClass == ITEM_SUBCLASS_AMMO_POUCH)
                                    ? InventoryResult::EQUIP_ERR_AMMO_ONLY
                                    : InventoryResult::EQUIP_ERR_ONLY_ONE_QUIVER;

            uint32 type = pProto->InventoryType;

            if (eslot == EQUIPMENT_SLOT_OFFHAND)
            {
                // Do not allow polearm to be equipped in the offhand (rare case for the only 1h polearm 41750)
                // xinef: same for fishing poles
                if (type == INVTYPE_WEAPON && (pProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM ||
                    pProto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE))
                    return InventoryResult::EQUIP_ERR_WRONG_SLOT;

                else if (type == INVTYPE_WEAPON || type == INVTYPE_WEAPONOFFHAND)
                {
                    if (!bot->CanDualWield())
                        return InventoryResult::EQUIP_ERR_2HANDED_EQUIPPED;
                }
                else if (type == INVTYPE_2HWEAPON)
                {
                    if (!bot->CanDualWield() || !bot->CanTitanGrip())
                        return InventoryResult::EQUIP_ERR_2HANDED_EQUIPPED;
                }

                if (bot->IsTwoHandUsed())
                    return InventoryResult::EQUIP_ERR_2HANDED_EQUIPPED;
            }

            // equip two-hand weapon case (with possible unequip 2 items)
            if (type == INVTYPE_2HWEAPON)
            {
                if (eslot == EQUIPMENT_SLOT_OFFHAND)
                {
                    if (!bot->CanTitanGrip())
                        return InventoryResult::EQUIP_ERR_CANT_EQUIP_EVER;
                }
                else if (eslot != EQUIPMENT_SLOT_MAINHAND)
                    return InventoryResult::EQUIP_ERR_CANT_EQUIP_EVER;

                if (!bot->CanTitanGrip())
                {
                    // offhand item must can be stored in inventory for offhand item and it also must be unequipped
                    Item* offItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    ItemPosCountVec off_dest;
                    if (offItem && (!not_loading ||
                        bot->CanUnequipItem(uint16(INVENTORY_SLOT_BAG_0) << 8 | EQUIPMENT_SLOT_OFFHAND,
                            false) != EQUIP_ERR_OK ||
                        bot->CanStoreItem(NULL_BAG, NULL_SLOT, off_dest, offItem, false) != EQUIP_ERR_OK))
                        return swap ? InventoryResult::EQUIP_ERR_CANT_SWAP : InventoryResult::EQUIP_ERR_INV_FULL;
                }
            }
            dest = ((INVENTORY_SLOT_BAG_0 << 8) | eslot);
            return EQUIP_ERR_OK;
        }
    }

    return !swap ? EQUIP_ERR_ITEM_NOT_FOUND : InventoryResult::EQUIP_ERR_CANT_SWAP;
}

uint8 PlayerbotAI::FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const
{
    uint8 slots[4];
    slots[0] = NULL_SLOT;
    slots[1] = NULL_SLOT;
    slots[2] = NULL_SLOT;
    slots[3] = NULL_SLOT;
    switch (proto->InventoryType)
    {
    case INVTYPE_HEAD:
        slots[0] = EQUIPMENT_SLOT_HEAD;
        break;
    case INVTYPE_NECK:
        slots[0] = EQUIPMENT_SLOT_NECK;
        break;
    case INVTYPE_SHOULDERS:
        slots[0] = EQUIPMENT_SLOT_SHOULDERS;
        break;
    case INVTYPE_BODY:
        slots[0] = EQUIPMENT_SLOT_BODY;
        break;
    case INVTYPE_CHEST:
    case INVTYPE_ROBE:
        slots[0] = EQUIPMENT_SLOT_CHEST;
        break;
    case INVTYPE_WAIST:
        slots[0] = EQUIPMENT_SLOT_WAIST;
        break;
    case INVTYPE_LEGS:
        slots[0] = EQUIPMENT_SLOT_LEGS;
        break;
    case INVTYPE_FEET:
        slots[0] = EQUIPMENT_SLOT_FEET;
        break;
    case INVTYPE_WRISTS:
        slots[0] = EQUIPMENT_SLOT_WRISTS;
        break;
    case INVTYPE_HANDS:
        slots[0] = EQUIPMENT_SLOT_HANDS;
        break;
    case INVTYPE_FINGER:
        slots[0] = EQUIPMENT_SLOT_FINGER1;
        slots[1] = EQUIPMENT_SLOT_FINGER2;
        break;
    case INVTYPE_TRINKET:
        slots[0] = EQUIPMENT_SLOT_TRINKET1;
        slots[1] = EQUIPMENT_SLOT_TRINKET2;
        break;
    case INVTYPE_CLOAK:
        slots[0] = EQUIPMENT_SLOT_BACK;
        break;
    case INVTYPE_RANGED:
    case INVTYPE_RANGEDRIGHT: // #621: 5.4.8 guns/crossbows go to the main hand (Player::FindEquipSlot)
    case INVTYPE_WEAPON:
    {
        slots[0] = EQUIPMENT_SLOT_MAINHAND;

        // suggest offhand slot only if know dual wielding
        // (this will be replace mainhand weapon at auto equip instead unwonted "you don't known dual wielding" ...
        if (bot->CanDualWield())
            slots[1] = EQUIPMENT_SLOT_OFFHAND;
        break;
    }
    case INVTYPE_SHIELD:
    case INVTYPE_WEAPONOFFHAND:
    case INVTYPE_HOLDABLE:
        slots[0] = EQUIPMENT_SLOT_OFFHAND;
        break;
    case INVTYPE_2HWEAPON:
        slots[0] = EQUIPMENT_SLOT_MAINHAND;
        if (Item* mhWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
        {
            if (ItemTemplate const* mhWeaponProto = mhWeapon->GetTemplate())
            {
                if (mhWeaponProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM ||
                    mhWeaponProto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF)
                {
                    bot->AutoUnequipOffhandIfNeed(true);
                    break;
                }
            }
        }

        if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
        {
            if (proto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || proto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF)
            {
                break;
            }
        }
        if (bot->CanDualWield() && bot->CanTitanGrip() && proto->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && proto->SubClass != ITEM_SUBCLASS_WEAPON_FISHING_POLE)
            slots[1] = EQUIPMENT_SLOT_OFFHAND;
        break;
    case INVTYPE_TABARD:
        slots[0] = EQUIPMENT_SLOT_TABARD;
        break;
    case INVTYPE_WEAPONMAINHAND:
        slots[0] = EQUIPMENT_SLOT_MAINHAND;
        break;
    case INVTYPE_BAG:
        slots[0] = INVENTORY_SLOT_BAG_START + 0;
        slots[1] = INVENTORY_SLOT_BAG_START + 1;
        slots[2] = INVENTORY_SLOT_BAG_START + 2;
        slots[3] = INVENTORY_SLOT_BAG_START + 3;
        break;
    default:
        return NULL_SLOT;
    }

    if (slot != NULL_SLOT)
    {
        if (swap || !bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            for (uint8 i = 0; i < 4; ++i)
                if (slots[i] == slot)
                    return slot;
    }
    else
    {
        // search free slot at first
        for (uint8 i = 0; i < 4; ++i)
            if (slots[i] != NULL_SLOT && !bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slots[i]))
                // in case 2hand equipped weapon (without titan grip) offhand slot empty but not free
                if (slots[i] != EQUIPMENT_SLOT_OFFHAND || !bot->IsTwoHandUsed())
                    return slots[i];

        // if not found free and can swap return first appropriate from used
        for (uint8 i = 0; i < 4; ++i)
            if (slots[i] != NULL_SLOT && swap)
                return slots[i];
    }

    // no free position
    return NULL_SLOT;
}
bool PlayerbotAI::HasAggro(Unit* unit)
{
    if (!unit)
    {
        return false;
    }
    bool isMT = PlayerBotSpec::IsMainTank(bot);
    Unit* victim = unit->GetVictim();
    if (victim && (victim->GetGUID() == bot->GetGUID() || (!isMT && victim->ToPlayer() && PlayerBotSpec::IsTank(victim->ToPlayer()))))
    {
        return true;
    }
    return false;
}