/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" (Scarlet Monastery, epic #373, issue #658). See MopSmShared.h for sources.
// TriggerNode syntax per the fork's UBStrategy.cpp:13-21.

#include "MopSmStrategy.h"
#include "MopDungeonShared.h"
#include "MopSmMultipliers.h"
#include "MopSmShared.h"
#include "Playerbots.h"

void MopDungeonScarletMonasteryStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Thalnos the Soulrender
    triggers.push_back(new TriggerNode("sm in spirit gale",
        NextAction::array(0, new NextAction("sm leave spirit gale", ACTION_EMERGENCY + 4), nullptr)));

    triggers.push_back(new TriggerNode("sm focus add",
        NextAction::array(0, new NextAction("sm focus add", ACTION_RAID + 2), nullptr)));
    // #781 add wave (AC AKStrategy.cpp:13 shape): the tank gathers the Fallen Crusaders / Empowered Zombies.
    triggers.push_back(new TriggerNode("sm tank thalnos adds",
        NextAction::array(0, new NextAction("sm tank thalnos adds", ACTION_RAID + 5), nullptr)));

    // Thalnos (Spirit Gale) and Whitemane (Mass Resurrection, Heal, Holy Smite)
    // #781: ACTION_INTERRUPT + 5 as AC MgTStrategy.cpp:23; the ready interrupters take turns
    triggers.push_back(new TriggerNode("sm priority interrupt",
        NextAction::array(0, new NextAction("sm priority interrupt", ACTION_INTERRUPT + 5), nullptr)));
    // #684: dispel Whitemane's Power Word: Shield (AC BTStrategy.cpp:96-97 "spellsteal rune shield" shape). Just below
    // the priority interrupt, so a Mass Resurrection / Heal kick is never delayed by the dispel.
    triggers.push_back(new TriggerNode("sm whitemane shield",
        NextAction::array(0, new NextAction("sm dispel whitemane shield", ACTION_INTERRUPT + 4), nullptr)));
    // #781 phase switch (AC FoS "switch to soul fragment" shape, FoSStrategy.cpp:14): Durand -> Whitemane
    triggers.push_back(new TriggerNode("sm switch to whitemane",
        NextAction::array(0, new NextAction("sm switch to whitemane", ACTION_RAID + 3), nullptr)));

    // Brother Korloff
    triggers.push_back(new TriggerNode("sm firestorm kick",
        NextAction::array(0, new NextAction("sm leave firestorm kick", ACTION_EMERGENCY + 8), nullptr)));

    triggers.push_back(new TriggerNode("sm in scorched earth",
        NextAction::array(0, new NextAction("sm leave scorched earth", ACTION_EMERGENCY + 5), nullptr)));
}

void MopDungeonScarletMonasteryStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MopSmFocusMultiplier(botAI));
    multipliers.push_back(new MopSmFirestormMultiplier(botAI));
    multipliers.push_back(new MopSmReviveGuardMultiplier(botAI));
    multipliers.push_back(new MopSmScorchedEarthMultiplier(botAI));
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "sm tank thalnos adds",
                                                            MopScarletMonastery::ThalnosTankWave()));
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "sm switch to whitemane",
                                                            MopScarletMonastery::WhitemaneSwitch()));
}
