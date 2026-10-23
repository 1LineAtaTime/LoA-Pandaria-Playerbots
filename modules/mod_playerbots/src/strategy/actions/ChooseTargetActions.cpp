/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChooseTargetActions.h"
#include "BotScenario.h"
#include <map>

#include "Event.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "PvpTriggers.h"
#include "RtiTargetValue.h"
#include "ServerFacade.h"

bool AttackEnemyPlayerAction::isUseful()
{
    // AC parity: a flag carrier does not pick fights
    if (PlayerHasFlag::IsCapturingFlag(bot))
        return false;

    return !sPlayerbotAIConfig->IsPvpProhibited(bot->GetZoneId(), bot->GetAreaId());
}

bool AttackEnemyFlagCarrierAction::isUseful()
{
    // AC parity (AC ChooseTargetActions.cpp:27-32): a carrier attacks the enemy carrier within 100 yd
    Unit* target = context->GetValue<Unit*>("enemy flag carrier")->Get();
    return target && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target), 100.0f) &&
           PlayerHasFlag::IsCapturingFlag(bot);
}

// #269 (AC ChooseTargetActions.cpp:34-40).
bool AggressiveTargetAction::isUseful()
{
    if (bot->IsInCombat())
        return false;

    return true;
}

bool AttackAnythingAction::isUseful()
{
    // AC parity (issue #61): AC ChooseTargetActions.cpp:104-133. The MoP-only status/25-yd gate and
    // "can move around" check are dropped; AC has neither.
    if (!bot || !botAI)
        return false;

    // Test harness: a wander scenario started with "nofight" does not start fights.
    if (BotScenario::NoFight(bot))
        return false;

    if (!botAI->AllowActivity(GRIND_ACTIVITY))
        return false;

    if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT))
        return false;

    if (bot->IsInCombat())
        return false;

    Unit* target = GetTarget();
    if (!target || !target->IsInWorld())
        return false;

    std::string const name = std::string(target->GetName());
    if (!name.empty() &&
        (name.find("Dummy") != std::string::npos || name.find("Charge Target") != std::string::npos ||
         name.find("Melee Target") != std::string::npos || name.find("Ranged Target") != std::string::npos))
        return false;

    return true;
}

bool DropTargetAction::Execute(Event event)
{
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    if (target && target->isDead())
    {
        ObjectGuid guid = target->GetGUID();
        // #151 (AC ChooseTargetActions.cpp:49): re-enabled now that "available loot" is a real
        // LootObjectStack* (loot-stack-port.md section 2 step 4.6).
        if (guid)
            context->GetValue<LootObjectStack*>("available loot")->Get()->Add(guid);
    }

    context->GetValue<Unit*>("current target")->Set(nullptr);

    bot->SetTarget(ObjectGuid::Empty);
    bot->SetSelection(ObjectGuid());
    botAI->ChangeEngine(BOT_STATE_NON_COMBAT);
    // #269 (AC ChooseTargetActions.cpp:65-70): stop the hunter's auto shot, otherwise the pet keeps
    // hitting the corpse the target was dropped from.
    if (bot->GetClass() == CLASS_HUNTER)
    {
        Spell const* spell = bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL);
        if (spell && spell->m_spellInfo->Id == 75)
            bot->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
    }
    bot->AttackStop();

    return true;
}

bool AttackAnythingAction::Execute(Event event)
{
    bool result = AttackAction::Execute(event);
    if (!sPlayerbotAIConfig->debugBotNames.empty() && sPlayerbotAIConfig->debugBotNames.count(bot->GetName()))
        TC_LOG_DEBUG("playerbots", "[ATTACK] %s attack-anything result=%d target=%s", bot->GetName().c_str(), result ? 1 : 0,
                     GetTarget() ? GetTarget()->GetName().c_str() : "(none)");
    if (result)
    {
        if (Unit* grindTarget = GetTarget())
        {
            if (char const* grindName = grindTarget->GetName().c_str())
            {
                context->GetValue<ObjectGuid>("pull target")->Set(grindTarget->GetGUID());
                bot->GetMotionMaster()->Clear();
            }
        }
    }

    return result;
}

bool AttackAnythingAction::isPossible()
{
    return AttackAction::isPossible() && GetTarget();
}

bool DpsAssistAction::isUseful()
{
    // #269 (AC ChooseTargetActions.cpp:142-148): the flag-carrier guard was commented out in this
    // fork; a bot that is capturing a flag does not pick a DPS-assist target.
    if (PlayerHasFlag::IsCapturingFlag(bot))
        return false;

    return true;
}

bool AttackRtiTargetAction::Execute(Event event)
{
    Unit* rtiTarget = AI_VALUE(Unit*, "rti target");

    // #269 (AC ChooseTargetActions.cpp:154-169): fallback - if the "rti target" value did not resolve
    // a valid unit yet, try to resolve the raid icon directly from the group.
    if (!rtiTarget)
    {
        if (Group* group = bot->GetGroup())
        {
            std::string const rti = AI_VALUE(std::string, "rti");
            int32 const index = RtiTargetValue::GetRtiIndex(rti);
            if (index >= 0)
            {
                ObjectGuid const guid = group->GetTargetIcon(index);
                if (!guid.IsEmpty())
                    rtiTarget = botAI->GetUnit(guid);
            }
        }
    }

    if (rtiTarget && rtiTarget->IsInWorld() && rtiTarget->GetMapId() == bot->GetMapId())
    {
        botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({ rtiTarget->GetGUID() });
        bool result = Attack(botAI->GetUnit(rtiTarget->GetGUID()));
        if (result)
        {
            context->GetValue<ObjectGuid>("pull target")->Set(rtiTarget->GetGUID());
            return true;
        }
    }
    else
    {
        botAI->TellError("I dont see my rti attack target");
    }

    return false;
}

bool AttackRtiTargetAction::isUseful()
{
    if (botAI->ContainsStrategy(STRATEGY_TYPE_HEAL))
        return false;

    return true;
}
