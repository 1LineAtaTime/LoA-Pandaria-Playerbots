/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// AC Ai/Dungeon/Ramp/RampMultipliers.h (#568, epic #569)
#ifndef PLAYERBOTS_RAMPMULTIPLIERS_H
#define PLAYERBOTS_RAMPMULTIPLIERS_H

#include "Multiplier.h"
#include "RampBossHelper.h"

// Omor the Unscarred

class OmorTreacheryAuraFleeFromPlayersMultiplier : public Multiplier
{
public:
    OmorTreacheryAuraFleeFromPlayersMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "omor treachery aura flee from players")
    {
    }
    float GetValue(Action* action) override;

private:
    OmorBossHelper helper;
};

// Vazruden & Nazan

class NazanSetTremorTotemMultiplier : public Multiplier
{
public:
    NazanSetTremorTotemMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "nazan set tremor totem")
    {
    }
    float GetValue(Action* action) override;
};

class NazanSetFireResistanceTotemMultiplier : public Multiplier
{
public:
    NazanSetFireResistanceTotemMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "nazan set fire resistance totem")
    {
    }
    float GetValue(Action* action) override;
};

class NazanSetFireResistanceAuraMultiplier : public Multiplier
{
public:
    NazanSetFireResistanceAuraMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "nazan set fire resistance aura")
    {
    }
    float GetValue(Action* action) override;
};

#endif
