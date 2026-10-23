/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity (map 939) strategy "cata-woe" - issue #672, epic #373. AC has no Well of Eternity strategy; this is AC's
// dungeon-strategy pattern (Ai/Dungeon/HoS/HoSStrategy.cpp:9-33, Ai/Dungeon/UB/UBStrategy.cpp:30-34 for the target
// exclusions) with edges for the mechanics that wipe a pull. Sources: see CataWoeShared.h.
//
// Boss x edges:
//   Peroth'arn : leave Fel Flames / keep away from the hunt-phase Eyes / healers hold heals on the Fel Decay target (multiplier)
//   Queen Azshara : interrupt Total Obedience (the wipe cast) / leave the Arcane Bomb marker / never DPS Azshara herself
//                   (her damage is zeroed, boss_queen_azshara.cpp:195-200) - only the Enchanted Magi die
//   Mannoroth  : kill the Dreadlord Debilitators (REACT_PASSIVE, both must die) / leave Mannoroth alone until Varo'then is dead
//                (Fel Drain sacrifices Varo'then if Mannoroth is hurt first, boss_mannoroth.cpp:288-293)

#include "CataWoeStrategy.h"
#include "CataWoeMultipliers.h"
#include "CataWoeShared.h"
#include "Creature.h"
#include "Group.h"
#include "Playerbots.h"

void CataDungeonWellOfEternityStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Peroth'arn
    triggers.push_back(new TriggerNode("woe fel flames",
        NextAction::array(0, new NextAction("woe leave fel flames", ACTION_EMERGENCY + 6), nullptr)));
    triggers.push_back(new TriggerNode("woe eye of perotharn",
        NextAction::array(0, new NextAction("woe leave eye of perotharn", ACTION_EMERGENCY + 5), nullptr)));
    // #695: the escort's crystal gate (the designated bot uses the unlocked Portal Energy Focus)
    triggers.push_back(new TriggerNode("woe portal energy focus",
        NextAction::array(0, new NextAction("woe use portal energy focus", ACTION_RAID + 5), nullptr)));

    // Queen Azshara
    triggers.push_back(new TriggerNode("woe total obedience",
        NextAction::array(0, new NextAction("woe interrupt total obedience", ACTION_INTERRUPT + 10), nullptr)));
    triggers.push_back(new TriggerNode("woe arcane bomb",
        NextAction::array(0, new NextAction("woe leave arcane bomb", ACTION_EMERGENCY + 4), nullptr)));
    // #695: "You are mine now!" - kill the Hand of the Queen to free the puppet
    triggers.push_back(new TriggerNode("woe hand of the queen",
        NextAction::array(0, new NextAction("woe kill hand of the queen", ACTION_RAID + 7), nullptr)));

    // Mannoroth and Varo'then
    triggers.push_back(new TriggerNode("woe debilitator",
        NextAction::array(0, new NextAction("woe kill debilitator", ACTION_RAID + 6), nullptr)));
    // #695: Fel Firestorm flames
    triggers.push_back(new TriggerNode("woe fel firestorm",
        NextAction::array(0, new NextAction("woe leave fel firestorm", ACTION_EMERGENCY + 6), nullptr)));
}

void CataDungeonWellOfEternityStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new CataWoeHazardMultiplier(botAI));
    multipliers.push_back(new CataWoeFelDecayMultiplier(botAI));
}

void CataDungeonWellOfEternityStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type == TargetValueExclusionType::None)
        return;

    Player* bot = botAI->GetBot();

    // #695: a puppet (Servant of the Queen) is immune to all damage and fights for Azshara; the Hand above it is the target.
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->GetSource())
                if (WellOfEternity::IsPuppet(member))
                    exclusions.insert(member->GetGUID());
    }

    // Azshara is invulnerable (damage zeroed, health refilled): DPS on her is wasted. The tank still picks her up.
    if (type != TargetValueExclusionType::Tank)
    {
        if (Creature* azshara = WellOfEternity::GetAzshara(bot))
            exclusions.insert(azshara->GetGUID());
    }

    // Hurting Mannoroth before Varo'then is dead makes Mannoroth drain Varo'then to full health. Only while the bots can
    // actually hit Varo'then: he is IMMUNE_TO_PC when Illidan is not around (npc_mannoroth_varothen::Reset,
    // boss_mannoroth.cpp:519-523), and then excluding Mannoroth would leave the bots without any target.
    Creature* varothen = WellOfEternity::GetVarothen(bot);
    if (varothen && !varothen->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_PC))
    {
        if (Creature* mannoroth = WellOfEternity::GetMannoroth(bot))
            exclusions.insert(mannoroth->GetGUID());
    }
}
