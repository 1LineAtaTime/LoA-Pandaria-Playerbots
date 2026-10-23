/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple strategy "mop-snt" (epic #373, issue #656). See MopSntShared.h for sources.

#include "MopSntMultipliers.h"
#include "AiObjectContext.h"
#include "AttackersValue.h"
#include "ChooseTargetActions.h"
#include "Creature.h"
#include "FollowActions.h"
#include "MopSntActions.h"
#include "MopSntShared.h"
#include "MovementActions.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"

float MopSntHazardMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<MopSntEscapeAction*>(action))
        return 1.0f;

    // #656 w52: no master-follow while the bot has to cross the bridge to Ner'onok (AC MgTMultipliers.cpp:125 vetoes
    // FollowAction the same way). During the flight the bots drop their target and run the non-combat engine, whose
    // follow pulled the tank back to the headless master at the stage (follow OK 7 / approach OK 8 in the w52 stall).
    if (dynamic_cast<FollowAction*>(action))
    {
        Creature* neronok = nullptr;
        float x = 0.0f;
        float y = 0.0f;
        bool flying = false;
        if (MopSnt::NeronokRegroupPoint(bot, neronok, x, y, flying))
            return 0.0f;
    }

    // #781: the interrupt (a MovementAction only for the run to Ner'onok) and a bomb throw (a cast, not a move) are
    // not vetoed by a hazard. The wave 33 pavalak run logged 75 "mop snt hazard made action mop snt siege bomb
    // useless" = the 75 IMPOSSIBLE of the issue: the carried bomb expired unthrown while the bot dodged.
    if (dynamic_cast<MopSntInterruptAction*>(action))
        return 1.0f;

    // #687: the resin hop is a MovementAction and the answer to the resin, never vetoed by a hazard
    if (dynamic_cast<MopSntResinHopAction*>(action))
        return 1.0f;

    if (dynamic_cast<MopSntSiegeBombAction*>(action) && bot->HasAura(MopSnt::SNT_SPELL_SIEGE_EXPLOSIVE_CARRY))
        return 1.0f;

    // #656 w54: the same for a carried tar keg (the 10 s throw override must not run out while the bot dodges)
    if (dynamic_cast<MopSntTarKegAction*>(action) && bot->HasAura(MopSnt::SNT_SPELL_CARRYING_TAR))
        return 1.0f;

    auto const& hazard = AI_VALUE_REF(MopSnt::Hazard, "mop snt hazard");
    // #656 w48: crossing the bridge to the landed Ner'onok is not held by a Caustic Pitch pool - walking through it beats
    // standing in it (w48: "hazard made action mop snt neronok approach useless" 41x while the tank and the mage died in
    // pools). The escape (EMERGENCY + 3) still goes first; the approach (RAID + 1) only runs when it has nothing to do.
    if (hazard.kind == MopSnt::HazardKind::Pitch && dynamic_cast<MopSntNeronokApproachAction*>(action))
        return 1.0f;

    return hazard.kind == MopSnt::HazardKind::None ? 1.0f : 0.0f;
}

float MopSntFocusMultiplier::GetValue(Action* action)
{
    bool const picker = dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action) ||
                        dynamic_cast<TankAssistAction*>(action);

    bool const drop = !picker && dynamic_cast<DropTargetAction*>(action) != nullptr;

    if (!picker && !drop)
        return 1.0f;

    if (!bot->IsInCombat())
        return 1.0f;

    // #656 w47: the tank stays on Jin'bak while his Sap Globules crawl in - AC FoS BronjahmMultiplier
    // (AC Ai/Dungeon/FoS/FoSMultipliers.cpp:19: no TankAssistAction while Bronjahm's soul fragments crawl to him).
    // w47: Jolerie's tank assist (9 OK in globule waves 1-2) put him on globules (two Charges), Jin'bak walked off his tank and was
    // picked up again 4 yd from the puddle centre, where the tank died to Sap Residue (24 stacks, 813k).
    if (PlayerBotSpec::IsTank(bot) && dynamic_cast<TankAssistAction*>(action))
    {
        Unit* current = AI_VALUE(Unit*, "current target");
        if (current && current->IsAlive() && current->GetEntry() == MopSnt::SNT_NPC_JINBAK)
            return 0.0f;
    }

    if (PlayerBotSpec::IsHeal(bot) || PlayerBotSpec::IsTank(bot) || !PlayerBotSpec::IsDps(bot))
        return 1.0f;

    ObjectGuid const focus = AI_VALUE(ObjectGuid, "mop snt focus target");
    if (focus.IsEmpty())
        return 1.0f;

    if (!drop)
        return 0.0f;

    // keep the focus target even if the generic code wants to drop it, but drop a dead / invalid one
    Unit* current = AI_VALUE(Unit*, "current target");
    if (!current || current->GetGUID() != focus)
        return 1.0f;

    if (!current->IsAlive() || !AttackersValue::IsValidTarget(current, bot))
        return 1.0f;

    return 0.0f;
}
