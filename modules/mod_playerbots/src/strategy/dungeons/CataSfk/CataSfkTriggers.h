/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep (epic #373, issue #668). See CataSfkShared.h for sources and ids.

#ifndef PLAYERBOTS_CATASFKTRIGGERS_H
#define PLAYERBOTS_CATASFKTRIGGERS_H

#include "Trigger.h"

class SfkPainAndSufferingTrigger : public Trigger
{
public:
    SfkPainAndSufferingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk pain and suffering") {}
    bool IsActive() override;
};

class SfkWorgenSpiritTrigger : public Trigger
{
public:
    SfkWorgenSpiritTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk worgen spirit") {}
    bool IsActive() override;
};

class SfkWorgenChannelTrigger : public Trigger
{
public:
    SfkWorgenChannelTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk worgen channel") {}
    bool IsActive() override;
};

class SfkDesecrationTrigger : public Trigger
{
public:
    SfkDesecrationTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk desecration") {}
    bool IsActive() override;
};

class SfkUnholyEmpowermentTrigger : public Trigger
{
public:
    SfkUnholyEmpowermentTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk unholy empowerment") {}
    bool IsActive() override;
};

class SfkShieldConeTrigger : public Trigger
{
public:
    SfkShieldConeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk shield of the perfidious") {}
    bool IsActive() override;
};

class SfkPoisonMixtureTrigger : public Trigger
{
public:
    SfkPoisonMixtureTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk poisonous mixture") {}
    bool IsActive() override;
};

class SfkKeepMovingTrigger : public Trigger
{
public:
    SfkKeepMovingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk walden keep moving") {}
    bool IsActive() override;
};

class SfkToxicCoagulantTrigger : public Trigger
{
public:
    SfkToxicCoagulantTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk toxic coagulant") {}
    bool IsActive() override;
};

class SfkCursedBulletsTrigger : public Trigger
{
public:
    SfkCursedBulletsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk cursed bullets") {}
    bool IsActive() override;
};

class SfkPistolBarrageTrigger : public Trigger
{
public:
    SfkPistolBarrageTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk pistol barrage") {}
    bool IsActive() override;
};

class SfkBloodthirstyGhoulTrigger : public Trigger
{
public:
    SfkBloodthirstyGhoulTrigger(PlayerbotAI* botAI) : Trigger(botAI, "sfk bloodthirsty ghoul") {}
    bool IsActive() override;
};


#endif
