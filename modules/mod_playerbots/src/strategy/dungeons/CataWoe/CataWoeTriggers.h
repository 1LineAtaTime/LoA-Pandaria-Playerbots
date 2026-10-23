/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity bot strategy "cata-woe" - triggers (issue #672, epic #373). Pattern: AC Ai/Dungeon/HoS/HoSTriggers.h.
// Every trigger is "a Shared predicate says so"; the object name equals the context key so the harness trail (T:<name>)
// and the strategy line use the same string.

#ifndef PLAYERBOTS_CATAWOETRIGGERS_H
#define PLAYERBOTS_CATAWOETRIGGERS_H

#include "CataWoeShared.h"
#include "Trigger.h"

class CataWoeConditionTrigger : public Trigger
{
public:
    using Condition = bool (*)(Player*);

    CataWoeConditionTrigger(PlayerbotAI* botAI, std::string const name, Condition condition)
        : Trigger(botAI, name), _condition(condition)
    {
    }

    bool IsActive() override;

private:
    Condition _condition;
};

// The bot stands inside a circular ground hazard of this kind.
class CataWoeHazardTrigger : public Trigger
{
public:
    CataWoeHazardTrigger(PlayerbotAI* botAI, std::string const name, WellOfEternity::HazardKind kind)
        : Trigger(botAI, name), _kind(kind)
    {
    }

    bool IsActive() override;

private:
    WellOfEternity::HazardKind _kind;
};

#endif
