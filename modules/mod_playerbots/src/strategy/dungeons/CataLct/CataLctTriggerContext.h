/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Lost City of the Tol'vir (epic #373, issue #666). See CataLctShared.h for sources and ids.

#ifndef PLAYERBOTS_CATALCTTRIGGERCONTEXT_H
#define PLAYERBOTS_CATALCTTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "CataLctTriggers.h"

class CataDungeonLctTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CataDungeonLctTriggerContext()
    {
        creators["lct land mine"] = &CataDungeonLctTriggerContext::lct_land_mine;
        creators["lct shockwave"] = &CataDungeonLctTriggerContext::lct_shockwave;
        creators["lct bad intentions"] = &CataDungeonLctTriggerContext::lct_bad_intentions;
        creators["lct off platform"] = &CataDungeonLctTriggerContext::lct_off_platform;
        creators["lct dust flail"] = &CataDungeonLctTriggerContext::lct_dust_flail;
        creators["lct frenzied crocolisk"] = &CataDungeonLctTriggerContext::lct_frenzied_crocolisk;
        creators["lct heavens fury"] = &CataDungeonLctTriggerContext::lct_heavens_fury;
        creators["lct soul fragment"] = &CataDungeonLctTriggerContext::lct_soul_fragment;
        creators["lct hallowed ground"] = &CataDungeonLctTriggerContext::lct_hallowed_ground;
        creators["lct harbinger"] = &CataDungeonLctTriggerContext::lct_harbinger;
        creators["lct blaze fire"] = &CataDungeonLctTriggerContext::lct_blaze_fire;
        creators["lct siamat storm"] = &CataDungeonLctTriggerContext::lct_siamat_storm;
        creators["lct siamat servant"] = &CataDungeonLctTriggerContext::lct_siamat_servant;
        creators["lct lightning charge"] = &CataDungeonLctTriggerContext::lct_lightning_charge;
    }

private:
    static Trigger* lct_land_mine(PlayerbotAI* botAI) { return new LctLandMineTrigger(botAI); }
    static Trigger* lct_shockwave(PlayerbotAI* botAI) { return new LctShockwaveTrigger(botAI); }
    static Trigger* lct_bad_intentions(PlayerbotAI* botAI) { return new LctBadIntentionsTrigger(botAI); }
    static Trigger* lct_off_platform(PlayerbotAI* botAI) { return new LctOffPlatformTrigger(botAI); }
    static Trigger* lct_dust_flail(PlayerbotAI* botAI) { return new LctDustFlailTrigger(botAI); }
    static Trigger* lct_frenzied_crocolisk(PlayerbotAI* botAI) { return new LctCrocoliskTrigger(botAI); }
    static Trigger* lct_heavens_fury(PlayerbotAI* botAI) { return new LctHeavensFuryTrigger(botAI); }
    static Trigger* lct_soul_fragment(PlayerbotAI* botAI) { return new LctSoulFragmentTrigger(botAI); }
    static Trigger* lct_hallowed_ground(PlayerbotAI* botAI) { return new LctHallowedGroundTrigger(botAI); }
    static Trigger* lct_harbinger(PlayerbotAI* botAI) { return new LctHarbingerTrigger(botAI); }
    static Trigger* lct_blaze_fire(PlayerbotAI* botAI) { return new LctBlazeFireTrigger(botAI); }
    static Trigger* lct_siamat_storm(PlayerbotAI* botAI) { return new LctSiamatStormTrigger(botAI); }
    static Trigger* lct_siamat_servant(PlayerbotAI* botAI) { return new LctSiamatServantTrigger(botAI); }
    static Trigger* lct_lightning_charge(PlayerbotAI* botAI) { return new LctLightningChargeTrigger(botAI); }
};

#endif
