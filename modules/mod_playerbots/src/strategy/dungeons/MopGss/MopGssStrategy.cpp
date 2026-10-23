/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun (map 962) strategy "mop-gss" (epic #373, issue #655). Sources: see MopGssShared.h.

#include "MopGssStrategy.h"
#include "MopDungeonShared.h"
#include "MopGssMultipliers.h"
#include "MopGssShared.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Timer.h"

void MopGssStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // --- Saboteur Kip'tilak ---
    // Sabotage on this bot: leave the group and the Stable Munitions; Sabotage on a groupmate: stay off the two axis
    // lanes through it; the explosion bunnies' lines are dodged sideways as they run.
    triggers.push_back(new TriggerNode("mop gss sabotage self",
        NextAction::array(0, new NextAction("mop gss escape", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("mop gss sabotage cross",
        NextAction::array(0, new NextAction("mop gss escape", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("mop gss munition lane",
        NextAction::array(0, new NextAction("mop gss escape", ACTION_EMERGENCY + 4), nullptr)));
    // #781 World in Flames: close to 70% / 30% every munition is about to explode - stand off all their axis lanes.
    triggers.push_back(new TriggerNode("mop gss world in flames",
        NextAction::array(0, new NextAction("mop gss escape", ACTION_EMERGENCY + 3), nullptr)));

    // --- Striker Ga'dok ---
    // Strafing Run lanes (the stalkers carrying the damage aura, and the boss's own flight line).
    triggers.push_back(new TriggerNode("mop gss strafe",
        NextAction::array(0, new NextAction("mop gss escape", ACTION_EMERGENCY + 4), nullptr)));
    // #781 add wave (AC AKStrategy.cpp:13 shape): the landed Krik'thik Strikers die first; the tank picks them up.
    triggers.push_back(new TriggerNode("mop gss strikers",
        NextAction::array(0, new NextAction("mop gss strikers", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("mop gss tank strikers",
        NextAction::array(0, new NextAction("mop gss tank strikers", ACTION_RAID + 5), nullptr)));

    // --- Commander Ri'mok ---
    // Frenzied Assault cone (everybody but the tank); Saboteurs die first and their Bombard is interrupted.
    triggers.push_back(new TriggerNode("mop gss frenzy",
        NextAction::array(0, new NextAction("mop gss escape", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(new TriggerNode("mop gss saboteur interrupt",
        NextAction::array(0, new NextAction("mop gss saboteur interrupt", ACTION_INTERRUPT + 5), nullptr)));
    // #655 Viscous Fluid: the others stand out of the pools, the tank walks Ri'mok out of them; the tank picks up the
    // Saboteurs / Swarmers that come down the ladders.
    // w58: the Bombardiers' armed stalkers on the ramp (47.5k Fire within 5 yd) - everybody, the tank included.
    triggers.push_back(new TriggerNode("mop gss bomb",
        NextAction::array(0, new NextAction("mop gss escape", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("mop gss fluid",
        NextAction::array(0, new NextAction("mop gss escape", ACTION_EMERGENCY + 2), nullptr)));
    triggers.push_back(new TriggerNode("mop gss tank fluid",
        NextAction::array(0, new NextAction("mop gss tank fluid", ACTION_RAID + 3), nullptr)));
    triggers.push_back(new TriggerNode("mop gss tank rimok adds",
        NextAction::array(0, new NextAction("mop gss tank rimok adds", ACTION_RAID + 5), nullptr)));

    // #821 after Ga'dok: one elected damage dealer fires Ga'dok's Flak Cannon until the Krik'thik Bombardiers over
    // Ri'mok's ramp are shot down (npc_flak_cannon, boss_striker_gadok.cpp:775-818).
    triggers.push_back(new TriggerNode("mop gss flak",
        NextAction::array(0, new NextAction("mop gss flak", ACTION_RAID + 4), nullptr)));

    // --- Raigonn ---
    // Fixate: the target runs; Engulfing Winds are avoided; the Screeching Swarm cast is interrupted.
    triggers.push_back(new TriggerNode("mop gss fixate",
        NextAction::array(0, new NextAction("mop gss escape", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(new TriggerNode("mop gss winds",
        NextAction::array(0, new NextAction("mop gss escape", ACTION_EMERGENCY + 2), nullptr)));
    triggers.push_back(new TriggerNode("mop gss swarm interrupt",
        NextAction::array(0, new NextAction("mop gss swarm interrupt", ACTION_INTERRUPT + 6), nullptr)));
    // #655 phase 1: melee damage dealers ride a cannon onto the Weak Spot; everybody on the ground keeps out of the
    // Battering Headbutt impact; the tank picks up the Swarm Bringers / Protectorates.
    triggers.push_back(new TriggerNode("mop gss cannon",
        NextAction::array(0, new NextAction("mop gss cannon", ACTION_RAID + 4), nullptr)));
    triggers.push_back(new TriggerNode("mop gss headbutt",
        NextAction::array(0, new NextAction("mop gss escape", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("mop gss tank raigonn adds",
        NextAction::array(0, new NextAction("mop gss tank raigonn adds", ACTION_RAID + 5), nullptr)));

    // Focus: Saboteurs (Ri'mok), Weak Spot for ranged DPS and Swarm Bringers (Raigonn).
    triggers.push_back(new TriggerNode("mop gss focus target",
        NextAction::array(0, new NextAction("mop gss focus target", ACTION_RAID + 2), nullptr)));
}

void MopGssStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MopGssHazardMultiplier(botAI));
    multipliers.push_back(new MopGssFocusMultiplier(botAI));
    multipliers.push_back(new MopGssCannonMultiplier(botAI));
    multipliers.push_back(new MopGssTankFaceMultiplier(botAI));  // #655 w69
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "mop gss strikers",
                                                            MopGss::StrikerWave(MopDungeon::WaveRole::Dps)));
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "mop gss tank strikers",
                                                            MopGss::StrikerWave(MopDungeon::WaveRole::Tank)));
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "mop gss tank rimok adds",
                                                            MopGss::RimokAddWave(MopDungeon::WaveRole::Tank)));
    multipliers.push_back(new MopDungeon::MopWaveMultiplier(botAI, "mop gss tank raigonn adds",
                                                            MopGss::RaigonnAddWave(MopDungeon::WaveRole::Tank)));
}

// #655 Raigonn phase 1: Impervious Carapace (107118) takes 99% off his body until the Weak Spot (56895) dies, and w60-w63 had
// 217 target ticks on the body against one ranged bot on the Weak Spot. Damage dealers skip him while it lasts (the tank
// keeps him; the focus value sends the ranged to the Weak Spot, the melee ride the Artillery). Cached: the target values ask
// several times per tick and the answer needs a grid search (shape of CataLct/CataLctStrategy.cpp AppendTargetExclusions).
// w65: the tank's "tank target" too (Tank type) - in phase 1 Raigonn never melees (REACT_PASSIVE, boss_raigonn.cpp:107/359), the
// tank held his body 95 ticks while 8-10 adds beat him to death at the phase change; he kills the adds instead.
void MopGssStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type != TargetValueExclusionType::Dps && type != TargetValueExclusionType::Attacker &&
        type != TargetValueExclusionType::Tank)
        return;

    Player* bot = botAI->GetBot();
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MopGss::GSS_MAP_ID)
        return;

    uint32 const now = getMSTime();
    if (!_cacheValid || now - _cacheTime >= 300)
    {
        _cache.clear();
        MopGss::CollectCarapaceExclusions(bot, _cache);
        // #655 w68: the tank's pick also skips the units he can never reach (Engulfers 30 yd up, the Weak Spot on Raigonn's back)
        _tankCache.clear();
        if (PlayerBotSpec::IsTank(bot))
            MopGss::CollectTankExclusions(bot, _tankCache);
        _cacheTime = now;
        _cacheValid = true;
    }

    exclusions.insert(_cache.begin(), _cache.end());
    if (type == TargetValueExclusionType::Tank)
        exclusions.insert(_tankCache.begin(), _tankCache.end());
}
