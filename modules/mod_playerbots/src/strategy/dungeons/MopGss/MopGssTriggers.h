/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun strategy "mop-gss" (epic #373, issue #655) - triggers. One hazard trigger class serves
// every avoid / spread edge: it is active while the "mop gss hazard" value says the bot's current hazard is `kind`.

#ifndef PLAYERBOTS_MOPGSSTRIGGERS_H
#define PLAYERBOTS_MOPGSSTRIGGERS_H

#include "MopGssShared.h"
#include "Trigger.h"

class MopGssHazardTrigger : public Trigger
{
public:
    MopGssHazardTrigger(PlayerbotAI* botAI, std::string const name, MopGss::HazardKind kind)
        : Trigger(botAI, name), _kind(kind)
    {
    }

    bool IsActive() override;

private:
    MopGss::HazardKind const _kind;
};

// A caster of an interrupt-worthy spell is in range and this bot has an interrupt spell.
class MopGssInterruptTrigger : public Trigger
{
public:
    MopGssInterruptTrigger(PlayerbotAI* botAI, std::string const name, MopGss::InterruptGroup group)
        : Trigger(botAI, name, 250), _group(group)
    {
    }

    bool IsActive() override;

private:
    MopGss::InterruptGroup const _group;
};

// A DPS bot is not yet on the add / weak spot it should be killing.
class MopGssFocusTrigger : public Trigger
{
public:
    MopGssFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop gss focus target") {}
    bool IsActive() override;
};

// #655 Ri'mok stands in Viscous Fluid and attacks this tank (AC Ai/Dungeon/AC ShirrakTankPositionBossTrigger shape).
class MopGssTankFluidTrigger : public Trigger
{
public:
    MopGssTankFluidTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop gss tank fluid", 500) {}
    bool IsActive() override;
};

// #655 Raigonn phase 1: a melee damage dealer on the ground and a free cannon (AC Ai/Dungeon/TOC ToCMountNearTrigger shape).
class MopGssCannonTrigger : public Trigger
{
public:
    MopGssCannonTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop gss cannon", 500) {}
    bool IsActive() override;
};

// #655 w65: a tank add pickup (MopDungeon add wave, AC AK Nadox shape) that waits while the fight's boss hits somebody else -
// Ri'mok's Swarmers never stop coming, so the pickups kept the tank off Ri'mok; "mop gss focus target" takes him back first.
class MopGssTankAddsTrigger : public MopDungeon::MopWaveTrigger
{
public:
    MopGssTankAddsTrigger(PlayerbotAI* botAI, std::string const name, MopDungeon::WaveSpec spec, uint32 bossEntry)
        : MopDungeon::MopWaveTrigger(botAI, name, std::move(spec)), _bossEntry(bossEntry)
    {
    }

    bool IsActive() override { return !MopGss::TankLostBoss(bot, _bossEntry) && MopDungeon::MopWaveTrigger::IsActive(); }

private:
    uint32 const _bossEntry;
};

// #821 Ga'dok is DONE, Krik'thik Bombardiers are alive and this bot is the one elected to fire the Flak Cannon.
class MopGssFlakTrigger : public Trigger
{
public:
    MopGssFlakTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop gss flak", 1000) {}
    bool IsActive() override;

private:
    bool _wasActive = false;
};

#endif
