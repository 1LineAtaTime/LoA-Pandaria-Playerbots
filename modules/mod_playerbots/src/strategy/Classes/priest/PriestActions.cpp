/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PriestActions.h"

#include "Event.h"
#include "Playerbots.h"

bool CastFadeAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!bot->GetGroup() || (target && target->IsPlayer()))  // players have no threat
        return false;
    // w78 TP: carrier Billis faded with a player and a creature on him (the current target was the creature) - Fade does
    // nothing against a player, AC never drops threat on players in a BG (t575 #693 row). Off while any player-controlled
    // unit is attacking.
    for (Unit* attacker : bot->getAttackers())
        if (attacker && attacker->IsControlledByPlayer())
            return false;
    return true;
}

bool CastRemoveShadowformAction::isUseful() { return botAI->HasAura("shadowform", AI_VALUE(Unit*, "self target")); }

bool CastRemoveShadowformAction::isPossible() { return true; }

bool CastRemoveShadowformAction::Execute(Event event)
{
    botAI->RemoveAura("shadowform");
    return true;
}

Unit* CastPowerWordShieldOnAlmostFullHealthBelowAction::GetTarget()
{
    Group* group = bot->GetGroup();
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        if (player->isDead())
        {
            continue;
        }
        if (player->GetHealthPct() > sPlayerbotAIConfig->almostFullHealth)
        {
            continue;
        }
        if (player->GetDistance2d(bot) > sPlayerbotAIConfig->spellDistance)
        {
            continue;
        }
        if (botAI->HasAnyAuraOf(player, "weakened soul", "power word: shield", nullptr))
        {
            continue;
        }
        return player;
    }
    return nullptr;
}

bool CastPowerWordShieldOnAlmostFullHealthBelowAction::isUseful()
{
    Group* group = bot->GetGroup();
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player)
            continue;
        if (player->isDead())
        {
            continue;
        }
        if (player->GetHealthPct() > sPlayerbotAIConfig->almostFullHealth)
        {
            continue;
        }
        if (player->GetDistance2d(bot) > sPlayerbotAIConfig->spellDistance)
        {
            continue;
        }
        if (botAI->HasAnyAuraOf(player, "weakened soul", "power word: shield", nullptr))
        {
            continue;
        }
        return true;
    }
    return false;
}

Unit* CastPowerWordShieldOnNotFullAction::GetTarget()
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
        if (botAI->HasAnyAuraOf(player, "weakened soul", "power word: shield", nullptr))
        {
            continue;
        }
        calc.probe(player->GetHealthPct(), player);
    }
    return (Unit*)calc.param;
}

bool CastPowerWordShieldOnNotFullAction::isUseful()
{
    return GetTarget();
}

// Lane 8 / #863: shared gate for the MoP PvP crowd-control rows (Psychic Scream, Void Tendrils): a BG/arena map,
// an enemy PLAYER as current target, and not already crowd-controlled ("is cc", lane 0 contract).
static bool PriestPvpCcTargetOk(PlayerbotAI* botAI)
{
    if (!botAI->GetAiObjectContext()->GetValue<bool>("in pvp instance")->Get())
        return false;
    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (!target || !target->IsPlayer())
        return false;
    return !botAI->GetAiObjectContext()->GetValue<bool>("is cc", "current target")->Get();
}

bool CastPsychicScreamAction::isUseful()
{
    // MoP rotation row 503/25800: target within 4 yd of the priest
    Unit* target = AI_VALUE(Unit*, "current target");
    return PriestPvpCcTargetOk(botAI) && target && bot->GetDistance(target) <= 4.0f && CastSpellAction::isUseful();
}

bool CastVoidTendrilsAction::isUseful()
{
    // MoP rotation row 25802: target within 7 yd of the priest. Lane E (#863): MoP 5.4.8 has no pvp / player condition; a creature
    // that is attacking the bot is accepted too (shadow dummy-style w74: 0 casts, the harness target stays 8 yd away).
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || bot->GetExactDist(target) > 8.0f + target->GetObjectSize())
        return false;
    if (!target->IsPlayer())
        return target->GetVictim() == bot && !AI_VALUE2(bool, "is cc", "current target") && CastSpellAction::isUseful();
    return PriestPvpCcTargetOk(botAI) && CastSpellAction::isUseful();
}

bool CastSilenceOnEnemyCasterAction::isUseful()
{
    return AI_VALUE(bool, "in pvp instance") && CastSpellOnEnemyCasterAction::isUseful();
}

// Lane 8 / #863: Holy Word: Chastise only becomes Holy Word: Sanctuary under Chakra: Sanctuary (MoP rotation row 25716).
bool CastHolyWordSanctuaryOnPartyAction::isUseful()
{
    return botAI->HasAura("chakra: sanctuary", bot) && HealPartyMemberAction::isUseful();
}

// #812: Holy Word: Chastise only becomes Holy Word: Serenity under Chakra: Serenity (see PriestActions.h).
bool CastHolyWordSerenityOnPartyAction::isUseful()
{
    return botAI->HasAura("chakra: serenity", bot) && HealPartyMemberAction::isUseful();
}
