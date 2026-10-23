/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/NearestNpcsValue.h) into
 * the Legends of Azeroth Pandaria fork.
 *
 * This value was referenced by the death/revive actions but never registered in this fork's
 * ValueContext. AI_VALUE expands to `context->GetValue<T>(name)->Get()`, so every one of those
 * call sites dereferenced a null pointer - see ReleaseSpiritAction / ReviveFromCorpseAction.
 */

#ifndef _PLAYERBOT_NEARESTNPCSVALUE_H
#define _PLAYERBOT_NEARESTNPCSVALUE_H

#include "NearestUnitsValue.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

/// Every non-player unit within sight range of the bot.
class NearestNpcsValue : public NearestUnitsValue
{
public:
    NearestNpcsValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->sightDistance)
        : NearestUnitsValue(botAI, "nearest npcs", range)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

// AC Ai/Base/Value/NearestNpcsValue.h:28-39 (#545)
class NearestHostileNpcsValue : public NearestUnitsValue
{
public:
    NearestHostileNpcsValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->sightDistance)
        : NearestUnitsValue(botAI, "nearest hostile npcs", range)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

/// Every vehicle that is alive and still has a free seat, within sight range of the bot.
/// 1:1 with AC mod-playerbots Ai/Base/Value/NearestNpcsValue.h:41-52 (ctor) and .cpp:41-58
/// (FindUnits / AcceptUnit). Registered as "nearest vehicles" in ValueContext.h; read by
/// EnterVehicleAction::Execute (VehicleActions.cpp) and, in AC, by PvpTriggers.cpp:307,
/// TOCActions.cpp:185 and ICCActions_GSB.cpp:114.
class NearestVehiclesValue : public NearestUnitsValue
{
public:
    NearestVehiclesValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->sightDistance)
        : NearestUnitsValue(botAI, "nearest vehicles", range)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

#endif
