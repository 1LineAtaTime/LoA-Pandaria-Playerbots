/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/ListSpellsAction.h (#611).
 * Not driven on either core: only CastRandomSpellAction ("cast random spell"/"craft random item") reaches it.
 */

#ifndef PLAYERBOTS_LISTSPELLSACTION_H
#define PLAYERBOTS_LISTSPELLSACTION_H

#include "InventoryAction.h"

#include <string>
#include <utility>
#include <vector>

class PlayerbotAI;

class ListSpellsAction : public InventoryAction
{
public:
    ListSpellsAction(PlayerbotAI* botAI, std::string const name = "spells") : InventoryAction(botAI, name) {}

    bool Execute(Event event) override;
    virtual std::vector<std::pair<uint32, std::string>> GetSpellList(std::string filter = "");

    static void InitSpellCaches();

};

#endif
