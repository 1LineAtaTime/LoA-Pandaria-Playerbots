/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man) actions, MoP-only lane L7 (#669). Pattern: AC Ai/Dungeon/HoS/HoSActions.h.

#ifndef PLAYERBOTS_CATAZAACTIONS_H
#define PLAYERBOTS_CATAZAACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "CataZaShared.h"
#include "MovementActions.h"

// Step out of the nearest clear spot around the hazards of one kind (circles and cones).
class CataZaEscapeAction : public MovementAction
{
public:
    CataZaEscapeAction(PlayerbotAI* botAI, std::string const name, CataZulAman::HazardKind kind)
        : MovementAction(botAI, name), _kind(kind)
    {
    }

    bool Execute(Event event) override;

private:
    CataZulAman::HazardKind _kind;
};

// Akil'zon: run under the lifted player into the storm's eye.
class CataZaJoinStormEyeAction : public MovementAction
{
public:
    CataZaJoinStormEyeAction(PlayerbotAI* botAI) : MovementAction(botAI, "za join storm eye") {}
    bool Execute(Event event) override;
};

// Akil'zon: keep ranged / healers apart.
class CataZaSpreadAction : public MovementAction
{
public:
    CataZaSpreadAction(PlayerbotAI* botAI) : MovementAction(botAI, "za static spread") {}
    bool Execute(Event event) override;
};

// Nalorakk / Daakara bear: a Surge-debuffed bot closes on the boss so somebody else is the most distant player.
class CataZaSurgeCloseAction : public MovementAction
{
public:
    CataZaSurgeCloseAction(PlayerbotAI* botAI) : MovementAction(botAI, "za surge close") {}
    bool Execute(Event event) override;
};

// Halazzi: the tank drags the boss out of a Water Totem's healing area.
class CataZaLeaveWaterTotemAction : public MovementAction
{
public:
    CataZaLeaveWaterTotemAction(PlayerbotAI* botAI) : MovementAction(botAI, "za leave water totem") {}
    bool Execute(Event event) override;
};

// Attack the priority add / totem (Kidnapper, Hatchers, Lightning Totem, Malacrass servants).
class CataZaFocusAction : public AttackAction
{
public:
    CataZaFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "za focus target") {}
    bool Execute(Event event) override;
};

// Malacrass: interrupt a heal / hard cast of the boss or a servant.
class CataZaInterruptAction : public Action
{
public:
    CataZaInterruptAction(PlayerbotAI* botAI) : Action(botAI, "za malacrass interrupt") {}
    bool Execute(Event event) override;
};

#endif
