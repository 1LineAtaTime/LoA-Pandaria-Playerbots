/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Not one of issue #149's 27 names, but the same bug: "item for spell" has no
 * NamedObjectContext<UntypedValue> creator (ValueContext.h had it commented out) even though
 * HasItemForSpellTrigger::IsActive() (strategy/triggers/GenericTriggers.cpp:601),
 * CastItemSpellAction (strategy/actions/GenericSpellActions.cpp:221), SpellCastUsefulValue.cpp:48
 * and PlayerbotAI.cpp:2451 all call AI_VALUE2(Item*, "item for spell", spellId) live. AC's
 * ItemForSpellValue::Calculate() (Ai/Base/Value/ItemForSpellValue.cpp) reads SpellInfo::SpellName /
 * ::Targets fields that do not exist in this fork's TrinityCore-derived SpellInfo, so rather than
 * mis-port it this is a neutral stub returning nullptr -- callers already null-check the result.
 */

#ifndef _PLAYERBOT_ITEMFORSPELLVALUE_H
#define _PLAYERBOT_ITEMFORSPELLVALUE_H

#include "NamedObjectContext.h"
#include "Value.h"

class Item;
class PlayerbotAI;

class ItemForSpellValue : public CalculatedValue<Item*>, public Qualified
{
public:
    ItemForSpellValue(PlayerbotAI* botAI, std::string const name = "item for spell")
        : CalculatedValue<Item*>(botAI, name, 1)
    {
    }

    Item* Calculate() override;
};

#endif
