/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Gurub (Cataclysm 5-man) triggers, MoP-only lane L7 (#670). Pattern: AC Ai/Dungeon/HoS/HoSTriggers.h.

#ifndef PLAYERBOTS_CATAZGTRIGGERS_H
#define PLAYERBOTS_CATAZGTRIGGERS_H

#include "CataZgShared.h"
#include "Trigger.h"

// The bot stands inside (or on the edge of) a ground hazard of the given kind.
class CataZgHazardTrigger : public Trigger
{
public:
    CataZgHazardTrigger(PlayerbotAI* botAI, std::string const name, CataZulGurub::HazardKind kind)
        : Trigger(botAI, name), _kind(kind)
    {
    }

    bool IsActive() override;

private:
    CataZulGurub::HazardKind _kind;
};

// Venoxis: the bot is Toxic-Linked and still close to its partner.
class CataZgToxicLinkTrigger : public Trigger
{
public:
    CataZgToxicLinkTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zg toxic link") {}
    bool IsActive() override;
};

// Venoxis (#670 w68): the tank has to walk him off the poisoned altar to the floor spot.
class CataZgVenoxisTankSpotTrigger : public Trigger
{
public:
    CataZgVenoxisTankSpotTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zg venoxis tank spot") {}
    bool IsActive() override;
};

// Jin'do: Shadows of Hakkar is up and the bot is outside a Deadzone.
class CataZgShadowsTrigger : public Trigger
{
public:
    CataZgShadowsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zg shadows of hakkar") {}
    bool IsActive() override;
};

// A priority add exists for this bot to focus.
class CataZgFocusTrigger : public Trigger
{
public:
    CataZgFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zg focus target") {}
    bool IsActive() override;
};

// Jin'do phase 2: this bot fetches the next Gurubashi Spirit.
class CataZgSpiritPullTrigger : public Trigger
{
public:
    CataZgSpiritPullTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zg pull gurubashi spirit") {}
    bool IsActive() override;
};

// Jin'do phase 2: this bot is not yet standing on the shielded chain the group works on.
class CataZgChainStandTrigger : public Trigger
{
public:
    CataZgChainStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zg hakkar chains") {}
    bool IsActive() override;
};

// Jin'do phase 2: a non-tank bot is hitting the Gurubashi Spirit while a chain is still shielded.
class CataZgSpareSpiritTrigger : public Trigger
{
public:
    CataZgSpareSpiritTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zg spare gurubashi spirit") {}
    bool IsActive() override;
};

// Zanzil (#822): this bot is the elected one for the Frostburn Formula cauldron and a Zanzili Berserker is up.
class CataZgCauldronTrigger : public Trigger
{
public:
    CataZgCauldronTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zg frostburn cauldron") {}
    bool IsActive() override;
};

// Zanzil (#679): Graveyard Gas is on and this bot has no Toxic Torment yet.
class CataZgGraveyardGasTrigger : public Trigger
{
public:
    CataZgGraveyardGasTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zg graveyard gas") {}
    bool IsActive() override;
};

// Kilnara / Zanzil / Venoxis is casting something worth interrupting.
class CataZgInterruptTrigger : public Trigger
{
public:
    CataZgInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, "zg interrupt") {}
    bool IsActive() override;
};

#endif
