/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RogueOpeningActions.h"

#include "Playerbots.h"

Value<Unit*>* CastSapAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

bool CastCheapShotAction::isUseful()
{
    return CastMeleeSpellAction::isUseful() &&
           (!AI_VALUE(bool, "in pvp instance") || !AI_VALUE2(bool, "is cc", "current target"));
}

bool CastAmbushAction::isUseful()
{
    // #604 (AC Rogue/Action/RogueOpeningActions.h:36-40 has no gate): 5.4.8 Ambush "Requires Stealth", but Shadow Dance 51713
    // (spec 261 L80, cls.py; Wowhead mop-classic tooltip 2026-10-05: "allowing the use of abilities that ordinarily require
    // Stealth") also enables it. Subterfuge keeps the Stealth aura itself for 3 s, so "stealth" already covers it.
    return CastMeleeSpellAction::isUseful() && (botAI->HasAura("stealth", bot) || botAI->HasAura("shadow dance", bot));
}
