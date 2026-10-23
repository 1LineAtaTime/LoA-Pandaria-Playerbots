/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity bot strategy "cata-woe" - multipliers (issue #672, epic #373). Pattern: AC Ai/Dungeon/HoS/HoSMultipliers.h.
// The hazard multiplier keeps a 100 ms cache: the engine asks it once per queued action, the checks behind it are grid searches.

#ifndef PLAYERBOTS_CATAWOEMULTIPLIERS_H
#define PLAYERBOTS_CATAWOEMULTIPLIERS_H

#include "Define.h"
#include "Multiplier.h"

// While a ground hazard needs the bot to move, only this strategy's own moving actions may move it
// (AC HoS KrystallusMultiplier shape).
class CataWoeHazardMultiplier : public Multiplier
{
public:
    CataWoeHazardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "well of eternity hazards") {}
    float GetValue(Action* action) override;

private:
    bool NeedsMove();

    uint32 _stamp = 0;
    bool _cached = false;
};

// Peroth'arn's Fel Decay: "Any healing done to the afflicted player causes the healer to suffer 50% of the amount healed as
// Shadow damage" (Wowpedia "Peroth'arn"). Healers hold their heals on the target until it is in real danger.
class CataWoeFelDecayMultiplier : public Multiplier
{
public:
    CataWoeFelDecayMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "perotharn fel decay") {}
    float GetValue(Action* action) override;
};

#endif
