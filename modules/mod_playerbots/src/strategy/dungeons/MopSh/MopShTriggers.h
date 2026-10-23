/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" (Scarlet Halls, epic #373, issue #657). See MopShShared.h for sources.

#ifndef PLAYERBOTS_MOPSHTRIGGERS_H
#define PLAYERBOTS_MOPSHTRIGGERS_H

#include "Trigger.h"

// Braun channels Piercing Throw and the bot stands on the knife line to a different target.
class MopShKnifeLineTrigger : public Trigger
{
public:
    MopShKnifeLineTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sh knife line") {}
    bool IsActive() override;
};

// Harlan is winding up / spinning Blades of Light.
class MopShBladesOfLightTrigger : public Trigger
{
public:
    MopShBladesOfLightTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sh blades of light") {}
    bool IsActive() override;
};

// Koegler is casting something worth interrupting (Pyroblast, Book Burner, Dragon's Breath, Fireball Volley).
class MopShKoeglerInterruptTrigger : public Trigger
{
public:
    MopShKoeglerInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sh koegler interrupt") {}
    bool IsActive() override;
};

// The bot stands within reach of a burning book case.
class MopShBurningBooksTrigger : public Trigger
{
public:
    MopShBurningBooksTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sh burning books") {}
    bool IsActive() override;
};

// #683: Koegler aims Book Burner at a Book Case and the bot is the elected interceptor (w106: nearest to the beam, the tank when
// close enough), not yet on the beam.
class MopShBookBurnerInterceptTrigger : public Trigger
{
public:
    MopShBookBurnerInterceptTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sh book burner intercept") {}
    bool IsActive() override;
};

// Koegler channels Greater Dragon's Breath and the bot is in the front arc.
class MopShDragonBreathTrigger : public Trigger
{
public:
    MopShDragonBreathTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sh dragon breath") {}
    bool IsActive() override;
};

#endif
