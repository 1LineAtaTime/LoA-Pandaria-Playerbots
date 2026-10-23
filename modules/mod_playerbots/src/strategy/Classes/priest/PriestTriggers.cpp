/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 */

#include "PriestTriggers.h"

#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

bool ShadowProtectionTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("prayer of shadow protection", GetTarget());
}

bool PowerWordFortitudeTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("power word: fortitude", GetTarget()) &&
           !botAI->HasAura("prayer of fortitude", GetTarget());
}

bool DivineSpiritTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("divine spirit", GetTarget()) &&
           !botAI->HasAura("prayer of spirit", GetTarget());
}

bool InnerFireTrigger::IsActive()
{
    // #790: Spirit of Redemption (27827, 15 sec after death; "can cast any healing spell", Wowhead mop-classic tooltip
    // 27827, 2026-10-12) is a baseline Holy passive in 5.4.8 (20711, level 30), a talent in WotLK. The spirit form has
    // no auras, so this fired every tick of the 15 s and Inner Fire 588 failed with SPELL_FAILED_NOT_SHAPESHIFT (71),
    // which CanCastSpell lets through like AC (wave 35 SM Durand, Gineriana: 68 PREREQ/FAILED, all in the spirit
    // windows right before "T:dead"). Same guard as AC's IsDisallowedShapeshiftForm (NonCombatActions.cpp:27-30).
    if (bot->HasSpiritOfRedemptionAura())
        return false;

    Unit* target = GetTarget();
    return SpellTrigger::IsActive() && !botAI->HasAura(spell, target);
}

bool FearWardOnMainTankTrigger::IsActive()
{
    uint32 const spellId = AI_VALUE2(uint32, "spell id", spell);
    if (!spellId || bot->HasSpellCooldown(spellId))
        return false;

    return BuffOnMainTankTrigger::IsActive();
}

bool ShadowformTrigger::IsActive() { return !botAI->HasAura("shadowform", bot); }

// #812: same rule as ShadowformTrigger, gated on the spell being known (Holy L56, spec257) so non-Holy priests and
// low-level Holy priests never queue it.
// Lane 8 / #863 (MoP rotation row 25723): >= 3 group members (the bot included) within 30 yd of the bot below 80% HP.
static bool PriestWantsSanctuary(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;
    uint32 hurt = 0;
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member && member->IsAlive() && member->GetHealthPct() < 80.0f && bot->GetDistance(member) <= 30.0f)
            ++hurt;
    }
    return hurt >= 3;
}

bool ChakraSerenityTrigger::IsActive()
{
    return AI_VALUE2(uint32, "spell id", spell) && !botAI->HasAura("chakra: serenity", bot) &&
           !PriestWantsSanctuary(bot);  // MoP rotation row 25724: Serenity only when Sanctuary is not wanted
}

bool ChakraSanctuaryTrigger::IsActive()
{
    return AI_VALUE2(uint32, "spell id", "chakra: sanctuary") && !botAI->HasAura("chakra: sanctuary", bot) &&
           PriestWantsSanctuary(bot);
}

bool SurgeOfLightProcTrigger::IsActive() { return bot->HasAura(114255); }

bool LightwellCanCastTrigger::IsActive()
{
    constexpr uint32 SPELL_LIGHTWELL = 126135;
    return bot->IsInCombat() && bot->HasSpell(SPELL_LIGHTWELL) && !bot->HasSpellCooldown(SPELL_LIGHTWELL);
}

bool ChakraSanctuaryMissingTrigger::IsActive()
{
    return AI_VALUE2(uint32, "spell id", "chakra: sanctuary") && !botAI->HasAura("chakra: sanctuary", bot);
}

// w75: MoP's 7 yd is conservative; Void Tendrils 108920 roots everything within 8 yd (Wowhead tooltip) and the area
// check (WorldObjectSpellAreaTargetCheck, Spell.cpp:9436) adds the target's object size. The harness mob 32285 stands
// exactly 8.0 yd away, so the old 7 yd gate could never open.
bool VoidTendrilsTargetCloseTrigger::IsActive()
{
    if (!bot->IsInCombat() || !bot->HasSpell(108920) || bot->HasSpellCooldown(108920))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive() || target->IsFriendlyTo(bot) || bot->GetExactDist(target) > 8.0f + target->GetObjectSize())
        return false;

    return target->IsPlayer() || target->GetVictim() == bot;
}

bool PriestSelfHealthTrigger::IsActive() { return bot->IsInCombat() && bot->GetHealthPct() <= pct; }

bool ShadowfiendTrigger::IsActive() { return BoostTrigger::IsActive() && !bot->HasSpellCooldown(34433); }

BindingHealTrigger::BindingHealTrigger(PlayerbotAI* botAI)
    : PartyMemberLowHealthTrigger(botAI, "binding heal", sPlayerbotAIConfig->lowHealth, 0)
{
}

bool BindingHealTrigger::IsActive()
{
    return PartyMemberLowHealthTrigger::IsActive() &&
           AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig->mediumHealth;
}

const std::set<uint32> MindSearChannelCheckTrigger::MIND_SEAR_SPELL_IDS = {
    48045,  // Mind Sear Rank 1
    53023   // Mind Sear Rank 2
};

bool MindSearChannelCheckTrigger::IsActive()
{
    Player* bot = botAI->GetBot();

    // Check if the bot is channeling a spell
    if (Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        // Only trigger if the spell being channeled is Mind Sear
        if (MIND_SEAR_SPELL_IDS.count(spell->m_spellInfo->Id))
        {
            uint8 attackerCount = AI_VALUE(uint8, "attacker count");
            return attackerCount < minEnemies;
        }
    }

    // Not channeling Mind Sear
    return false;
}

// #735 (AC ComboPointsAvailableTrigger::IsActive, AC GenericTriggers.cpp:94)
bool ShadowOrbsAvailableTrigger::IsActive() { return int32(bot->GetPower(POWER_SHADOW_ORBS)) >= amount; }
