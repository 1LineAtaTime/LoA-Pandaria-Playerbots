/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MageActions.h"

#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "ServerFacade.h"
#include "SharedDefines.h"

// #385 (AC MageActions.cpp:32-36): only in combat, only when the gem is in the bags. Item 36799 = Mana Gem
// (the single 5.4.8 conjured gem, Wowhead mop-classic tooltip item 36799, 2026-10-04).
bool UseManaGemAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return AI_VALUE2(bool, "combat", "self target") && bot->GetItemCount(36799, false) > 0;  // Mana Gem
}

Value<Unit*>* CastPolymorphAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

bool CastFrostNovaAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 10.f);
}

bool CastBlinkAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 5.0f);
}

// #408 (AC parity, AC MageActions.cpp:151-159): face away from the current target, then Blink past
// it. Kept identical to AC - botAI->CastSpell resolves the "blink" spell name the same way here.
bool CastBlinkBackAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    bot->SetOrientation(bot->GetAngle(target) + M_PI);
    return CastSpellAction::Execute(event);
}

// Lane 10 (#865): Blizzard only when NOT in an arena.
bool CastBlizzardAction::isUseful() { return !bot->InArena() && CastSpellAction::isUseful(); }

bool CastSlowAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 25.0f);
}
bool CastDeepFreezeAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target) return false;

    return !botAI->HasAura("deep freeze", target);
}
bool CastConeOfColdAction::isUseful()
{
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose = sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 10.f);
    return facingTarget && targetClose;
}

bool CastFrozenOrbAction::isUseful()
{
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    // Rotation fix lane B (#865): 10 -> 30 yd; the MoP rotation casts Frozen Orb "always" at the current target.
    bool targetClose = sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 30.f);
    return facingTarget && targetClose;
}

bool CastDragonsBreathAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose = bot->IsWithinCombatRange(target, 10.0f);
    return facingTarget && targetClose;
}

bool CastBlastWaveAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    bool targetClose = bot->IsWithinCombatRange(target, 10.0f);
    return targetClose;
}

Unit* CastFocusMagicOnPartyAction::GetTarget()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Unit* casterDps = nullptr;
    Unit* healer = nullptr;
    Unit* target = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;
        
        if (member->GetMap() != bot->GetMap() || bot->GetDistance(member) > sPlayerbotAIConfig->spellDistance)
            continue;

        if (member->HasAura(54646))
            continue;

        if (member->GetClass() == CLASS_MAGE)
            return member;

        if (!casterDps && PlayerBotSpec::IsCaster(member) && PlayerBotSpec::IsDps(member))
            casterDps = member;
        
        if (!healer && PlayerBotSpec::IsHeal(member))
            healer = member;

        if (!target)
            target = member;
    }
    
    if (casterDps)
        return casterDps;

    if (healer)
        return healer;

    return target;
}

bool CastRuneOfPowerAction::Execute(Event event)
{
    return botAI->CastSpell(spell, bot);
}
