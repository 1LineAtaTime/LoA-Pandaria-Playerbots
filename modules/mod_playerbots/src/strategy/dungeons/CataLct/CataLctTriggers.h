/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Lost City of the Tol'vir (epic #373, issue #666). See CataLctShared.h for sources and ids.

#ifndef PLAYERBOTS_CATALCTTRIGGERS_H
#define PLAYERBOTS_CATALCTTRIGGERS_H

#include "Trigger.h"

class LctLandMineTrigger : public Trigger
{
public:
    LctLandMineTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct land mine") {}
    bool IsActive() override;
};

class LctShockwaveTrigger : public Trigger
{
public:
    LctShockwaveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct shockwave") {}
    bool IsActive() override;
};

class LctBadIntentionsTrigger : public Trigger
{
public:
    LctBadIntentionsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct bad intentions") {}
    bool IsActive() override;
};

class LctOffPlatformTrigger : public Trigger
{
public:
    LctOffPlatformTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct off platform") {}
    bool IsActive() override;
};

class LctDustFlailTrigger : public Trigger
{
public:
    LctDustFlailTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct dust flail") {}
    bool IsActive() override;
};

class LctCrocoliskTrigger : public Trigger
{
public:
    LctCrocoliskTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct frenzied crocolisk") {}
    bool IsActive() override;
};

class LctHeavensFuryTrigger : public Trigger
{
public:
    LctHeavensFuryTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct heavens fury") {}
    bool IsActive() override;
};

class LctSoulFragmentTrigger : public Trigger
{
public:
    LctSoulFragmentTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct soul fragment") {}
    bool IsActive() override;
};

class LctHallowedGroundTrigger : public Trigger
{
public:
    LctHallowedGroundTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct hallowed ground") {}
    bool IsActive() override;
};

class LctHarbingerTrigger : public Trigger
{
public:
    LctHarbingerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct harbinger") {}
    bool IsActive() override;
};

class LctBlazeFireTrigger : public Trigger
{
public:
    LctBlazeFireTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct blaze fire") {}
    bool IsActive() override;
};

class LctSiamatStormTrigger : public Trigger
{
public:
    LctSiamatStormTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct siamat storm") {}
    bool IsActive() override;
};

class LctSiamatServantTrigger : public Trigger
{
public:
    LctSiamatServantTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct siamat servant") {}
    bool IsActive() override;
};

class LctLightningChargeTrigger : public Trigger
{
public:
    LctLightningChargeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "lct lightning charge") {}
    bool IsActive() override;
};


#endif
