/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TradeValues.{h,cpp} for issue #193 (S7 of epic #163).
 * Registered as the "items useful to give" AI value (strategy/value/ValueContext.h), AC's key (AC
 * ValueContext.h:309; #272 renamed it from the class's default name "useful to give", which nothing read). See
 * TradeValues.cpp for the 5.4.8 adaptations.
 */

#ifndef PLAYERBOTS_TRADEVALUES_H
#define PLAYERBOTS_TRADEVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"

class Item;
class PlayerbotAI;

class ItemsUsefulToGiveValue : public CalculatedValue<std::vector<Item*>>, public Qualified
{
public:
    ItemsUsefulToGiveValue(PlayerbotAI* botAI, std::string const name = "useful to give") : CalculatedValue(botAI, name)
    {
    }

    std::vector<Item*> Calculate() override;
};

#endif
