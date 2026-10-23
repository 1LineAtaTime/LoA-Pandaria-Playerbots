/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Gurub (Cataclysm 5-man) strategy "cata-zg", MoP-only lane L7 (#670). Pattern: AC Ai/Dungeon/HoS/HoSStrategy.cpp
// (trigger -> action per mechanic) with MgTStrategy.cpp priorities. Boss mechanics: see CataZgShared.h for sources.

#include "CataZgStrategy.h"
#include "CataZgMultipliers.h"
#include "Playerbots.h"

void CataDungeonZulGurubStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // High Priest Venoxis: Toxic Link (10k nature per second to both players until the link breaks).
    triggers.push_back(new TriggerNode("zg toxic link",
        NextAction::array(0, new NextAction("zg break toxic link", ACTION_EMERGENCY + 6), nullptr)));
    // High Priest Venoxis: Pools of Acrid Tears (phase 2) and the Bloodvenom tendrils (phase 3).
    triggers.push_back(new TriggerNode("zg acrid pools",
        NextAction::array(0, new NextAction("zg escape acrid pools", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("zg bloodvenom",
        NextAction::array(0, new NextAction("zg escape bloodvenom", ACTION_EMERGENCY + 5), nullptr)));
    // High Priest Venoxis phase 2: Breath of Hethiss, 15 yd cone in front, 180k over 3 s (#670 w57/w60 wipes).
    triggers.push_back(new TriggerNode("zg breath of hethiss",
        NextAction::array(0, new NextAction("zg escape breath of hethiss", ACTION_EMERGENCY + 6), nullptr)));
    // High Priest Venoxis (#670 w68): his altar and stairs lie under permanent Poison Clouds (18.5-21.5k nature per 2 s each, heroic);
    // the tank walks him down to the floor spot and nobody stands in a cloud.
    triggers.push_back(new TriggerNode("zg poison cloud",
        NextAction::array(0, new NextAction("zg escape poison cloud", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("zg venoxis tank spot",
        NextAction::array(0, new NextAction("zg venoxis tank spot", ACTION_EMERGENCY + 1), nullptr)));
    // High Priest Venoxis stages one and two (#679): the Venomous Effusion maze walls (Wowpedia: do not touch the walls). Whispers of
    // Hethiss is interrupted by "zg interrupt" below.
    triggers.push_back(new TriggerNode("zg venomous effusion",
        NextAction::array(0, new NextAction("zg escape venomous effusion", ACTION_EMERGENCY + 4), nullptr)));

    // Bloodlord Mandokir: Devastating Slam cone; Ohgan is a focus target.
    triggers.push_back(new TriggerNode("zg devastating slam",
        NextAction::array(0, new NextAction("zg escape devastating slam", ACTION_EMERGENCY + 7), nullptr)));

    // High Priestess Kilnara: Wave of Agony cone; interrupt Tears of Blood / Shadow Bolt; panthers are focus targets.
    triggers.push_back(new TriggerNode("zg wave of agony",
        NextAction::array(0, new NextAction("zg escape wave of agony", ACTION_EMERGENCY + 4), nullptr)));

    // Zanzil: the Zanzili Fire trail; interrupt Voodoo Bolt; the Berserker is a focus target.
    triggers.push_back(new TriggerNode("zg zanzil fire",
        NextAction::array(0, new NextAction("zg escape zanzil fire", ACTION_EMERGENCY + 5), nullptr)));
    // Zanzil (#822): the elected DPS bot drinks from the Frostburn Formula cauldron and freezes the Berserker (Wowpedia "Zanzil
    // (tactics)": one ranged player uses the frost cauldron on the Berserker, then all DPS focus it).
    triggers.push_back(new TriggerNode("zg frostburn cauldron",
        NextAction::array(0, new NextAction("zg frostburn cauldron", ACTION_RAID + 3), nullptr)));
    // Zanzil (#679): Graveyard Gas - every bot drinks from Zanzil's Cauldron of Toxic Torment before the gas lands (Wowpedia "Zanzil
    // (tactics)": "Every group member will need to get the debuff from the green cauldron in order to survive").
    triggers.push_back(new TriggerNode("zg graveyard gas",
        NextAction::array(0, new NextAction("zg toxic torment cauldron", ACTION_EMERGENCY + 2), nullptr)));
    // Zanzil: the player a Zanzili Berserker pursues kites it (it walks at half speed).
    triggers.push_back(new TriggerNode("zg berserker pursuit",
        NextAction::array(0, new NextAction("zg kite berserker", ACTION_EMERGENCY + 3), nullptr)));

    // Jin'do: stand in a Deadzone for Shadows of Hakkar, otherwise stay out of it.
    triggers.push_back(new TriggerNode("zg shadows of hakkar",
        NextAction::array(0, new NextAction("zg enter deadzone", ACTION_EMERGENCY + 6), nullptr)));
    triggers.push_back(new TriggerNode("zg deadzone",
        NextAction::array(0, new NextAction("zg escape deadzone", ACTION_EMERGENCY + 2), nullptr)));
    // Jin'do phase 2: the tank fetches a Gurubashi Spirit, everybody stands on the shielded chain so its Body Slam breaks
    // the Brittle Barrier, and nobody but the tank hits the spirit while a chain is still shielded.
    triggers.push_back(new TriggerNode("zg pull gurubashi spirit",
        NextAction::array(0, new NextAction("zg pull gurubashi spirit", ACTION_RAID + 6), nullptr)));
    triggers.push_back(new TriggerNode("zg hakkar chains",
        NextAction::array(0, new NextAction("zg stand at hakkar chain", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("zg spare gurubashi spirit",
        NextAction::array(0, new NextAction("zg drop gurubashi spirit", ACTION_RAID + 4), nullptr)));

    // #679 w106: above every escape - the interrupt is instant and off the GCD (no stop, CastSpell only stops for a cast time), and
    // w106 bots stepping out of clouds / effusion walls at EMERGENCY+4..+6 let 10 of 24 Whispers ticks land first. isUseful() keeps
    // it out of the queue unless an interrupt can land now.
    triggers.push_back(new TriggerNode("zg interrupt",
        NextAction::array(0, new NextAction("zg interrupt", ACTION_EMERGENCY + 8), nullptr)));

    // Ohgan, awake panthers (Kilnara phase 2), Zanzili Berserker, exposed Hakkar's Chains.
    triggers.push_back(new TriggerNode("zg focus target",
        NextAction::array(0, new NextAction("zg focus target", ACTION_RAID + 2), nullptr)));
}

void CataDungeonZulGurubStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new CataZgHazardMultiplier(botAI));
    multipliers.push_back(new CataZgLinkMultiplier(botAI));
    multipliers.push_back(new CataZgVenoxisCloudMultiplier(botAI));
    multipliers.push_back(new CataZgWhispersInterruptMultiplier(botAI));
    multipliers.push_back(new CataZgShadowsMultiplier(botAI));
    multipliers.push_back(new CataZgFocusMultiplier(botAI));
    multipliers.push_back(new CataZgFormulaMultiplier(botAI));
    multipliers.push_back(new CataZgChainMultiplier(botAI));
    multipliers.push_back(new CataZgSpareSpiritMultiplier(botAI));
}
