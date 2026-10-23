/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/AoeHealValues.cpp) into the
 * Legends of Azeroth Pandaria fork (issue #149).
 */

#include "AoeHealValues.h"

#include "Playerbots.h"

uint8 AoeHealValue::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    float range = 0;
    if (qualifier == "low")
        range = static_cast<float>(sPlayerbotAIConfig->lowHealth);
    else if (qualifier == "medium")
        range = static_cast<float>(sPlayerbotAIConfig->mediumHealth);
    else if (qualifier == "critical")
        range = static_cast<float>(sPlayerbotAIConfig->criticalHealth);
    else if (qualifier == "almost full")
        range = static_cast<float>(sPlayerbotAIConfig->almostFullHealth);

    uint8 count = 0;
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* player = ObjectAccessor::FindPlayer(itr->guid);
        if (!player || !player->IsAlive())
            continue;

        if (player->GetDistance(bot) >= sPlayerbotAIConfig->sightDistance)
            continue;

        float percent = (static_cast<float>(player->GetHealth()) / player->GetMaxHealth()) * 100;
        if (percent <= range)
            ++count;
    }

    return count;
}
