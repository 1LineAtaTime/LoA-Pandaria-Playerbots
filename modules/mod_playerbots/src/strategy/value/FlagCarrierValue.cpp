/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/PvpValues.cpp) into the
 * Legends of Azeroth Pandaria fork.
 */

#include "FlagCarrierValue.h"

// DatabaseEnv.h must precede the battleground headers: they reach BattlegroundQueue.h, which pulls
// in RatedPvp.h, which uses CharacterDatabaseTransaction without declaring it.
#include "DatabaseEnv.h"

#include "Battleground.h"
#include "BattlegroundEY.h"
#include "BattlegroundTP.h"
#include "BattlegroundWS.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

Unit* FlagCarrierValue::Calculate()
{
    if (!bot || !bot->InBattleground())
        return nullptr;

    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return nullptr;

    // Branch on the MAP rather than on GetBattlegroundTypeId(). A battleground entered through the
    // Random Battleground queue keeps BATTLEGROUND_RB as its type id (CreateNewBattleground does
    // SetTypeID(originalBgTypeId)), so a type check would silently do nothing for exactly the queue
    // most players use. The map is always the real one.
    uint32 const mapId = bg->GetMapId();

    ObjectGuid carrierGuid;

    switch (mapId)
    {
        case 489:  // Warsong Gulch
        case 726:  // Twin Peaks - a Warsong clone, same per-team flag pickers
        case 1105: // Deepwind Gorge (#798, no AC spec - AC WSG pattern extended): BattlegroundDG keeps the same per-team
                   // picker table (m_flagKeepers[team], BattlegroundDG.h:319/327-328): the picker at index T is the
                   // player carrying TEAM T's cart, i.e. a thief of the other team - so the cart carrier of the
                   // enemy (the thief of OUR cart) is again the one at our own team index. Mirrors AC Ai/Base/Value/PvpValues.cpp:20-44
                   // (BG_WS branch: GetFlagPickerGUID(sameTeam ? enemy : own team index)).
        {
            // GetFlagPickerGUID(team) returns the player carrying THAT team's flag, so the enemy
            // carrier is the one holding our own flag.
            // #572 (MoP-only): the BG side, not the race - with Battleground.IgnoreFaction = 1 a bot can
            // play for the other faction's team (same rule as BGTactics::BgTeamIdOf).
            TeamId const ourTeam = bot->GetBGTeam() == ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE;
            TeamId const wanted = sameTeam ? (ourTeam == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE) : ourTeam;

            carrierGuid = bg->GetFlagPickerGUID(wanted);
            break;
        }
        case 566:  // Eye of the Storm - one flag, either side can hold it
        {
            ObjectGuid const guid = bg->GetFlagPickerGUID();
            if (guid.IsEmpty())
                return nullptr;

            Player* fc = ObjectAccessor::GetPlayer(bg->GetBgMap(), guid);
            if (!fc)
                return nullptr;

            bool const isOurs = fc->GetBGTeam() == bot->GetBGTeam();  // #572: BG side, not race
            if (sameTeam != isOurs)
                return nullptr;

            carrierGuid = guid;
            break;
        }
        default:
            return nullptr;
    }

    if (carrierGuid.IsEmpty())
        return nullptr;

    Player* carrier = ObjectAccessor::GetPlayer(bg->GetBgMap(), carrierGuid);
    if (!carrier)
        return nullptr;

    if (!ignoreRange && !bot->IsWithinDistInMap(carrier, sPlayerbotAIConfig->sightDistance))
        return nullptr;

    return carrier;
}
