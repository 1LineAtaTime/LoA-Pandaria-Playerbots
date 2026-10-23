/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace strategy "mop-msp" (epic #373, issue #654) - triggers. One hazard trigger class serves every
// avoid / spread / stack edge: it is active while the "mop msp hazard" value says the bot's current hazard is `kind`.

#ifndef PLAYERBOTS_MOPMSPTRIGGERS_H
#define PLAYERBOTS_MOPMSPTRIGGERS_H

#include "MopDungeonShared.h"
#include "MopMspShared.h"
#include "Trigger.h"

class MopMspHazardTrigger : public Trigger
{
public:
    MopMspHazardTrigger(PlayerbotAI* botAI, std::string const name, MopMsp::HazardKind kind)
        : Trigger(botAI, name), _kind(kind)
    {
    }

    bool IsActive() override;

private:
    MopMsp::HazardKind const _kind;
};

// A caster of an interrupt-worthy spell is in range and this bot has an interrupt spell. #781: the bots whose interrupt
// is ready take turns (MopDungeon::InterruptGate), the tank included.
class MopMspInterruptTrigger : public Trigger
{
public:
    MopMspInterruptTrigger(PlayerbotAI* botAI, std::string const name, MopMsp::InterruptGroup group)
        : Trigger(botAI, name, 250), _group(group)
    {
    }

    bool IsActive() override;

private:
    MopMsp::InterruptGroup const _group;
    MopDungeon::InterruptGate _gate;
};

// A DPS bot in Gekkan's room is not yet on the protector it should be killing.
class MopMspFocusTrigger : public Trigger
{
public:
    MopMspFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop msp focus target") {}
    bool IsActive() override;
};

#endif
