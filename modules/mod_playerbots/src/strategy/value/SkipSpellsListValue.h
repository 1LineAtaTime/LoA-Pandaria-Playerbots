/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/SkipSpellsListValue.h) into
 * the Legends of Azeroth Pandaria fork (issue #149). Note (issue #98): SpellCastUsefulValue.cpp
 * deliberately does not read this value yet, so porting the container itself does not reactivate
 * that dormant code path; it only means the name now exists instead of dereferencing a null Value.
 */

#ifndef _PLAYERBOT_SKIPSPELLSLISTVALUE_H
#define _PLAYERBOT_SKIPSPELLSLISTVALUE_H

#include "Value.h"
#include <set>

class PlayerbotAI;

class SkipSpellsListValue : public ManualSetValue<std::set<uint32>&>
{
public:
    SkipSpellsListValue(PlayerbotAI* botAI, std::string const name = "skip spells list")
        : ManualSetValue<std::set<uint32>&>(botAI, list, name)
    {
    }

    std::string const Save() override;
    bool Load(std::string const value) override;

private:
    std::set<uint32> list;
};

#endif
