/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines (epic #373, issue #667). See CataDmShared.h for sources and ids.

#ifndef PLAYERBOTS_CATADMMULTIPLIERS_H
#define PLAYERBOTS_CATADMMULTIPLIERS_H

#include "Define.h"
#include "Multiplier.h"

// While a bot stands in one of this dungeon's ground effects / cones only the avoid actions may move it.
class DmAvoidMultiplier : public Multiplier
{
public:
    DmAvoidMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata-dm avoid") {}

    float GetValue(Action* action) override;

private:
    uint32 _lastCheck = 0;
    bool _danger = false;
};

#endif
