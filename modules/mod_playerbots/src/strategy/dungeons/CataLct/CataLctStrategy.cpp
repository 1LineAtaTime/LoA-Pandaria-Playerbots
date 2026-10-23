/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Lost City of the Tol'vir (epic #373, issue #666). See CataLctShared.h for sources and ids.

#include "CataLctStrategy.h"
#include "CataLctMultipliers.h"
#include "CataLctShared.h"
#include "Playerbots.h"
#include "Timer.h"

void CataDungeonLctStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("lct land mine",
        NextAction::array(0, new NextAction("lct avoid land mine", ACTION_EMERGENCY + 6), nullptr)));

    // #680: Husam's Shockwave lines and the Bad Intentions wall throw (re-enabled in boss_general_husam.cpp)
    triggers.push_back(new TriggerNode("lct shockwave",
        NextAction::array(0, new NextAction("lct avoid shockwave", ACTION_EMERGENCY + 6), nullptr)));

    triggers.push_back(new TriggerNode("lct bad intentions",
        NextAction::array(0, new NextAction("lct drop from wall", ACTION_EMERGENCY + 7), nullptr)));

    // #820: a bot that fell off Siamat's platform (no navmesh on the destructible GO) goes back up first
    triggers.push_back(new TriggerNode("lct off platform",
        NextAction::array(0, new NextAction("lct return to platform", ACTION_EMERGENCY + 8), nullptr)));

    triggers.push_back(new TriggerNode("lct dust flail",
        NextAction::array(0, new NextAction("lct leave dust flail", ACTION_EMERGENCY + 4), nullptr)));

    triggers.push_back(new TriggerNode("lct frenzied crocolisk",
        NextAction::array(0, new NextAction("lct kill crocolisk", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("lct heavens fury",
        NextAction::array(0, new NextAction("lct avoid heavens fury", ACTION_EMERGENCY + 6), nullptr)));

    triggers.push_back(new TriggerNode("lct soul fragment",
        NextAction::array(0, new NextAction("lct kill soul fragment", ACTION_RAID + 2), nullptr)));

    // #666 w68: Repentance - out of Hallowed Ground first, then everyone on the Harbinger (Soul Fragments outrank it for dps)
    triggers.push_back(new TriggerNode("lct hallowed ground",
        NextAction::array(0, new NextAction("lct avoid hallowed ground", ACTION_EMERGENCY + 5), nullptr)));

    triggers.push_back(new TriggerNode("lct harbinger",
        NextAction::array(0, new NextAction("lct kill harbinger", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("lct blaze fire",
        NextAction::array(0, new NextAction("lct avoid blaze fire", ACTION_EMERGENCY + 5), nullptr)));

    triggers.push_back(new TriggerNode("lct siamat storm",
        NextAction::array(0, new NextAction("lct avoid siamat storm", ACTION_EMERGENCY + 6), nullptr)));

    triggers.push_back(new TriggerNode("lct siamat servant",
        NextAction::array(0, new NextAction("lct kill servant", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("lct lightning charge",
        NextAction::array(0, new NextAction("lct avoid lightning charge", ACTION_EMERGENCY + 7), nullptr)));
}

void CataDungeonLctStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new LctAvoidMultiplier(botAI));
    multipliers.push_back(new LctFocusMultiplier(botAI));
}

// #819 (High Prophet Barim, heroic): damage dealers and DoT pickers skip Blaze of the Heavens (48906) while Barim is alive and
// attackable; the tank target keeps it (the phoenix may chase the tank). #666 w68: also during Repentance, and then the tank
// skips the egg too (the Harbinger is the stage's only target). Cached because the target values ask several times per tick
// and the answer needs a grid search.
void CataDungeonLctStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type != TargetValueExclusionType::Dps && type != TargetValueExclusionType::Attacker &&
        type != TargetValueExclusionType::Tank)
        return;

    Player* bot = botAI->GetBot();
    if (!bot || !bot->IsAlive() || bot->GetMapId() != CataLct::MAP_ID)
        return;

    uint32 const now = getMSTime();
    if (!_cacheValid || now - _cacheTime >= 300)
    {
        _cache.clear();
        _tankCache.clear();
        CataLct::CollectBlazeExclusions(bot, _cache, _tankCache);
        _cacheTime = now;
        _cacheValid = true;
    }

    if (type == TargetValueExclusionType::Tank)
        exclusions.insert(_tankCache.begin(), _tankCache.end());
    else
        exclusions.insert(_cache.begin(), _cache.end());
}
