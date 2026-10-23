/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep (epic #373, issue #668). See CataSfkShared.h for sources and ids.

#ifndef PLAYERBOTS_CATASFKTRIGGERCONTEXT_H
#define PLAYERBOTS_CATASFKTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "CataSfkTriggers.h"

class CataDungeonSfkTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CataDungeonSfkTriggerContext()
    {
        creators["sfk pain and suffering"] = &CataDungeonSfkTriggerContext::sfk_pain_and_suffering;
        creators["sfk worgen spirit"] = &CataDungeonSfkTriggerContext::sfk_worgen_spirit;
        creators["sfk worgen channel"] = &CataDungeonSfkTriggerContext::sfk_worgen_channel;
        creators["sfk desecration"] = &CataDungeonSfkTriggerContext::sfk_desecration;
        creators["sfk unholy empowerment"] = &CataDungeonSfkTriggerContext::sfk_unholy_empowerment;
        creators["sfk shield of the perfidious"] = &CataDungeonSfkTriggerContext::sfk_shield_of_the_perfidious;
        creators["sfk poisonous mixture"] = &CataDungeonSfkTriggerContext::sfk_poisonous_mixture;
        creators["sfk walden keep moving"] = &CataDungeonSfkTriggerContext::sfk_walden_keep_moving;
        creators["sfk toxic coagulant"] = &CataDungeonSfkTriggerContext::sfk_toxic_coagulant;
        creators["sfk cursed bullets"] = &CataDungeonSfkTriggerContext::sfk_cursed_bullets;
        creators["sfk pistol barrage"] = &CataDungeonSfkTriggerContext::sfk_pistol_barrage;
        creators["sfk bloodthirsty ghoul"] = &CataDungeonSfkTriggerContext::sfk_bloodthirsty_ghoul;
    }

private:
    static Trigger* sfk_pain_and_suffering(PlayerbotAI* botAI) { return new SfkPainAndSufferingTrigger(botAI); }
    static Trigger* sfk_worgen_spirit(PlayerbotAI* botAI) { return new SfkWorgenSpiritTrigger(botAI); }
    static Trigger* sfk_worgen_channel(PlayerbotAI* botAI) { return new SfkWorgenChannelTrigger(botAI); }
    static Trigger* sfk_desecration(PlayerbotAI* botAI) { return new SfkDesecrationTrigger(botAI); }
    static Trigger* sfk_unholy_empowerment(PlayerbotAI* botAI) { return new SfkUnholyEmpowermentTrigger(botAI); }
    static Trigger* sfk_shield_of_the_perfidious(PlayerbotAI* botAI) { return new SfkShieldConeTrigger(botAI); }
    static Trigger* sfk_poisonous_mixture(PlayerbotAI* botAI) { return new SfkPoisonMixtureTrigger(botAI); }
    static Trigger* sfk_walden_keep_moving(PlayerbotAI* botAI) { return new SfkKeepMovingTrigger(botAI); }
    static Trigger* sfk_toxic_coagulant(PlayerbotAI* botAI) { return new SfkToxicCoagulantTrigger(botAI); }
    static Trigger* sfk_cursed_bullets(PlayerbotAI* botAI) { return new SfkCursedBulletsTrigger(botAI); }
    static Trigger* sfk_pistol_barrage(PlayerbotAI* botAI) { return new SfkPistolBarrageTrigger(botAI); }
    static Trigger* sfk_bloodthirsty_ghoul(PlayerbotAI* botAI) { return new SfkBloodthirstyGhoulTrigger(botAI); }
};

#endif
