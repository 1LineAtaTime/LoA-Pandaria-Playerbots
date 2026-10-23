/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Gurub (Cataclysm 5-man) multipliers, MoP-only lane L7 (#670). Pattern: AC Ai/Dungeon/HoS/HoSMultipliers.h and
// MgTMultipliers.h (zero the movement / target pickers that would walk a bot back into a hazard).

#ifndef PLAYERBOTS_CATAZGMULTIPLIERS_H
#define PLAYERBOTS_CATAZGMULTIPLIERS_H

#include "Multiplier.h"
#include "Define.h"

// Evaluates its condition at most every 100 ms: GetValue() runs for every action of every tick and the
// conditions below do grid searches.
class CataZgCachedMultiplier : public Multiplier
{
public:
    CataZgCachedMultiplier(PlayerbotAI* botAI, std::string const name) : Multiplier(botAI, name) {}

protected:
    bool Cached();
    virtual bool Compute() = 0;

private:
    uint32 _stamp = 0;
    bool _value = false;
    bool _valid = false;
};

// Inside (or at the edge of) any ground hazard: no formation / follow / reach movement.
class CataZgHazardMultiplier : public CataZgCachedMultiplier
{
public:
    CataZgHazardMultiplier(PlayerbotAI* botAI) : CataZgCachedMultiplier(botAI, "zg hazard") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// Venoxis (#679 w106): the class interrupt actions stand down for the whole fight - they spent kick / pummel on his other casts and on
// the not-yet-live Whispers channel, so no interrupt was ready for the next Whispers of Hethiss. "zg interrupt" does Whispers.
class CataZgWhispersInterruptMultiplier : public CataZgCachedMultiplier
{
public:
    CataZgWhispersInterruptMultiplier(PlayerbotAI* botAI) : CataZgCachedMultiplier(botAI, "zg whispers interrupt") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// Venoxis (#670 w68): he stands in his Poison Clouds - no reach / formation move after him.
class CataZgVenoxisCloudMultiplier : public CataZgCachedMultiplier
{
public:
    CataZgVenoxisCloudMultiplier(PlayerbotAI* botAI) : CataZgCachedMultiplier(botAI, "zg venoxis cloud") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// Toxic Link: the formation must not walk the pair back together.
class CataZgLinkMultiplier : public CataZgCachedMultiplier
{
public:
    CataZgLinkMultiplier(PlayerbotAI* botAI) : CataZgCachedMultiplier(botAI, "zg link") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// Shadows of Hakkar: stay in the Deadzone until it wears off.
class CataZgShadowsMultiplier : public CataZgCachedMultiplier
{
public:
    CataZgShadowsMultiplier(PlayerbotAI* botAI) : CataZgCachedMultiplier(botAI, "zg shadows") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// Jin'do phase 2: on the way to / standing on the chain, no formation / follow / reach movement pulls the bot off it.
class CataZgChainMultiplier : public CataZgCachedMultiplier
{
public:
    CataZgChainMultiplier(PlayerbotAI* botAI) : CataZgCachedMultiplier(botAI, "zg chain") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// Jin'do phase 2: while a chain is still shielded the non-tank bots never pick the tank's target (the Gurubashi Spirit).
class CataZgSpareSpiritMultiplier : public CataZgCachedMultiplier
{
public:
    CataZgSpareSpiritMultiplier(PlayerbotAI* botAI) : CataZgCachedMultiplier(botAI, "zg spare spirit") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// Zanzil (#822): a group member carries the Frostburn Formula aura 96325 (a dispellable Magic buff): no Purify / Dispel Magic /
// Cleanse until its charge is used, or the healer strips it before the mage's spell procs it.
class CataZgFormulaMultiplier : public CataZgCachedMultiplier
{
public:
    CataZgFormulaMultiplier(PlayerbotAI* botAI) : CataZgCachedMultiplier(botAI, "zg formula") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

// A focus target exists: the generic target pickers must not switch the bot off it.
class CataZgFocusMultiplier : public CataZgCachedMultiplier
{
public:
    CataZgFocusMultiplier(PlayerbotAI* botAI) : CataZgCachedMultiplier(botAI, "zg focus") {}
    float GetValue(Action* action) override;

protected:
    bool Compute() override;
};

#endif
