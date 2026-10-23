/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" (Scarlet Halls, epic #373, issue #657). See MopShShared.h for sources.

#include "MopShTriggers.h"
#include "MopShShared.h"
#include "Creature.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Spell.h"

using namespace MopScarletHalls;

bool MopShKnifeLineTrigger::IsActive()
{
    Unit* boss = FindBoss(bot, NPC_HOUNDMASTER_BRAUN);
    if (!boss)
        return false;

    Spell* spell = boss->FindCurrentSpellBySpellId(SPELL_PIERCING_THROW);
    if (!spell)
        return false;

    // GUID, not GetUnitTarget(): the target pointer is not safe before the spell's first update (Spell.h:665).
    Unit* victim = botAI->GetUnit(spell->m_targets.GetUnitTargetGUID());
    if (!victim || victim == bot)
        return false;

    return bot->IsInBetween(boss, victim, KNIFE_LINE_HALF_WIDTH);
}

bool MopShBladesOfLightTrigger::IsActive()
{
    if (!BladesOfLightActive(bot))
        return false;

    // #781: only a bot that is not yet clear of Harlan and his route ahead runs (the multiplier still freezes the
    // other movement of everybody while the blades are up)
    Creature* boss = FindBoss(bot, NPC_ARMSMASTER_HARLAN);
    return boss && BladesSafety(boss, bot->GetPositionX(), bot->GetPositionY()) < 1.0f;
}

bool MopShKoeglerInterruptTrigger::IsActive()
{
    Unit* boss = FindBoss(bot, NPC_FLAMEWEAVER_KOEGLER);
    return boss && CurrentCastPriority(boss) > 0 && boss->IsNonMeleeSpellCasted(true);
}

bool MopShBurningBooksTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    float clearDistance = 0.0f;
    return FindBurningBookcase(bot, clearDistance) != nullptr;
}

bool MopShBookBurnerInterceptTrigger::IsActive()
{
    // #683 w106: the elected interceptor (BookBurnerInterceptor), not always the tank.
    if (bot->HasSpiritOfRedemptionAura())
        return false;

    Unit* boss = nullptr;
    Unit* bookcase = nullptr;
    if (!BookBurnerAimed(bot, boss, bookcase))
        return false;

    if (bot->IsInBetween(boss, bookcase, BOOK_BURNER_BEAM_HALF_WIDTH - 0.5f))
        return false;

    return BookBurnerInterceptor(bot, boss, bookcase) == bot;
}

bool MopShDragonBreathTrigger::IsActive()
{
    Unit* boss = nullptr;
    return InDragonBreathCone(bot, boss);
}
