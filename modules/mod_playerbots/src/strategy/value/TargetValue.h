/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TARGETVALUE_H
#define _PLAYERBOT_TARGETVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"
#include "WorldPosition.h"
// #268 S6: TravelTarget is now constructed here, so the complete type is required (the forward
// declaration above is still fine for the pointer member).
#include "TravelMgr.h"

class PlayerbotAI;
class ThreatManager;
class Unit;
// #149: AC's TravelTarget (Ai/World/Travel/TravelTarget) drives the quest/grind/boss/explore
// destination system that this fork's TravelMgr deliberately does not port (see TravelMgr.h).
// Forward-declared only so TravelTargetValue below has an exact-matching pointer type; it is
// never dereferenced here.
class TravelTarget;
// #413 (AC TargetValue.h:17-19): full definition is in Strategy.h (AC Strategy.h:17-23).
enum class TargetValueExclusionType : uint8;

GuidSet GatherStrategyTargetExclusions(PlayerbotAI* botAI, TargetValueExclusionType type);

class FindTargetStrategy
{
public:
    FindTargetStrategy(PlayerbotAI* botAI) : result(nullptr), botAI(botAI) {}

    Unit* GetResult();
    virtual TargetValueExclusionType GetExclusionType();
    virtual void CheckAttacker(Unit* attacker, ThreatManager* threatMgr) = 0;
    void GetPlayerCount(Unit* creature, uint32* tankCount, uint32* dpsCount);
    bool IsHighPriority(Unit* attacker);

protected:
    Unit* result;
    PlayerbotAI* botAI;
    std::map<Unit*, uint32> tankCountCache;
    std::map<Unit*, uint32> dpsCountCache;
    bool foundHighPriority = false;
};

class FindNonCcTargetStrategy : public FindTargetStrategy
{
public:
    FindNonCcTargetStrategy(PlayerbotAI* botAI) : FindTargetStrategy(botAI) {}

protected:
    virtual bool IsCcTarget(Unit* attacker);
};

class TargetValue : public UnitCalculatedValue
{
public:
    TargetValue(PlayerbotAI* botAI, std::string const name = "target", int checkInterval = 1)
        : UnitCalculatedValue(botAI, name, checkInterval)
    {
    }

protected:
    Unit* FindTarget(FindTargetStrategy* strategy);
};

// AC Ai/Base/Value/TargetValue.h:133-140 (#545): "find target"::<lowercase name> - a unit on the bot's
// threat list whose name equals the qualifier; the dungeon strategies resolve their bosses through it.
class FindTargetValue : public UnitCalculatedValue, public Qualified
{
public:
    FindTargetValue(PlayerbotAI* botAI) : UnitCalculatedValue(botAI, "find target", /*2 * 1000*/ 1) {}

public:
    Unit* Calculate() override;
};

class PullStrategyTargetValue : public ManualSetValue<ObjectGuid>
{
public:
    PullStrategyTargetValue(PlayerbotAI* botAI, std::string const name = "pull strategy target")
        : ManualSetValue<ObjectGuid>(botAI, ObjectGuid::Empty, name)
    {
    }
};

class PullTargetValue : public ManualSetValue<ObjectGuid>{
public:
    PullTargetValue(PlayerbotAI* botAI, std::string const name = "pull target")
        : ManualSetValue<ObjectGuid>(botAI, ObjectGuid::Empty, name)
    {
    }
};

// #194 (S8, AC parity Ai/Base/Value/TargetValue.h): the unit a bot is currently "talking" to via
// TalkAction, so both sides of a bot-to-bot chat can point at each other and GreetAction/TalkAction
// don't repeatedly pick the same target.
class TalkTargetValue : public ManualSetValue<ObjectGuid>
{
public:
    TalkTargetValue(PlayerbotAI* botAI, std::string const name = "talk target")
        : ManualSetValue<ObjectGuid>(botAI, ObjectGuid::Empty, name)
    {
    }
};

// #163 S8 (AC parity, Ai/Base/Value/TargetValue.h:158-162): the master-picked list of party members
// a healer should focus on, set by the "focus heal targets" chat command (SetFocusHealTargetsAction)
// and consumed by PartyMemberToHealValue when the "focus heal targets" strategy is active. AC uses
// std::list<ObjectGuid>; GuidVector (std::vector<ObjectGuid>, ObjectGuid.h) is this fork's existing
// equivalent container for the same job.
class FocusHealTargetValue : public ManualSetValue<GuidVector>
{
public:
    FocusHealTargetValue(PlayerbotAI* botAI, std::string const name = "focus heal targets")
        : ManualSetValue<GuidVector>(botAI, GuidVector(), name)
    {
    }
};

// #149 (AC TargetValue.h): the bot's current RPG wander destination. Ported as a plain manual-set
// value; the RPG action layer that would populate it (AC's DoRpg/movement) is out of scope here.
class RpgTargetValue : public ManualSetValue<GuidPosition>
{
public:
    RpgTargetValue(PlayerbotAI* botAI, std::string const name = "rpg target")
        : ManualSetValue<GuidPosition>(botAI, GuidPosition(), name)
    {
    }
};

// #268 S6: this was a permanent-null stub ("this fork has no travel target subsystem"). #268 S5 ported
// TravelTarget, so the value now owns one exactly as AC does - which is what makes "travel target" live
// and lets the #269 TravelAction ladder fire at all. Before this, every AI_VALUE(TravelTarget*, "travel
// target") in that ladder was null.
class TravelTargetValue : public ManualSetValue<TravelTarget*>
{
public:
    TravelTargetValue(PlayerbotAI* botAI, std::string const name = "travel target")
        : ManualSetValue<TravelTarget*>(botAI, new TravelTarget(botAI), name)
    {
    }

    virtual ~TravelTargetValue() { delete value; }
};

// #149 (AC TargetValue.h): guids the RPG target picker should skip. Registered under the
// singular name used by this fork's (dormant) PlayerbotAI.cpp call site, unlike AC's plural
// "ignore rpg targets".
class IgnoreRpgTargetValue : public ManualSetValue<GuidSet&>
{
public:
    IgnoreRpgTargetValue(PlayerbotAI* botAI, std::string const name = "ignore rpg target")
        : ManualSetValue<GuidSet&>(botAI, data, name)
    {
    }

private:
    GuidSet data;
};

// #149 (AC TargetValue.h): adapted to this fork's LastMovement, which stores the last short move
// as `lastMoveShort` directly rather than AC's PathGenerator-backed `lastPath.getBack()`.
class LastLongMoveValue : public CalculatedValue<WorldPosition>
{
public:
    LastLongMoveValue(PlayerbotAI* botAI) : CalculatedValue<WorldPosition>(botAI, "last long move", 30 * 1000) {}

    WorldPosition Calculate() override;
};

// #272 (AC Ai/Base/Value/TargetValue.h:89-95): follows LastLongMoveValue, as in AC. Not registered yet (lead).
class HomeBindValue : public CalculatedValue<WorldPosition>
{
public:
    HomeBindValue(PlayerbotAI* botAI) : CalculatedValue<WorldPosition>(botAI, "home bind", 30 * 1000) {}

    WorldPosition Calculate() override;
};

#endif
