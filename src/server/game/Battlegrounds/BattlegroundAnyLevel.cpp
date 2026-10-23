/*
* This file is part of the Pandaria 5.4.8 Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * #702 phase 4 (#831): battlegrounds any level. See BattlegroundAnyLevel.h for the switch and its call sites.
 *
 * Sources (read 2026-10-15):
 *   - SPP: celguar/mangosbot-bots@b9263a55 playerbot/RandomPlayerbotMgr.cpp:1056-1220 CheckBgQueue counts real players
 *     and bots per [queue type][bracket of the player's level] (:1112-1124) and marks a bracket "NeedBots" for both
 *     teams when a real player waits (:1201-1216); playerbot/strategy/actions/BattleGroundJoinAction.cpp:320-351
 *     canJoinBg lets a bot in only when Player::GetBGAccessByLevel passes AND its own level bracket is the bracket
 *     (GetBattlegroundBracketByLevel for MANGOSBOT_TWO). cmangos/mangos-wotlk@a8321d4f src/game/Server/DBCStores.cpp:
 *     1073-1095 GetBattlegroundBracketByLevel is the same lookup as ours (null below the lowest bracket) and
 *     src/game/Entities/Player.cpp:21961-21974 GetBGAccessByLevel the same min/max gate; mangosd.conf.dist.in has no
 *     battleground level override (only Instance.IgnoreLevel, :941). So SPP fills a low-level player's OWN bracket with
 *     bots of that bracket and never lets anyone into a battleground below its range. This phase keeps SPP's fill rule
 *     (bots fill the bracket the requester is in, both teams) and adds the bracket fallback + a level window on top.
 *   - mod-modernWoW: conan513/mod-modernWoW@61632b32 src/Features/DynamicScaling/CreatureScalingScript.cpp. Mode 2
 *     "Equal TTK" (:6-41 design, :244-258 code): creature HP is left alone and the player's outgoing damage is
 *     multiplied by GetBaseHP(contentLevel) / GetBaseHP(playerLevel), the creature_classlevelstats health of the two
 *     levels (:154-165 GetBaseHPAtLevel), falling back to contentLevel / playerLevel when a value is missing
 *     (:258); creature -> player damage takes the inverse, GetBaseHP(playerLevel) / GetBaseHP(contentLevel)
 *     (:296-312 GetIncomingScale). Both are the same expression: baseHP(target level) / baseHP(source level). Hooks: ModifyMeleeDamage
 *     (:351), ModifySpellDamageTaken (:406), ModifyPeriodicDamageAurasTick (:450); its ModifyHealReceived is
 *     an empty stub (:491-494). conf/mod-modernwow.conf.dist:43-68 documents the three modes (default 2). modernWoW has no PvP
 *     scaling at all (no player -> player branch anywhere in the module).
 *
 * MoP differences:
 *   - The scaling here is unit -> unit inside a battleground (player, pet or NPC on either side) and symmetric: the
 *     lower-level side's hits go up, the higher-level side's go down, so both kill each other in about the same number
 *     of hits whatever the gap. It applies only when the two levels differ and one of the two units is below the
 *     battleground's own minimum level (it is there through the any-level fallback): a native bracket keeps its stock
 *     numbers even with the key on. Healing takes the same ratio (healer -> target), which modernWoW never did.
 *   - The curve is creature_classlevelstats.basehp4 (the MoP column, CLASS 1) for BOTH levels. It is complete for
 *     levels 1-95 in the 5.4.8 data (test DB: holes only at 96-100); mixing the per-level columns Creature::SelectLevel
 *     uses for battleground NPCs (#175: classic <= 71, MoP > 86) would overstate a 20-vs-90 gap twofold (484 vs 968 at
 *     level 20). Examples: 20 -> 968, 85 -> 158,079, 90 -> 393,941; a level-20 hit on a level-90 player x406.97,
 *     a level-90 hit on a level-20 player x0.002457.
 *   - Damage hooks are this core's UnitScript hooks (ScriptMgr.h:417-426: (target, attacker) order on all three).
 *     Heals are scaled in Unit::ApplyPvPHealMods (direct heals Spell.cpp, SpellEffects.cpp; heal-over-time ticks and
 *     absorb shields SpellAuraEffects.cpp), because UnitScript::ModifyHealReceived gets (healer, target) from
 *     Unit::HealBySpell and (target, healer) from the periodic tick (AutoBalanceLevelScaling.cpp:67 notes the same).
 *     The periodic hook therefore skips positive spells (heal ticks), exactly like AutoBalance's.
 */

#include "BattlegroundAnyLevel.h"
#include "ArenaAnyLevel.h"  // #702 P5 (#832): Arena.AnyLevel.* (off by default)
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "Config.h"
#include "CreatureData.h"
#include "DBCStores.h"
#include "Log.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "World.h"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>

namespace
{
    std::atomic<bool> g_enabled{ false };
    std::atomic<uint32> g_minLevel{ 10 };
    std::atomic<uint32> g_mode{ 0 };            // 0 = lowest bracket, 1 = highest bracket
    std::atomic<bool> g_pvpScalingConf{ false };
    std::atomic<bool> g_scriptsRegistered{ false };

    bool MinLevelOk(uint32 level) { return level >= g_minLevel.load(std::memory_order_relaxed); }

    // Arenas (#832), the rated battleground and solo queue are rated or match-made content: never a fallback.
    bool Eligible(Battleground const* bg)
    {
        return bg && !bg->IsArena() && !bg->IsRatedBG() && bg->GetTypeID() != BATTLEGROUND_RATED_10_VS_10 &&
            bg->GetTypeID() != BATTLEGROUND_AA;
    }

    // The map's lowest (mode 0) or highest (mode 1) PvpDifficulty bracket; GetBattlegroundBracketById walks the same
    // per-map rows as GetBattlegroundBracketByLevel (DBCStores.cpp:1190-1226).
    PvPDifficultyEntry const* FallbackBracket(uint32 mapId)
    {
        bool const highest = g_mode.load(std::memory_order_relaxed) == 1;
        PvPDifficultyEntry const* pick = nullptr;
        for (uint32 id = BG_BRACKET_ID_FIRST; id < MAX_BATTLEGROUND_BRACKETS; ++id)
        {
            PvPDifficultyEntry const* entry = sDBCManager.GetBattlegroundBracketById(mapId, BattlegroundBracketId(id));
            if (!entry)
                continue;
            if (!pick || (highest ? entry->MaxLevel > pick->MaxLevel : entry->MinLevel < pick->MinLevel))
                pick = entry;
        }
        return pick;
    }

    bool PvPScalingActive()
    {
        return g_enabled.load(std::memory_order_relaxed) && g_pvpScalingConf.load(std::memory_order_relaxed) &&
            g_scriptsRegistered.load(std::memory_order_relaxed);
    }

    // creature_classlevelstats MoP-column health of a level (see the file header for why one column).
    float LevelHealthRatio(uint8 targetLevel, uint8 sourceLevel)
    {
        if (targetLevel == sourceLevel || !targetLevel || !sourceLevel)
            return 1.0f;

        static_assert(EXPANSION_MISTS_OF_PANDARIA < MAX_CREATURE_BASE_HP, "basehp column index out of range");
        uint32 const hpTarget = sObjectMgr->GetCreatureBaseStats(targetLevel, UNIT_CLASS_WARRIOR)->BaseHealth[EXPANSION_MISTS_OF_PANDARIA];
        uint32 const hpSource = sObjectMgr->GetCreatureBaseStats(sourceLevel, UNIT_CLASS_WARRIOR)->BaseHealth[EXPANSION_MISTS_OF_PANDARIA];
        if (hpTarget > 1 && hpSource > 1)
            return float(hpTarget) / float(hpSource);

        // modernWoW CreatureScalingScript.cpp:258: the plain level ratio when the table has no value
        return float(targetLevel) / float(sourceLevel);
    }

    // The ratio for one hit or heal from source to target, or 1 when the pair is not an any-level pair in a battleground.
    float PairMultiplier(Unit const* source, Unit const* target)
    {
        if (!source || !target || source == target || !source->IsInWorld() || !target->IsInWorld())
            return 1.0f;

        Map* map = source->FindMap();
        if (!map || map != target->FindMap() || !map->IsBattleground())
            return 1.0f;

        Battleground* bg = static_cast<BattlegroundMap*>(map)->GetBG();
        if (!bg)
            return 1.0f;

        uint8 const sourceLevel = source->GetLevel();
        uint8 const targetLevel = target->GetLevel();
        if (sourceLevel == targetLevel)
            return 1.0f;

        // only a pair with an any-level participant (below the battleground's own range), and never NPC vs NPC
        uint32 const minLevel = bg->GetMinLevel();
        if (sourceLevel >= minLevel && targetLevel >= minLevel)
            return 1.0f;
        if (!source->IsControlledByPlayer() && !target->IsControlledByPlayer())
            return 1.0f;

        return LevelHealthRatio(targetLevel, sourceLevel);
    }

    // rounded, at least 1, at most INT32_MAX (no lround: long is 32-bit on Windows); same as AutoBalance's ScaledAmount
    double ScaledAmount(double amount, float multiplier)
    {
        double const scaled = std::floor(amount * double(multiplier) + 0.5);
        return std::max(1.0, std::min(scaled, double(std::numeric_limits<int32>::max())));
    }

    class BattlegroundAnyLevelPvPScalingUnitScript : public UnitScript
    {
    public:
        BattlegroundAnyLevelPvPScalingUnitScript() : UnitScript("Battleground_AnyLevel_PvPScaling_UnitScript") { }

        void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage) override
        {
            if (!damage || !PvPScalingActive())
                return;
            float const multiplier = PairMultiplier(attacker, target);
            if (multiplier != 1.0f)
                damage = uint32(ScaledAmount(double(damage), multiplier));
        }

        void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* /*spellInfo*/) override
        {
            if (damage <= 0 || !PvPScalingActive())
                return;
            float const multiplier = PairMultiplier(attacker, target);
            if (multiplier != 1.0f)
                damage = int32(ScaledAmount(double(damage), multiplier));
        }

        void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, int32& damage, SpellInfo const* spellInfo) override
        {
            // this hook also runs for heal-over-time ticks (SpellAuraEffects.cpp HandlePeriodicHealAurasTick), which
            // Unit::ApplyPvPHealMods scales; damage only here
            if (damage <= 0 || !spellInfo || spellInfo->IsPositive() || !PvPScalingActive())
                return;
            float const multiplier = PairMultiplier(attacker, target);
            if (multiplier != 1.0f)
                damage = int32(ScaledAmount(double(damage), multiplier));
        }
    };
}

void BattlegroundAnyLevel::LoadConfig()
{
    g_enabled.store(sConfigMgr->GetBoolDefault("Battleground.AnyLevel.Enable", false), std::memory_order_relaxed);
    g_minLevel.store(uint32(std::max(1, sConfigMgr->GetIntDefault("Battleground.AnyLevel.MinLevel", 10))), std::memory_order_relaxed);
    int32 const mode = sConfigMgr->GetIntDefault("Battleground.AnyLevel.Mode", 0);
    g_mode.store(mode == 1 ? 1u : 0u, std::memory_order_relaxed);
    g_pvpScalingConf.store(sConfigMgr->GetIntDefault("Battleground.AnyLevel.PvPScaling", 0) == 1, std::memory_order_relaxed);

    if (g_enabled.load(std::memory_order_relaxed))
        TC_LOG_INFO("server.loading", "Battleground.AnyLevel: ON (min level %u, mode %s bracket, PvP scaling %s)",
            g_minLevel.load(std::memory_order_relaxed), mode == 1 ? "highest" : "lowest",
            g_pvpScalingConf.load(std::memory_order_relaxed) ?
                (g_scriptsRegistered.load(std::memory_order_relaxed) ? "on" : "on at the next restart") : "off");

    ArenaAnyLevel::LoadConfig();  // #702 P5 (#832): Arena.AnyLevel.* (ArenaAnyLevel.h), same reload path, no World.cpp change
}

void BattlegroundAnyLevel::RegisterScripts()
{
    if (g_scriptsRegistered.load(std::memory_order_relaxed) || !g_enabled.load(std::memory_order_relaxed) ||
        !g_pvpScalingConf.load(std::memory_order_relaxed))
        return;

    new BattlegroundAnyLevelPvPScalingUnitScript();
    g_scriptsRegistered.store(true, std::memory_order_relaxed);
    TC_LOG_INFO("server.loading", "Battleground.AnyLevel: PvP scaling hooks registered (melee, spell, periodic; heals in Unit::ApplyPvPHealMods)");
}

bool BattlegroundAnyLevel::Enabled()
{
    return g_enabled.load(std::memory_order_relaxed);
}

PvPDifficultyEntry const* BattlegroundAnyLevel::GetBracketByLevel(Battleground const* bg, uint32 mapId, uint32 level)
{
    PvPDifficultyEntry const* native = sDBCManager.GetBattlegroundBracketByLevel(mapId, level);
    if (native)
        return native;

    // #702 P5 (#832): a live unrated arena instance (Arena.AnyLevel skirmish) on a map without this level's bracket
    // (Tol'viron, the Tiger's Peak) gives the bracket it was created for. nullptr with Arena.AnyLevel.Enable = 0.
    if (PvPDifficultyEntry const* arena = ArenaAnyLevel::InstanceBracket(bg))
        return arena;

    if (!Enabled() || !MinLevelOk(level) || !Eligible(bg))
        return native;

    return FallbackBracket(mapId);
}

bool BattlegroundAnyLevel::AllowsLevel(Battleground const* bg, uint32 level)
{
    if (!Enabled() || !MinLevelOk(level) || !Eligible(bg))
        return false;

    return GetBracketByLevel(bg, bg->GetMapId(), level) != nullptr;
}

void BattlegroundAnyLevel::ScaleHealing(Unit const* healer, Unit const* target, uint32& amount)
{
    if (!amount || !PvPScalingActive())
        return;

    float const multiplier = PairMultiplier(healer, target);
    if (multiplier != 1.0f)
        amount = uint32(ScaledAmount(double(amount), multiplier));
}

// ---------------------------------------------------------------------------------------------------------------------
// #702 phase 5 (#832): arenas any level (Arena.AnyLevel.Enable, off by default). ArenaAnyLevel.h has the design, the
// sources and the call sites; off, every function below returns false / nullptr.
// ---------------------------------------------------------------------------------------------------------------------
namespace
{
    std::atomic<bool> g_arenaEnabled{ false };

    // An unrated arena match (a skirmish); rated arenas and solo queue are never touched.
    bool UnratedArena(Battleground const* bg)
    {
        return g_arenaEnabled.load(std::memory_order_relaxed) && bg && bg->IsArena() && !bg->IsRated();
    }
}

void ArenaAnyLevel::LoadConfig()
{
    g_arenaEnabled.store(sConfigMgr->GetBoolDefault("Arena.AnyLevel.Enable", false), std::memory_order_relaxed);

    if (g_arenaEnabled.load(std::memory_order_relaxed))
        TC_LOG_INFO("server.loading", "Arena.AnyLevel: ON (an arena join below level %u is an unrated skirmish, drawn on any arena)",
            sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL));
}

bool ArenaAnyLevel::Enabled()
{
    return g_arenaEnabled.load(std::memory_order_relaxed);
}

bool ArenaAnyLevel::IsSkirmishJoin(uint32 level)
{
    return Enabled() && level < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
}

bool ArenaAnyLevel::AnyArenaMap(bool isRated)
{
    return Enabled() && !isRated;
}

PvPDifficultyEntry const* ArenaAnyLevel::InstanceBracket(Battleground const* bg)
{
    if (!UnratedArena(bg) || !bg->GetInstanceID())
        return nullptr;

    // The skirmish was queued and created under the "All Arenas" template's map (HandleBattlemasterJoinArena,
    // BattlegroundQueue::BattlegroundQueueUpdate's GetBattlegroundBracketById(bgTemplate->GetMapId(), bracket_id)), and
    // Battleground::SetBracket kept that bracket id on the instance.
    Battleground const* allArenas = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA);
    if (!allArenas)
        return nullptr;

    return sDBCManager.GetBattlegroundBracketById(allArenas->GetMapId(), bg->GetBracketId());
}

PvPDifficultyEntry const* ArenaAnyLevel::MapBracket(Battleground const* bg)
{
    if (!UnratedArena(bg))
        return nullptr;

    // the arena map's lowest row (Tol'viron and the Tiger's Peak have one, level 90)
    PvPDifficultyEntry const* pick = nullptr;
    for (uint32 id = BG_BRACKET_ID_FIRST; id < MAX_BATTLEGROUND_BRACKETS; ++id)
        if (PvPDifficultyEntry const* entry = sDBCManager.GetBattlegroundBracketById(bg->GetMapId(), BattlegroundBracketId(id)))
            if (!pick || entry->MinLevel < pick->MinLevel)
                pick = entry;

    return pick;
}
