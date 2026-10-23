/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/CancelChannelAction.cpp (#423). No behavioural divergence:
 * 5.4.8 keeps the same channel API AC uses (Unit::GetCurrentSpell(CurrentSpellTypes) - core Unit.h:2068,
 * Unit::InterruptSpell(CurrentSpellTypes, bool withDelayed = true, bool withInstant = true) - core Unit.h:2055;
 * CURRENT_CHANNELED_SPELL is core Unit.h:789), so the port is line for line AC's.
 *
 * Deliberately NOT routed through PlayerbotAI::InterruptSpell() (AI/PlayerbotAI.cpp:2835): that helper
 * cancels every spell type from CURRENT_MELEE_SPELL to CURRENT_CHANNELED_SPELL, while AC's action (and this
 * port) cancels the channel only, which is what the "blizzard channel check" / "hurricane channel check"
 * trigger nodes mean by it.
 */

#include "CancelChannelAction.h"

#include "Player.h"
#include "PlayerbotAI.h"

// AC CancelChannelAction.cpp:11-21
bool CancelChannelAction::Execute(Event /*event*/)
{
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        bot->InterruptSpell(CURRENT_CHANNELED_SPELL);

        return true;
    }

    return false;
}
