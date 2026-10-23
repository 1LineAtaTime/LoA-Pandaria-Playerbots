/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DruidActions.h"

#include "Event.h"
#include "Playerbots.h"

// #363: the blanket HoT actions need the role helpers, Typhoon needs the distance facade, and
// Lifebloom needs a complete Aura. SpellAuraEffects.h -> SpellAuras.h is the include chain the
// module already relies on (AI/PlayerbotAI.cpp uses GetStackAmount()/GetDuration() the same way).
#include "PlayerbotSpec.h"
#include "ServerFacade.h"
#include "SpellAuraEffects.h"

// #384: see the DruidDispelSpell comment in DruidActions.h. Player::HasSpell(uint32) is the only overload in this
// core (Player.h:2017); 88423 = Nature's Cure (restoration), otherwise 2782 Remove Corruption.
std::string const DruidDispelSpell(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return (bot && bot->HasSpell(88423)) ? "nature's cure" : "remove corruption";
}

CastCurePoisonAction::CastCurePoisonAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, DruidDispelSpell(botAI)) {}

CastCurePoisonOnPartyAction::CastCurePoisonOnPartyAction(PlayerbotAI* botAI)
    : CurePartyMemberAction(botAI, DruidDispelSpell(botAI), DISPEL_POISON)
{
}

CastAbolishPoisonAction::CastAbolishPoisonAction(PlayerbotAI* botAI)
    : CastCureSpellAction(botAI, DruidDispelSpell(botAI))
{
}

CastAbolishPoisonOnPartyAction::CastAbolishPoisonOnPartyAction(PlayerbotAI* botAI)
    : CurePartyMemberAction(botAI, DruidDispelSpell(botAI), DISPEL_POISON)
{
}

CastDruidRemoveCurseOnPartyAction::CastDruidRemoveCurseOnPartyAction(PlayerbotAI* ai)
    : CurePartyMemberAction(ai, DruidDispelSpell(ai), DISPEL_CURSE)
{
}

NextAction** CastAbolishPoisonAction::getAlternatives()
{
    return NextAction::merge(NextAction::array(0, new NextAction("cure poison"), nullptr),
                             CastSpellAction::getPrerequisites());
}

NextAction** CastAbolishPoisonOnPartyAction::getAlternatives()
{
    return NextAction::merge(NextAction::array(0, new NextAction("cure poison on party"), nullptr),
                             CastSpellAction::getPrerequisites());
}

Value<Unit*>* CastEntanglingRootsCcAction::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", "entangling roots");
}

bool CastEntanglingRootsCcAction::Execute(Event event) { return botAI->CastSpell("entangling roots", GetTarget()); }

Value<Unit*>* CastHibernateCcAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", "hibernate"); }

bool CastHibernateCcAction::Execute(Event event) { return botAI->CastSpell("hibernate", GetTarget()); }

Value<Unit*>* CastCycloneCcAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", "cyclone"); }

bool CastCycloneCcAction::Execute(Event event) { return botAI->CastSpell("cyclone", GetTarget()); }

NextAction** CastReviveAction::getPrerequisites()
{
    return NextAction::merge(NextAction::array(0, new NextAction("caster form"), nullptr),
                             ResurrectPartyMemberAction::getPrerequisites());
}

NextAction** CastRebirthAction::getPrerequisites()
{
    return NextAction::merge(NextAction::array(0, new NextAction("caster form"), nullptr),
                             ResurrectPartyMemberAction::getPrerequisites());
}

bool CastRebirthAction::isUseful()
{
    return CastSpellAction::isUseful() &&
           AI_VALUE2(float, "distance", GetTargetName()) <= sPlayerbotAIConfig->spellDistance;
}

Unit* CastRejuvenationOnNotFullAction::GetTarget()
{
    Group* group = bot->GetGroup();
    MinValueCalculator calc(100);
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        if (player->isDead() || player->IsFullHealth())
        {
            continue;
        }
        if (player->GetDistance2d(bot) > sPlayerbotAIConfig->spellDistance)
        {
            continue;
        }
        if (botAI->HasAura("rejuvenation", player))
        {
            continue;
        }
        calc.probe(player->GetHealthPct(), player);
    }
    return (Unit*)calc.param;
}

bool CastRejuvenationOnNotFullAction::isUseful()
{
    return GetTarget();
}

// #363 (AC parity, Ai/Class/Druid/Action/DruidActions.cpp:45-53). Re-seed Lifebloom when it is
// missing, under 3 stacks, or within 2s of expiring. Lifebloom (25289) still exists in 5.4.8.
bool CastLifebloomOnMainTankAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !CastSpellAction::isUseful())
        return false;

    Aura* lifebloom = botAI->GetAura("lifebloom", target, true, true);
    return !lifebloom || lifebloom->GetStackAmount() < 3 || lifebloom->GetDuration() < 2000;
}

// #363 (AC parity, Ai/Class/Druid/Action/DruidActions.cpp:145-151). Typhoon (38243) is unchanged
// in 5.4.8. Hard rules 1 and 4: sServerFacade is the pointer-returning instance().
bool CastTyphoonAction::isUseful()
{
    bool facingTarget = AI_VALUE2(bool, "facing", "current target");
    bool targetClose = sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", GetTargetName()), 15.f);
    return facingTarget && targetClose;
}

// #363 (AC parity, Ai/Class/Druid/Action/DruidActions.cpp:212-228). Innervate (10912) unchanged in
// 5.4.8. Body in the .cpp: bot->HasSpellCooldown() needs a complete Player type (hard rule 2).
bool CastInnervateOnHealerAction::isPossible()
{
    Unit* target = GetTarget();
    if (!target || !target->IsInWorld())
        return false;

    if (botAI->HasAura("innervate", target))
        return false;

    uint32 spellId = AI_VALUE2(uint32, "spell id", "innervate");
    return spellId && !bot->HasSpellCooldown(spellId);
}

NextAction** CastInnervateOnHealerAction::getPrerequisites()
{
    return NextAction::array(0, new NextAction("caster form"), nullptr);
}

// --- Blanket HoT actions (#363, AC parity, Ai/Class/Druid/Action/DruidActions.cpp:261-319) ---

Unit* CastBlanketHotAction::GetBlanketTarget(std::string const& auraName)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    auto eligible = [&](Player* member) -> bool
    {
        return member && member->IsAlive() && !member->IsGameMaster() &&
               bot->GetDistance2d(member) <= sPlayerbotAIConfig->spellDistance &&
               !botAI->HasAura(auraName, member, false, true);
    };

    Player* firstMelee = nullptr;
    Player* firstRanged = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!eligible(member))
            continue;

        if (PlayerBotSpec::IsTank(member))
            return member;
        else if (!firstMelee && PlayerBotSpec::IsMelee(member) && !PlayerBotSpec::IsTank(member))
            firstMelee = member;
        else if (!firstRanged && PlayerBotSpec::IsRanged(member))
            firstRanged = member;

        if (firstMelee && firstRanged)
            break;
    }

    if (firstMelee) return firstMelee;
    return firstRanged;
}

Unit* CastRejuvenationBlanketAction::GetTarget() { return GetBlanketTarget("rejuvenation"); }

bool CastRejuvenationBlanketAction::isUseful() { return GetTarget() != nullptr; }

Unit* CastWildGrowthBlanketAction::GetTarget() { return GetBlanketTarget("wild growth"); }

bool CastWildGrowthBlanketAction::isUseful() { return GetTarget() != nullptr; }

// Rotation lane 5 (#860): see CastSkullBashOnEnemyCasterAction in DruidActions.h.
bool CastSkullBashOnEnemyCasterAction::isUseful()
{
    return CastSpellOnEnemyCasterAction::isUseful() && AI_VALUE(bool, "in pvp instance");
}
