/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Trigger/TravelTriggers.h)
 * into the Legends of Azeroth Pandaria fork, issue #269.
 */

#ifndef _PLAYERBOT_TRAVELTRIGGERS_H
#define _PLAYERBOT_TRAVELTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

class NoTravelTargetTrigger : public Trigger
{
public:
    NoTravelTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no travel target") {}

    bool IsActive() override;
};

class FarFromTravelTargetTrigger : public Trigger
{
public:
    FarFromTravelTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "far from travel target") {}

    bool IsActive() override;
};

// checkInterval 10: the fork's Trigger ctor (strategy/Trigger.cpp:14) uses the same rule as AC's
// (Bot/Engine/Trigger/Trigger.cpp:14): values 2..99 are seconds (x1000), so 10 = 10 s on both.
class NearDarkPortalTrigger : public Trigger
{
public:
    NearDarkPortalTrigger(PlayerbotAI* botAI) : Trigger(botAI, "near dark portal", 10) {}

    virtual bool IsActive();
};

class AtDarkPortalAzerothTrigger : public Trigger
{
public:
    AtDarkPortalAzerothTrigger(PlayerbotAI* botAI) : Trigger(botAI, "at dark portal azeroth", 10) {}

    bool IsActive() override;
};

class AtDarkPortalOutlandTrigger : public Trigger
{
public:
    AtDarkPortalOutlandTrigger(PlayerbotAI* botAI) : Trigger(botAI, "at dark portal outland", 10) {}

    bool IsActive() override;
};

#endif
