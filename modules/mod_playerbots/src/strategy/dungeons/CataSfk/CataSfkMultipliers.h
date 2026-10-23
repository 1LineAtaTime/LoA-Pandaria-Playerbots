/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep (epic #373, issue #668). See CataSfkShared.h for sources and ids.

#ifndef PLAYERBOTS_CATASFKMULTIPLIERS_H
#define PLAYERBOTS_CATASFKMULTIPLIERS_H

#include "Define.h"
#include "Multiplier.h"

// While a bot stands in one of this dungeon's ground effects / cones only the avoid actions may move it. Walden, heroic:
// no rooting cast while Toxic Coagulant ticks on the bot, no movement but the poison escape while the red cauldron is up.
// Godfrey, heroic: no class interrupt spent on Pistol Barrage (it cannot be interrupted; Cursed Bullets follows it).
class SfkAvoidMultiplier : public Multiplier
{
public:
    SfkAvoidMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata-sfk avoid") {}

    float GetValue(Action* action) override;

private:
    uint32 _lastCheck = 0;
    bool _danger = false;
    bool _coagulant = false;
    bool _catalyst = false;
    bool _barrage = false;
};

#endif
