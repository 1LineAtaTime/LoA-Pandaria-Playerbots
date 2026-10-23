/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopTjs strategy "mop-tjs" (epic #373, issue #651).

#include "MopTjsStrategy.h"
#include "MopDungeonShared.h"
#include "MopTjsMultipliers.h"
#include "MopTjsShared.h"
#include "Playerbots.h"
#include "Timer.h"

void MopTjsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Every ground mechanic (Sha Residue, Hydrolance, Wash Away, the Liu waves, Touch of Nothingness) goes through
    // one danger trigger so the bot never steps out of one hazard into another.
    triggers.push_back(new TriggerNode("mop tjs danger",
        NextAction::array(0, new NextAction("mop tjs avoid", ACTION_EMERGENCY + 8), nullptr)));
    // #781 phase switch (AC FoS "switch to soul fragment" shape, FoSStrategy.cpp:14): Liu -> Yu'lon at 30%.
    triggers.push_back(new TriggerNode("mop tjs switch to yulon",
        NextAction::array(0, new NextAction("mop tjs switch to yulon", ACTION_RAID + 3), nullptr)));
    // #815 Wise Mari phase 1 (AC UK "keleseth frost tomb" -> "attack frost tomb", UKStrategy.cpp:13, ACTION_RAID + 1):
    // kill the Corrupt Living Waters even when none of them shows up in "attackers" (out of sight of every bot).
    triggers.push_back(new TriggerNode("mop tjs living water",
        NextAction::array(0, new NextAction("mop tjs attack living water", ACTION_RAID + 1), nullptr)));
    // #651 Lorewalker Stonestep (same AC UK Frost Tomb shape, ACTION_RAID + 1): the Suns, corrupted Zao and Strife/Peril
    // never put a player on their threat list, so the bots go for them by entry.
    triggers.push_back(new TriggerNode("mop tjs lorewalker",
        NextAction::array(0, new NextAction("mop tjs attack lorewalker", ACTION_RAID + 1), nullptr)));
}

void MopTjsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MopTjsAvoidMultiplier(botAI));
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "mop tjs switch to yulon", MopTjs::YulonSwitch()));
}

void MopTjsStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type == TargetValueExclusionType::None)
        return;

    Player* bot = botAI->GetBot();
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MopTjs::MAP_TJS)
        return;

    bool const tank = type == TargetValueExclusionType::Tank;
    ExclusionCache& cache = _cache[tank ? 0 : 1];
    uint32 const now = getMSTime();
    if (!cache.valid || now - cache.time >= MopTjs::EXCLUSION_CACHE_MS)
    {
        cache.guids.clear();
        MopTjs::CollectExclusions(bot, tank, cache.guids);
        cache.time = now;
        cache.valid = true;
    }

    exclusions.insert(cache.guids.begin(), cache.guids.end());
}
