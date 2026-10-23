/*
 * Ported from AC mod-playerbots Ai/Base/Actions/PetsAction.cpp (issue #194, S8). AC's "attack"/
 * "follow"/"stay" pet sub-commands are a hand copy of AC core's WorldSession::HandlePetActionHelper
 * with public CharmInfo/MotionMaster/CreatureAI calls
 * (call-by-call mapping; option (ii): a literal port using this fork's own public API, no core or
 * PlayerbotAI.h change).
 *
 * Deviations from AC, all noted at the call site below:
 *  - CharmInfo::SetPlayerReactState/SetForcedSpell/SetForcedTargetGUID/RemoveStayPosition and
 *    Unit::ClearInPetCombat do not exist on this fork's core and have no useful MoP equivalent for a
 *    bot's own pet (Part A.1); dropped, or replaced by CreatureAI::OnPetCommand, which is the MoP
 *    core's own way of clearing PetAI's delayed-spell queue (PetAI.cpp:770-774, called by the core's
 *    own pet-command handler at Handlers/PetHandler.cpp:415-416).
 *  - AC's PlayerbotTextMgr::GetBotTextOrDefault() text-variant lookup does not exist on this fork
 *    (same as every other chat/tell string ported so far, e.g. ChatShortcutActions.cpp); plain default
 *    strings are used directly.
 *  - AiPlayerbot.PetChatCommandDebug is read locally via sConfigMgr with AC's key and default (0),
 *    the way LootAction.cpp reads AiPlayerbot.FreeMethodLoot, instead of being added to
 *    PlayerbotAIConfig.h (a widely included header - see the research doc Part A.4 step 4).
 */

#include "PetsAction.h"

#include "Config.h"
#include "CreatureAI.h"
#include "DatabaseEnv.h"
#include "Event.h"
#include "MotionMaster.h"
#include "World.h"
#include "Pet.h"
#include "PetDefines.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

namespace
{
    // Shared by PetsAction and SetPetStanceAction: every controlled pet/guardian except totems.
    void CollectControlledPets(Player* bot, std::vector<Creature*>& targets)
    {
        Pet* pet = bot->GetPet();
        if (pet)
            targets.push_back(pet);

        for (Unit::ControlList::const_iterator itr = bot->m_Controlled.begin(); itr != bot->m_Controlled.end(); ++itr)
        {
            Creature* creature = dynamic_cast<Creature*>(*itr);
            if (!creature)
                continue;
            if (pet && creature == pet)
                continue;
            if (creature->IsTotem())
                continue;
            targets.push_back(creature);
        }
    }

    // AC key AiPlayerbot.PetChatCommandDebug, default 0 (AC PlayerbotAIConfig.cpp:723,
    // conf/playerbots.conf.dist:1241). Not present on this fork's PlayerbotAIConfig - read locally,
    // like LootAction.cpp reads AiPlayerbot.FreeMethodLoot.
    bool PetChatCommandDebug()
    {
        static bool const v = sConfigMgr->GetBoolDefault("AiPlayerbot.PetChatCommandDebug", false);
        return v;
    }
}

// AC parity: PlayerbotAI::PetFollow (AC Bot/PlayerbotAI.cpp:6712-6736), ported as a free helper (see
// PetsAction.h). Acts only on bot->GetPet() (the main pet; guardians are not touched), same as AC.
void PlayerbotPetFollow(Player* bot)
{
    Pet* pet = bot->GetPet();
    if (!pet)
        return;

    pet->AttackStop();
    pet->CastStop();
    // AC Unit::ClearInPetCombat(): not ported. This fork's UNIT_FLAG_PET_IN_COMBAT is owned by
    // SetInCombatState()/ClearInCombat() and doubles as a "can swim" flag (Unit.cpp), so clearing it by
    // hand here would desync the owner's own combat/swim bookkeeping.
    pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());

    CharmInfo* charmInfo = pet->GetCharmInfo();
    if (!charmInfo)
        return;

    charmInfo->SetCommandState(COMMAND_FOLLOW);
    charmInfo->SetIsCommandAttack(false);
    charmInfo->SetIsAtStay(false);
    charmInfo->SetIsReturning(true);
    charmInfo->SetIsCommandFollow(true);
    charmInfo->SetIsFollowing(false);
    // AC CharmInfo::RemoveStayPosition(): no MoP reader outside COMMAND_STAY/MOVE_TO - no-op, dropped.
    // AC Pet::ClearCastWhenWillAvailable()+SetForcedSpell(0)+SetForcedTargetGUID(): this fork's
    // equivalent queue is PetAI::m_delayedSpell, cleared by CreatureAI::OnPetCommand
    // (PetAI.cpp:770-774), which is also what the core's own COMMAND_FOLLOW handler calls.
    if (pet->IsAIEnabled && pet->AI())
        pet->AI()->OnPetCommand(COMMAND_FOLLOW);
}

bool PetsAction::Execute(Event event)
{
    std::string param = event.getParam();
    if (param.empty() && !defaultCmd.empty())
        param = defaultCmd;

    if (param.empty())
    {
        botAI->TellError("Usage: pet <aggressive|defensive|passive|stance|attack|follow|stay>");
        return false;
    }

    std::vector<Creature*> targets;
    CollectControlledPets(bot, targets);

    if (targets.empty())
    {
        botAI->TellError("You have no pet or guardian pet.");
        return false;
    }

    if (param == "stance")
    {
        for (Creature* target : targets)
        {
            std::string type = target->IsPet() ? "pet" : "guardian";
            std::string stance;
            switch (target->GetReactState())
            {
                case REACT_AGGRESSIVE:
                    stance = "aggressive";
                    break;
                case REACT_DEFENSIVE:
                    stance = "defensive";
                    break;
                case REACT_PASSIVE:
                    stance = "passive";
                    break;
                default:
                    stance = "unknown";
                    break;
            }
            botAI->TellMaster("Current stance of " + type + " \"" + target->GetName() + "\": " + stance + ".");
        }
        return true;
    }

    // AC PetsAction.cpp:153-257: force every controlled pet/guardian to attack the master's target.
    if (param == "attack")
    {
        Player* master = botAI->GetMaster();
        Unit* targetUnit = nullptr;
        if (master)
        {
            ObjectGuid masterTargetGuid = master->GetTarget();
            if (!masterTargetGuid.IsEmpty())
                targetUnit = botAI->GetUnit(masterTargetGuid);
        }

        if (!targetUnit)
        {
            botAI->TellError("No valid target selected by master.");
            return false;
        }
        if (!targetUnit->IsAlive())
        {
            botAI->TellError("Target is not alive.");
            return false;
        }
        if (!bot->IsValidAttackTarget(targetUnit))
        {
            botAI->TellError("Target is not a valid attack target for the bot.");
            return false;
        }
        // 5.4.8: DuelInfo's member is lowercase `opponent` on this core (Player.h), unlike AC's
        // WotLK-era `Opponent`.
        if (sPlayerbotAIConfig->IsPvpProhibited(bot->GetZoneId(), bot->GetAreaId()) &&
            (targetUnit->IsPlayer() || targetUnit->IsPet()) &&
            (!bot->duel || bot->duel->opponent != targetUnit))
        {
            botAI->TellError("I cannot command my pet to attack players in PvP prohibited areas.");
            return false;
        }

        bool didAttack = false;
        for (Creature* petCreature : targets)
        {
            CharmInfo* charmInfo = petCreature->GetCharmInfo();
            if (!charmInfo)
                continue;

            petCreature->ClearUnitState(UNIT_STATE_FOLLOW);
            if (petCreature->GetVictim() != targetUnit ||
                (petCreature->GetVictim() == targetUnit && !charmInfo->IsCommandAttack()))
            {
                if (petCreature->GetVictim())
                    petCreature->AttackStop();

                if (!petCreature->IsPlayer() && petCreature->ToCreature()->IsAIEnabled)
                {
                    charmInfo->SetIsCommandAttack(true);
                    charmInfo->SetIsAtStay(false);
                    charmInfo->SetIsFollowing(false);
                    charmInfo->SetIsCommandFollow(false);
                    charmInfo->SetIsReturning(false);

                    petCreature->ToCreature()->AI()->AttackStart(targetUnit);
                    // Core parity: Handlers/PetHandler.cpp:415-416 calls OnPetCommand after every
                    // ACT_COMMAND; PetsAction issues the equivalent public call directly.
                    petCreature->AI()->OnPetCommand(COMMAND_ATTACK);

                    didAttack = true;
                }
                else
                {
                    if (petCreature->GetVictim() && petCreature->GetVictim() != targetUnit)
                        petCreature->AttackStop();

                    charmInfo->SetIsCommandAttack(true);
                    charmInfo->SetIsAtStay(false);
                    charmInfo->SetIsFollowing(false);
                    charmInfo->SetIsCommandFollow(false);
                    charmInfo->SetIsReturning(false);

                    petCreature->Attack(targetUnit, true);
                    didAttack = true;
                }
            }
        }

        if (didAttack && PetChatCommandDebug())
            botAI->TellMaster("Pet commanded to attack your target.");
        else if (!didAttack)
            botAI->TellError("Pet did not attack. (Already attacking or unable to attack target)");
        return didAttack;
    }

    if (param == "follow")
    {
        PlayerbotPetFollow(bot);
        if (PetChatCommandDebug())
            botAI->TellMaster("Pet commanded to follow.");
        return true;
    }

    // AC PetsAction.cpp:271-310.
    if (param == "stay")
    {
        for (Creature* target : targets)
        {
            bool controlledMotion =
                target->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE;
            if (!controlledMotion)
            {
                target->StopMoving();  // AC Unit::StopMovingOnCurrentPos() equivalent
                target->GetMotionMaster()->Clear(false);
                target->GetMotionMaster()->MoveIdle();
            }

            if (CharmInfo* charmInfo = target->GetCharmInfo())
            {
                charmInfo->SetCommandState(COMMAND_STAY);
                charmInfo->SetIsCommandAttack(false);
                charmInfo->SetIsCommandFollow(false);
                charmInfo->SetIsFollowing(false);
                charmInfo->SetIsReturning(false);
                charmInfo->SetIsAtStay(!controlledMotion);
                if (controlledMotion)  // AC SaveStayPosition(true)
                    charmInfo->SaveStayPosition(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
                else  // AC SaveStayPosition(false)
                    charmInfo->SaveStayPosition();
                // AC ClearCastWhenWillAvailable()+SetForcedSpell(0)+SetForcedTargetGUID():
                if (target->IsAIEnabled && target->AI())
                    target->AI()->OnPetCommand(COMMAND_STAY);
            }
        }
        if (PetChatCommandDebug())
            botAI->TellMaster("Pet commanded to stay.");
        return true;
    }

    ReactStates react;
    if (param == "aggressive")
        react = REACT_AGGRESSIVE;
    else if (param == "defensive")
        react = REACT_DEFENSIVE;
    else if (param == "passive")
        react = REACT_PASSIVE;
    else
    {
        botAI->TellError("Unknown pet command: " + param + ". Use: pet <aggressive|defensive|passive|stance|attack|follow|stay>");
        return false;
    }

    // AC also calls charmInfo->SetPlayerReactState(react) here; dropped (Part A.1 - no MoP reader for
    // a bot's own pet/guardian).
    for (Creature* target : targets)
        target->SetReactState(react);

    if (PetChatCommandDebug())
        botAI->TellMaster("Pet stance set to " + param + ".");

    return true;
}

bool SetPetStanceAction::Execute([[maybe_unused]] Event event)
{
    std::vector<Creature*> targets;
    CollectControlledPets(bot, targets);

    if (targets.empty())
        return false;

    // AC parity (PetsAction.cpp:474-497): map the config int to a stance, defaulting to defensive.
    ReactStates react = REACT_DEFENSIVE;
    switch (sPlayerbotAIConfig->defaultPetStance)
    {
        case 0:
            react = REACT_PASSIVE;
            break;
        case 2:
            react = REACT_AGGRESSIVE;
            break;
        default:
            react = REACT_DEFENSIVE;
            break;
    }

    for (Creature* target : targets)
        target->SetReactState(react);

    return true;
}
