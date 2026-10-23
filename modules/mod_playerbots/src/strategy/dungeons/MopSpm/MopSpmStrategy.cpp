/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSpm strategy "mop-spm" (epic #373, issue #653).

#include "MopSpmStrategy.h"
#include "MopSpmMultipliers.h"
#include "MopSpmShared.h"
#include "Playerbots.h"
#include "Timer.h"

void MopSpmStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Ground mechanics (Static Field, Invoke Lightning spread, Lightning Breath, Fists of Fury, Tornado Kick, Chase Down,
    // Whirling Steel, Ring of Malice) go through one danger trigger so the bot never steps out of one hazard into another.
    triggers.push_back(new TriggerNode("mop spm danger",
        NextAction::array(0, new NextAction("mop spm avoid", ACTION_EMERGENCY + 8), nullptr)));

    // Master Snowdrift's Parry Stance: stop hitting him.
    triggers.push_back(new TriggerNode("mop spm parry stance",
        NextAction::array(0, new NextAction("mop spm parry stop", ACTION_EMERGENCY + 6), nullptr)));

    // Taran Zhu's Haze of Hate: Meditate.
    triggers.push_back(new TriggerNode("mop spm haze",
        NextAction::array(0, new NextAction("mop spm meditate", ACTION_EMERGENCY + 7), nullptr)));

    // Gripping Hatred / the Azure Serpent: skull mark, same level as MgT's focus marks.
    triggers.push_back(new TriggerNode("mop spm priority",
        NextAction::array(0, new NextAction("mop spm mark priority", ACTION_RAID + 2), nullptr)));
}

void MopSpmStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MopSpmAvoidMultiplier(botAI));
    multipliers.push_back(new MopSpmParryMultiplier(botAI));
}

void MopSpmStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type == TargetValueExclusionType::None)
        return;

    Player* bot = botAI->GetBot();
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MopSpm::MAP_SPM)
        return;

    uint32 const now = getMSTime();
    if (!_cacheValid || now - _cacheTime >= MopSpm::EXCLUSION_CACHE_MS)
    {
        _cache.clear();
        MopSpm::CollectExclusions(bot, _cache);
        _cacheTime = now;
        _cacheValid = true;
    }

    exclusions.insert(_cache.begin(), _cache.end());
}
