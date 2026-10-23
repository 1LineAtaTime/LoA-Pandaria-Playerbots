/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/SnareTargetValue.cpp) into
 * the Legends of Azeroth Pandaria fork (issue #149).
 */

#include "SnareTargetValue.h"

#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "ServerFacade.h"

Unit* SnareTargetValue::Calculate()
{
    std::string const spell = qualifier;

    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        if (bot->GetDistance(unit) > botAI->GetRange("spell"))
            continue;

        Unit* chaseTarget;
        switch (unit->GetMotionMaster()->GetCurrentMovementGeneratorType())
        {
            case FLEEING_MOTION_TYPE:
                return unit;
            case CHASE_MOTION_TYPE:
            {
                chaseTarget = sServerFacade->GetChaseTarget(unit);
                if (!chaseTarget)
                    continue;
                Player* chaseTargetPlayer = ObjectAccessor::FindPlayer(chaseTarget->GetGUID());
                // check if need to snare
                bool shouldSnare = true;

                // do not slow down if bot is melee and mob/bot attack each other
                if (chaseTargetPlayer && !PlayerBotSpec::IsRanged(bot) && chaseTargetPlayer == bot)
                    shouldSnare = false;

                if (!unit->isMoving())
                    shouldSnare = false;

                if (unit->HasAuraType(SPELL_AURA_MOD_ROOT))
                    shouldSnare = false;

                if (chaseTargetPlayer && shouldSnare && !PlayerBotSpec::IsTank(chaseTargetPlayer))
                {
                    return unit;
                }
                break;
            }
            default:
                break;
        }
    }

    return nullptr;
}
