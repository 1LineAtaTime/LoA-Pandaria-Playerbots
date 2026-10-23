/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Trigger/PvpTriggers.h and the
 * TimerBGTrigger from GenericTriggers.h) into the Legends of Azeroth Pandaria fork.
 *
 * These are the triggers the in-battleground strategies fire on. Only the ones those strategies
 * actually reference are ported; the rest of upstream's PvP triggers belong to strategies this
 * fork does not have.
 */

#ifndef _PLAYERBOT_PVPTRIGGERS_H
#define _PLAYERBOT_PVPTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;
class Player;

/// A hostile player is a valid target (world PvP and battlegrounds). Drives the "pvp" strategy's
/// "attack enemy player" action; ported from AC mod-playerbots (PvpTriggers).
class EnemyPlayerNear : public Trigger
{
public:
    EnemyPlayerNear(PlayerbotAI* botAI) : Trigger(botAI, "enemy player near", 3) {}

    bool IsActive() override;
};

/// In a battleground that has not started yet (doors still closed).
class BgWaitingTrigger : public Trigger
{
public:
    BgWaitingTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bg waiting", 30) {}

    bool IsActive() override;
};

/// In a battleground that is under way.
class BgActiveTrigger : public Trigger
{
public:
    BgActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bg active", 1) {}

    bool IsActive() override;
};

/// Queued, and the invitation to enter is waiting to be answered.
class BgInviteActiveTrigger : public Trigger
{
public:
    BgInviteActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bg invite active", 10) {}

    bool IsActive() override;
};

/// The bot is sitting in a battleground queue that can no longer pop: nobody is waiting in it and
/// no instance of it is running. Bots only ever join a queue to fill it for a real player
/// (shouldJoinBg), so once that player is gone the bot is waiting for something that will never
/// happen - and it keeps its queue slot, so it cannot join a queue that DOES need filling.
///
/// The condition is deliberately the mirror image of shouldJoinBg: leave a queue you would not now
/// join. An invitation already in flight is never dropped.
class BgQueueDeadTrigger : public Trigger
{
public:
    BgQueueDeadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bg queue dead", 5) {}

    bool IsActive() override;
};

/// The bot is carrying a flag and is in a position to run it in.
class PlayerHasFlag : public Trigger
{
public:
    PlayerHasFlag(PlayerbotAI* botAI) : Trigger(botAI, "player has flag") {}

    bool IsActive() override;
    static bool IsCapturingFlag(Player* bot);
};

/// The enemy flag carrier is close enough to be worth chasing.
class EnemyFlagCarrierNear : public Trigger
{
public:
    EnemyFlagCarrierNear(PlayerbotAI* botAI) : Trigger(botAI, "enemy flagcarrier near") {}

    bool IsActive() override;
};

/// Our own flag carrier is close enough to be worth escorting.
class TeamFlagCarrierNear : public Trigger
{
public:
    TeamFlagCarrierNear(PlayerbotAI* botAI) : Trigger(botAI, "team flagcarrier near") {}

    bool IsActive() override;
};

/// Fires at most once a minute; used to force a periodic objective re-think.
class TimerBGTrigger : public Trigger
{
public:
    TimerBGTrigger(PlayerbotAI* botAI) : Trigger(botAI, "timer bg"), lastCheck(0) {}

    bool IsActive() override;

private:
    time_t lastCheck;
};

/// Isle of Conquest (#393): a vehicle is within reach (AC Ai/Base/Trigger/PvpTriggers.h:128-134).
class VehicleNearTrigger : public Trigger
{
public:
    VehicleNearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "vehicle near", 10) {}

    bool IsActive() override;
};

/// The bot is aboard a vehicle (AC Ai/Base/Trigger/PvpTriggers.h:136-142). Drives IsleStrategy's weapon actions.
class InVehicleTrigger : public Trigger
{
public:
    InVehicleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "in vehicle") {}

    bool IsActive() override;
};

/// Alliance-only, Alterac Valley: the balanced-strategy Alliance side does not yet fully control the
/// Snowfall Graveyard. Drives AlteracStrategy's emergency "bg move to objective" (AC-identical,
/// Ai/Base/Trigger/PvpTriggers.h:144-150).
class AllianceNoSnowfallGY : public Trigger
{
public:
    AllianceNoSnowfallGY(PlayerbotAI* botAI) : Trigger(botAI, "alliance no snowfall gy") {}

    bool IsActive() override;
};

#endif
