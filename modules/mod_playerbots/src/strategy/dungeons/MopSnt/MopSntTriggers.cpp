/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple strategy "mop-snt" (epic #373, issue #656). See MopSntShared.h for sources.

#include "MopSntTriggers.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"

bool MopSntHazardTrigger::IsActive()
{
    auto const& hazard = AI_VALUE_REF(MopSnt::Hazard, "mop snt hazard");
    return hazard.kind == _kind;
}

bool MopSntInterruptTrigger::IsActive()
{
    if (!MopSnt::HasInterruptSpell(botAI))
        return false;

    Unit* target = MopSnt::FindInterruptTarget(bot, _group, MopSnt::SNT_NERONOK_CHASE_RANGE);
    if (!target)
        return false;

    // in range: the ready interrupters take turns (AC MgTStrategy.cpp:23 fires every bot; #781 staggers them)
    if (MopDungeon::CanInterruptNow(bot, target))
        return _gate.Allow(bot, target);

    // nobody can reach him yet: a bot with its interrupt ready runs to him - #656 w50: only when it gets there before the
    // channel ends (w50: 36 of 40 FAILED were chases from 80+ yd at a 4 s channel; the bridge approach covers the walk)
    float const range = MopDungeon::ReadyInterruptRange(bot);
    return range > 0.0f && MopDungeon::CountInterrupters(bot, target) == 0 &&
           MopSnt::CanReachBeforeCastEnds(bot, target, MopSnt::SNT_SPELL_GUSTING_WINDS, range);
}

bool MopSntFocusTrigger::IsActive()
{
    ObjectGuid const focus = AI_VALUE(ObjectGuid, "mop snt focus target");
    if (focus.IsEmpty())
        return false;

    Unit* current = AI_VALUE(Unit*, "current target");
    return !current || current->GetGUID() != focus;
}

bool MopSntSiegeBombTrigger::IsActive()
{
    if (!bot->IsAlive() || !bot->IsInCombat())
        return false;

    // the tank keeps the adds and the healers heal; the DPS break the Bulwark
    if (PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsHeal(bot) || !PlayerBotSpec::IsDps(bot))
        return false;

    if (!MopSnt::GetBulwarkedPavalak(bot))
        return false;

    if (bot->HasAura(MopSnt::SNT_SPELL_SIEGE_EXPLOSIVE_CARRY))
        return true;

    // #781 round 2: while the bot has a hazard to leave, the escape owns it; the pick-up waits (the hazard
    // multiplier would veto it anyway - wave 35 logged that 279 times as IMPOSSIBLE)
    auto const& hazard = AI_VALUE_REF(MopSnt::Hazard, "mop snt hazard");
    if (hazard.kind != MopSnt::HazardKind::None)
        return false;

    return MopSnt::FindClickableBomb(bot) != nullptr;
}

bool MopSntTarKegTrigger::IsActive()
{
    if (!bot->IsAlive() || PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsHeal(bot) || !PlayerBotSpec::IsDps(bot))
        return false;

    if (!MopSnt::FindTarTarget(bot))
        return false;

    if (bot->HasAura(MopSnt::SNT_SPELL_CARRYING_TAR))
        return true;

    // the escape owns a bot with a hazard to leave (as the siege bomb pick-up)
    auto const& hazard = AI_VALUE_REF(MopSnt::Hazard, "mop snt hazard");
    if (hazard.kind != MopSnt::HazardKind::None)
        return false;

    return MopSnt::FindClickableKeg(bot) != nullptr;
}

bool MopSntResinTrigger::IsActive()
{
    return bot->IsAlive() && bot->HasAura(MopSnt::SNT_SPELL_QUICK_DRY_RESIN) &&
           !bot->HasAura(MopSnt::SNT_SPELL_ENCASED_IN_RESIN);
}

bool MopSntNeronokFarTrigger::IsActive()
{
    // #656 w50: also while he is still flying to the other end (MopSnt::NeronokRegroupPoint)
    Creature* neronok = nullptr;
    float x = 0.0f;
    float y = 0.0f;
    bool flying = false;
    if (!MopSnt::NeronokRegroupPoint(bot, neronok, x, y, flying))
        return false;

    // a Sik'thik Builder (62633) met on the bridge is finished first
    Unit* current = AI_VALUE(Unit*, "current target");
    return !current || current == neronok || !current->IsAlive() ||
           bot->GetExactDist2d(current) > MopSnt::SNT_NERONOK_REGROUP_DIST;
}

bool MopSntSoakPuddleTrigger::IsActive()
{
    Creature* puddle = nullptr;
    return MopSnt::WantsToSoakPuddle(bot, puddle);
}
