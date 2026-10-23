/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/EnemyHealerTargetValue.cpp)
 * into the Legends of Azeroth Pandaria fork (issue #149).
 */

#include "EnemyHealerTargetValue.h"

#include "Playerbots.h"
#include "ServerFacade.h"

Unit* EnemyHealerTargetValue::Calculate()
{
    std::string const spell = qualifier;

    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit == target)
            continue;

        if (sServerFacade->GetDistance2d(bot, unit) > botAI->GetRange("spell"))
            continue;

        if (!botAI->IsInterruptableSpellCasting(unit, spell))
            continue;

        Spell* castingSpell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (castingSpell && castingSpell->m_spellInfo->IsPositive())
            return unit;

        castingSpell = unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        if (castingSpell && castingSpell->m_spellInfo->IsPositive())
            return unit;
    }

    return nullptr;
}
