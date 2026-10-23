/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSsb triggers (epic #373, issue #652).

#ifndef PLAYERBOTS_MOPSSBTRIGGERS_H
#define PLAYERBOTS_MOPSSBTRIGGERS_H

#include "Trigger.h"

class MopSsbDangerTrigger : public Trigger
{
public:
    MopSsbDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop ssb danger") {}
    bool IsActive() override;
};

// Blackout Brew stacks are about to stun the bot: keep moving.
class MopSsbBlackoutTrigger : public Trigger
{
public:
    MopSsbBlackoutTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop ssb blackout") {}
    bool IsActive() override;
};

// #690: Carbonation is live (a Fizzy Bubble to click, or flying but still low).
class MopSsbFizzyTrigger : public Trigger
{
public:
    MopSsbFizzyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop ssb fizzy") {}
    bool IsActive() override;
};

// #690: a Wall of Suds is about to walk over a bot that carries Sudsy.
class MopSsbWallTrigger : public Trigger
{
public:
    MopSsbWallTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop ssb wall") {}
    bool IsActive() override;
};

// #690: Yan-Zhu's Bubble Shield is up and this ranged bot has a bubble to pop.
class MopSsbBubbleTrigger : public Trigger
{
public:
    MopSsbBubbleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop ssb bubble") {}
    bool IsActive() override;
};

// A priority add is up and not yet carrying the group's skull (only the tracker bot reports it).
class MopSsbPriorityTrigger : public Trigger
{
public:
    MopSsbPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop ssb priority") {}
    bool IsActive() override;
};

#endif
