/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UBTRIGGERS_H
#define PLAYERBOTS_UBTRIGGERS_H

#include "Trigger.h"

// AC Ai/Dungeon/UB/UBTriggers.h:12
class UBFoulSporesTrigger : public Trigger
{
public:
    UBFoulSporesTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ub foul spores") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/UB/UBTriggers.h:19
class UBSporeCloudDangerTrigger : public Trigger
{
public:
    UBSporeCloudDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ub spore cloud danger") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/UB/UBTriggers.h:26
class UBUnderbatLashTrigger : public Trigger
{
public:
    UBUnderbatLashTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ub underbat lash") {}
    bool IsActive() override;
};

#endif
