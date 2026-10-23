/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopTjs triggers (epic #373, issue #651).

#ifndef PLAYERBOTS_MOPTJSTRIGGERS_H
#define PLAYERBOTS_MOPTJSTRIGGERS_H

#include "Trigger.h"

class MopTjsDangerTrigger : public Trigger
{
public:
    MopTjsDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop tjs danger") {}
    bool IsActive() override;
};

// #815 Wise Mari phase 1: a Corrupt Living Water / Droplet is up and this bot is not on it, or cannot see it
// (AC Seth TalonKingIkissBossOutOfLosTrigger, SethTriggers.cpp:79-84, for the out-of-sight half).
class MopTjsLivingWaterTrigger : public Trigger
{
public:
    MopTjsLivingWaterTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop tjs living water", 500) {}
    bool IsActive() override;
};

// #651 Lorewalker Stonestep: a Sun / corrupted Zao / Strife or Peril is attackable and this bot holds no event enemy
// (AC UK "keleseth frost tomb", UKTriggers.cpp, for an enemy that never enters the threat lists).
class MopTjsLorewalkerTrigger : public Trigger
{
public:
    MopTjsLorewalkerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop tjs lorewalker", 500) {}
    bool IsActive() override;
};

#endif
