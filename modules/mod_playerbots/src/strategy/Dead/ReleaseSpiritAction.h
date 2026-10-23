/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RELEASESPIRITACTION_H
#define _PLAYERBOT_RELEASESPIRITACTION_H

#include <cstdint>
#include <ctime>

#include "Action.h"
#include "ReviveFromCorpseAction.h"

class PlayerbotAI;

class ReleaseSpiritAction : public Action
{
public:
    ReleaseSpiritAction(PlayerbotAI* botAI, std::string const name = "release") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class AutoReleaseSpiritAction : public ReleaseSpiritAction
{
public:
    AutoReleaseSpiritAction(PlayerbotAI* botAI, std::string const name = "auto release")
        : ReleaseSpiritAction(botAI, name)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    /// Ported from AC mod-playerbots ReleaseSpiritAction.cpp:119-167. Finds a friendly spirit
    /// healer near the dead battleground bot, teleports to it when out of click range (IoC res
    /// point) and gossips it so the core queues the resurrection.
    bool HandleBattlegroundSpiritHealer();

    /// Ported from AC mod-playerbots ReleaseSpiritAction.cpp:194-218. Delays the release to
    /// spirit by 6 seconds in a battleground so a ranged spell already in flight is not
    /// re-directed onto the ghost.
    bool ShouldDelayBattlegroundRelease();

    uint32_t bg_gossip_time = 0;
    time_t bgReleaseAttemptTime = 0;
};

/// Sends CMSG_SELF_RES when the core has offered one (soulstone, reincarnation, ...). Ported from
/// the reference (Ai/Base/Actions/ReleaseSpiritAction.cpp:256). Without it the
/// "can self resurrect" -> "self resurrect" trigger wired in DeadStrategy is inert.
class SelfResurrectAction : public Action
{
public:
    SelfResurrectAction(PlayerbotAI* botAI) : Action(botAI, "self resurrect") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class RepopAction : public SpiritHealerAction
{
public:
    RepopAction(PlayerbotAI* botAI, std::string const name = "repop") : SpiritHealerAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

/// #149: ported from AC mod-playerbots AcceptResurrectAction.cpp, adapted to this core. AC builds and
/// hands the session a raw CMSG_RESURRECT_RESPONSE packet parsed out of the incoming SMSG_RESURRECT_REQUEST;
/// on this core that packet's GUID is bit-packed with a variable-length caster name spliced into the byte
/// sequence (Spell::SendResurrectRequest, Spell.cpp), so instead this uses Player::ResurrectUsingRequestData(),
/// the same call the opcode handler makes once it decides to accept (MiscHandler.cpp
/// HandleResurrectResponseOpcode), without needing to reconstruct the packet at all.
class AcceptResurrectAction : public Action
{
public:
    AcceptResurrectAction(PlayerbotAI* botAI) : Action(botAI, "accept resurrect") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
