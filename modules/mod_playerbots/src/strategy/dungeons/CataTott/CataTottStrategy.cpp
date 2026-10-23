/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (epic #373, issue #661). See CataTottShared.h for sources.

#include "CataTottStrategy.h"
#include "CataTottMultipliers.h"
#include "Playerbots.h"

void CataDungeonTottStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Lady Naz'jar: Summon Geyser (23k frost + knockback within 5 yd, 5 s after it appears under a random player)
    triggers.push_back(new TriggerNode("cata tott geyser",
        NextAction::array(0, new NextAction("cata tott leave geyser", ACTION_EMERGENCY + 8), nullptr)));

    // Lady Naz'jar, heroic Waterspout vortexes: touching one sucks the player in and stuns it
    triggers.push_back(new TriggerNode("cata tott vortex",
        NextAction::array(0, new NextAction("cata tott leave vortex", ACTION_EMERGENCY + 6), nullptr)));

    // Lady Naz'jar's Tempest Witch: Lightning Surge hits every friend within 8 yd of the charged player
    triggers.push_back(new TriggerNode("cata tott lightning surge",
        NextAction::array(0, new NextAction("cata tott spread surge", ACTION_EMERGENCY + 4), nullptr)));

    // Lady Naz'jar, Waterspout (66% / 33%): she deflects spells and ranged attacks; the adds are what has to die
    triggers.push_back(new TriggerNode("cata tott nazjar adds",
        NextAction::array(0, new NextAction("cata tott kill nazjar adds", ACTION_RAID + 2), nullptr)));

    // Commander Ulthok: Dark Fissure
    triggers.push_back(new TriggerNode("cata tott fissure",
        NextAction::array(0, new NextAction("cata tott leave fissure", ACTION_EMERGENCY + 7), nullptr)));

    // Mindbender Ghur'sha: Mind Fog (no attacks or casts inside, shadow damage)
    triggers.push_back(new TriggerNode("cata tott mind fog",
        NextAction::array(0, new NextAction("cata tott leave mind fog", ACTION_EMERGENCY + 6), nullptr)));

    // Erunak Stonespeaker (Ghur'sha's host): Earth Shards
    triggers.push_back(new TriggerNode("cata tott earth shards",
        NextAction::array(0, new NextAction("cata tott leave earth shards", ACTION_EMERGENCY + 7), nullptr)));

    // Ozumat, stage 2: Blight of Ozumat pools
    triggers.push_back(new TriggerNode("cata tott blight pool",
        NextAction::array(0, new NextAction("cata tott leave blight pool", ACTION_EMERGENCY + 5), nullptr)));

    // Ozumat, stage 2: the three Faceless Sappers end the stage
    triggers.push_back(new TriggerNode("cata tott sappers",
        NextAction::array(0, new NextAction("cata tott kill sappers", ACTION_RAID + 2), nullptr)));

    // Ozumat, stages 1-2: the adds attack the passive Neptulon, not the group, so they never become "attackers" on
    // their own; DPS and tank take them off him (he dies -> the encounter resets)
    triggers.push_back(new TriggerNode("cata tott neptulon adds",
        NextAction::array(0, new NextAction("cata tott kill neptulon adds", ACTION_RAID + 1), nullptr)));

    // Ozumat, stage 3: Tidal Surge is up and Ozumat is attackable - DPS race
    triggers.push_back(new TriggerNode("cata tott ozumat",
        NextAction::array(0, new NextAction("cata tott attack ozumat", ACTION_RAID + 1), nullptr)));

    // Naz'jar (Shock Blast, Chain Lightning), Erunak (Lava Bolt), Ozumat adds (Brain Spike, Shadow Bolt, Veil of Shadow)
    triggers.push_back(new TriggerNode("cata tott interrupt",
        NextAction::array(0, new NextAction("cata tott interrupt", ACTION_INTERRUPT + 5), nullptr)));
}

void CataDungeonTottStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new CataTottEvadeMultiplier(botAI));
    multipliers.push_back(new CataTottFocusMultiplier(botAI));
    multipliers.push_back(new CataTottAbsorbMagicMultiplier(botAI));
}
