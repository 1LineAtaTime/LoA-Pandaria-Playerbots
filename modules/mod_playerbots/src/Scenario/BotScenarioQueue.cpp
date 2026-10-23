/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #210 S10 P5: harness verbs for the LFG/BG group-queue 1:1 test plan
 * Split out into its own file, same reason as
 * BotScenarioVehicle.cpp (see that file's header): a harness/probe change should cost a few files plus
 * a relink, not a rebuild of BotScenario.cpp or anything it pulls in.
 *
 * Verbs (all via `.botscenario <name> <verb> [args]`, SOAP/console only, matching every other verb
 * in BotScenario.cpp's "S1b" pre-botAI list):
 *  - `spec <0|1|2|3>`: pins a bot's talent tab deterministically (the 1:1 test needs "holy priest",
 *    not "whatever InitTalentsTree's std::rand() % 3 picked" - AiFactory.cpp/BotFactory.cpp's own
 *    randomizer is untouched, this only overrides after login).
 *  - `bgtest on|off`: toggles BattlegroundMgr's testing mode (Battlegrounds/BattlegroundMgr.h:113/119,
 *    `IsTesting()`/`ToggleTesting()`) so a queue pops with fewer than a full team (`.debug bg`'s
 *    server-side twin, but reachable over SOAP - `.debug bg` is Console::No, see the doc section 4.1).
 *  - `bgend <A|H>`: force-ends the named player's current battleground with the given winner
 *    (`Battleground::EndBattleground(uint32 winner)`, `ALLIANCE`/`HORDE` from SharedDefines.h - NOT
 *    `TeamId`/`TEAM_ALLIANCE` - `Battleground.cpp:933-951` compares against those constants). Drives
 *    the B4 "match end" step without waiting out a real match.
 *  - `lfgstate`: prints the named player's and (if grouped) their group's `sLFGMgr->GetActiveState()`
 *    (LFGMgr.h:335) as a raw `LfgState` int, for L2-L6/L8's pass criteria - plus (round 3 addition)
 *    `InBattlegroundQueue()` and the player's queued BattlegroundQueueTypeId slots, since a bot
 *    self-queued for a BG silently refuses a group invite (PlayerbotSecurity.cpp:134 - "S9 root cause
 *    6") and that was the real explanation for round 1/2's LFG group-forming failures: `queuegate`
 *    only refused OTHER bots, not a roster bot already registered via masterprobe.
 *  - `queuegate on|off`: a global flag `BgJoinBlocked` (BotScenario.cpp) now unconditionally consults
 *    via `QueueGateOn()` below - ON refuses every bot's own BG self-queue, no exceptions, closing the
 *    round-1/2 gap above.
 *  - `clearqueue <player>`: round-setup only. Removes `<player>` from every BG queue slot it is in,
 *    via the same `RemoveBattlegroundQueueId` + `BattlegroundQueue::RemovePlayer` pair a real client's
 *    own queue-leave takes (mirrors `BGLeaveAction`'s queue-removal loop, strategy/bg/
 *    BattleGroundJoinAction.cpp) - not a teleport or any other shortcut.
 *  - `lfgjoin <player> <dungeonId> <rolesMask>`: #142. Joins that one ungrouped, unqueued online character to
 *    the LFG queue for exactly that dungeon with those roles via LFGMgr::JoinLfg (s10p1 L8b.fill 5th DPS).
 *  - `leavegroup <player>`: round-setup only (round 4 addition). `group->Disband()` for `<player>`'s
 *    current group, if any - what `.group disband <name>` would do, but every `.group` subcommand is
 *    Console::No on this core (checked cs_group.cpp directly) so none of them reach SOAP. Needed
 *    because a group persists in the DB across a server restart (unlike online status), and round 3
 *    found a stale leftover group confusing a fresh round's invites before it ever sent one.
 *  - `unaura <player> <spellId>`: round-setup only, to clear a leftover aura (e.g. Deserter, spell
 *    26013 on both cores) carried over from a PREVIOUS round before it contaminates this one. Never
 *    call it from inside a test step - it is not something a real player or bot could do.
 *  - `bgqueueinfo`: read-only, #275. One line per queued BattlegroundQueueTypeId slot naming the
 *    queue BUCKET the player or their group is really sitting in. #214/#274 (premade vs normal
 *    bucketing) could only be settled by waiting for a battleground to pop, which this harness
 *    cannot reliably produce; this reads the same fact straight off the queue's own m_QueuedGroups
 *    (BattlegroundQueue.h:111, public) instead, so the observed bucket index is the evidence rather
 *    than anything recomputed from isRated/isPremade.
 *
 * Linkage (Kezan worker's finding, BotScenarioVehicle.cpp:581-587): a new Scenario/*.cpp with nothing
 * but file-static registration has its whole .obj dropped by the linker unless something in an
 * already-linked TU calls a real symbol from it. RegisterQueueScenarioHandlers() below is that symbol -
 * BotScenario.cpp's HandlerRegistrar calls it once (a forward declaration at the call site, not a
 * shared-header one - see that file's hook comment). It has nothing to register today (none of these
 * verbs are a stateful ScenarioKind - see the vehicle file for what that pattern actually looks like);
 * it exists purely so the call site has something to call.
 */

#include "BotScenario.h"
#include "BotFactory.h"

#include "Playerbots.h"
#include "PlayerbotSpec.h"  // #598: role
#include "Player.h"
// DatabaseEnv.h must precede BattlegroundQueue.h: the latter pulls in RatedPvp.h, which uses
// CharacterDatabaseTransaction without declaring it (same ordering rule as
// strategy/bg/BattleGroundJoinAction.cpp's own header comment).
#include "DatabaseEnv.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "BattlegroundQueue.h"
#include "BattlegroundDG.h"  // #797: dgstate / dgdropcart / dgpick (MoP-only verbs, Deepwind Gorge)
#include "Chat.h"
#include "DBCStores.h"
#include "GameObject.h"
#include "Group.h"
#include "GroupMgr.h"  // #895: ratedbg group
#include "Map.h"
#include "InstanceSaveMgr.h"
#include "InstanceScript.h"
#include "LFGMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Item.h"
#include "Creature.h"     // #561: despawnnear
#include "GridObject.h"      // #519: must precede CellImpl.h (5.4.8)
#include "CellImpl.h"        // #519: cansee
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "AutoBalanceLevelScaling.h"  // #702: abinfo
#include "BattlegroundAnyLevel.h"     // #831: pvpscale
#include "ScriptMgr.h"                // #831: pvpscale (the UnitScript damage dispatch)
#include "SpellMgr.h"     // #558: useitem
#include "WorldPacket.h"
#include "WorldSession.h"

#include <atomic>
#include <list>
#include <cstdio>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace BotScenario
{
namespace
{
    // #142 w30: a holder count, not a bool. Concurrent drivers each bracket their own window with on/off; as a bool,
    // the t703 rated-arena release (on ... off) on MoP at 08:58:25 switched off the gate s10p1 had set at 08:57:10, and
    // the s10p1 cast self-queued for BGs and refused every bgD invite. on = +1 holder, off = -1 (floor 0), reset = 0.
    std::atomic<int> g_queueGateHolders{0};

    bool HandleSpec(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* bot = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
        if (!botAI)
        {
            handler->SendSysMessage(("botscenario: no online bot named '" + name + "'").c_str());
            return false;
        }

        uint32 tab = 0;
        // #741: tab = ChrSpecialization TabPage, 0-3 (druid is the only class with a 4th spec, Restoration).
        if (!(in >> tab) || tab >= MAX_SPECIALIZATIONS - 1)
        {
            handler->SendSysMessage("botscenario: usage: botscenario <bot> spec <0|1|2|3>");
            return false;
        }
        if (tab == 3 && bot->GetClass() != CLASS_DRUID)
        {
            handler->SendSysMessage("botscenario: spec tab 3 exists for druids only (Restoration)");
            return false;
        }

        // Same sequence BotFactory::InitTalentsTree uses to pick a spec on first login
        // (Factory/BotFactory.cpp:371-387), with the tab fixed instead of std::rand() % 3.
        bot->ResetTalents(true, true, true);

        WorldPacket p(CMSG_SET_PRIMARY_TALENT_TREE);
        p << uint32(tab);
        bot->GetSession()->HandeSetTalentSpecialization(p);
        bot->ActivateSpec(0);
        bot->LearnSpecializationSpells();

        // #734: the pinned spec gets its tier talents and glyphs exactly as a randomized bot does
        // (AiPlayerbot.PremadeSpecLink/PremadeSpecGlyph.<class>.<tab>), so a 1:1 run compares talented bots.
        {
            BotFactory factory(bot, bot->GetLevel());
            factory.InitTalentTiers();
            factory.InitGlyphs(false);
        }

        botAI->ResetStrategies();

        handler->PSendSysMessage("botscenario: %s spec tab=%u spec=%u", name.c_str(), tab,
                                  uint32(bot->GetSpecialization()));
        return true;
    }

    bool HandleBgTest(ChatHandler* handler, std::istringstream& in)
    {
        std::string mode;
        in >> mode;
        if (mode != "on" && mode != "off")
        {
            handler->SendSysMessage("botscenario: usage: botscenario - bgtest on|off");
            return false;
        }

        bool const want = (mode == "on");
        if (sBattlegroundMgr->IsTesting() != want)
            sBattlegroundMgr->ToggleTesting();

        handler->PSendSysMessage("botscenario: bgtest %s", sBattlegroundMgr->IsTesting() ? "on" : "off");
        return true;
    }

    bool HandleBgEnd(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        std::string team;
        in >> team;

        uint32 winner;
        if (team == "A" || team == "a")
            winner = ALLIANCE;
        else if (team == "H" || team == "h")
            winner = HORDE;
        else
        {
            handler->SendSysMessage("botscenario: usage: botscenario <player> bgend <A|H>");
            return false;
        }

        Battleground* bg = target->GetBattleground();
        if (!bg)
        {
            handler->SendSysMessage(("botscenario: " + name + " is not in a battleground").c_str());
            return false;
        }

        bg->EndBattleground(winner);
        handler->PSendSysMessage("botscenario: %s bgend map=%u winner=%s", name.c_str(),
                                  uint32(bg->GetMapId()), team.c_str());
        return true;
    }

    // Space-joined list of this player's queued BattlegroundQueueTypeId slots ("-" if none) -
    // round-setup evidence for the S9-root-cause-6 gate (PlayerbotSecurity refuses an invite while
    // InBattlegroundQueue(), silently) that #210's queuegate now blocks bots from ever hitting.
    std::string QueuedBgTypes(Player* target)
    {
        std::string out;
        for (uint8 slot = 0; slot < PLAYER_MAX_BATTLEGROUND_QUEUES; ++slot)
        {
            BattlegroundQueueTypeId const qid = target->GetBattlegroundQueueTypeId(slot);
            if (qid == BATTLEGROUND_QUEUE_NONE)
                continue;
            if (!out.empty())
                out += ",";
            out += std::to_string(uint32(qid));
        }
        return out.empty() ? "-" : out;
    }

    bool HandleLfgState(ChatHandler* handler, std::string const& name)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        uint32 const playerState = uint32(sLFGMgr->GetActiveState(target->GetGUID()));
        uint32 const inQueue = uint32(target->InBattlegroundQueue());
        std::string const queues = QueuedBgTypes(target);

        if (Group* group = target->GetGroup())
        {
            uint32 const groupState = uint32(sLFGMgr->GetActiveState(group->GetGUID()));
            handler->PSendSysMessage("botscenario: %s lfgstate player=%u group=%u bgqueue=%u queues=%s",
                                      name.c_str(), playerState, groupState, inQueue, queues.c_str());
        }
        else
        {
            handler->PSendSysMessage("botscenario: %s lfgstate player=%u (no group) bgqueue=%u queues=%s",
                                      name.c_str(), playerState, inQueue, queues.c_str());
        }

        return true;
    }

    // #833 (w66 L8): a relog did not take a bot out of its LFR queue (lfgstate player=2 after the relog, then lfgjoin 517
    // "already in LFG state 2"). Leave every LFG queue the player holds, whatever its state, as CMSG_LFG_LEAVE does per queue.
    bool HandleLfgLeave(ChatHandler* handler, std::string const& name)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        std::vector<uint32> queueIds;
        if (lfg::PlayerQueueDataMap const* queues = sLFGMgr->GetPlayerQueues(target->GetGUID()))
            for (auto const& itr : *queues)
                queueIds.push_back(itr.first);

        uint32 const before = uint32(sLFGMgr->GetActiveState(target->GetGUID()));
        for (uint32 queueId : queueIds)
            sLFGMgr->LeaveLfg(target->GetGUID(), queueId);

        handler->PSendSysMessage("botscenario: %s lfgleave queues=%u state %u -> %u", name.c_str(), uint32(queueIds.size()), before,
                                 uint32(sLFGMgr->GetActiveState(target->GetGUID())));
        return true;
    }

    // #598: what the role helpers say for a player (bot or real), both lookup modes. bySpec=1 is the path a real player
    // without bot AI takes (PlayerbotSpec.cpp); the two lines must agree for a bot.
    bool HandleRole(ChatHandler* handler, std::string const& name)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        for (int bySpec = 0; bySpec <= 1; ++bySpec)
        {
            bool const b = bySpec != 0;
            handler->PSendSysMessage("botscenario: %s role bySpec=%d spec=%u tank=%d heal=%d dps=%d ranged=%d melee=%d caster=%d",
                                     name.c_str(), bySpec, uint32(target->GetTalentSpecialization(target->GetActiveSpec())),
                                     int(PlayerBotSpec::IsTank(target, b)), int(PlayerBotSpec::IsHeal(target, b)),
                                     int(PlayerBotSpec::IsDps(target, b)), int(PlayerBotSpec::IsRanged(target, b)),
                                     int(PlayerBotSpec::IsMelee(target, b)), int(PlayerBotSpec::IsCaster(target, b)));
        }
        return true;
    }

    // #433 harness verbs (additem / strategy / imbueinfo / dismount), identical on both cores apart from
    // the reply helper (printf-style core vs fmt-style core are avoided by building std::string first).
    void ReplyLine(ChatHandler* handler, std::string const& line)
    {
        handler->SendSysMessage(line.c_str());
    }

    std::string JoinStrategies(PlayerbotAI* botAI, BotState state)
    {
        std::string out;
        for (std::string const& s : botAI->GetStrategies(state))
        {
            if (!out.empty())
                out += ",";
            out += s;
        }
        return out;
    }

    // `additem <entry> [count]`: in-memory inventory add via the CanStoreNewItem + StoreNewItem pair
    // that Player::StoreNewItemInBestSlots uses, but keeping the failure reason and a partial count.
    bool HandleAddItem(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }

        uint32 entry = 0, count = 1;
        in >> entry;
        if (!entry)
        {
            ReplyLine(handler, "botscenario: usage: botscenario <player> additem <entry> [count]");
            return false;
        }
        if (!(in >> count) || !count)
            count = 1;

        if (!sObjectMgr->GetItemTemplate(entry))
        {
            ReplyLine(handler, "botscenario: " + name + " additem entry=" + std::to_string(entry) +
                " stored=0 result=no_such_item");
            return false;
        }

        uint32 noSpace = 0;
        ItemPosCountVec dest;
        InventoryResult msg = target->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, entry, count, &noSpace);
        uint32 storeCount = count;
        if (msg != EQUIP_ERR_OK)
        {
            // Partial fit: store what fits (noSpace is the shortfall), report the error code.
            storeCount = noSpace < count ? count - noSpace : 0;
            dest.clear();
            if (storeCount && target->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, entry, storeCount) != EQUIP_ERR_OK)
                storeCount = 0;
        }

        uint32 stored = 0;
        if (storeCount && target->StoreNewItem(dest, entry, true))
            stored = storeCount;

        ReplyLine(handler, "botscenario: " + name + " additem entry=" + std::to_string(entry) +
            " requested=" + std::to_string(count) + " stored=" + std::to_string(stored) +
            " result=" + (msg == EQUIP_ERR_OK && stored == count ? std::string("ok") : "err" + std::to_string(uint32(msg))) +
            " total=" + std::to_string(target->GetItemCount(entry)));
        return stored > 0;
    }

    // `strategy <+name|-name|?> <co|nc>`: PlayerbotAI::ChangeStrategy, the call the "co"/"nc" chat
    // commands end in (Engine::ChangeStrategy parses the +/- list). `?` lists without changing.
    bool HandleStrategy(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* bot = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
        if (!botAI)
        {
            ReplyLine(handler, "botscenario: no online bot named '" + name + "'");
            return false;
        }

        // H1 (w29 rotation lane): the strategy list may hold names with spaces ("+strength of earth,-healer dps co"): take the
        // rest of the line, the LAST word is the engine (co|nc|empty for "?" alone), everything before it is the change string.
        std::string rest, change, which;
        std::getline(in, rest);
        while (!rest.empty() && (rest.front() == ' ' || rest.front() == '\t')) rest.erase(rest.begin());
        while (!rest.empty() && (rest.back() == ' ' || rest.back() == '\t' || rest.back() == '\r')) rest.pop_back();
        size_t const sp = rest.find_last_of(' ');
        if (sp == std::string::npos) change = rest;
        else if (rest.compare(sp + 1, std::string::npos, "co") == 0 || rest.compare(sp + 1, std::string::npos, "nc") == 0)
        { which = rest.substr(sp + 1); change = rest.substr(0, sp); }
        else change = rest;
        while (!change.empty() && change.back() == ' ') change.pop_back();
        bool const list = (change == "?");
        if (change.empty() || (!list && which != "co" && which != "nc") ||
            (!which.empty() && which != "co" && which != "nc"))
        {
            ReplyLine(handler, "botscenario: usage: botscenario <bot> strategy <+name|-name|?> <co|nc>");
            return false;
        }

        if (!list)
            botAI->ChangeStrategy(change, which == "co" ? BOT_STATE_COMBAT : BOT_STATE_NON_COMBAT);

        std::string out = "botscenario: " + name + " strategy";
        if (which.empty() || which == "co")
            out += " co=" + JoinStrategies(botAI, BOT_STATE_COMBAT);
        if (which.empty() || which == "nc")
            out += " nc=" + JoinStrategies(botAI, BOT_STATE_NON_COMBAT);
        ReplyLine(handler, out);
        return true;
    }

    bool HandleImbueInfo(ChatHandler* handler, std::string const& name)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }

        std::string out = "botscenario: " + name + " imbueinfo";
        for (uint8 slot = EQUIPMENT_SLOT_MAINHAND; slot <= EQUIPMENT_SLOT_OFFHAND; ++slot)
        {
            Item* item = target->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            out += slot == EQUIPMENT_SLOT_MAINHAND ? " mh=" : " oh=";
            if (!item)
                out += "none";
            else
                out += std::to_string(item->GetEntry()) + " ench=" +
                    std::to_string(item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT)) + " dur=" +
                    std::to_string(item->GetEnchantmentDuration(TEMP_ENCHANTMENT_SLOT));
        }
        // GetItemCount counts every item the bot carries (bags + equipped), not the bank.
        static uint32 const entries[] = {12404, 7965, 7964, 12643, 20750, 20746, 20747, 6256};
        out += " bags:";
        for (uint32 e : entries)
            out += " " + std::to_string(e) + "=" + std::to_string(target->GetItemCount(e));
        ReplyLine(handler, out);
        return true;
    }

    // ===== #561 / #558 / #560 (Phase 6 lane C, epic #569): event + vehicle dungeon verbs. Same code and reply text on both
    // trees; each does its own player lookup like the verbs above. =====
    //
    // `hasitem <itemId>` (read-only): how many of the item the character carries (bags + equipped, no bank), whether one is
    // equipped, and the entry of the vehicle it rides (0 = none). ToC: the Argent Lance 46106 the lance rack hands out and the
    // Argent mount the bot boarded; Oculus: the essence the dragon keeper's gossip gave and the drake it summoned.
    // `questreward <questId>`: mark a quest rewarded on an ONLINE character the way the GM `.quest add/complete/reward`
    // chain does (add -> complete -> RewardQuest), so an access_requirement quest gate (Pit of Saron 24499, Shado-Pan
    // Monastery 30752) is satisfied in memory. An offline DB row is wiped by the login randomize (ResetQuests, #813).
    bool HandleQuestReward(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }
        uint32 id = 0;
        Quest const* quest = (in >> id) && id ? sObjectMgr->GetQuestTemplate(id) : nullptr;
        if (!quest)
        {
            ReplyLine(handler, "botscenario: usage: botscenario <player> questreward <questId> (unknown quest)");
            return false;
        }
        if (!target->GetQuestRewardStatus(id))
        {
            if (target->GetQuestStatus(id) == QUEST_STATUS_NONE)
                target->AddQuest(quest, nullptr);
            target->CompleteQuest(id);
            target->RewardQuest(quest, 0, target, false);
        }
        ReplyLine(handler, "botscenario: " + name + " questreward " + std::to_string(id) + " rewarded=" +
            std::to_string(uint32(target->GetQuestRewardStatus(id))));
        return true;
    }

    bool HandleHasItem(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }
        uint32 entry = 0;
        if (!(in >> entry) || !entry)
        {
            ReplyLine(handler, "botscenario: usage: botscenario <player> hasitem <itemId>");
            return false;
        }
        Unit* vehicleBase = target->GetVehicleBase();
        ReplyLine(handler, "botscenario: " + name + " hasitem " + std::to_string(entry) +
            " count=" + std::to_string(target->GetItemCount(entry)) +
            " equipped=" + std::to_string(uint32(target->HasItemOrGemWithIdEquipped(entry, 1))) +
            " vehicle=" + std::to_string(vehicleBase ? vehicleBase->GetEntry() : 0u));
        return true;
    }

    // `useitem <itemId>`: the character uses that item from its bags - the server side of a right-click, which neither
    // headless client can send on both cores. The item's first on-use spell is cast by the character on itself with the
    // item as the cast item (triggered, as `castself`), so nothing happens when the item is not in the bags. Oculus: the
    // essence the master took from the dragon keeper's gossip (Eternos 27659 -> Amber Essence 37859, spell 49461)
    // summons the drake, which boards its summoner (AC oculus.cpp npc_oculus_drake::IsSummonedBy, fork oculus.cpp
    // npc_ruby_emerald_amber_drake::IsSummonedBy).
    bool HandleUseItem(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }
        uint32 entry = 0;
        if (!(in >> entry) || !entry)
        {
            ReplyLine(handler, "botscenario: usage: botscenario <player> useitem <itemId>");
            return false;
        }
        Item* item = target->GetItemByEntry(entry);
        uint32 spellId = 0;
        if (item && item->GetTemplate())
            for (auto const& itemSpell : item->GetTemplate()->Spells)
                if (itemSpell.SpellId > 0)
                {
                    spellId = uint32(itemSpell.SpellId);
                    break;
                }
        bool const ok = item && spellId && sSpellMgr->GetSpellInfo(spellId);
        if (ok)
            target->CastSpell(target, spellId, true, item);
        ReplyLine(handler, "botscenario: useitem " + name + " item=" + std::to_string(entry) + " spell=" + std::to_string(spellId) +
            (ok ? " ok" : (item ? " failed (no spell)" : " failed (no item)")));
        return ok;
    }

    // `despawnnear <range> <entry> [entry...]`: despawn every live creature of those entries within <range> yd (max 500) of
    // the character. ToC: the instance despawns the Argent mounts once the joust is over and every rider is set down (AC
    // instance_trial_of_the_champion.cpp:176-181 and :587, VehicleList), so Eadric is always fought on foot; the Eadric leg
    // does the same before its pull. A DB spawn comes back after its respawn time (MoP 300 s, AC 999999 s for these).
    bool HandleDespawnNear(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }
        float range = 0.0f;
        if (!(in >> range) || range <= 0.0f || range > 500.0f)
        {
            ReplyLine(handler, "botscenario: usage: botscenario <player> despawnnear <range 1-500> <entry> [entry...]");
            return false;
        }
        std::string per;
        uint32 total = 0;
        uint32 entry = 0;
        while (in >> entry)
        {
            if (!entry)
                continue;
            std::list<Creature*> found;
            target->GetCreatureListWithEntryInGrid(found, entry, range);
            uint32 n = 0;
            for (Creature* creature : found)
                if (creature && creature->IsAlive())
                {
                    creature->DespawnOrUnsummon();
                    ++n;
                }
            per += (per.empty() ? "" : " ") + std::to_string(entry) + ":" + std::to_string(n);
            total += n;
        }
        if (per.empty())
        {
            ReplyLine(handler, "botscenario: usage: botscenario <player> despawnnear <range 1-500> <entry> [entry...]");
            return false;
        }
        ReplyLine(handler, "botscenario: despawnnear " + name + " range=" + std::to_string(uint32(range)) + " despawned=" +
            std::to_string(total) + " (" + per + ")");
        return true;
    }

    // `encounter <id> <state 0-5>`: record an encounter state the way the instance's own script does. The fork's scripts keep
    // it in the boss table (InstanceScript::SetBossState, e.g. fork instance_oculus.cpp:202-226: Varos DONE puts Urom in
    // phase 1); several AC scripts keep their own array behind SetData instead (AC instance_oculus.cpp:136-160: Varos DONE
    // clears Urom's UNIT_FLAG_NON_ATTACKABLE; AC boss_urom.cpp:122-131 reads it back on every Reset). Both calls are made, so
    // one driver line releases the next encounter on both cores; on an instance that only uses one of them the other is a
    // no-op (an id past the boss table is refused by SetBossState, the base SetData does nothing). Unlike `bossstate`, it
    // reaches the AC SetData scripts. The reply's "ok" part is identical on both cores; the bracket shows what changed.
    bool HandleEncounter(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }
        uint32 id = 0, st = 0;
        InstanceScript* inst = target->GetInstanceScript();
        if (!(in >> id >> st) || !inst || st > uint32(TO_BE_DECIDED))
        {
            ReplyLine(handler, "botscenario: usage: botscenario <player> encounter <id> <state 0-5> (player in an instance)");
            return false;
        }
        uint32 const bossBefore = uint32(inst->GetBossState(id));
        uint32 const dataBefore = inst->GetData(id);
        inst->SetBossState(id, EncounterState(st));
        inst->SetData(id, st);
        ReplyLine(handler, "botscenario: encounter " + name + " id=" + std::to_string(id) + " state=" + std::to_string(st) +
            " ok (boss " + std::to_string(bossBefore) + "->" + std::to_string(uint32(inst->GetBossState(id))) +
            ", data " + std::to_string(dataBefore) + "->" + std::to_string(inst->GetData(id)) + ")");
        return true;
    }

    // `bosshp <entry> <mult> [range 1-500]` (#817): scale the max health of every live creature of <entry> around the
    // character (default range 500 yd, so the whole instance map) to <mult> x its original max and refill it, so a timed or
    // HP-gated boss mechanic can be proven on both cores at the same multiplier (the MoP bots kill the same boss in a third of
    // AC's time at the same level and comp). The same four writes both cores' Creature::SelectLevel makes (AC Creature.cpp:
    // 1536-1550, fork Creature.cpp:1252-1278): create-health, max-health, health and the UNIT_MOD_HEALTH base value, so
    // UpdateMaxHealth (any aura change) keeps the new value instead of recomputing the old one. The original max is cached per
    // creature GUID on first sight, so a repeated `bosshp` sets orig x mult (it does not compound) and `bosshp <entry> 1`
    // restores it. Scripts that refill with SetHealth(GetMaxHealth()) / CountPctFromMaxHealth on a phase change follow the new
    // max; a script that sets a fixed max, or a respawn (SelectLevel runs again), puts the old value back - re-issue then.
    bool HandleBossHp(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }
        uint32 entry = 0;
        float mult = 0.0f;
        float range = 500.0f;
        if (!(in >> entry >> mult) || !entry || !(mult > 0.0f) || mult > 1000.0f)
        {
            ReplyLine(handler, "botscenario: usage: botscenario <player> bosshp <entry> <mult 0-1000> [range 1-500]");
            return false;
        }
        float r = 0.0f;
        if (in >> r)
            range = r < 1.0f ? 1.0f : (r > 500.0f ? 500.0f : r);

        static std::unordered_map<uint64, uint32> originalMax;
        std::list<Creature*> found;
        target->GetCreatureListWithEntryInGrid(found, entry, range);
        // w58/w59: the grid search found no live Slad'ran / Bronjahm / Hungarfen on MoP (n=0 from the master AND from the tank
        // next to the boss) while AC found them. Fall back to the map's DB-spawn store (whole instance, no grid range).
        if (found.empty())
            for (auto const& pair : target->GetMap()->GetCreatureBySpawnIdStore())
                if (pair.second && pair.second->GetEntry() == entry)
                    found.push_back(pair.second);
        uint32 n = 0;
        uint32 maxNow = 0;
        for (Creature* creature : found)
        {
            if (!creature || !creature->IsAlive())
                continue;
            uint64 const key = creature->GetGUID().GetRawValue();
            auto it = originalMax.find(key);
            if (it == originalMax.end())
                it = originalMax.emplace(key, creature->GetMaxHealth()).first;
            uint32 const newMax = std::max<uint32>(1, uint32(float(it->second) * mult));
            creature->SetCreateHealth(newMax);
            creature->SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, (float)newMax);
            creature->SetMaxHealth(newMax);
            creature->SetHealth(newMax);
            // the loot/reward gate (Unit::Kill -> IsDamageEnoughForLootingAndReward) needs players/vehicles to deal half of the health
            // the creature had at spawn; with the scaled-down max the damage is capped at the new health, so a mult < 0.5 never
            // credited a kill (t438 w76 L14/L15/L18: kills, objective 0). Re-arm the requirement from the new health.
            creature->ResetPlayerDamageReq();
            maxNow = std::max(maxNow, creature->GetMaxHealth());
            ++n;
        }
        ReplyLine(handler, "botscenario: bosshp " + std::to_string(entry) + " n=" + std::to_string(n) + " max=" + std::to_string(maxNow));
        return n > 0;
    }

    // #702 `abinfo [entry] [range]`: harness-only, read-only, MoP-only (the AC tree has no AutoBalance). One line with the
    // AutoBalance level-scaling state of the character's instance ("ab map=... iid=... active=... enabled=... players=..."),
    // then, with an entry, one line per live creature of that entry within [range] yd (default 200; whole instance if the
    // grid search finds none): its level, original level, max health and spell-damage multiplier. The same lines print with
    // the feature off (active=0 state=none / tracked=0), which is what the toggle-off legs assert.
    bool HandleAbInfo(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }

        uint32 entry = 0;
        float range = 200.0f;
        in >> entry;
        float r = 0.0f;
        if (in >> r)
            range = r < 1.0f ? 1.0f : (r > 500.0f ? 500.0f : r);

        ReplyLine(handler, "botscenario: abinfo " + name + " " + AutoBalance::DescribeMap(target->GetMap()));
        if (!entry)
            return true;

        std::list<Creature*> found;
        target->GetCreatureListWithEntryInGrid(found, entry, range);
        if (found.empty())
            for (auto const& pair : target->GetMap()->GetCreatureBySpawnIdStore())
                if (pair.second && pair.second->GetEntry() == entry)
                    found.push_back(pair.second);

        uint32 n = 0;
        for (Creature* creature : found)
        {
            if (!creature || !creature->IsAlive())
                continue;
            ReplyLine(handler, "botscenario: abinfo " + name + " " + AutoBalance::DescribeCreature(creature));
            if (++n >= 5)
                break;
        }
        if (!n)
            ReplyLine(handler, "botscenario: abinfo " + name + " entry=" + std::to_string(entry) + " none");
        return true;
    }

    // `engage <entry> [range]` (#558 w57): the nearest live <entry> within [range] yd (default 30) of the character starts
    // fighting it, as a player's first hit does: the creature puts the character on its threat list (0 threat), which runs its
    // JustEngagedWith. Oculus' Urom: on the outer platforms AC's UromMultiplier (OCMultipliers.cpp, GetPhaseByCurrentPosition < 3)
    // zeroes every bot action aimed at him, so a real player has to pull him there; the headless master does it with this
    // (the AC headless client has no attack verb). Same reply on both trees.
    bool HandleEngage(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }
        uint32 entry = 0;
        float range = 30.0f;
        in >> entry;
        if (!(in >> range) || range <= 0.0f || range > 200.0f)
            range = 30.0f;
        Creature* creature = entry ? target->FindNearestCreature(entry, range, true) : nullptr;
        if (!creature)
        {
            ReplyLine(handler, "botscenario: engage " + name + " entry=" + std::to_string(entry) + " none");
            return false;
        }
        creature->GetThreatManager().AddThreat(target, 0.0f, nullptr, true, true);   // MoP: no Unit::EngageWithTarget; AC's body
        ReplyLine(handler, "botscenario: engage " + name + " entry=" + std::to_string(entry) + " ok");
        return true;
    }

    // `cansee <entry> [range]` (#519, read-only): the visibility twin of npcguid. npcguid is FindNearestCreature, which only
    // filters on the phase MASK and a private-object owner, so it returns creatures the character cannot see (a _phases-phased
    // NPC, a quest-visibility-gated NPC, a serverside-invisible one) and "npcguid guid=N" is no proof of visibility. This verb
    // collects every live creature of the entry within <range> yd (default 100, max 500) with the phase mask forced open, then
    // asks the character's own WorldObject::CanSeeOrDetect(creature, false, distanceCheck=true) (map + phase + quest-visibility
    // + stealth/detect + sight range = what the client is sent) and InSamePhase for each. Reply, one line:
    //   botscenario: cansee <name> entry=E range=R total=N samephase=P seen=S nearest=<guid|0> nearestSeen=<guid|0> nearestSeenDist=D
    // `seen>0` is the visibility proof; `total>seen` means a copy exists that the character cannot see. AC mirrors the verb
    // (same text) in its BotScenarioQueue.cpp.
    bool HandleCanSee(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }
        uint32 entry = 0;
        float range = 100.0f;
        in >> entry;
        if (!entry)
        {
            ReplyLine(handler, "botscenario: usage: botscenario <player> cansee <entry> [range 1-500]");
            return false;
        }
        if (!(in >> range) || range <= 0.0f || range > 500.0f)
            range = 100.0f;

        std::list<Creature*> found;
        {
            Trinity::AllCreaturesOfEntryInRange check(target, entry, range);
            Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(target, found, check);
            searcher.i_phaseMask = PHASEMASK_ANYWHERE;   // GetCreatureListWithEntryInGrid would drop other-mask creatures
            Cell::VisitAllObjects(target, searcher, range);
        }

        uint32 total = 0, samePhase = 0, seen = 0;
        Creature* nearest = nullptr;
        Creature* nearestSeen = nullptr;
        float nearestDist = 0.0f, nearestSeenDist = 0.0f;
        for (Creature* creature : found)
        {
            if (!creature || !creature->IsAlive())
                continue;
            ++total;
            float const dist = target->GetDistance(creature);
            if (!nearest || dist < nearestDist)
            {
                nearest = creature;
                nearestDist = dist;
            }
            if (target->InSamePhase(creature))
                ++samePhase;
            if (target->CanSeeOrDetect(creature, false, true))
            {
                ++seen;
                if (!nearestSeen || dist < nearestSeenDist)
                {
                    nearestSeen = creature;
                    nearestSeenDist = dist;
                }
            }
        }

        char buf[320];
        std::snprintf(buf, sizeof(buf),
            "botscenario: cansee %s entry=%u range=%.0f total=%u samephase=%u seen=%u nearest=%llu nearestSeen=%llu nearestSeenDist=%.1f",
            name.c_str(), entry, range, total, samePhase, seen,
            (unsigned long long)(nearest ? nearest->GetGUID().GetRawValue() : 0),
            (unsigned long long)(nearestSeen ? nearestSeen->GetGUID().GetRawValue() : 0), nearestSeen ? nearestSeenDist : 0.0f);
        ReplyLine(handler, buf);
        return true;
    }

    bool HandleDismount(ChatHandler* handler, std::string const& name)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }

        bool const was = target->IsMounted() || target->HasAuraType(SPELL_AURA_MOUNTED);
        target->RemoveAurasByType(SPELL_AURA_MOUNTED);
        if (target->IsMounted())
            target->Dismount();
        bool const now = target->IsMounted() || target->HasAuraType(SPELL_AURA_MOUNTED);
        ReplyLine(handler, "botscenario: " + name + " dismount was_mounted=" + std::to_string(uint32(was)) +
            " mounted=" + std::to_string(uint32(now)));
        return !now;
    }

    // `setfly <on|off>` (#401): on = ride a flying-mount spell (Golden Gryphon 32235 Alliance / Tawny Wind
    // Rider 32243 Horde, the ids PlayerbotFactory/BotFactory already use; both exist in each core's
    // Spell.dbc) cast triggered, so the real MOUNTED/fly auras run; if the bot is still not flying
    // (area/skill refused) force CAN_FLY|FLYING|DISABLE_GRAVITY. off = dismount + clear them.
    bool HandleSetFly(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }

        std::string mode;
        in >> mode;
        if (mode != "on" && mode != "off")
        {
            ReplyLine(handler, "botscenario: usage: botscenario <bot> setfly <on|off>");
            return false;
        }

        uint32 const flyFlags = MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY;
        bool const on = (mode == "on");
        if (on)
        {
            if (!target->IsMounted())
                target->CastSpell(target, target->GetTeamId() == TEAM_ALLIANCE ? 32235u : 32243u, true);
            if (!target->IsFlying())
            {
                target->SetCanFly(true);
                target->SetDisableGravity(true);
                target->AddUnitMovementFlag(flyFlags);
                target->SendMovementFlagUpdate();
            }
        }
        else
        {
            target->RemoveAurasByType(SPELL_AURA_MOUNTED);
            if (target->IsMounted())
                target->Dismount();
            target->SetCanFly(false);
            target->SetDisableGravity(false);
            target->RemoveUnitMovementFlag(flyFlags);
            target->SendMovementFlagUpdate();
        }

        bool const mounted = target->IsMounted() || target->HasAuraType(SPELL_AURA_MOUNTED);
        char hex[16];
        snprintf(hex, sizeof(hex), "%x", uint32(target->GetUnitMovementFlags()));
        ReplyLine(handler, "botscenario: " + name + " setfly " + mode + " mounted=" + std::to_string(uint32(mounted)) +
            " flying=" + std::to_string(uint32(target->IsFlying())) + " flags=" + hex);
        return on ? target->IsFlying() : !target->IsFlying();
    }

    bool HandleClearQueue(ChatHandler* handler, std::string const& name)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        // Same queue-removal loop BGLeaveAction runs for a bot leaving from outside a match
        // (strategy/bg/BattleGroundJoinAction.cpp), i.e. the path a real client's own queue-leave
        // takes - not a teleport or any other shortcut.
        uint32 removed = 0;
        for (uint8 slot = 0; slot < PLAYER_MAX_BATTLEGROUND_QUEUES; ++slot)
        {
            BattlegroundQueueTypeId const qid = target->GetBattlegroundQueueTypeId(slot);
            if (qid == BATTLEGROUND_QUEUE_NONE)
                continue;

            target->RemoveBattlegroundQueueId(qid);
            sBattlegroundMgr->GetBattlegroundQueue(qid).RemovePlayer(target->GetGUID(), true, true);
            ++removed;
        }

        handler->PSendSysMessage("botscenario: %s clearqueue removed=%u", name.c_str(), removed);
        return true;
    }

    bool HandleLeaveGroup(ChatHandler* handler, std::string const& name)
    {
        // Round-setup only: `.group disband <name>` does exactly this but is Console::No (every
        // `.group` subcommand is, on both cores - checked cs_group.cpp directly), so it cannot be
        // reached over SOAP. Disbands the WHOLE group rather than just removing this one member,
        // since round setup wants nobody left grouped at all, not a partially-untangled leftover
        // group with other stale members still in it.
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        Group* group = target->GetGroup();
        if (!group)
        {
            handler->PSendSysMessage("botscenario: %s leavegroup no group", name.c_str());
            return true;
        }

        group->Disband();
        handler->PSendSysMessage("botscenario: %s leavegroup disbanded", name.c_str());
        return true;
    }

    bool HandleAcceptInvite(ChatHandler* handler, std::string const& name)
    {
        // #782 (w34 HoO MoP): "accept invitation" is a one-shot SMSG_GROUP_INVITE packet trigger (AC PlayerbotAI.cpp:300,
        // WorldPacketHandlerStrategy.cpp:14). A bot in combat when the invite lands can lose the queued action on the engine
        // switch, and a re-invite never reaches it while the first invite is still pending. This runs the same
        // AcceptInvitationAction on the pending invite, so the bot joins exactly as if the trigger had fired.
        Player* bot = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        PlayerbotAI* botAI = bot ? GET_PLAYERBOT_AI(bot) : nullptr;
        if (!botAI)
        {
            handler->SendSysMessage(("botscenario: no online bot named '" + name + "'").c_str());
            return false;
        }

        if (!bot->GetGroupInvite())
        {
            Group* group = bot->GetGroup();
            handler->PSendSysMessage("botscenario: %s acceptinvite no pending invite group=%u", name.c_str(),
                                     group ? uint32(group->GetMembersCount()) : 0u);
            return true;
        }

        bool const ok = botAI->DoSpecificAction("accept invitation", Event(), true);
        Group* group = bot->GetGroup();
        handler->PSendSysMessage("botscenario: %s acceptinvite %s group=%u", name.c_str(), ok ? "ok" : "failed",
                                 group ? uint32(group->GetMembersCount()) : 0u);
        return true;
    }

    bool HandleUnbind(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        // #789 (w36 LCT MoP): random bots keep heroic binds across waves (every LCT group bot was bound to instance 16 with
        // Husam already dead), so the group re-entered the cleared instance and three boss legs had no boss. `.instance unbind`
        // needs a selected player (no SOAP), so the harness drops one map's binds on an online player, every difficulty.
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        uint32 mapId = 0;
        if (!(in >> mapId))
        {
            handler->SendSysMessage("botscenario: usage: botscenario <player> unbind <mapId>");
            return false;
        }

        // #801 (w42 SM MoP): the reply lists each dropped bind as <instanceId>[P] (P = permanent), and the target's GROUP bind
        // for the map is dropped too. Group::Create copies every leader bind, permanent ones included, into the group
        // (Group.cpp:170 -> Player::ConvertInstancesToGroup, Player.cpp:20732-20754), and CreateInstanceForPlayer sends each
        // member without its own permanent bind into the group's instance (MapInstanced.cpp:146-170). Exact-difficulty lookup.
        uint32 dropped = 0;
        std::string list, glist;
        for (uint8 d = 0; d < MAX_DIFFICULTY; ++d)
        {
            auto const itr = target->GetBoundInstances(Difficulty(d)).find(mapId);
            if (itr == target->GetBoundInstances(Difficulty(d)).end())
                continue;
            list += " " + std::to_string(itr->second.save->GetInstanceId()) + (itr->second.perm ? "P" : "");
            target->UnbindInstance(mapId, Difficulty(d));
            ++dropped;
        }
        if (Group* group = target->GetGroup())
            for (uint8 d = 0; d < MAX_DIFFICULTY; ++d)
            {
                auto const itr = group->GetBoundInstances(Difficulty(d)).find(mapId);
                if (itr == group->GetBoundInstances(Difficulty(d)).end())
                    continue;
                glist += " " + std::to_string(itr->second.save->GetInstanceId()) + (itr->second.perm ? "P" : "");
                group->UnbindInstance(mapId, d);
            }

        handler->PSendSysMessage("botscenario: %s unbind map=%u dropped=%u ids=%s group=%s", name.c_str(), mapId, dropped,
                                 list.empty() ? " -" : list.c_str(), glist.empty() ? " -" : glist.c_str());
        return true;
    }

    bool HandleInstInfo(ChatHandler* handler, std::string const& name)
    {
        // #801: read-only "instinfo": the instance the player is in now (id, completed-encounter mask, every boss state
        // 0 NOT_STARTED .. 3 DONE), so a dungeon driver can prove each leg starts in a fresh instance. Same reply on both cores.
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        InstanceScript* inst = target->GetInstanceScript();
        std::string states;
        for (uint32 id = 0; inst && id < inst->GetEncounterCount(); ++id)
            states += (id ? "," : "") + std::to_string(uint32(inst->GetBossState(id)));
        handler->PSendSysMessage("botscenario: instinfo %s map=%u iid=%u mask=%u bosses=%s", name.c_str(), target->GetMapId(),
                                 target->GetInstanceId(), inst ? inst->GetCompletedEncounterMask() : 0u, inst ? states.c_str() : "-");
        return true;
    }

    bool HandleUnaura(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        // Round-setup only (see this file's "unaura" doc comment): clears a leftover aura - e.g.
        // Deserter (spell 26013, confirmed identical id on this core: Entities/Player/Player.cpp
        // "CastSpell(this, 26013, true); // Deserter") - from a PREVIOUS round so it does not
        // contaminate this one. Never call this from inside a test step itself.
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        uint32 spellId = 0;
        if (!(in >> spellId) || !spellId)
        {
            handler->SendSysMessage("botscenario: usage: botscenario <player> unaura <spellId>");
            return false;
        }

        bool const had = target->HasAura(spellId);
        target->RemoveAurasDueToSpell(spellId);
        handler->PSendSysMessage("botscenario: %s unaura spell=%u had=%u", name.c_str(), spellId,
                                  uint32(had));
        return true;
    }

    // #142 `lfgjoin <dungeonId> <rolesMask>`: harness-only. Makes this ONE online, ungrouped, unqueued
    // character join the LFG queue for exactly that dungeon with exactly those roles (0x2 tank, 0x4 healer,
    // 0x8 damage; decimal or 0x-hex), through the same LFGMgr::JoinLfg a client's CMSG_LFG_JOIN ends in. The
    // s10p1 L8b.fill leg uses it to put a deterministic 5th DPS in the queue instead of waiting for a random
    // bot to self-queue. Runs on the world thread (SOAP/console commands are drained there), so the
    // "JoinLfg is not threadsafe" caveat on the bot path does not apply. Same reply line on both cores.
    bool HandleLfgJoin(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }

        std::string dungeonArg, rolesArg;
        in >> dungeonArg >> rolesArg;
        uint32 dungeonId = 0, roles = 0;
        try
        {
            if (!dungeonArg.empty())
                dungeonId = uint32(std::stoul(dungeonArg, nullptr, 0));
            if (!rolesArg.empty())
                roles = uint32(std::stoul(rolesArg, nullptr, 0));
        }
        catch (...)
        {
            dungeonId = 0;
            roles = 0;
        }

        std::string const head = "botscenario: lfgjoin " + name + " dungeon=" + std::to_string(dungeonId) +
            " roles=" + std::to_string(roles) + " ";
        if (!dungeonId || !roles || roles > 0xF)
        {
            ReplyLine(handler, head + "failed: usage: botscenario <player> lfgjoin <dungeonId> <rolesMask 1-15>");
            return false;
        }

        if (target->GetGroup())
        {
            ReplyLine(handler, head + "failed: player is grouped");
            return false;
        }
        if (sLFGMgr->GetActiveState(target->GetGUID()) != lfg::LFG_STATE_NONE)
        {
            ReplyLine(handler, head + "failed: already in LFG state " + std::to_string(uint32(sLFGMgr->GetActiveState(target->GetGUID()))));
            return false;
        }
        if (target->InBattlegroundQueue() || target->InBattleground())
        {
            ReplyLine(handler, head + "failed: player is in a battleground or BG queue");
            return false;
        }

        lfg::LfgDungeonSet dungeons;
        dungeons.insert(dungeonId);
        sLFGMgr->JoinLfg(target, lfg::LfgRoles(roles), dungeons, "");

        lfg::LfgState const after = sLFGMgr->GetActiveState(target->GetGUID());
        if (after == lfg::LFG_STATE_NONE)
        {
            ReplyLine(handler, head + "failed: JoinLfg rejected it (state NONE: dungeon id invalid, level/lock, deserter or role rule)");
            return false;
        }

        ReplyLine(handler, head + "ok state=" + std::to_string(uint32(after)));
        return true;
    }

    // Human name of a BattlegroundQueue.h:66-73 bucket index, i.e. of the second dimension of
    // m_QueuedGroups[bracket][bucket]. Names are taken straight off the enum so a printed line
    // diffs against the core's own index arithmetic without a translation table.
    char const* BgQueueBucketName(uint32 bucket)
    {
        switch (bucket)
        {
            case BG_QUEUE_PREMADE_ALLIANCE: return "PREMADE_ALLIANCE";
            case BG_QUEUE_PREMADE_HORDE:    return "PREMADE_HORDE";
            case BG_QUEUE_NORMAL_ALLIANCE:  return "NORMAL_ALLIANCE";
            case BG_QUEUE_NORMAL_HORDE:     return "NORMAL_HORDE";
            default:                        return "UNKNOWN";
        }
    }

    // Find the bucket a queued group is ACTUALLY sitting in, by walking the queue's own
    // m_QueuedGroups (BattlegroundQueue.h:111; `protected:` only starts at :132) and matching the
    // live GroupQueueInfo pointer. Every bracket is scanned, not just the level-derived one,
    // because the observed index IS the evidence: a group filed under some other bracket must
    // still be found, and a pointer found in no bucket at all is reported as -1 rather than
    // guessed. Nothing here is recomputed from isRated/isPremade - that is the value under test.
    bool FindBgQueueBucket(BattlegroundQueue& queue, GroupQueueInfo* live, int32& bracket, uint32& bucket)
    {
        for (uint32 b = 0; b < MAX_BATTLEGROUND_BRACKETS; ++b)
            for (uint32 t = 0; t < BG_QUEUE_GROUP_TYPES_COUNT; ++t)
                for (GroupQueueInfo* candidate : queue.m_QueuedGroups[b][t])
                    if (candidate == live)
                    {
                        bracket = int32(b);
                        bucket = t;
                        return true;
                    }

        bracket = -1;
        bucket = BG_QUEUE_GROUP_TYPES_COUNT;
        return false;
    }

    // #275 `bgqueueinfo`: read-only. Reports, per queued slot, the queue bucket the player or
    // their group occupies, so premade-vs-normal bucketing (#214/#274) is settled by READING a
    // value instead of waiting for a pop this harness cannot reliably produce.
    bool HandleBgQueueInfo(ChatHandler* handler, std::string const& name)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        uint32 const guidLow = target->GetGUID().GetCounter();
        uint32 const grouped = target->GetGroup() ? 1 : 0;
        uint64 const rawGuid = target->GetGUID().GetRawValue();

        // Same slot enumeration QueuedBgTypes() walks for `lfgstate`, so a bot that self-queued
        // and is silently refusing group invites (PlayerbotSecurity.cpp:134) shows up here too.
        std::vector<BattlegroundQueueTypeId> queued;
        for (uint8 slot = 0; slot < PLAYER_MAX_BATTLEGROUND_QUEUES; ++slot)
        {
            BattlegroundQueueTypeId const qid = target->GetBattlegroundQueueTypeId(slot);
            if (qid != BATTLEGROUND_QUEUE_NONE)
                queued.push_back(qid);
        }

        if (queued.empty())
        {
            handler->PSendSysMessage("BGQUEUEINFO player=%s guid=%u grouped=%u inbg=%u bgtype=%u none", name.c_str(), guidLow, grouped,
                target->GetBattlegroundId(), uint32(target->GetBattlegroundTypeId())); // #631: BG state without .saveall
            return true;
        }

        handler->PSendSysMessage("BGQUEUEINFO player=%s guid=%u grouped=%u inbg=%u bgtype=%u", name.c_str(), guidLow, grouped,
            target->GetBattlegroundId(), uint32(target->GetBattlegroundTypeId())); // #631

        for (BattlegroundQueueTypeId qid : queued)
        {
            BattlegroundQueue& queue = sBattlegroundMgr->GetBattlegroundQueue(qid);
            BattlegroundTypeId const bgTypeId = BattlegroundMgr::BGTemplateId(qid);
            Battleground* bgTemplate = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);

            // Bracket the core itself would file this player under: the same
            // sDBCManager.GetBattlegroundBracketByLevel(map, level) a real queue-join makes
            // (BattlegroundMgr.cpp:1148). Printed alongside the observed bracket, and -1 when the
            // level has no PvPDifficulty row for this map rather than assumed.
            int32 levelBracket = -1;
            if (bgTemplate)
                if (PvPDifficultyEntry const* bracketEntry =
                        sDBCManager.GetBattlegroundBracketByLevel(bgTemplate->GetMapId(), target->GetLevel()))
                    levelBracket = int32(uint32(bracketEntry->GetBracketId()));

            // GetPlayerGroupInfoData COPIES the GroupQueueInfo into the caller's struct
            // (BattlegroundQueue.cpp:544-551) and hands back no pointer, so the live pointer has to
            // come from m_QueuedPlayers (BattlegroundQueue.h:96-97, public). The copy still carries
            // the per-slot facts read below (group size, team, rated flags).
            GroupQueueInfo copy;
            GroupQueueInfo* live = nullptr;
            if (queue.GetPlayerGroupInfoData(rawGuid, &copy))
            {
                auto qItr = queue.m_QueuedPlayers.find(rawGuid);
                if (qItr != queue.m_QueuedPlayers.end())
                    live = qItr->second.GroupInfo;
            }

            int32 observedBracket = -1;
            uint32 observedBucket = BG_QUEUE_GROUP_TYPES_COUNT;
            bool const located = FindBgQueueBucket(queue, live, observedBracket, observedBucket);

            // isPremade is NOT a GroupQueueInfo field on this core: it exists only as an AddGroup
            // argument (BattlegroundQueue.h:89) and is not recorded afterwards, so it cannot be
            // reported as a value this core still holds. It is reported as what the OBSERVED bucket
            // implies instead (the two PREMADE_* buckets are 0 and 1).
            uint32 const isPremade = (located && observedBucket < 2) ? 1 : 0;

            handler->PSendSysMessage("BGQUEUEINFO slot=%u bg=%u bracket=%d levelBracket=%d bucket=%s isPremade=%u groupSize=%u minPerTeam=%u maxPerTeam=%u queueIdx=%d",
                uint32(qid), uint32(bgTypeId), located ? observedBracket : levelBracket, levelBracket,
                BgQueueBucketName(observedBucket), isPremade, uint32(copy.Players.size()),
                bgTemplate ? bgTemplate->GetMinPlayersPerTeam() : 0,
                bgTemplate ? bgTemplate->GetMaxPlayersPerTeam() : 0,
                located ? int32(observedBucket) : -1);
        }

        return true;
    }

    // ---- #797 Deepwind Gorge cart gold accounting (MoP-only: AC has no DG) ---------------------------------
    // The ground-cart GameObject of the cart owned by team `owner` (0 = Alliance cart, 1 = Horde cart), or null.
    // Looked up through the map, like BgTacticsDG.cpp CartObject: Battleground::GetBGObject logs an error for a slot
    // the core never filled (the ground carts exist only after a drop).
    GameObject* DgGroundCart(Battleground* bg, Map* map, uint32 owner)
    {
        uint32 const slot = BG_DG_OBJECT_CART_ALLY_GROUND + owner;
        if (!map || slot >= bg->BgObjects.size() || bg->BgObjects[slot].IsEmpty())
            return nullptr;
        GameObject* go = map->GetGameObject(bg->BgObjects[slot]);
        return (go && go->isSpawned()) ? go : nullptr;
    }

    // The BASE-cart GameObject of the cart owned by team `owner` (0 = Alliance cart, 1 = Horde cart), or null. The base cart is
    // despawned (RESPAWN_ONE_DAY) while carried / on the ground, so no isSpawned() test here: callers check DgCartText == "base".
    GameObject* DgBaseCart(Battleground* bg, Map* map, uint32 owner)
    {
        uint32 const slot = BG_DG_OBJECT_CART_ALLIANCE + owner;
        if (!map || slot >= bg->BgObjects.size() || bg->BgObjects[slot].IsEmpty())
            return nullptr;
        return map->GetGameObject(bg->BgObjects[slot]);
    }

    // "base", "ground" or "carried:<name>" for the cart of team `owner`; same wording as the [BGDG] cart-change lines.
    std::string DgCartText(Battleground* bg, Map* map, uint32 owner)
    {
        ObjectGuid const g = bg->GetFlagPickerGUID(int32(owner));
        if (!g.IsEmpty())
        {
            Player* carrier = ObjectAccessor::FindPlayer(g);
            return std::string("carried:") + (carrier ? carrier->GetName() : "?");
        }
        return DgGroundCart(bg, map, owner) ? "ground" : "base";
    }

    // Resolves <player> to a Player in a Deepwind Gorge battleground that is running; replies and returns null otherwise.
    Player* DgPlayer(ChatHandler* handler, std::string const& name, Battleground*& bg, bool needRunning)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return nullptr;
        }
        bg = target->GetBattleground();
        if (!bg || bg->GetTypeID() != BATTLEGROUND_DG)
        {
            handler->SendSysMessage(("botscenario: " + name + " is not in a Deepwind Gorge battleground").c_str());
            return nullptr;
        }
        if (needRunning && bg->GetStatus() != STATUS_IN_PROGRESS)
        {
            handler->SendSysMessage(("botscenario: " + name + "'s Deepwind Gorge battleground is not in progress").c_str());
            return nullptr;
        }
        return target;
    }

    // #797 read-only "dgstate <player>": both teams' gold and both carts' state in one line, so a driver gets exact numbers
    // instead of the 30 s [BGDG] state line.  cartA/cartH = the ALLIANCE / HORDE cart (carried by an enemy, on the ground, at base).
    bool HandleDgState(ChatHandler* handler, std::string const& name)
    {
        Battleground* bg = nullptr;
        Player* target = DgPlayer(handler, name, bg, false);
        if (!target)
            return false;

        Map* map = target->GetMap();
        std::string ground[2];
        for (uint32 t = 0; t < 2; ++t)
            if (GameObject* go = DgGroundCart(bg, map, t))
            {
                char buf[96];
                snprintf(buf, sizeof(buf), " ground%c=%.1f,%.1f,%.1f", t ? 'H' : 'A', go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
                ground[t] = buf;
            }
        std::string base[2];  // #797: base cart positions (baseA / baseH), so a driver can teleport a thief next to the enemy base cart
        for (uint32 t = 0; t < 2; ++t)
            if (GameObject* go = DgBaseCart(bg, map, t))
            {
                char buf[96];
                snprintf(buf, sizeof(buf), " base%c=%.1f,%.1f,%.1f", t ? 'H' : 'A', go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
                base[t] = buf;
            }
        handler->PSendSysMessage("botscenario: dgstate %s inst=%u status=%u team=%u alive=%u gold=%u-%u cartA=%s cartH=%s carrying=%u map=%u pos=%.1f,%.1f,%.1f%s%s%s%s",
            name.c_str(), bg->GetInstanceID(), uint32(bg->GetStatus()), uint32(target->GetBGTeam()), target->IsAlive() ? 1u : 0u,
            bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE), DgCartText(bg, map, 0).c_str(), DgCartText(bg, map, 1).c_str(),
            (bg->GetFlagPickerGUID(0) == target->GetGUID() || bg->GetFlagPickerGUID(1) == target->GetGUID()) ? 1u : 0u,
            target->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), ground[0].c_str(), ground[1].c_str(), base[0].c_str(), base[1].c_str());
        return true;
    }

    // #797 "dgdropcart <carrier>": makes <carrier> drop the DG cart it carries through the core's own
    // Battleground::EventPlayerDroppedFlag (the call Player death / Unit aura removal make: Player.cpp:26369, Unit.cpp:18256,
    // SpellAuraEffects.cpp:3708).  Replies with the cart owner's gold before/after the drop, read inside the same call, so no
    // node tick can blur the delta, and the ground cart position (ground=x,y,z) for the driver to teleport a teammate onto.
    // The re-pick-up is a separate verb ("dgpick"), so the drop never depends on a thief being in range of a moving carrier.
    bool HandleDgDropCart(ChatHandler* handler, std::string const& name)
    {
        Battleground* bg = nullptr;
        Player* carrier = DgPlayer(handler, name, bg, true);
        if (!carrier)
            return false;

        int32 owner = -1;  // the picker table is indexed by the cart's owner team (BattlegroundDG.h GetFlagPickerGUID)
        for (int32 t = 0; t < 2; ++t)
            if (bg->GetFlagPickerGUID(t) == carrier->GetGUID())
                owner = t;
        if (owner < 0)
        {
            handler->PSendSysMessage("botscenario: dgdropcart %s is not carrying a cart", name.c_str());
            return false;
        }

        Map* map = carrier->GetMap();
        uint32 const before = bg->GetTeamScore(uint32(owner));
        bg->EventPlayerDroppedFlag(carrier);
        uint32 const afterDrop = bg->GetTeamScore(uint32(owner));
        char gbuf[64] = "none";
        if (GameObject* go = DgGroundCart(bg, map, uint32(owner)))
            snprintf(gbuf, sizeof(gbuf), "%.1f,%.1f,%.1f", go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
        handler->PSendSysMessage("botscenario: dgdropcart %s cart=%c owner-gold before=%u after=%u delta=%d cart-now=%s ground=%s gold=%u-%u",
            name.c_str(), owner ? 'H' : 'A', before, afterDrop, int32(afterDrop) - int32(before), DgCartText(bg, map, uint32(owner)).c_str(), gbuf,
            bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE));
        return true;
    }

    // #797 "dgpick <player>": <player> clicks the nearest ground DG cart within 10 yd through the core's own
    // Battleground::EventPlayerClickedOnFlag (the call GameObject::Use makes, GameObject.cpp:1793/1846-1855; its ground branch
    // re-checks IsWithinDistInMap(go, 10)).  An enemy of the cart's owner steals it back (no gold change: the cart still
    // carries the gold taken at the base pick-up, #797); a member of the owner's team returns it (the owner gets that
    // gold back).  The owner's gold is read before and after inside this one call, so the delta is exact.  REFUSED
    // (nothing clicked) when no ground cart is within 10 yd.
    bool HandleDgPick(ChatHandler* handler, std::string const& name)
    {
        Battleground* bg = nullptr;
        Player* thief = DgPlayer(handler, name, bg, true);
        if (!thief)
            return false;
        if (!thief->IsAlive())
        {
            handler->PSendSysMessage("botscenario: dgpick REFUSED %s is dead", name.c_str());
            return false;
        }

        Map* map = thief->GetMap();
        GameObject* go = nullptr;
        int32 owner = -1;
        float best = 0.0f;
        for (uint32 t = 0; t < 2; ++t)
            if (GameObject* g = DgGroundCart(bg, map, t))
            {
                float const d = thief->GetDistance(g);
                if (!go || d < best)
                {
                    go = g;
                    owner = int32(t);
                    best = d;
                }
            }
        if (!go)
        {
            handler->PSendSysMessage("botscenario: dgpick REFUSED %s no ground cart (cartA=%s cartH=%s)", name.c_str(),
                DgCartText(bg, map, 0).c_str(), DgCartText(bg, map, 1).c_str());
            return false;
        }
        if (!thief->IsWithinDistInMap(go, 10.0f))
        {
            handler->PSendSysMessage("botscenario: dgpick REFUSED %s cart-dist=%.1f > 10 yd for the nearest ground cart (cart=%c), nothing clicked",
                name.c_str(), best, owner ? 'H' : 'A');
            return false;
        }

        uint32 const thiefTeam = uint32(thief->GetBGTeam());
        uint32 const before = bg->GetTeamScore(uint32(owner));
        bg->EventPlayerClickedOnFlag(thief, go);  // may Delete() go
        uint32 const after = bg->GetTeamScore(uint32(owner));
        handler->PSendSysMessage("botscenario: dgpick %s cart=%c thief-team=%u kind=%s owner-gold before=%u after=%u delta=%d cart-now=%s gold=%u-%u",
            name.c_str(), owner ? 'H' : 'A', thiefTeam, (thiefTeam == ALLIANCE) == (owner == int32(TEAM_ALLIANCE)) ? "return" : "steal", before, after,
            int32(after) - int32(before), DgCartText(bg, map, uint32(owner)).c_str(), bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE));
        return true;
    }

    // #797 "dgtake <player>": <player> takes the ENEMY team's cart from its BASE through the core's own
    // Battleground::EventPlayerClickedOnFlag (BattlegroundDG.cpp:694 base-pick-up branches: m_flagGold = min(200, score),
    // UpdateTeamScore(owner, -m_flagGold)). Forces the carry the bots' own steal branch never started (it needs enemy gold >= 100),
    // so the drop / re-pick modes have a carrier. Owner gold is read before and after inside this one call. REFUSED (nothing
    // clicked) when dead, already carrying, the enemy cart is not at its base, or the base cart is > 10 yd away.
    bool HandleDgTake(ChatHandler* handler, std::string const& name)
    {
        Battleground* bg = nullptr;
        Player* thief = DgPlayer(handler, name, bg, true);
        if (!thief)
            return false;
        if (!thief->IsAlive())
        {
            handler->PSendSysMessage("botscenario: dgtake REFUSED %s is dead", name.c_str());
            return false;
        }
        if (bg->GetFlagPickerGUID(0) == thief->GetGUID() || bg->GetFlagPickerGUID(1) == thief->GetGUID())
        {
            handler->PSendSysMessage("botscenario: dgtake REFUSED %s already carries a cart", name.c_str());
            return false;
        }

        Map* map = thief->GetMap();
        uint32 const owner = (uint32(thief->GetBGTeam()) == ALLIANCE) ? uint32(TEAM_HORDE) : uint32(TEAM_ALLIANCE);  // the cart a thief steals
        GameObject* go = DgBaseCart(bg, map, owner);
        if (!go || DgCartText(bg, map, owner) != "base")
        {
            handler->PSendSysMessage("botscenario: dgtake REFUSED %s no base cart of cart=%c (cart-now=%s)", name.c_str(), owner ? 'H' : 'A',
                DgCartText(bg, map, owner).c_str());
            return false;
        }
        float const dist = thief->GetDistance(go);
        if (!thief->IsWithinDistInMap(go, 10.0f))
        {
            handler->PSendSysMessage("botscenario: dgtake REFUSED %s dist=%.1f > 10 yd from the base cart (cart=%c), nothing clicked",
                name.c_str(), dist, owner ? 'H' : 'A');
            return false;
        }

        uint32 const before = bg->GetTeamScore(owner);
        bg->EventPlayerClickedOnFlag(thief, go);
        uint32 const after = bg->GetTeamScore(owner);
        handler->PSendSysMessage("botscenario: dgtake %s cart=%c thief-team=%u owner-gold before=%u after=%u delta=%d cart-now=%s gold=%u-%u",
            name.c_str(), owner ? 'H' : 'A', uint32(thief->GetBGTeam()), before, after, int32(after) - int32(before),
            DgCartText(bg, map, owner).c_str(), bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE));
        return true;
    }

    bool HandleQueueGate(ChatHandler* handler, std::istringstream& in)
    {
        std::string mode;
        in >> mode;
        if (mode != "on" && mode != "off" && mode != "reset")
        {
            handler->SendSysMessage("botscenario: usage: botscenario - queuegate on|off|reset");
            return false;
        }

        int holders = 0;
        if (mode == "on")
            holders = ++g_queueGateHolders;
        else if (mode == "reset")
            g_queueGateHolders.store(0);
        else
        {
            int cur = g_queueGateHolders.load();
            while (cur > 0 && !g_queueGateHolders.compare_exchange_weak(cur, cur - 1))
                ;
            holders = cur > 0 ? cur - 1 : 0;
        }
        handler->PSendSysMessage("botscenario: queuegate %s (holders=%d, gate %s)", mode.c_str(), holders, holders > 0 ? "on" : "off");
        return true;
    }
    // #831 `pvpscale <target|*> [amount]` (MoP-only, harness-only, read-only): one synthetic hit of <amount> (default 100000)
    // from the named player to <target> (a player name, or * / @all = every other player on the named player's map, at most 40)
    // through this core's own UnitScript dispatch - sScriptMgr->ModifyMeleeDamage / ModifySpellDamageTaken /
    // ModifyPeriodicDamageAurasTick, the calls Unit::CalculateMeleeDamage, Unit::CalculateSpellDamageTaken and the periodic
    // damage tick make, so every registered damage script runs (Battleground.AnyLevel.PvPScaling's among them when it is
    // registered at startup) - and the heal through BattlegroundAnyLevel::ScaleHealing, the first step of
    // Unit::ApplyPvPHealMods. Nothing is dealt or healed: the amounts are locals. One line per pair:
    //   botscenario: pvpscale <src> lvl=<n> -> <tgt> lvl=<n> map=<id> tmap=<id> bg=<0|1> bgmin=<n|-> enabled=<0|1>
    //   amount=<n> melee=<n> spell=<n> periodic=<n|-> heal=<n>
    // Scaling on: a pair inside one battleground with different levels, one of them below bgmin, reads
    // round(amount x baseHP(tgt lvl) / baseHP(src lvl)) on all four (BattlegroundAnyLevel.cpp PairMultiplier); any other
    // pair, and every pair with the key off, reads amount. The direct spell hit passes no SpellInfo (the any-level hook does
    // not use it there, and spellwatch / AutoBalance return at once on a null one); the periodic tick passes Shadow Word:
    // Pain (589, a harmful aura) because that hook skips positive spells.
    std::string PvPScaleLine(Player* source, Player* target, uint32 amount, SpellInfo const* dot)
    {
        uint32 melee = amount;
        sScriptMgr->ModifyMeleeDamage(target, source, melee);
        int32 spell = int32(amount);
        sScriptMgr->ModifySpellDamageTaken(target, source, spell, nullptr);
        std::string periodic = "-";
        if (dot)
        {
            int32 tick = int32(amount);
            sScriptMgr->ModifyPeriodicDamageAurasTick(target, source, tick, dot);
            periodic = std::to_string(tick);
        }
        uint32 heal = amount;
        BattlegroundAnyLevel::ScaleHealing(source, target, heal);

        Map* map = source->GetMap();
        Battleground* bg = (map && map->IsBattleground()) ? static_cast<BattlegroundMap*>(map)->GetBG() : nullptr;
        return "botscenario: pvpscale " + source->GetName() + " lvl=" + std::to_string(uint32(source->GetLevel())) + " -> " +
            target->GetName() + " lvl=" + std::to_string(uint32(target->GetLevel())) + " map=" + std::to_string(source->GetMapId()) +
            " tmap=" + std::to_string(target->GetMapId()) + " bg=" + (bg ? "1" : "0") +
            " bgmin=" + (bg ? std::to_string(bg->GetMinLevel()) : std::string("-")) +
            " enabled=" + (BattlegroundAnyLevel::Enabled() ? "1" : "0") + " amount=" + std::to_string(amount) +
            " melee=" + std::to_string(melee) + " spell=" + std::to_string(spell) + " periodic=" + periodic +
            " heal=" + std::to_string(heal);
    }

    bool HandlePvPScale(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* source = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!source || !source->IsInWorld())
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }

        std::string other;
        uint32 amount = 100000;
        in >> other;
        uint32 a = 0;
        if (in >> a)
            amount = a < 1 ? 1 : (a > 1000000000u ? 1000000000u : a);
        if (other.empty())
        {
            ReplyLine(handler, "botscenario: usage: botscenario <player> pvpscale <target|*|@all> [amount]");
            return false;
        }

        SpellInfo const* dot = sSpellMgr->GetSpellInfo(589);
        if (other != "*" && other != "@all")
        {
            Player* target = ObjectAccessor::FindPlayerByName(other);
            if (!target || !target->IsInWorld() || target == source)
            {
                ReplyLine(handler, "botscenario: pvpscale no other online player named '" + other + "'");
                return false;
            }
            ReplyLine(handler, PvPScaleLine(source, target, amount, dot));
            return true;
        }

        uint32 n = 0;
        Map::PlayerList const& players = source->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end() && n < 40; ++itr)
        {
            Player* target = itr->GetSource();
            if (!target || target == source || !target->IsInWorld())
                continue;
            ReplyLine(handler, PvPScaleLine(source, target, amount, dot));
            ++n;
        }
        if (!n)
            ReplyLine(handler, "botscenario: pvpscale " + name + " no other player on map " + std::to_string(source->GetMapId()));
        return true;
    }

    // #830 `lfgreward <dungeonId>` (MoP-only, read-only): the random-dungeon reward LFGMgr::FinishDungeon would give this
    // character for that dungeon finder entry now - LFGMgr::GetRandomDungeonReward at the character's level, which with
    // LFG.AnyLevel.Loot = 1 maps an entry outside the level range to the random 5-man reward of the character's own level.
    // firstDone=1: the daily first-run quest is already rewarded, FinishDungeon would give `other`.
    bool HandleLfgReward(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            ReplyLine(handler, "botscenario: no online player named '" + name + "'");
            return false;
        }

        uint32 dungeonId = 0;
        if (!(in >> dungeonId) || !dungeonId)
        {
            ReplyLine(handler, "botscenario: usage: botscenario <player> lfgreward <dungeonId>");
            return false;
        }

        std::string const head = "botscenario: lfgreward " + name + " dungeon=" + std::to_string(dungeonId) +
            " level=" + std::to_string(uint32(target->GetLevel()));
        lfg::LfgReward const* reward = sLFGMgr->GetRandomDungeonReward(dungeonId, uint8(target->GetLevel()));
        if (!reward)
        {
            ReplyLine(handler, head + " none");
            return true;
        }

        Quest const* first = sObjectMgr->GetQuestTemplate(reward->firstQuest);
        bool const firstDone = first && !target->CanRewardQuest(first, false);
        ReplyLine(handler, head + " maxLevel=" + std::to_string(reward->maxLevel) + " first=" + std::to_string(reward->firstQuest) +
            " other=" + std::to_string(reward->otherQuest) + " firstDone=" + (firstDone ? "1" : "0"));
        return true;
    }
    // #895 `ratedbg <group|join|state|test on|off>` (MoP-only; AC has no rated BG). Beyond AC: rated battlegrounds for bots.
    //  - join: the group leader <player> files the group for the rated BG exactly as the client's
    //    CMSG_BATTLEMASTER_JOIN_RATED does (WorldSession::HandleBattlemasterJoinRated, BattleGroundHandler.cpp:773-852;
    //    the 5.4.8 packet is empty). The handler returns silently on every failure, so the same preconditions are
    //    pre-checked here and reported (SOAP-FAULT), and the result is read back from the player's queue slot.
    //  - state: read-only. Queue status, current BG type id / RandomTypeID / rated flag, rated-BG rating row
    //    (RatedPvpMgr PVP_SLOT_RATED_BG, ArenaTeam.cpp:23-39,90) and Conquest points (currency 390).
    //  - test on|off: `.debug ratedbg` (cs_debug.cpp:873) is Console::No, so this sets the same switch
    //    (BattlegroundMgr::ToggleRatedBgTesting; team size 2 instead of 10) over SOAP.
    bool HandleRatedBg(ChatHandler* handler, std::string const& name, std::istringstream& in)
    {
        std::string sub;
        in >> sub;

        if (sub == "test")
        {
            std::string mode;
            in >> mode;
            if (mode != "on" && mode != "off")
            {
                handler->SendSysMessage("botscenario: usage: botscenario - ratedbg test on|off");
                return false;
            }
            if (sBattlegroundMgr->IsRatedBgTesting() != (mode == "on"))
                sBattlegroundMgr->ToggleRatedBgTesting();
            handler->PSendSysMessage("botscenario: ratedbg test %s", sBattlegroundMgr->IsRatedBgTesting() ? "on" : "off");
            return true;
        }

        Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
        if (!target)
        {
            handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
            return false;
        }

        BattlegroundQueueTypeId const ratedQueue = BattlegroundMgr::BGQueueTypeId(BATTLEGROUND_RATED_10_VS_10, 0);

        if (sub == "group")
        {
            // test setup, same construction as the harness BG stage (BotScenario.cpp:2453-2458): <player> leads a new
            // group made of the named online players. Faction mixing inside one group is refused by the core, so the
            // members must share the leader's faction.
            if (target->GetGroup())
            {
                handler->SendSysMessage("botscenario: ratedbg group: leader is already grouped (leavegroup first)");
                return false;
            }
            std::vector<Player*> members;
            std::string mn;
            while (in >> mn)
            {
                Player* m = ObjectAccessor::FindPlayerByName(mn);
                if (!m || m == target || m->GetGroup() || m->GetTeam() != target->GetTeam())
                {
                    handler->SendSysMessage(("botscenario: ratedbg group: member '" + mn + "' is offline, grouped, same as the leader or the other faction").c_str());
                    return false;
                }
                members.push_back(m);
            }
            Group* group = new Group();
            group->Create(target);
            sGroupMgr->AddGroup(group);
            for (Player* m : members)
                group->AddMember(m);
            handler->PSendSysMessage("botscenario: ratedbg group ok leader=%s members=%u", name.c_str(), group->GetMembersCount());
            return true;
        }

        if (sub == "join")
        {
            Battleground* tmpl = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_RATED_10_VS_10);
            if (!tmpl)
            {
                handler->SendSysMessage("botscenario: ratedbg join: no rated BG template (BattlemasterList 100 / battleground_template)");
                return false;
            }
            if (target->InBattleground())
            {
                handler->SendSysMessage("botscenario: ratedbg join: player is already in a battleground");
                return false;
            }
            Group* group = target->GetGroup();
            if (!group || group->GetLeaderGUID() != target->GetGUID())
            {
                handler->SendSysMessage("botscenario: ratedbg join: player is not a group leader");
                return false;
            }
            uint32 const need = sBattlegroundMgr->IsRatedBgTesting() ? 2 : tmpl->GetMaxPlayersPerTeam();
            if (group->GetMembersCount() != need)
            {
                handler->PSendSysMessage("botscenario: ratedbg join: group has %u members, rated BG needs exactly %u (ratedbg test %s)",
                    group->GetMembersCount(), need, sBattlegroundMgr->IsRatedBgTesting() ? "on" : "off");
                return false;
            }
            if (!sDBCManager.GetBattlegroundBracketByLevel(tmpl->GetMapId(), target->GetLevel()))
            {
                handler->SendSysMessage("botscenario: ratedbg join: no PvPDifficulty bracket for the leader's level");
                return false;
            }

            WorldPacket packet(CMSG_BATTLEMASTER_JOIN_RATED, 0);
            target->GetSession()->HandleBattlemasterJoinRated(packet);

            if (!target->InBattlegroundQueueForBattlegroundQueueType(ratedQueue))
            {
                handler->SendSysMessage("botscenario: ratedbg join: core refused (CanJoinBattlegroundQueue: faction/level/size/deserter or already queued)");
                return false;
            }
            handler->PSendSysMessage("botscenario: ratedbg join ok leader=%s members=%u queue=%u", name.c_str(),
                group->GetMembersCount(), uint32(ratedQueue));
            return true;
        }

        if (sub == "state")
        {
            Battleground* bg = target->GetBattleground();
            RatedPvpInfo const* info = RatedPvpMgr::Instance()->GetInfo(PVP_SLOT_RATED_BG, target->GetGUID());
            handler->PSendSysMessage("RATEDBG player=%s queued=%u invited=%u inbg=%u bgtype=%u randomtype=%u isratedbg=%u status=%u "
                "rating=%u mmr=%u weekgames=%u seasongames=%u seasonwins=%u winstreak=%u conquest=%u testing=%u",
                name.c_str(), target->InBattlegroundQueueForBattlegroundQueueType(ratedQueue) ? 1 : 0,
                target->IsInvitedForBattlegroundQueueType(ratedQueue) ? 1 : 0, bg ? bg->GetInstanceID() : 0,
                bg ? uint32(bg->GetTypeID()) : 0, bg ? uint32(bg->GetTypeID(true)) : 0, bg && bg->IsRatedBG() ? 1 : 0,
                bg ? uint32(bg->GetStatus()) : 0, info ? info->Rating : 0, info ? info->MatchmakerRating : 0,
                info ? info->WeekGames : 0, info ? info->SeasonGames : 0, info ? info->SeasonWins : 0,
                info ? info->WinStreak : 0, target->GetCurrency(390, false), sBattlegroundMgr->IsRatedBgTesting() ? 1 : 0); // 390 = Conquest Points
            return true;
        }

        handler->SendSysMessage("botscenario: usage: botscenario <player> ratedbg <group|join|state|test on|off>");
        return false;
    }
}  // namespace

// Called from BotScenario.cpp's HandleScenarioCommand once these verb names are recognised there -
// see this file's header for the exact hook text to add once BotScenario.cpp is free to edit.
bool HandleQueueScenarioCommand(ChatHandler* handler, std::string const& name, std::string const& scenario,
                                 std::istringstream& in)
{
    if (scenario == "spec")
        return HandleSpec(handler, name, in);
    if (scenario == "bgtest")
        return HandleBgTest(handler, in);
    if (scenario == "bgend")
        return HandleBgEnd(handler, name, in);
    if (scenario == "lfgstate")
        return HandleLfgState(handler, name);
    if (scenario == "queuegate")
        return HandleQueueGate(handler, in);
    if (scenario == "clearqueue")
        return HandleClearQueue(handler, name);
    if (scenario == "lfgjoin")
        return HandleLfgJoin(handler, name, in);
    if (scenario == "role")                     // #598
        return HandleRole(handler, name);
    if (scenario == "lfgleave")                 // #833
        return HandleLfgLeave(handler, name);
    if (scenario == "leavegroup")
        return HandleLeaveGroup(handler, name);
    if (scenario == "acceptinvite")
        return HandleAcceptInvite(handler, name);
    if (scenario == "unbind")
        return HandleUnbind(handler, name, in);
    if (scenario == "instinfo")
        return HandleInstInfo(handler, name);
    if (scenario == "additem")
        return HandleAddItem(handler, name, in);
    if (scenario == "strategy")
        return HandleStrategy(handler, name, in);
    if (scenario == "imbueinfo")
        return HandleImbueInfo(handler, name);
    if (scenario == "questreward")
        return HandleQuestReward(handler, name, in);
    if (scenario == "engage")
        return HandleEngage(handler, name, in);
    if (scenario == "hasitem")
        return HandleHasItem(handler, name, in);
    if (scenario == "useitem")
        return HandleUseItem(handler, name, in);
    if (scenario == "despawnnear")
        return HandleDespawnNear(handler, name, in);
    if (scenario == "encounter")
        return HandleEncounter(handler, name, in);
    if (scenario == "bosshp")
        return HandleBossHp(handler, name, in);
    if (scenario == "abinfo")
        return HandleAbInfo(handler, name, in);
    if (scenario == "lfgreward")                // #830
        return HandleLfgReward(handler, name, in);
    if (scenario == "pvpscale")                 // #831
        return HandlePvPScale(handler, name, in);
    if (scenario == "cansee")                   // #519
        return HandleCanSee(handler, name, in);
    if (scenario == "dismount")
        return HandleDismount(handler, name);
    if (scenario == "setfly")
        return HandleSetFly(handler, name, in);
    if (scenario == "unaura")
        return HandleUnaura(handler, name, in);
    if (scenario == "bgqueueinfo")
        return HandleBgQueueInfo(handler, name);
    if (scenario == "dgstate")
        return HandleDgState(handler, name);
    if (scenario == "dgdropcart")
        return HandleDgDropCart(handler, name);
    if (scenario == "dgpick")
        return HandleDgPick(handler, name);
    if (scenario == "dgtake")
        return HandleDgTake(handler, name);
    if (scenario == "ratedbg")                  // #895
        return HandleRatedBg(handler, name, in);

    return false;
}

// True while a 1:1 LFG/BG cast wants every bot's own BG self-queue refused, unconditionally
// (including an already-registered/masterprobed roster bot - see this file's "queuegate" doc comment
// for why that distinction mattered). Consulted from BotScenario.cpp's BgJoinBlocked(); a future
// LfgJoinBlocked (#142) can reuse this same accessor.
bool QueueGateOn() { return g_queueGateHolders.load(std::memory_order_relaxed) > 0; }

void RegisterQueueScenarioHandlers()
{
    // Deliberately empty - see this file's header ("Linkage") for why it still needs to exist and be
    // called.
}

}  // namespace BotScenario
