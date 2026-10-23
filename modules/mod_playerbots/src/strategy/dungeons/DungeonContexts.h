/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// #545 (epic #569): the dungeon half of AC Bot/Engine/BuildShared{Strategy,Action,Trigger,Value}Contexts.cpp.
// AC builds its context lists once and shares them; this fork builds them per AiObjectContext
// (strategy/AiObjectContext.cpp), so the dungeon contexts are added there through these helpers. Keeping
// the per-dungeon includes in DungeonContexts.cpp means a new dungeon touches only this folder.

#ifndef PLAYERBOTS_DUNGEONCONTEXTS_H
#define PLAYERBOTS_DUNGEONCONTEXTS_H

#include "NamedObjectContext.h"

class Action;
class Strategy;
class Trigger;
class UntypedValue;

namespace DungeonContexts
{
    void AddStrategyContexts(NamedObjectContextList<Strategy>& strategyContexts);
    void AddActionContexts(NamedObjectContextList<Action>& actionContexts);
    void AddTriggerContexts(NamedObjectContextList<Trigger>& triggerContexts);
    void AddValueContexts(NamedObjectContextList<UntypedValue>& valueContexts);
}

#endif
