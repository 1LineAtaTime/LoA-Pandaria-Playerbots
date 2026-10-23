#include "AttackerWithoutAuraTargetValue.h"

#include "Playerbots.h"
#include "Strategy.h"
#include "TargetValue.h"

Unit* AttackerWithoutAuraTargetValue::Calculate()
{
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    // #413 (AC AttackerWithoutAuraTargetValue.cpp:15,22)
    GuidSet const dynamicExclusions = GatherStrategyTargetExclusions(botAI, TargetValueExclusionType::Attacker);
    // Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    uint32 max_health = 0;
    Unit* result = nullptr;
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || dynamicExclusions.find(guid) != dynamicExclusions.end())
            continue;

        if (!bot->IsWithinCombatRange(unit, botAI->GetRange(range)))
            continue;

        if (unit->GetHealth() < max_health)
        {
            continue;
        }

        if (!botAI->HasAura(qualifier, unit, false, true))
        {
            max_health = unit->GetHealth();
            result = unit;
        }
    }

    return result;
}

Unit* MeleeAttackerWithoutAuraTargetValue::Calculate()
{
    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    // #413 (AC AttackerWithoutAuraTargetValue.cpp:44)
    GuidSet const dynamicExclusions = GatherStrategyTargetExclusions(botAI, TargetValueExclusionType::Attacker);
    // Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    uint32 max_health = 0;
    Unit* result = nullptr;
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || dynamicExclusions.find(guid) != dynamicExclusions.end())
            continue;

        if (!bot->IsWithinMeleeRange(unit))
            continue;
        
        if (checkArc && !bot->HasInArc(CAST_ANGLE_IN_FRONT, unit))
            continue;

        if (unit->GetHealth() < max_health)
        {
            continue;
        }

        if (!botAI->HasAura(qualifier, unit, false, true))
        {
            max_health = unit->GetHealth();
            result = unit;
        }
    }

    return result;
}
