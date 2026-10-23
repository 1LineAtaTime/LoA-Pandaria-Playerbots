/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported 1:1 from AC mod-playerbots Ai/Base/Value/FishValues.cpp:14-55 (issue #412). The only
 * adaptation is the includes; Player::GetSkillValue, GameObject::getLootState/GetRespawnTime/
 * GetOwnerGUID and FISHING_BOBBER_READY_TIME (GameObject.h:133) all exist on this core.
 * CanFishValue also clears a stale swim flag (MoP-only, see the comment there).
 */

#include "FishValues.h"

#include "FishingAction.h"
#include "GameObject.h"
#include "Playerbots.h"

bool CanFishValue::Calculate()
{
    int32 SkillFishing = bot->GetSkillValue(SKILL_FISHING);

    if (SkillFishing == 0)
        return false;

    if (bot->isSwimming())
    {
        // AC parity: really swimming means "cannot fish". A stale swim flag on dry land is cleared by
        // RefreshBotLiquidMovementFlags (MovementActions.cpp, #590), called every AI tick.
        if (bot->IsInWater())
            return false;
    }

    if (bot->IsInCombat())
        return false;

    return true;
}

bool CanUseFishingBobberValue::Calculate()
{
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects no los");
    for (auto const& guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
        {
            if (go->GetEntry() != FISHING_BOBBER)
                continue;
            if (go->GetOwnerGUID() != bot->GetGUID())
                continue;

            // #412 evidence only (no AC counterpart): proves the bobber spawned and when it bites. AC's bite signal
            // is the same on 5.4.8: GameObject::Update turns a FISHINGNODE GO_READY once
            // time() > respawnTime - FISHING_BOBBER_READY_TIME (GameObject.cpp:429-450, AC GameObject.cpp:502).
            TC_LOG_DEBUG("playerbots", "[BOBBER] %s guid=%u lootState=%u readyIn=%d", bot->GetName().c_str(),
                         uint32(go->GetGUID().GetCounter()), uint32(go->getLootState()),
                         int32(go->GetRespawnTime() - FISHING_BOBBER_READY_TIME - time(nullptr)));

            if (go->getLootState() == GO_READY)
                return true;

            // Not ready yet -> delay next check
            time_t bobberActiveTime = go->GetRespawnTime() - FISHING_BOBBER_READY_TIME;
            if (bobberActiveTime > time(0))
                botAI->SetNextCheckDelay((bobberActiveTime - time(0)) * IN_MILLISECONDS + 500);
            else
                botAI->SetNextCheckDelay(1000);

            return false;
        }
    }
    return false;
}
