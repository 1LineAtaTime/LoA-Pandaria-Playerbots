/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TELEPORTTOMASTERACTION_H
#define _PLAYERBOT_TELEPORTTOMASTERACTION_H

#include "MovementActions.h"

class Event;
class Player;
class PlayerbotAI;
struct MapEntry;

// Teleports the bot next to its master, through the same entry checks the core makes for a player. Run by the
// follow strategy when the master is on another map or instance (the "summon" chat command is AzerothCore's
// SummonAction). The bot
// teleports itself from its own AI update; the far teleport is then acknowledged by PlayerbotHolder::UpdateSessions.
class TeleportToMasterAction : public MovementAction
{
public:
    TeleportToMasterAction(PlayerbotAI* botAI) : MovementAction(botAI, "teleport to master") {}

    bool Execute(Event event) override;
    bool isPossible() override;

private:
    struct Destination
    {
        uint32 mapId = 0;
        uint32 instanceId = 0;

        bool operator==(Destination const& other) const
        {
            return mapId == other.mapId && instanceId == other.instanceId;
        }
    };

    bool CanLeave(Player* master, std::string& reason);
    bool CanRevive(Player* master, std::string& reason);  // read-only: ReviveBotWhenSummoned
    void Revive();                                        // only after CanEnter has passed
    bool CanEnter(Player* master, std::string& reason);
    std::string const GetEntryRefusal(MapEntry const* entry);
    bool Teleport(Player* master);
    void Refuse(Destination const& destination, std::string const& reason, bool manual);

    // State of the automatic follow, kept per destination so that a refusal is told once and retried slowly
    Destination lastDestination;
    uint32 attempts = 0;
    uint32 attemptTime = 0;
    bool refused = false;
    uint32 refusedTime = 0;
    std::string refusedReason;
};

#endif
