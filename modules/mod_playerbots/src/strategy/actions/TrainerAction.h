/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/TrainerAction.h) into the
 * Legends of Azeroth Pandaria fork, issue #258 slice "Trainer + GossipHello" (the NPC interaction layer):
 * the "trainer" action is the path that reaches a trainer in the world and teaches what it offers.
 *
 * Only the TrainerAction class is ported. AC's TrainerAction.h/.cpp also carry MaintenanceAction
 * (AC .cpp:188-287), BisGearAction (:289-538, :312-538), RemoveGlyphAction (:540-548) and AutoGearAction
 * (:569-732); those are separate AC features driven by PlayerbotFactory/BisListMgr/PlayerbotTextMgr and are
 * outside this slice (none of the slice's anchors - .cpp:18,45,57,86,97,103,150,171,178 - fall in them).
 *
 * No action is registered here - Factory/AiFactory.cpp, ActionContext.h, ChatActionContext.h and
 * WorldPacketActionContext.h are owned by the single later registration slice of #258.
 */

#ifndef _PLAYERBOT_TRAINERACTION_H
#define _PLAYERBOT_TRAINERACTION_H

#include "Action.h"

#include <sstream>
#include <string>

class Creature;
class PlayerbotAI;
struct SpellInfo;    // `struct` - src/server/game/Spells/SpellInfo.h declares it as a struct
struct TrainerSpell; // `struct` - src/server/game/Entities/Creature/Creature.h:204

class TrainerAction : public Action
{
public:
    TrainerAction(PlayerbotAI* botAI) : Action(botAI, "trainer") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
    Unit* GetTarget() override;

private:
    Creature* GetCreatureTarget();
    void Iterate(Creature* creature, bool learnSpells, uint32 spellId);
    void Learn(SpellInfo const* spellInfo, TrainerSpell const* trainerSpell, uint32 cost, std::ostringstream& out);
    void TellHeader(Creature* creature);
    void TellFooter(uint32 totalCost);
};

#endif