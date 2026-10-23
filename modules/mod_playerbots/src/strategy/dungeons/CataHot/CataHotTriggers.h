/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight bot strategy "cata-hot" - triggers (issue #673, epic #373). Pattern: AC Ai/Dungeon/HoS/HoSTriggers.h.
// Every trigger is "a Shared predicate says so"; the object name equals the context key so the harness trail (T:<name>)
// and the strategy line use the same string.

#ifndef PLAYERBOTS_CATAHOTTRIGGERS_H
#define PLAYERBOTS_CATAHOTTRIGGERS_H

#include "CataHotShared.h"
#include "Trigger.h"

class CataHotConditionTrigger : public Trigger
{
public:
    using Condition = bool (*)(Player*);

    CataHotConditionTrigger(PlayerbotAI* botAI, std::string const name, Condition condition)
        : Trigger(botAI, name), _condition(condition)
    {
    }

    bool IsActive() override;

private:
    Condition _condition;
};

// The bot stands inside a circular ground hazard of this kind.
class CataHotHazardTrigger : public Trigger
{
public:
    CataHotHazardTrigger(PlayerbotAI* botAI, std::string const name, HourOfTwilight::HazardKind kind)
        : Trigger(botAI, name), _kind(kind)
    {
    }

    bool IsActive() override;

private:
    HourOfTwilight::HazardKind _kind;
};

#endif
