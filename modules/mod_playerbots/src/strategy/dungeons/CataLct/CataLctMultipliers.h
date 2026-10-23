/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Lost City of the Tol'vir (epic #373, issue #666). See CataLctShared.h for sources and ids.

#ifndef PLAYERBOTS_CATALCTMULTIPLIERS_H
#define PLAYERBOTS_CATALCTMULTIPLIERS_H

#include "Define.h"
#include "Multiplier.h"

// While a bot stands in one of this dungeon's ground effects / cones only the avoid actions may move it.
class LctAvoidMultiplier : public Multiplier
{
public:
    LctAvoidMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata-lct avoid") {}

    float GetValue(Action* action) override;

private:
    uint32 _lastCheck = 0;
    bool _danger = false;
};

// High Prophet Barim, Repentance (#666 w69): while the bot's current target is a living Soul Fragment or the Harbinger of
// Darkness, the generic target pickers ("dps assist", "dps aoe", "tank assist") and "drop target" may not switch it away.
// Same shape as fork CataBc/CataBcMultipliers.cpp CataBcChainsMultiplier (MgT MgTFocusOrderMultiplier).
class LctFocusMultiplier : public Multiplier
{
public:
    LctFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata-lct focus") {}

    float GetValue(Action* action) override;
};

#endif
