/*
** Made by Traesh https://github.com/Traesh
** AzerothCore 2019 http://www.azerothcore.org/
** Conan513 https://github.com/conan513
** Made into a module by Micrah https://github.com/milestorme/
*/

#include "Chat.h"
#include "ChannelMgr.h"
#include "Config.h"
#include "cs_playerbots.h"
#include "Group.h"
#include "Log.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSocket.h"

#include "Playerbots.h"
#include "PlayerbotAIConfig.h"
#include "NewRpgDuelSpot.h"  // #201 (#614): sDuelSpotMgr for the duel reset
#include "RandomPlayerbotMgr.h"
#include "RandomItemManager.h"
#include "PlayerbotTextMgr.h"
#include "PlayerbotWorldThreadProcessor.h"
#include "TravelNode.h"

#include "Authentication/AuthCrypt.h"
#include "CharacterHandler.h"

#include <boost/filesystem.hpp>
#include <vector>
#include <future>
#include <unordered_set>
#include <random>
#include <thread>

#ifndef _PLAYERBOT_CONFIG
# define _PLAYERBOT_CONFIG  "playerbots.conf"
#endif

// #72: PlayerbotAI.cpp - the world-diff maxima AC's AutoScaleActivity reads (AC core sWorldUpdateTime).
void PlayerbotRecordWorldUpdateDiff(uint32 diff);

// #148: AC Bot/RandomBotLevelMgr.cpp ported into RandomPlayerbotMgr.cpp (file-local class, these are its hooks).
void RandomBotLevelMgrUpdate(uint32 diff);
void RandomBotLevelMgrOnLevelChanged(Player* player, uint8 oldLevel);
void RandomBotLevelMgrReloadConfig();

class mod_playerbots : public PlayerScript
{
public:
    mod_playerbots() : PlayerScript("mod_playerbots") {}

    void OnLogin(Player* player) override
    {
        // Announce Module
        ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00mod playerbots |rmodule.");
    }

};

class PlayerbotsWorldScript : public WorldScript
{
public:
    PlayerbotsWorldScript() : WorldScript("PlayerbotsWorldScript") {}

    void OnConfigLoad(bool reloaded) override
    {
        if (!reloaded)
        {
            uint32 oldMSTime = getMSTime();
            std::string conf_file = boost::filesystem::absolute(_PLAYERBOT_CONFIG).generic_string();

            TC_LOG_INFO("playerbots", " ");
            TC_LOG_INFO("playerbots", "Loading Playerbots Config at %s ...", conf_file.c_str());

            std::string err;
            if (!sConfigMgr->LoadAdditionalFile(conf_file, true, err))
            {
                TC_LOG_FATAL("playerbots", ">> Load playerbots failed, %s", err.c_str());
                std::this_thread::sleep_for(std::chrono::seconds(5));
                sWorld->StopNow(1);
                return;
            }
            sPlayerbotAIConfig->Initialize();

            TC_LOG_INFO("playerbots", ">> Loaded playerbots config in %u ms", GetMSTimeDiffToNow(oldMSTime));
            TC_LOG_INFO("playerbots", " ");

            sRandomPlayerbotMgr->Reserve(sPlayerbotAIConfig->maxRandomBots);
            sRandomItemMgr->Init();

            TC_LOG_INFO("playerbots", "Playerbots enabled: %s", sPlayerbotAIConfig->enabled ? "Yes" : "No");
            TC_LOG_INFO("playerbots", "Playerbots min/max to load: %u/%u", sPlayerbotAIConfig->minRandomBots, sPlayerbotAIConfig->maxRandomBots);
            TC_LOG_INFO("playerbots", "Playerbots autologin: %s", sPlayerbotAIConfig->randomBotAutologin ? "Yes" : "No");
        }
    }
    void OnStartup() override
    {
        // Runs after World::SetInitialWorldSettings, so DBCs, ObjectMgr and MapMgr are all up.
        //
        // sTravelMgr.Init() is the counterpart of upstream AC's TravelMgr::Init(): it builds the
        // flight-master / travel-hub caches and the taxi graph the "travel flight" RPG action
        // needs. It is DBC + spawn data only, so it costs seconds, and it always runs.
        // #322: AC PlayerbotAIConfig.cpp:789-790 loads the bot text tables (ai_playerbot_texts /
        // ai_playerbot_texts_chance) in Initialize(), before PlayerbotFactory::Init and TravelMgr::Init.
        PlayerbotTextMgr::instance().LoadBotTexts();
        PlayerbotTextMgr::instance().LoadBotTextChance();

        sTravelMgr.Init();

        // #201 (#612): DBCs are loaded now; drop duel spots on maps that do not exist (Initialize() ran before them).
        sPlayerbotAIConfig->ValidateDuelSpotMaps();

        // The walk-node graph (TravelNode) is the other half of upstream's travel layer. Upstream
        // builds it at startup but nothing consumes it at runtime any more - its graph-routing
        // block inside MovementActions::MoveTo is commented out, and only debug commands read it.
        // It is therefore off by default here too; enabling it costs a long one-time generation.
        TC_LOG_INFO("playerbots", "Travel nodes: enableTravelNodes=%d", sPlayerbotAIConfig->enableTravelNodes ? 1 : 0);
        if (sPlayerbotAIConfig->enableTravelNodes)
            sTravelNodeMap.LoadOrGenerate();
    }

    void OnUpdate(uint32 diff) override
    {
        // AC Script/Playerbots.cpp:466-468: run the operations bot AI queued from the map-update threads
        // (world thread, after sMapMgr->Update).
        PlayerbotWorldThreadProcessor::instance().Update(diff);
        // #148: AC RandomBotLevelMgr.cpp:1126-1129 (replaces the fork-only RandomPlayerbotBracketMgr, broken since #808).
        RandomBotLevelMgrUpdate(diff);
    }
};

class PlayerbotsServerScript : public ServerScript
{
public:
    PlayerbotsServerScript() : ServerScript("PlayerbotsServerScript") {}
    void OnPacketReceive(WorldSession* sessionBot, WorldPacket& packet) override
    {
        Player* playerBot = sessionBot->GetPlayer();
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(playerBot))
                playerbotMgr->HandleMasterIncomingPacket(packet);
    }
};

class PlayerbotsScript : public PlayerbotScript
{
public:
    PlayerbotsScript() : PlayerbotScript("PlayerbotsScript") {}

    void OnPlayerbotCheckKillTask(Player* /*player*/, Unit* /*victim*/) override
    {
    }

    void OnPlayerbotCheckPetitionAccount(Player* player, bool& found) override
    {
        if (found && GET_PLAYERBOT_AI(player))
            found = false;
    }

    bool OnPlayerbotCheckUpdatesToSend(Player* player) override
    {
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
            return botAI->IsRealPlayer();

        return true;
    }

    void OnPlayerbotPacketSent(Player* player, WorldPacket const* packet) override
    {
        if (!player)
            return;
        
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
        {
            botAI->HandleBotOutgoingPacket(*packet);
        }
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            playerbotMgr->HandleMasterOutgoingPacket(*packet);
        }
    }

    void OnPlayerbotUpdate(uint32 diff) override
    {
        PlayerbotRecordWorldUpdateDiff(diff);  // AC World::Update -> sWorldUpdateTime.UpdateWithDiff(diff)
        sRandomPlayerbotMgr->UpdateAI(diff);
        sRandomPlayerbotMgr->UpdateSessions();
    }

    void OnPlayerbotUpdateSessions(Player* player) override
    {
        if (player)
            if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
                playerbotMgr->UpdateSessions();
    }

    void OnPlayerbotLogout(Player* player) override
    {
        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || botAI->IsRealPlayer())
            {
                playerbotMgr->LogoutAllBots();
            }
        }

        sRandomPlayerbotMgr->OnPlayerLogout(player);
    }

    void OnPlayerbotLogoutBots() override { sRandomPlayerbotMgr->LogoutAllBots(); }
};
namespace
{
// #201 (#614): azerothcore/mod-duel-reset: full health and
// mana; at the end of every duel (any end type) also every spell cooldown and Forbearance (25771).
void ResetDuelant(Player* player, bool duelEnded)
{
    if (!player)
        return;
    if (duelEnded)
    {
        player->RemoveAllSpellCooldown();
        player->RemoveAurasDueToSpell(25771);
    }
    player->SetFullHealth();
    if (player->GetMaxPower(POWER_MANA) > 0)
        player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));
}

// #868: refresh the activity cache (true now: PlayerbotAI::AllowActive duel rule) and end any pending AI wait, so the
// bot's next update is a full tick. Called from Player::UpdateDuelFlag (map thread, the same thread that updates both
// duellists' AI through OnAfterUpdate).
void WakeDuelant(Player* player)
{
    PlayerbotAI* botAI = player ? GET_PLAYERBOT_AI(player) : nullptr;
    if (!botAI)
        return;
    botAI->AllowActivity(ALL_ACTIVITY, true);
    // a running cast keeps its own wait (WaitForSpellCast), like the #754 battleground wake in PlayerbotAI::UpdateAI
    if (!player->IsNonMeleeSpellCasted(false, false, true))
        botAI->SetNextCheckDelay(0);
}
}  // namespace

class PlayerbotsPlayerScript : public PlayerScript
{
public:
    PlayerbotsPlayerScript() : PlayerScript("PlayerbotsPlayerScript") {}

    // #148: AC RandomBotLevelMgr.cpp:1151-1156 (ResetBotLevel on level-up).
    void OnLevelChanged(Player* player, uint8 oldLevel) override { RandomBotLevelMgrOnLevelChanged(player, oldLevel); }

    void OnLogin(Player* player) override
    {
        if (!player->GetSession()->IsBot())
        {
            sPlayerbotsMgr->AddPlayerbotData(player, false);
            sRandomPlayerbotMgr->OnPlayerLogin(player);

            if (sPlayerbotAIConfig->enabled || sPlayerbotAIConfig->randomBotAutologin)
            {
                std::string roundedTime =
                    std::to_string(std::ceil((sPlayerbotAIConfig->maxRandomBots * 0.11 / 60) * 10) / 10.0);
                roundedTime = roundedTime.substr(0, roundedTime.find('.') + 2);

                ChatHandler(player->GetSession()).SendSysMessage(std::string("Playerbots: bot initialization at server startup takes about '" + roundedTime + "' minutes.").c_str());
            }
        }
    }

    void OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Player* receiver) override
    {
        if (type != CHAT_MSG_WHISPER)
            return;

        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(receiver))
            botAI->HandleCommand(type, msg, player);
    }

    void OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Group* group) override
    {
        if (!group)
            return;

        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* const member = itr->GetSource();
            if (!member)
                continue;

            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(member))
                botAI->HandleCommand(type, msg, player);
        }
    }

    void OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg, Guild* /*guild*/) override
    {
        if (type != CHAT_MSG_GUILD)
            return;

        PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player);
        if (!playerbotMgr)
            return;

        for (PlayerBotMap::const_iterator it = playerbotMgr->GetPlayerBotsBegin(); it != playerbotMgr->GetPlayerBotsEnd(); ++it)
        {
            Player* const bot = it->second;
            if (!bot || bot->GetGuildId() != player->GetGuildId())
                continue;

            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
                botAI->HandleCommand(type, msg, player);
        }
    }

    // #201 (#614): duel reset for EVERY duel inside a duel spot, bots and players alike (owner decision). Gated by
    // DuelSpotEnable && DuelSpotReset. Hooks: ScriptMgr.h:752-755, called from Player::UpdateDuelFlag / DuelComplete.
    void OnDuelStart(Player* player1, Player* player2) override
    {
        // #868 (BEYOND AC): wake both bot sides when the fight starts, for every duel. AllowActive now answers true in a
        // duel, but a bot that cached "inactive" just before (AllowActivity, ~5 s cache) ran a minimal tick and then
        // slept PassiveDelay (10 s): the accepting bot answered the request in such a tick and stood ~7 s into the fight.
        WakeDuelant(player1);
        WakeDuelant(player2);
        if (!sPlayerbotAIConfig->duelSpotEnable || !sPlayerbotAIConfig->duelSpotReset ||
            !sDuelSpotMgr->IsDuelInSpot(player1, player2))
            return;
        ResetDuelant(player1, false);
        ResetDuelant(player2, false);
    }

    void OnDuelEnd(Player* winner, Player* loser, DuelCompleteType type) override
    {
        // #201 (#612, wave-13 t1): the no-rematch window starts here, for every end type (also a decline), whatever
        // DuelSpotReset says. NoteDuelEnd itself checks DuelSpotEnable and IsDuelInSpot.
        sDuelSpotMgr->NoteDuelEnd(winner, loser);
        // #201 (SPEC amendment A3): reset on EVERY end type (WON / FLED / INTERRUPTED / SURRENDER), owner decision.
        if (!sPlayerbotAIConfig->duelSpotEnable || !sPlayerbotAIConfig->duelSpotReset ||
            !sDuelSpotMgr->IsDuelInSpot(winner, loser))
            return;
        ResetDuelant(winner, true);
        ResetDuelant(loser, true);
    }

    void OnAfterUpdate(Player* player, uint32 diff) override
    {
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
        {
            botAI->UpdateAI(diff);
        }

        if (PlayerbotMgr* playerbotMgr = GET_PLAYERBOT_MGR(player))
        {
            playerbotMgr->UpdateAI(diff);
        }
    }
};
void AddSC_mod_playerbots()
{
    new mod_playerbots();

    new PlayerbotsServerScript();
    new PlayerbotsWorldScript();
    new PlayerbotsScript();
    new PlayerbotsPlayerScript();

    AddSC_playerbots_commandscript();
}
