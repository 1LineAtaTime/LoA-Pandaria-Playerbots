/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man) multipliers, MoP-only lane L7 (#669). Pattern: AC Ai/Dungeon/HoS/HoSMultipliers.h and
// MgTMultipliers.h (zero the movement / target pickers that would walk a bot back into a hazard).

#ifndef PLAYERBOTS_CATAZAMULTIPLIERS_H
#define PLAYERBOTS_CATAZAMULTIPLIERS_H

#include "Multiplier.h"
#include "Define.h"

// Evaluates its condition at most every 100 ms: GetValue() runs for every action of every tick and the
// conditions below do grid searches.
class CataZaCachedMultiplier : public Multiplier
{
public:
    CataZaCachedMultiplier(PlayerbotAI* botAI, std::string const name) : Multiplier(botAI, name) {}

protected:
    bool Cached();
    virtual bool Compute() = 0;

private:
    uint32 _stamp = 0;
    bool _value = false;
    bool _valid = false;
};

// Inside (or at the edge of) any ground hazard: no formation / follow / reach movement.
class CataZaHazardMultiplier : public CataZaCachedMultiplier
{
public:
    CataZaHazardMultiplier(PlayerbotAI* botAI) : CataZaCachedMultiplier(botAI, "za hazard") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// Electrical Storm is up: stay in (or run to) the eye.
class CataZaStormMultiplier : public CataZaCachedMultiplier
{
public:
    CataZaStormMultiplier(PlayerbotAI* botAI) : CataZaCachedMultiplier(botAI, "za storm") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// Static Disruption: the ranged formation must not pull the spread back together.
class CataZaSpreadMultiplier : public CataZaCachedMultiplier
{
public:
    CataZaSpreadMultiplier(PlayerbotAI* botAI) : CataZaCachedMultiplier(botAI, "za spread") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// Surge debuff: hold position next to the boss.
class CataZaSurgeMultiplier : public CataZaCachedMultiplier
{
public:
    CataZaSurgeMultiplier(PlayerbotAI* botAI) : CataZaCachedMultiplier(botAI, "za surge") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// A focus target exists: the generic target pickers must not switch the bot off it.
class CataZaFocusMultiplier : public CataZaCachedMultiplier
{
public:
    CataZaFocusMultiplier(PlayerbotAI* botAI) : CataZaCachedMultiplier(botAI, "za focus") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

#endif
