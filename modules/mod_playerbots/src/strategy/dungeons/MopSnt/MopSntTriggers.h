/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple strategy "mop-snt" (epic #373, issue #656) - triggers. One hazard trigger class serves
// every avoid edge: it is active while the "mop snt hazard" value says the bot's current hazard is `kind`.

#ifndef PLAYERBOTS_MOPSNTTRIGGERS_H
#define PLAYERBOTS_MOPSNTTRIGGERS_H

#include "MopDungeonShared.h"
#include "MopSntShared.h"
#include "Trigger.h"

class MopSntHazardTrigger : public Trigger
{
public:
    MopSntHazardTrigger(PlayerbotAI* botAI, std::string const name, MopSnt::HazardKind kind)
        : Trigger(botAI, name), _kind(kind)
    {
    }

    bool IsActive() override;

private:
    MopSnt::HazardKind const _kind;
};

// A caster of an interrupt-worthy spell is in range and this bot has an interrupt spell. #781: the ready interrupters
// take turns (MopDungeon::InterruptGate); when none is in range, a bot with a ready interrupt is sent after him.
class MopSntInterruptTrigger : public Trigger
{
public:
    MopSntInterruptTrigger(PlayerbotAI* botAI, std::string const name, MopSnt::InterruptGroup group)
        : Trigger(botAI, name, 250), _group(group)
    {
    }

    bool IsActive() override;

private:
    MopSnt::InterruptGroup const _group;
    MopDungeon::InterruptGate _gate;
};

// A DPS bot at Jin'bak is not yet on the Sap Globule it should be killing.
class MopSntFocusTrigger : public Trigger
{
public:
    MopSntFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop snt focus target") {}
    bool IsActive() override;
};

// Pa'valak is behind his Bulwark and a DPS bot can pick up (or is carrying) a Siege Explosive.
class MopSntSiegeBombTrigger : public Trigger
{
public:
    MopSntSiegeBombTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop snt siege bomb", 500) {}
    bool IsActive() override;
};

// #656 w54: a DPS bot carries a tar keg with a target for it, or a ready keg is in reach while a wave / Vo'jak is in throw range.
class MopSntTarKegTrigger : public Trigger
{
public:
    MopSntTarKegTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop snt tar keg", 500) {}
    bool IsActive() override;
};

// #781: a DPS bot at Jin'bak should step into the Sap Puddle to shrink it before Detonate.
class MopSntSoakPuddleTrigger : public Trigger
{
public:
    MopSntSoakPuddleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop snt soak puddle", 500) {}
    bool IsActive() override;
};

// #687: the bot carries Ner'onok's Quick-Dry Resin (121447) and is not encased (121448) yet: it has to keep jumping.
class MopSntResinTrigger : public Trigger
{
public:
    MopSntResinTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop snt resin") {}
    bool IsActive() override;
};

// #656: Ner'onok has landed at the other end of the bridge, out of the bot's reach (MopSnt::FindNeronokAcrossBridge).
class MopSntNeronokFarTrigger : public Trigger
{
public:
    MopSntNeronokFarTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop snt neronok far", 500) {}
    bool IsActive() override;
};

#endif
