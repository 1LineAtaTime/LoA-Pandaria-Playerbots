/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Gurub (Cataclysm 5-man) actions, MoP-only lane L7 (#670). Pattern: AC Ai/Dungeon/HoS/HoSActions.h.

#ifndef PLAYERBOTS_CATAZGACTIONS_H
#define PLAYERBOTS_CATAZGACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "CataZgShared.h"
#include "MovementActions.h"

// Step out of the nearest clear spot around the hazards of one kind (circles and cones).
class CataZgEscapeAction : public MovementAction
{
public:
    CataZgEscapeAction(PlayerbotAI* botAI, std::string const name, CataZulGurub::HazardKind kind)
        : MovementAction(botAI, name), _kind(kind)
    {
    }

    bool Execute(Event event) override;

private:
    CataZulGurub::HazardKind _kind;
};

// Venoxis: run away from the Toxic Link partner until the link breaks.
class CataZgBreakToxicLinkAction : public MovementAction
{
public:
    CataZgBreakToxicLinkAction(PlayerbotAI* botAI) : MovementAction(botAI, "zg break toxic link") {}
    bool Execute(Event event) override;
};

// Venoxis (#670 w68): the tank walks him off the poisoned altar to the floor spot (GetVenoxisTankSpot).
class CataZgVenoxisTankSpotAction : public MovementAction
{
public:
    CataZgVenoxisTankSpotAction(PlayerbotAI* botAI) : MovementAction(botAI, "zg venoxis tank spot") {}
    bool Execute(Event event) override;
};

// Jin'do: stand inside a Deadzone while Shadows of Hakkar is up.
class CataZgEnterDeadzoneAction : public MovementAction
{
public:
    CataZgEnterDeadzoneAction(PlayerbotAI* botAI) : MovementAction(botAI, "zg enter deadzone") {}
    bool Execute(Event event) override;
};

// Attack the priority add (Ohgan, awake panthers, Zanzili Berserker, exposed chains).
class CataZgFocusAction : public AttackAction
{
public:
    CataZgFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "zg focus target") {}
    bool Execute(Event event) override;
};

// Jin'do phase 2: fetch the next Gurubashi Spirit (walk until it is in reach and in sight, then attack it).
class CataZgPullSpiritAction : public AttackAction
{
public:
    CataZgPullSpiritAction(PlayerbotAI* botAI) : AttackAction(botAI, "zg pull gurubashi spirit") {}
    bool Execute(Event event) override;
};

// Jin'do phase 2: stand on the shielded chain the group works on, so the Body Slam lands on it.
class CataZgStandAtChainAction : public MovementAction
{
public:
    CataZgStandAtChainAction(PlayerbotAI* botAI) : MovementAction(botAI, "zg stand at hakkar chain") {}
    bool Execute(Event event) override;
};

// Jin'do phase 2: a non-tank bot drops the Gurubashi Spirit as its target (it must live to Body Slam).
class CataZgDropSpiritAction : public Action
{
public:
    CataZgDropSpiritAction(PlayerbotAI* botAI) : Action(botAI, "zg drop gurubashi spirit") {}
    bool Execute(Event event) override;
};

// Zanzil (#822): the elected bot walks to Zanzil's Cauldron of Frostburn Formula (GO 208314), drinks from it (GameObject::Use, as
// UseAction::UseGameObject / BgTacticsIC.cpp:521) and then hits the Zanzili Berserker with its next spell: the aura 96325 triggers
// 96331 (stun 15 s, 65% of max health) on the victim.
class CataZgCauldronAction : public AttackAction
{
public:
    CataZgCauldronAction(PlayerbotAI* botAI) : AttackAction(botAI, "zg frostburn cauldron") {}
    bool Execute(Event event) override;

private:
    void ZgLog(char const* event, float dist, bool force);

    uint32 _lastUseAt = 0;
    uint32 _armedAt = 0;
    uint32 _lastCastAt = 0;
    uint32 _lastLogAt = 0;
    uint32 _giveUpAt = 0;
    uint32 _bestAt = 0;
    float _bestDist = 1.0e9f;
};

// Zanzil (#679): Graveyard Gas is coming - walk to Zanzil's Cauldron of Toxic Torment (GO 208315) and drink (GameObject::Use, as the
// Frostburn cauldron action does): 96328 cuts the gas to 10% for 30 s. Every bot does it, the tank too (Zanzil follows it).
class CataZgToxicTormentAction : public MovementAction
{
public:
    CataZgToxicTormentAction(PlayerbotAI* botAI) : MovementAction(botAI, "zg toxic torment cauldron") {}
    bool Execute(Event event) override;

private:
    uint32 _lastUseAt = 0;
    uint32 _bestAt = 0;
    float _bestDist = 1.0e9f;
};

// Kilnara / Zanzil / Venoxis: interrupt the cast that hurts.
class CataZgInterruptAction : public Action
{
public:
    CataZgInterruptAction(PlayerbotAI* botAI) : Action(botAI, "zg interrupt") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
