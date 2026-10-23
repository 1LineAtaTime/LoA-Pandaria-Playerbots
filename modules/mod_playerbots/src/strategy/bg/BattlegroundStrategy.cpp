/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Strategy/BattlegroundStrategy.cpp)
 * into the Legends of Azeroth Pandaria fork.
 */

#include "BattlegroundStrategy.h"

#include "BattleGroundJoinAction.h"
#include "Playerbots.h"

void BgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // AC's BGStrategy is a PassThroughStrategy, whose actions all fire at relevance 100
    // (Ai/Base/Strategy/PassThroughStrategy.h:17). Match that here so the queue/accept/strategy-swap
    // actions cannot lose to some other non-combat action the way a 2.0/3.0 relevance could
    // (bg-parity-stage-design.md fix 6).

    // Look for a queue worth joining now and then...
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("bg join", 100.0f), nullptr)));

    // ...and, while an invite is pending, re-request the battleground status (AC BattlegroundStrategy.cpp:13,
    // #396): the server answers with SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION, whose "bg status" packet trigger
    // (WorldPacketHandlerStrategy) runs BGStatusAction to accept it - a pending invite expires, so this keeps
    // re-firing (trigger check interval 10) until the bot is inside.
    triggers.push_back(
        new TriggerNode("bg invite active", NextAction::array(0, new NextAction("bg status check", 100.0f), nullptr)));

    // Entering or leaving a match has to swap the bot's whole strategy set; nothing else notices.
    triggers.push_back(
        new TriggerNode("timer", NextAction::array(0, new NextAction("bg strategy check", 100.0f), nullptr)));

    // Drop a queue that can no longer pop. Bots only join a queue to fill it for a real player, so
    // when that player is gone the bot would otherwise wait in it forever - and while it holds that
    // queue slot it cannot join a queue that does need filling.
    //
    // This DEVIATES from upstream AC, which registers "bg leave" as a command action and wires it to
    // no trigger, so nothing there drops a stale queue automatically either. Upstream is not
    // obviously bitten by it because a bot realm normally runs RandomBotAutoJoinBG, which keeps
    // opening queues and spawning instances, so a queue rarely stays dead. With auto-join off - the
    // sane setting on a realm with real players, and what this fork defaults to - it strands bots.
    // See issue #50 if this should instead be solved the way upstream implicitly does.
    triggers.push_back(
        new TriggerNode("bg queue dead", NextAction::array(0, new NextAction("bg leave", 4.0f), nullptr)));
}

void BattlegroundStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("bg waiting", NextAction::array(0, new NextAction("bg move to start", ACTION_BG), nullptr)));

    triggers.push_back(
        new TriggerNode("bg active", NextAction::array(0, new NextAction("bg move to objective", ACTION_BG), nullptr)));

    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("bg check objective", ACTION_BG + 1), nullptr)));

    // A corpse's objective is stale by the time it releases, so throw it away on death.
    triggers.push_back(new TriggerNode(
        "dead", NextAction::array(0, new NextAction("bg reset objective force", ACTION_EMERGENCY), nullptr)));
}

void WarsongStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", ACTION_EMERGENCY), nullptr)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("attack enemy flag carrier", ACTION_RAID + 1.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "team flagcarrier near", NextAction::array(0, new NextAction("bg protect fc", ACTION_RAID), nullptr)));

    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("bg use buff", ACTION_BG), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));

    // Carrying the flag outranks everything else the bot might wander off to do.
    triggers.push_back(new TriggerNode(
        "player has flag", NextAction::array(0, new NextAction("bg move to objective", ACTION_EMERGENCY), nullptr)));

    triggers.push_back(new TriggerNode(
        "timer bg", NextAction::array(0, new NextAction("bg reset objective force", ACTION_EMERGENCY), nullptr)));
}

void ArathiStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", ACTION_EMERGENCY), nullptr)));

    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("bg use buff", ACTION_BG), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
}

void EyeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", ACTION_EMERGENCY), nullptr)));

    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("bg use buff", ACTION_BG), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));

    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("attack enemy flag carrier", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode(
        "player has flag", NextAction::array(0, new NextAction("bg move to objective", ACTION_EMERGENCY), nullptr)));
}

void AlteracStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // AC-identical (Ai/Base/Strategy/BattlegroundStrategy.cpp:39-43).
    triggers.push_back(new TriggerNode(
        "alliance no snowfall gy",
        NextAction::array(0, new NextAction("bg move to objective", ACTION_EMERGENCY), nullptr)));

    triggers.push_back(new TriggerNode(
        "timer bg", NextAction::array(0, new NextAction("bg reset objective force", ACTION_EMERGENCY), nullptr)));
}

void IsleStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // AC-identical (Ai/Base/Strategy/BattlegroundStrategy.cpp:64-81) - same triggers, actions and
    // priorities; only the NextAction construction differs (this fork's NextAction::array form).
    // AC marks the priorities "TODO: Do Priorities", so they are kept exactly as AC has them.
    triggers.push_back(
        new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", ACTION_MOVE), nullptr)));
    triggers.push_back(
        new TriggerNode("timer", NextAction::array(0, new NextAction("enter vehicle", ACTION_MOVE + 8.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("random", NextAction::array(0, new NextAction("leave vehicle", ACTION_MOVE + 7.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("in vehicle", NextAction::array(0, new NextAction("hurl boulder", ACTION_MOVE + 9.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("in vehicle", NextAction::array(0, new NextAction("fire cannon", ACTION_MOVE + 9.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("in vehicle", NextAction::array(0, new NextAction("napalm", ACTION_MOVE + 9.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("enemy is close", NextAction::array(0, new NextAction("steam blast", ACTION_MOVE + 9.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("in vehicle", NextAction::array(0, new NextAction("ram", ACTION_MOVE + 9.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("enemy is close", NextAction::array(0, new NextAction("ram", ACTION_MOVE + 9.1f), nullptr)));
    triggers.push_back(new TriggerNode(
        "enemy out of melee", NextAction::array(0, new NextAction("steam rush", ACTION_MOVE + 9.2f), nullptr)));
    triggers.push_back(new TriggerNode(
        "in vehicle", NextAction::array(0, new NextAction("incendiary rocket", ACTION_MOVE + 9.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("in vehicle", NextAction::array(0, new NextAction("rocket blast", ACTION_MOVE + 9.0f), nullptr)));
    // AC: "this is bugged: it doesn't work, and stops glaive throw working (which is needed to take
    // down gate)" - blade salvo stays commented out, as on AC.
    // triggers.push_back(new TriggerNode("in vehicle", NextAction::array(0, new NextAction("blade salvo", ACTION_MOVE + 9.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("in vehicle", NextAction::array(0, new NextAction("glaive throw", ACTION_MOVE + 9.0f), nullptr)));
}

// #717 - Strand of the Ancients. No AC spec (AC has no SotA strategy): ArathiStrategy's set (AC
// BattlegroundStrategy.cpp:45-51 - the graveyard banners are captured like AB's) plus IsleStrategy's vehicle-weapon
// triggers (AC :64-81) for the two SotA vehicles: the Battleground Demolisher's Hurl Boulder (52338, a siege shot at
// the gate through IsSiegeShot) and the Antipersonnel Cannon's Rocket Blast (49872). Isle's "enter vehicle" /
// "leave vehicle" (timer / random) are left out: BgTacticsSA boards and leaves by role, and a random exit would drop
// a demolisher on its way to a gate. "ram" as in Isle (AC :73-74): the Battleground Demolisher's spell2 Ram (60206,
// #736 - restored in the world DB, AC creature_template_spell 28781 index 1).
void StrandStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", ACTION_EMERGENCY), nullptr)));
    // #750 (beyond AC - AC has no SotA tactics): "bg waiting" is checked every 30 s, too slow for the SotA defender
    // warm-up posts (BgTacticsSA StartSpot) in a 60 s warm-up. Harmless in the match: moveToStart returns false outside
    // STATUS_WAIT_JOIN. Wave 21: "often" (RandomTrigger, 1 in 5 every 2 s: ~10 s mean, long tail) still left defenders
    // standing at the wall-jump landing until the round began (Taranno 13 s, Henri 33 s after the prep jump, 0 "often"
    // pushes of this action in between - the next leg only starts from StartSpot). "timer" fires every second; StartSpot
    // paces itself (OnPrepLeg, BotDue), so this only shortens the wait between legs.
    triggers.push_back(
        new TriggerNode("timer", NextAction::array(0, new NextAction("bg move to start", ACTION_BG), nullptr)));
    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("bg use buff", ACTION_BG), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
    triggers.push_back(
        new TriggerNode("in vehicle", NextAction::array(0, new NextAction("hurl boulder", ACTION_MOVE + 9.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("in vehicle", NextAction::array(0, new NextAction("ram", ACTION_MOVE + 9.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("enemy is close", NextAction::array(0, new NextAction("ram", ACTION_MOVE + 9.1f), nullptr)));
    triggers.push_back(
        new TriggerNode("in vehicle", NextAction::array(0, new NextAction("rocket blast", ACTION_MOVE + 9.0f), nullptr)));
}

// #576-#579 - the Cataclysm/MoP-only battlegrounds. No AC spec: each reuses the trigger set of the AC
// strategy whose game it extends (Ai/Base/Strategy/BattlegroundStrategy.cpp:26-62), with only the
// triggers whose values cover the map. "enemy flagcarrier near" / "team flagcarrier near" / "player has
// flag" are left out where their values (FlagCarrierValue, PlayerHasFlag) do not know the map (they know
// WSG/TP/EotS and, since #798, Deepwind Gorge), so on the others they could never fire. The carried
// objectives are handled inside BGTactics (isCarryingObjective).

// Battle for Gilneas - ArathiStrategy's set (AC BattlegroundStrategy.cpp:45-51).
void GilneasStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("bg use buff", ACTION_BG), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
}

// Temple of Kotmogu - WarsongStrategy's set (AC :26-37) minus the flag-carrier-value triggers, plus its
// "timer bg" objective refresh.
void KotmoguStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("bg use buff", ACTION_BG), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
    triggers.push_back(new TriggerNode(
        "timer bg", NextAction::array(0, new NextAction("bg reset objective force", ACTION_EMERGENCY), nullptr)));
}

// Silvershard Mines - ArathiStrategy's set; the carts move, so also Warsong's "timer bg" refresh.
void SilvershardStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("bg use buff", ACTION_BG), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
    triggers.push_back(new TriggerNode(
        "timer bg", NextAction::array(0, new NextAction("bg reset objective force", ACTION_EMERGENCY), nullptr)));
}

// Deepwind Gorge - Arathi's node set plus Warsong's "timer bg" refresh and flag-carrier triggers (carts are stolen
// like flags; #798). The three flag-carrier triggers are WarsongStrategy's (AC Ai/Base/Strategy/
// BattlegroundStrategy.cpp:30,31,35) at AC's relevances: FlagCarrierValue and PlayerHasFlag::IsCapturingFlag know
// map 1105 (the per-team cart picker table of BattlegroundDG), so they can fire here.
void DeepwindStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("bg active", NextAction::array(0, new NextAction("bg check flag", ACTION_EMERGENCY), nullptr)));

    // AC :30 - a cart carrier hunts the thief carrying its own team's cart (action useful only while capturing).
    triggers.push_back(new TriggerNode(
        "enemy flagcarrier near",
        NextAction::array(0, new NextAction("attack enemy flag carrier", ACTION_RAID + 1.0f), nullptr)));

    // AC :31 - everyone else escorts the friendly cart carrier.
    triggers.push_back(new TriggerNode(
        "team flagcarrier near", NextAction::array(0, new NextAction("bg protect fc", ACTION_RAID), nullptr)));

    triggers.push_back(
        new TriggerNode("often", NextAction::array(0, new NextAction("bg use buff", ACTION_BG), nullptr)));
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("bg use buff", ACTION_MOVE), nullptr)));

    // AC :35 - carrying a cart outranks everything else (BGTactics::selectObjective sends a carrier to its base).
    triggers.push_back(new TriggerNode(
        "player has flag", NextAction::array(0, new NextAction("bg move to objective", ACTION_EMERGENCY), nullptr)));

    triggers.push_back(new TriggerNode(
        "timer bg", NextAction::array(0, new NextAction("bg reset objective force", ACTION_EMERGENCY), nullptr)));
}

void ArenaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // AC-identical (Ai/Base/Strategy/BattlegroundStrategy.cpp:83-87): one trigger, one action. An
    // arena has no objectives, so "arena tactics" is the whole strategy - AC does not give it the
    // per-map / objective triggers WarsongStrategy and friends have.
    triggers.push_back(new TriggerNode(
        "no possible targets", NextAction::array(0, new NextAction("arena tactics", ACTION_BG), nullptr)));
}