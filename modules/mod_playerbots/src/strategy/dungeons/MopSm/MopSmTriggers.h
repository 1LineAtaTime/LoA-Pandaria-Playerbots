/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" (Scarlet Monastery, epic #373, issue #658). See MopSmShared.h for sources.

#ifndef PLAYERBOTS_MOPSMTRIGGERS_H
#define PLAYERBOTS_MOPSMTRIGGERS_H

#include "MopDungeonShared.h"
#include "Trigger.h"

// The bot stands inside a Thalnos Spirit Gale (aura 115291).
class MopSmInSpiritGaleTrigger : public Trigger
{
public:
    MopSmInSpiritGaleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sm in spirit gale") {}
    bool IsActive() override;
};

// Thalnos or Whitemane is casting Spirit Gale / Mass Resurrection / Heal / Holy Smite. #781: the bots whose interrupt
// is ready take turns (MopDungeon::InterruptGate), the tank included.
class MopSmPriorityInterruptTrigger : public Trigger
{
public:
    MopSmPriorityInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sm priority interrupt") {}
    bool IsActive() override;

private:
    MopDungeon::InterruptGate _gate;
};

// An Empowering Spirit or Evicted Soul is alive and the bot is a damage dealer.
class MopSmFocusAddTrigger : public Trigger
{
public:
    MopSmFocusAddTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sm focus add") {}
    bool IsActive() override;
};

// Korloff channels Firestorm Kick within 16 yd of the bot.
class MopSmFirestormKickTrigger : public Trigger
{
public:
    MopSmFirestormKickTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sm firestorm kick") {}
    bool IsActive() override;
};

// #684: Whitemane wears her Power Word: Shield and the bot has an offensive magic dispel (AC BT "essence of desire has
// rune shield" shape).
class MopSmWhitemaneShieldTrigger : public Trigger
{
public:
    MopSmWhitemaneShieldTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sm whitemane shield") {}
    bool IsActive() override;
};

// The bot stands in Korloff's Scorched Earth fire.
class MopSmScorchedEarthTrigger : public Trigger
{
public:
    MopSmScorchedEarthTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sm in scorched earth") {}
    bool IsActive() override;
};

#endif
