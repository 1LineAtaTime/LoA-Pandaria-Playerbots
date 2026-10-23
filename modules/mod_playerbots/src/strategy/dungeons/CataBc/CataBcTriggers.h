/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#ifndef PLAYERBOTS_CATABCTRIGGERS_H
#define PLAYERBOTS_CATABCTRIGGERS_H

#include "CataBcShared.h"
#include "Trigger.h"

// Rom'ogg: The Skullcracker wind-up, everyone free to move leaves the 12 yd blast
class CataBcSkullcrackerTrigger : public Trigger
{
public:
    CataBcSkullcrackerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata bc skullcracker") {}
    bool IsActive() override;
};

// Rom'ogg: Chains of Woe root on the bot (the chains must be destroyed)
class CataBcChainsTrigger : public Trigger
{
public:
    CataBcChainsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata bc chains") {}
    bool IsActive() override;
};

// Rom'ogg: Quake aura on the bot (ground cracks under the player and ruptures after 3 s)
class CataBcQuakeTrigger : public Trigger
{
public:
    CataBcQuakeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata bc quake") {}
    bool IsActive() override;
};

// Beauty: standing in a Lava Drool puddle
class CataBcLavaDroolTrigger : public Trigger
{
public:
    CataBcLavaDroolTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata bc lava drool", 200) {}
    bool IsActive() override;
};

// Corla: the bot carries 60+ stacks of Evolution (100 = it transforms)
class CataBcEvolutionTrigger : public Trigger
{
public:
    CataBcEvolutionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata bc evolution") {}
    bool IsActive() override;
};

// Corla: a unit in reach is casting Dark Command
class CataBcInterruptTrigger : public Trigger
{
public:
    CataBcInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata bc interrupt") {}
    bool IsActive() override;
};

// Karsh: the tank manages the pillar of molten metal
class CataBcKarshPillarTrigger : public Trigger
{
public:
    CataBcKarshPillarTrigger(PlayerbotAI* botAI) : Trigger(botAI, "cata bc karsh pillar") {}
    bool IsActive() override;
};

#endif
