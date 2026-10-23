/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinTriggers.h"

#include "PaladinActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

// #357 slice 6 (AC PaladinTriggers.cpp:19-27) - 1:1. Every spell AC tests still exists in 5.4.8, so no
// adaptation is needed here; the fork's old body tested "seal of insight", a healing seal Cataclysm removed.
bool SealTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->HasAura("seal of justice", target) && !botAI->HasAura("seal of command", target) &&
           !botAI->HasAura("seal of vengeance", target) && !botAI->HasAura("seal of corruption", target) &&
           !botAI->HasAura("seal of righteousness", target) && !botAI->HasAura("seal of light", target) &&
           // #525: Seal of Truth (31801) is the 5.x Seal of Corruption/Vengeance; without this clause the
           // trigger stayed active while Truth was up. Seal of Insight (20165) takes the Seal of Wisdom slot
           // of AC's mana-swap clause (Wowhead 2026-10-04).
           !botAI->HasAura("seal of truth", target) &&
           (!botAI->HasAura("seal of wisdom", target) || AI_VALUE2(uint8, "mana", "self target") > 70) &&
           (!botAI->HasAura("seal of insight", target) || AI_VALUE2(uint8, "mana", "self target") > 70);
}

// #357 slice 6 (AC PaladinTriggers.cpp:29-33) - 1:1
bool CrusaderAuraTrigger::IsActive()
{
    Unit* target = GetTarget();
    return AI_VALUE2(bool, "mounted", "self target") && !botAI->HasAura("crusader aura", target);
}

bool BlessingTrigger::IsActive()
{
	Unit* target = GetTarget();
	return SpellTrigger::IsActive() && !botAI->HasAnyAuraOf(target, "blessing of kings", "blessing of might", nullptr);
}

bool HandOfFreedomOnPartyTrigger::IsActive()
{
	Unit* target = GetTarget();
	if (!target)
		return false;

	// #163 S8 fix per research (B.4): AC checks 30 yd explicitly here even though MoP's own Hand of
	// Freedom range is 40 yd (5.0.4) - AC is the spec, so this keeps AC's tighter radius rather than
	// relying on "party member snared target"'s own (looser) distance check.
	if (target != bot && bot->GetExactDist2dSq(target->GetPositionX(), target->GetPositionY()) > 30.0f * 30.0f)
		return false;

	return botAI->CanCastSpell("hand of freedom", target) && !botAI->HasAura("hand of freedom", target);
}

// #357 slice 6 (AC PaladinTriggers.cpp:65-67) - 1:1
bool DivineShieldLowHealthTrigger::IsActive()
{
    return botAI->HasAura("divine shield", bot) && AI_VALUE2(uint8, "health", "self target") < 80;
}

// #357 slice 6 (AC PaladinTriggers.cpp:77-80) - 1:1
bool NotSensingUndeadTrigger::IsActive()
{
    return !botAI->HasAura("sense undead", bot);
}

// #735 (AC ComboPointsAvailableTrigger::IsActive / EnergyAvailable::IsActive, AC GenericTriggers.cpp:92-94)
bool HolyPowerAvailableTrigger::IsActive() { return int32(bot->GetPower(POWER_HOLY_POWER)) >= amount; }

// #735: 90174 = the Divine Purpose proc (see PaladinTriggers.h)
bool DivinePurposeTrigger::IsActive() { return bot->HasAura(90174); }

// ---- rotation lane 6 (#861) ----
bool PaladinSelfHealthTrigger::IsActive() { return AI_VALUE2(uint8, "health", "self target") <= pct; }

bool BastionOfGloryTrigger::IsActive()
{
    Aura* aura = bot->GetAura(114637, bot->GetGUID());
    return aura && aura->GetStackAmount() >= 3;
}

bool PaladinHurtGroupTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hurt;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->GetMapId() == bot->GetMapId() && member->GetHealthPct() < 50.0f &&
            bot->GetDistance(member) <= 40.0f)
            hurt.push_back(member);
    }

    if (radius >= 40.0f)
        return hurt.size() >= 3;

    for (Player* center : hurt)
    {
        uint32 count = 0;
        for (Player* other : hurt)
            if (center == other || center->GetDistance(other) <= radius)
                ++count;
        if (count >= 3)
            return true;
    }
    return false;
}
