/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "SpellCastUsefulValue.h"

#include "LastSpellCastValue.h"
#include "Playerbots.h"

bool SpellCastUsefulValue::Calculate()
{
    uint32 spellid = AI_VALUE2(uint32, "spell id", qualifier);
    if (!spellid)
        return true;  // there can be known alternatives

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if (!spellInfo)
        return true;  // there can be known alternatives

    if ((spellInfo->Attributes & SPELL_ATTR0_ON_NEXT_SWING_2) != 0 ||
        (spellInfo->Attributes & SPELL_ATTR0_ON_NEXT_SWING) != 0)
    {
        if (Spell* spell = bot->GetCurrentSpell(CURRENT_MELEE_SPELL))
            if (spell->m_spellInfo->Id == spellid && spell->IsNextMeleeSwingSpell() &&
                bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING))
                return false;
    }
    else
    {
        // uint32 lastSpellId = AI_VALUE(LastSpellCast&, "last spell cast").id;
        // if (spellid == lastSpellId)
        //     if (Spell* const pSpell = bot->FindCurrentSpellBySpellId(lastSpellId))
        //         return false;
    }

    if (spellInfo->IsAutoRepeatRangedSpell() && bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL) &&
        bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->m_spellInfo->Id == spellid)
    {
        return false;
    }

    // TODO: workaround
    if (qualifier == "windfury weapon" || qualifier == "flametongue weapon" ||
        qualifier == "frostbrand weapon" || qualifier == "rockbiter weapon" ||
        qualifier == "earthliving weapon" || qualifier == "spellstone")
    {
        if (Item* item = AI_VALUE2(Item*, "item for spell", spellid))
            if (item->IsInWorld() && item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                return false;
    }

    // NOTE (issue #98): upstream's "skip spells list" block is deliberately NOT ported here.
    // AI_VALUE(std::set<uint32>&, "skip spells list") dereferences a value that is not available in
    // every context in this fork and crashed the worldserver (ACCESS_VIOLATION in
    // SpellCastUsefulValue::Calculate, archive [22-9_14-52-17]). Re-add it only with a null-safe
    // lookup that is validated on a live run.
    return true;
}
