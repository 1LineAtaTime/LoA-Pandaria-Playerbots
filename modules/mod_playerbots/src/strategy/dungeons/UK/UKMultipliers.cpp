/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UKMultipliers.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "UKActions.h"
#include "UKTriggers.h"

// AC Ai/Dungeon/UK/UKMultipliers.cpp (PrinceKelesethMultiplier::GetValue)
float PrinceKelesethMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!boss) { return 1.0f; }

    // Suppress auto-targeting behaviour only when a tomb is up
    if (dynamic_cast<DpsAssistAction*>(action))
    {
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            Unit* unit = botAI->GetUnit(member);
            if (unit && unit->HasAura(SPELL_FROST_TOMB))
            {
                return 0.0f;
            }
        }
    }
    return 1.0f;
}

// AC Ai/Dungeon/UK/UKMultipliers.cpp (SkarvaldAndDalronnMultiplier::GetValue)
float SkarvaldAndDalronnMultiplier::GetValue(Action* action)
{
    // Only need to deal with Dalronn here. If he's dead, just fall back to normal dps strat
    Unit* dalronn = AI_VALUE2(Unit*, "find target", "dalronn the controller");
    if (!dalronn) { return 1.0f; }

    // Only suppress DpsAssistAction if Dalronn is alive
    if (dalronn->isTargetableForAttack() && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}

// AC Ai/Dungeon/UK/UKMultipliers.cpp (IngvarThePlundererMultiplier::GetValue)
float IngvarThePlundererMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ingvar the plunderer");
    bool isTank = PlayerBotSpec::IsTank(bot);
    if (!boss) { return 1.0f; }

    // Prevent movement actions overriding current movement, we're probably dodging a slam
    if (isTank && bot->isMoving() && dynamic_cast<MovementAction*>(action))
    {
        return 0.0f;
    }

    // If boss is casting a roar, do not allow beginning a spell cast that is non-instant
    if (boss->HasUnitState(UNIT_STATE_CASTING))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_STAGGERING_ROAR) ||
            boss->FindCurrentSpellBySpellId(SPELL_DREADFUL_ROAR))
        {
            if (dynamic_cast<CastSpellAction*>(action))
            {
                uint32 spellId = AI_VALUE2(uint32, "spell id", action->getName());
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
                if (!spellInfo) { return 1.0f; }

                uint32 castTime = spellInfo->CalcCastTime(bot->GetLevel());
                // 5.4.8: AC SpellInfo::CalcCastTime(Unit*) -> fork CalcCastTime(uint8 level = 0, Spell* = NULL)
                // (src/server/game/Spells/SpellInfo.h:504); closest faithful equivalent is the bot level.
                if (castTime != 0)
                {
                    return 0.0f;
                }
            }
        }
        // Done with non-tank logic
        if (!isTank) { return 1.0f; }

        // TANK ONLY
        if (boss->FindCurrentSpellBySpellId(SPELL_SMASH) ||
            boss->FindCurrentSpellBySpellId(SPELL_DARK_SMASH))
        {
            // Prevent movement actions during smash which can mess up boss position.
            // Allow through IngvarDodgeSmashAction only, as well as any non-movement actions.
            if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<IngvarDodgeSmashAction*>(action))
            {
                return 0.0f;
            }
        }
    }
    return 1.0f;
}
