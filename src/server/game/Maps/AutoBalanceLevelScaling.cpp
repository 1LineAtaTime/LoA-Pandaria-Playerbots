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
 * #702 AutoBalance (beyond AC core; AC + SPP get it from the mod-autobalance module).
 *
 * Port source: azerothcore/mod-autobalance @ 1d707cb61a0962ab01e4123681e2d3b66cc558a2
 *
 * Phase 1 (#828) - level scaling:
 *   - which maps scale ............ src/ABUtils.cpp:2291-2417 ShouldMapBeEnabled (Enable.* by size/heroic, Disable.PerInstance)
 *   - LFG level range of the map .. src/ABUtils.cpp:3038-3100 GetMapInfo (lfgMinLevel/lfgMaxLevel, heroic falls back to normal)
 *   - per-map settings ............ src/ABUtils.cpp:1979-2027 LoadDynamicLevelOverrides, :2068-2205 LoadMapSettings
 *   - level selection ............. src/ABAllCreatureScript.cpp:470-640 ModifyCreatureAttributes (85%/115% LFG window,
 *                                   skip range, "dynamic" and "fixed" methods, ceiling/floor)
 *   - hook point .................. src/ABAllCreatureScript.cpp:12-94 OnBeforeCreatureSelectLevel, called by AC
 *                                   Creature::SelectLevel (azerothcore-wotlk src/server/game/Entities/Creature/Creature.cpp:1523)
 *   - creature damage ............. src/ABUnitScript.cpp:52-70 ModifyMeleeDamage, :30-50 ModifySpellDamageTaken,
 *                                   :8-28 ModifyPeriodicDamageAurasTick, :186-459 _Modify_Damage_Healing
 *   - damage / health ratios ...... src/ABAllCreatureScript.cpp:1081-1114 (newBaseDamage / origBaseDamage),
 *                                   :707-767 (newBaseHealth / origHealth)
 *
 * Phase 2 (#829) - the rest of mod-autobalance:
 *   - player-count curve .......... src/ABUtils.cpp:625-665 getDefaultMultiplier (tanh curve, curveCeiling adjustment),
 *                                   :925-1099 getInflectionPointSettings (per size/difficulty, BossModifier, per-instance
 *                                   and boss per-instance overrides)
 *   - stat modifiers .............. src/ABUtils.cpp:1147-1582 getStatModifiers (size/difficulty/boss table, Boss.PerInstance,
 *                                   PerInstance, PerCreature overrides, -1 = keep)
 *   - player count ................ src/ABUtils.cpp:2419-2617 UpdateMapPlayerStats (non-GM players, MinPlayers,
 *                                   playerCountDifficultyOffset), :2068-2117 LoadMapSettings (MinPlayers per size/instance)
 *   - creature multipliers ........ src/ABAllCreatureScript.cpp:513-543 (ForcedID / DisabledID), :645-1214 (health, mana,
 *                                   armor, damage, CC duration; Min*Modifier clamps), :1334-1409 (XP / money modifiers)
 *   - world multipliers ........... src/ABUtils.cpp:675-923 getWorldMultiplier, :2981-3001 (map-level damage/healing for
 *                                   sources that are not enemy creatures)
 *   - damage / healing hooks ...... src/ABUnitScript.cpp:8-87, :186-459 _Modify_Damage_Healing (all branches)
 *   - crowd control ............... src/ABUnitScript.cpp:89-114 OnAuraApply, :461-514 _Modifier_CCDuration
 *   - rewards ..................... src/ABPlayerScript.cpp:39-74 OnPlayerGiveXP, :76-123 OnPlayerBeforeLootMoney
 *   - re-processing ............... src/ABWorldScript.cpp:9-15 (config time), src/ABPlayerScript.cpp:18-37 (level change),
 *                                   src/ABAllMapScript.cpp:11-227 (enter/leave + PlayerChangeNotify), src/ABPlayerScript.cpp:12-16
 *                                   (login announcement)
 *   - command ..................... src/ABCommandScript.h:21-35 + src/ABCommandScript.cpp:9-186 (.autobalance / .ab
 *                                   setoffset, getoffset, mapstat, creaturestat; English texts of src/Message.cpp)
 *   - config keys / defaults ...... src/ABWorldScript.cpp:17-500 (code defaults) and conf/AutoBalance.conf.dist
 *                                   (Enable.Global is 1 there, 0 here)
 *
 * MoP differences (forced by this core, all flagged in #702 / #829):
 *   - No AllCreatureScript / AllMapScript and no CustomData on Creature/Map: Creature::SelectLevel, Creature::UpdateEntry,
 *     Creature::Update, InstanceMap and WorldSession::HandleLootMoneyOpcode call these functions directly, and the
 *     per-instance / per-creature state lives in side tables keyed by instance id and creature guid, each instance behind
 *     its own mutex (maps update on several threads, and bots attach to a new map from their old map's thread -
 *     PlayerbotAI.cpp HandleMoveWorldportAck). The loot-money hook is a direct call (no new ScriptMgr hook, so ScriptMgr.h
 *     is untouched).
 *   - Map statistics: AB builds the highest/average creature level of an instance incrementally while creatures load and
 *     re-processes every creature when they change (OnAllCreatureUpdate + mapConfigTime). Here they are computed once per
 *     (map, difficulty) from the static spawns with the same level-range filter, so a creature's scaled level does not
 *     depend on grid load order (deterministic for the 1:1 tests). AB's DistanceCheck.PerInstance (wings) does not apply.
 *   - Stats: creature_classlevelstats has five expansion columns with holes on this core (#175), so the scaled stats read
 *     the column ExpansionColumnForLevel() picks, the same choice Creature::SelectLevel already makes for battleground
 *     NPCs; AB's 3-column smoothing (ABUtils.cpp:566-623) does not apply. Health, mana, armor, attack power and weapon
 *     damage then come from the core's own SelectLevel/UpdateEntry code for the scaled level, and the player-count
 *     multipliers are applied on top (= AB's origValue x scaledMultiplier, ABAllCreatureScript.cpp:759/896/1000).
 *   - Melee (#828): the core computes melee at the scaled level and the melee hook applies AB's damage ratio
 *     newBaseDamage / origBaseDamage (ABUnitScript.cpp:52-70 -> :438) x the player-count damage multiplier. The damage
 *     columns have holes (damage_exp3 exists for levels 80-85 only, all but damage_exp4 are 0 above 85), so a hole takes
 *     the nearest lower level of the same column, then the column of the level (BaseDamageOf); and the level ratio is capped
 *     so that a scaled creature never hits harder, relative to its own health, than at its original level (the scale-up
 *     guard: Deadmines Glubtok 16 -> 85 would get AB's ratio 27.4 on top of a weapon damage already read from the
 *     level-85 column). Without it a level-19 Grim Batol trash mob hit a level-20 player for 674-841 per swing (w63a L1).
 *   - Spells: 5.4.8 spells whose SpellEffectScaling multiplier is driven by a SpellScaling class already follow the
 *     caster's level (SpellEffectInfo::CalcValue), so they get the player-count damage multiplier only. Every other creature
 *     spell takes the creature's level health ratio (AB's own newBaseHealth / origHealth) instead of AB's damage ratio
 *     (#828: Cata/MoP spell base points carry the expansion's inflation, which the health columns follow and the damage
 *     columns do not; Grim Batol Twilight Bolt 9,750-10,250 -> 60-63 against a level-20 player). Spells that deal weapon
 *     damage take the melee multiplier. Percent-of-max-health periodic damage takes the player-count multiplier only
 *     (AB ABUnitScript.cpp:418-430). Creature heals follow the same rules as creature spells.
 *   - Heals (#829): AC calls ModifyHealReceived(this = healer, target) from Unit::HealBySpell although the hook is declared
 *     (target, healer); this fork mirrored that (#793). Unit::HealBySpell now passes (victim, this) like the periodic heal
 *     tick does, so AB's branches see the real healer. AC also scales a heal-over-time tick twice (both
 *     ModifyPeriodicDamageAurasTick and ModifyHealReceived run for it); here the periodic hook only scales damage ticks
 *     and the heal is scaled once in ModifyHealReceived.
 *     Battlegrounds are not dungeons (MapEntry::IsDungeon), so none of this touches a battleground heal: PvP heals belong to
 *     #831's BattlegroundAnyLevel::ScaleHealing (Unit::ApplyPvPHealMods), and a heal by a player is never changed here.
 *   - Rounding: damage and healing are rounded and kept >= 1 (phase 1; AB truncates). XP and money truncate as AB does.
 *   - Crowd control: the fork calls OnAuraApply also when an existing aura is refreshed (Unit.cpp, as AC); the refresh keeps
 *     the reduced maximum duration on this core (Aura::RefreshDuration does not recalculate it), so an aura whose maximum
 *     duration AutoBalance already set is not reduced again (AC would compound the reduction on every refresh).
 *   - Re-scaling after a change (players entering/leaving, level-up, .reload config, .ab setoffset) waits until the
 *     creature is alive and out of combat, and keeps its health (and mana) percentage. AB re-processes at once and keeps the
 *     percentages too, but combat-locks the player count of the whole map (it may not drop until every player left combat;
 *     ABPlayerScript.cpp:125-242 OnPlayerEnterCombat/LeaveCombat - this core has no player combat hooks). Waiting per
 *     creature gives the same protection for the creatures in the fight (they keep the count they were pulled with, also
 *     when it rises); idle creatures elsewhere follow the new count at once.
 *   - Not ported: the experimental boss-kill token reward (AutoBalance.reward.*, ABGlobalScript.cpp, needs a GlobalScript
 *     encounter hook; AB default off), the destructible game-object damage scaling (ABGameObjectScript.cpp, no game-object
 *     damage hook), AB's summon-clone detection (ABAllCreatureScript.cpp:1222-1256 / 1480-1622: a clone keeps its current
 *     health on its first scaling - here every creature keeps its health percentage), the deprecated AB key aliases
 *     (AutoBalance.rate.*, PerDungeonScaling, ... - no config of this fork ever had them), the AB-only ForcedID20 key (in
 *     AB's conf but never read by AB's code, ABWorldScript.cpp:29-34; kept unread), and the AB_ScriptMgr extension hooks.
 *   - Flex raids (RAID_DIFFICULTY_1025MAN_FLEX) are never scaled: their health already follows the player count through
 *     creature_scaling (InstanceScript::UpdateDynamicHealth).
 *   - The .autobalance command uses RBAC permissions (no SEC_* levels on this core): info commands need
 *     RBAC_PERM_COMMAND_INSTANCE_STATS, setoffset RBAC_PERM_COMMAND_INSTANCE_SET_BOSS_STATE; only the English texts.
 */

#include "AutoBalanceLevelScaling.h"
#include "Chat.h"
#include "Config.h"
#include "Creature.h"
#include "DBCStores.h"
#include "Log.h"
#include "LootMgr.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraDefines.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "TemporarySummon.h"
#include "WorldSession.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <vector>

std::atomic<bool> AutoBalance::g_active{ false };
std::atomic<bool> AutoBalance::g_levelScalingActive{ false };
std::atomic<bool> AutoBalance::g_hasInstances{ false };

namespace
{
    enum class ScalingMethod : uint8
    {
        Fixed,
        Dynamic
    };

    // AutoBalance.LevelScaling.DynamicLevel.PerInstance entry; -1 = keep the global value (AB ABLevelScalingDynamicLevelSettings.h)
    struct DynamicLevelOverride
    {
        int32 skipHigher = -1;
        int32 skipLower = -1;
        int32 ceiling = -1;
        int32 floor = -1;
    };

    // AB ABInflectionPointSettings.h (+ the BossModifier of the same size/difficulty)
    struct InflectionSettings
    {
        float value = 0.5f;
        float curveFloor = 0.0f;
        float curveCeiling = 1.0f;
        float bossModifier = 1.0f;
    };

    // AB ABStatModifiers.h; ccduration -1 = not set (CC durations unchanged)
    struct StatModifiers
    {
        float global = 1.0f;
        float health = 1.0f;
        float mana = 1.0f;
        float armor = 1.0f;
        float damage = 1.0f;
        float ccduration = -1.0f;
    };

    // The instance kinds AB's settings are keyed by (getInflectionPointSettings / getStatModifiers, ABUtils.cpp:938-1009, :1181-1461)
    enum SizeCategory : uint8
    {
        CAT_5M = 0,
        CAT_5M_HEROIC,
        CAT_RAID,
        CAT_RAID_HEROIC,
        CAT_10M,
        CAT_10M_HEROIC,
        CAT_15M,
        CAT_20M,
        CAT_25M,
        CAT_25M_HEROIC,
        CAT_40M,
        MAX_SIZE_CATEGORY
    };

    // config key stems per category, and the category a blank size-specific key inherits from (ABWorldScript.cpp:168-201, :295-415)
    char const* const InflectionKeys[MAX_SIZE_CATEGORY] =
    {
        "AutoBalance.InflectionPoint", "AutoBalance.InflectionPointHeroic", "AutoBalance.InflectionPointRaid",
        "AutoBalance.InflectionPointRaidHeroic", "AutoBalance.InflectionPointRaid10M", "AutoBalance.InflectionPointRaid10MHeroic",
        "AutoBalance.InflectionPointRaid15M", "AutoBalance.InflectionPointRaid20M", "AutoBalance.InflectionPointRaid25M",
        "AutoBalance.InflectionPointRaid25MHeroic", "AutoBalance.InflectionPointRaid40M"
    };
    char const* const StatModifierKeys[MAX_SIZE_CATEGORY] =
    {
        "AutoBalance.StatModifier", "AutoBalance.StatModifierHeroic", "AutoBalance.StatModifierRaid",
        "AutoBalance.StatModifierRaidHeroic", "AutoBalance.StatModifierRaid10M", "AutoBalance.StatModifierRaid10MHeroic",
        "AutoBalance.StatModifierRaid15M", "AutoBalance.StatModifierRaid20M", "AutoBalance.StatModifierRaid25M",
        "AutoBalance.StatModifierRaid25MHeroic", "AutoBalance.StatModifierRaid40M"
    };
    int32 const CategoryParent[MAX_SIZE_CATEGORY] =
    {
        -1, -1, -1, -1, CAT_RAID, CAT_RAID_HEROIC, CAT_RAID, CAT_RAID, CAT_RAID, CAT_RAID_HEROIC, CAT_RAID
    };

    // AB ABConfig.cpp:29-66 (WotLK spell / creature ids, kept as AB has them)
    uint32 const SpellIdsThatSpendPlayerHealth[] =
    {
        45529, 2687, 27869, 16666, 755, 3698, 3699, 3700, 11693, 11694, 11695, 27259, 47856,
        1454, 1455, 1456, 11687, 11688, 11689, 27222, 57946, 29858, 55213
    };
    uint32 const SpellIdsToNeverModify[] = { 1177 };

    template<size_t N>
    bool InList(uint32 const (&list)[N], uint32 id)
    {
        return std::find(std::begin(list), std::end(list), id) != std::end(list);
    }

    struct Settings
    {
        uint32 generation = 0;                                          // bumped by every LoadConfig / .ab setoffset

        bool enableGlobal = false;
        bool levelScaling = true;

        bool enable5M = true, enable10M = true, enable15M = true, enable20M = true, enable25M = true, enable40M = true;
        bool enableOtherNormal = true;
        bool enable5MHeroic = true, enable10MHeroic = true, enable25MHeroic = true, enableOtherHeroic = true;

        ScalingMethod method = ScalingMethod::Dynamic;
        int32 skipHigher = 3;
        int32 skipLower = 5;
        int32 ceilingDungeons = 1, floorDungeons = 5;
        int32 ceilingHeroicDungeons = 2, floorHeroicDungeons = 5;
        int32 ceilingRaids = 3, floorRaids = 5;
        int32 ceilingHeroicRaids = 3, floorHeroicRaids = 5;

        std::set<uint32> disabledMaps;                                  // AutoBalance.Disable.PerInstance
        std::unordered_map<uint32, int32> forcedCreatureIds;            // AutoBalance.ForcedID40/25/10/5/2, DisabledID (= 0)
        std::unordered_map<uint32, DynamicLevelOverride> dynamicOverrides;

        // #829 player-count scaling
        uint32 minPlayersNormal = 1, minPlayersHeroic = 1, minPlayersRaid = 1, minPlayersRaidHeroic = 1;
        std::unordered_map<uint32, uint32> minPlayersPerInstance, minPlayersPerHeroicInstance;
        int32 playerCountDifficultyOffset = 0;
        InflectionSettings inflection[MAX_SIZE_CATEGORY];
        StatModifiers statModifiers[MAX_SIZE_CATEGORY][2];              // [category][0 = creature, 1 = boss]
        std::unordered_map<uint32, InflectionSettings> inflectionOverrides;      // InflectionPoint.PerInstance (-1 = keep)
        std::unordered_map<uint32, InflectionSettings> bossInflectionOverrides;  // InflectionPoint.Boss.PerInstance (value only)
        std::unordered_map<uint32, StatModifiers> statOverrides, bossStatOverrides, creatureStatOverrides;
        float minHPModifier = 0.1f, minManaModifier = 0.01f, minDamageModifier = 0.01f;
        float minCCDurationModifier = 0.25f, maxCCDurationModifier = 1.0f;

        // #829 rewards / messages
        ScalingMethod rewardMethod = ScalingMethod::Dynamic;
        bool rewardXP = true, rewardMoney = true;
        float rewardXPModifier = 1.0f, rewardMoneyModifier = 1.0f;
        bool playerChangeNotify = true;
        bool announce = true;
    };

    std::mutex g_settingsLock;
    std::shared_ptr<Settings const> g_settings = std::make_shared<Settings>();
    std::atomic<uint32> g_generation{ 0 };

    std::shared_ptr<Settings const> GetSettings()
    {
        std::lock_guard<std::mutex> guard(g_settingsLock);
        return g_settings;
    }

    void PublishSettings(std::shared_ptr<Settings> s)
    {
        {
            std::lock_guard<std::mutex> guard(g_settingsLock);
            s->generation = g_generation.load() + 1;
            g_settings = s;
        }
        g_generation.store(s->generation);
        AutoBalance::g_levelScalingActive.store(s->enableGlobal && s->levelScaling, std::memory_order_relaxed);
        AutoBalance::g_active.store(s->enableGlobal, std::memory_order_relaxed);
    }

    struct CreatureState
    {
        uint32 entry = 0;
        uint8 unmodifiedLevel = 0;
        uint8 selectedLevel = 0;
        bool boss = false;
        uint32 instancePlayerCount = 0;                                 // AB AutoBalanceCreatureInfo::instancePlayerCount
        float healthMultiplier = 1.0f, scaledHealthMultiplier = 1.0f;
        float manaMultiplier = 1.0f, scaledManaMultiplier = 1.0f;
        float armorMultiplier = 1.0f, scaledArmorMultiplier = 1.0f;
        float damageMultiplier = 1.0f;                                  // player count only (AB DamageMultiplier)
        float meleeDamageMultiplier = 1.0f;                             // x level melee ratio (AB ScaledDamageMultiplier, #828)
        float spellDamageMultiplier = 1.0f;                             // x level health ratio (#828)
        float ccDurationMultiplier = 1.0f;
        float xpModifier = 1.0f;
        float moneyModifier = 1.0f;
        uint32 stamp = 0;
    };

    struct InstanceState
    {
        std::mutex lock;
        uint32 generation = 0;                                          // the Settings::generation it was set up with
        bool enabled = false;
        uint32 mapId = 0;
        uint32 maxPlayers = 0;
        bool heroic = false;
        SizeCategory category = CAT_5M;
        uint8 lfgMinLevel = 0;
        uint8 lfgMaxLevel = 0;
        uint8 lfgTargetLevel = 0;
        uint8 highestCreatureLevel = 0;
        float avgCreatureLevel = 0.0f;
        int32 skipHigher = 0;
        int32 skipLower = 0;
        int32 ceiling = 0;
        int32 floor = 0;
        uint8 highestPlayerLevel = 0;
        uint8 lowestPlayerLevel = 0;
        // #829 player count (AB AutoBalanceMapInfo)
        uint32 minPlayers = 1;
        uint32 realPlayerCount = 0;                                     // non-GM players inside
        uint32 playerCount = 0;                                         // AB playerCount (>= 1 once counted)
        uint32 adjustedPlayerCount = 0;
        bool counted = false;
        uint8 worldMultiplierTargetLevel = 0;
        float worldHealthMultiplier = 1.0f;
        float worldDamageHealingMultiplier = 1.0f;
        float scaledWorldDamageHealingMultiplier = 1.0f;
        uint32 stamp = 1;                                               // bumped whenever the creatures must re-scale
        std::unordered_map<ObjectGuid, CreatureState> creatures;
        // CC auras whose maximum duration AutoBalance set: (target, caster, spell) -> that duration
        std::map<std::tuple<ObjectGuid, ObjectGuid, uint32>, int32> ccScaled;
        // harness counters (abinfo): rewards, heals, CC
        uint32 xpEvents = 0;
        uint64 xpBefore = 0, xpAfter = 0;
        uint32 xpLastEntry = 0, xpLastBefore = 0, xpLastAfter = 0;
        float xpLastModifier = 1.0f;
        uint32 moneyEvents = 0;
        uint64 moneyBefore = 0, moneyAfter = 0;
        uint32 healEvents = 0;
        uint64 healBefore = 0, healAfter = 0;
        uint32 ccEvents = 0;
        int32 ccLastBefore = 0, ccLastAfter = 0;
    };

    std::shared_mutex g_instancesLock;
    std::unordered_map<uint32, std::shared_ptr<InstanceState>> g_instances;

    struct MapProfile
    {
        uint8 highest = 0;
        float average = 0.0f;
    };

    std::mutex g_profilesLock;
    std::unordered_map<uint64, MapProfile> g_profiles;                  // (mapId << 16) | spawnMode

    std::string Trim(std::string item)
    {
        item.erase(item.begin(), std::find_if(item.begin(), item.end(), [](unsigned char c) { return !std::isspace(c); }));
        item.erase(std::find_if(item.rbegin(), item.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), item.end());
        return item;
    }

    std::vector<std::string> SplitList(std::string const& text)
    {
        std::vector<std::string> out;
        std::string item;
        std::istringstream in(text);
        while (std::getline(in, item, ','))
        {
            item = Trim(item);
            if (!item.empty())
                out.push_back(item);
        }
        return out;
    }

    // AB ABUtils.cpp:1926 LoadDisabledDungeons: "33, 43, 129"
    std::set<uint32> LoadIdSet(std::string const& text)
    {
        std::set<uint32> ids;
        for (std::string const& item : SplitList(text))
        {
            try { ids.insert(uint32(std::stoul(item))); }
            catch (...) { TC_LOG_ERROR("server.loading", "AutoBalance: ignoring bad id '%s' in an id list", item.c_str()); }
        }
        return ids;
    }

    // AB ABUtils.cpp:1908-1922 LoadForcedCreatureIdsFromString (atoi, negative ids skipped)
    void LoadForcedIds(std::unordered_map<uint32, int32>& out, std::string const& text, int32 forcedPlayerCount)
    {
        std::string value;
        std::istringstream in(text);
        while (std::getline(in, value, ','))
        {
            int32 const creatureId = std::atoi(value.c_str());
            if (creatureId >= 0)
                out[uint32(creatureId)] = forcedPlayerCount;
        }
    }

    // AB ABUtils.cpp:1979-2023: "[InstanceID] [SkipHigherLevels] [SkipLowerLevels] [DynamicLevelCeiling] [DynamicLevelFloor], ..."
    std::unordered_map<uint32, DynamicLevelOverride> LoadDynamicOverrides(std::string const& text)
    {
        std::unordered_map<uint32, DynamicLevelOverride> overrides;
        for (std::string const& item : SplitList(text))
        {
            std::istringstream in(item);
            int64 mapId = -1;
            if (!(in >> mapId) || mapId < 0)
            {
                TC_LOG_ERROR("server.loading", "AutoBalance: ignoring bad DynamicLevel.PerInstance entry '%s'", item.c_str());
                continue;
            }
            DynamicLevelOverride o;
            int32 v;
            if (in >> v) o.skipHigher = v;
            if (in >> v) o.skipLower = v;
            if (in >> v) o.ceiling = v;
            if (in >> v) o.floor = v;
            overrides[uint32(mapId)] = o;
        }
        return overrides;
    }

    // the first token as a map/creature id and the rest as floats, missing ones -1 (AB ABUtils.cpp:2028-2066, :2237-2289)
    std::vector<std::pair<uint32, std::vector<float>>> LoadFloatRows(std::string const& text, size_t valueCount)
    {
        std::vector<std::pair<uint32, std::vector<float>>> rows;
        for (std::string const& item : SplitList(text))
        {
            std::istringstream in(item);
            std::string token;
            if (!(in >> token))
                continue;
            uint32 const id = uint32(std::max(0, std::atoi(token.c_str())));
            std::vector<float> values(valueCount, -1.0f);
            for (size_t i = 0; i < valueCount && (in >> token); ++i)
                values[i] = float(std::atof(token.c_str()));
            rows.emplace_back(id, values);
        }
        return rows;
    }

    std::unordered_map<uint32, InflectionSettings> LoadInflectionOverrides(std::string const& text)
    {
        std::unordered_map<uint32, InflectionSettings> out;
        for (auto const& row : LoadFloatRows(text, 3))
        {
            InflectionSettings ip;
            ip.value = row.second[0];
            ip.curveFloor = row.second[1];
            ip.curveCeiling = row.second[2];
            out[row.first] = ip;
        }
        return out;
    }

    std::unordered_map<uint32, StatModifiers> LoadStatOverrides(std::string const& text)
    {
        std::unordered_map<uint32, StatModifiers> out;
        for (auto const& row : LoadFloatRows(text, 6))
        {
            StatModifiers m;
            m.global = row.second[0];
            m.health = row.second[1];
            m.mana = row.second[2];
            m.armor = row.second[3];
            m.damage = row.second[4];
            m.ccduration = row.second[5];
            out[row.first] = m;
        }
        return out;
    }

    // AB ABUtils.cpp:2209-2233: "[InstanceID] [MinPlayers], ..."
    std::unordered_map<uint32, uint32> LoadMinPlayersPerInstance(std::string const& text)
    {
        std::unordered_map<uint32, uint32> out;
        for (auto const& row : LoadFloatRows(text, 1))
            out[row.first] = uint32(std::max(0.0f, row.second[0]));
        return out;
    }

    // A float key whose blank value means "inherit" (AB's GetOption<float>(key, broaderValue, false)); quiet when missing.
    float OptionalFloat(char const* key, float def)
    {
        std::string const value = Trim(sConfigMgr->GetStringDefault(key, "", true));
        if (value.empty())
            return def;
        try { return std::stof(value); }
        catch (...)
        {
            TC_LOG_ERROR("server.loading", "AutoBalance: bad value '%s' for %s, using %.3f", value.c_str(), key, def);
            return def;
        }
    }

    ScalingMethod LoadMethod(char const* key)
    {
        std::string method = Trim(sConfigMgr->GetStringDefault(key, "dynamic"));
        std::transform(method.begin(), method.end(), method.begin(), [](unsigned char c) { return char(std::tolower(c)); });
        if (method == "fixed")
            return ScalingMethod::Fixed;
        if (method != "dynamic")
            TC_LOG_ERROR("server.loading", "AutoBalance: invalid value '%s' for %s, using 'dynamic'", method.c_str(), key);
        return ScalingMethod::Dynamic;
    }

    SizeCategory CategoryOf(uint32 maxPlayers, bool heroic)
    {
        if (heroic)
        {
            if (maxPlayers <= 5)
                return CAT_5M_HEROIC;
            if (maxPlayers <= 10)
                return CAT_10M_HEROIC;
            if (maxPlayers <= 25)
                return CAT_25M_HEROIC;
            return CAT_RAID_HEROIC;
        }
        if (maxPlayers <= 5)
            return CAT_5M;
        if (maxPlayers <= 10)
            return CAT_10M;
        if (maxPlayers <= 15)
            return CAT_15M;
        if (maxPlayers <= 20)
            return CAT_20M;
        if (maxPlayers <= 25)
            return CAT_25M;
        if (maxPlayers <= 40)
            return CAT_40M;
        return CAT_RAID;
    }

    // AB ABUtils.cpp:2291-2417 ShouldMapBeEnabled
    bool ShouldMapBeEnabled(Map* map, Settings const& s)
    {
        if (!s.enableGlobal || !map->IsDungeon())
            return false;

        InstanceMap* instanceMap = map->ToInstanceMap();
        if (!instanceMap || instanceMap->GetMaxPlayers() < 1)
            return false;

        // #829: flex raids scale their health by player count through creature_scaling (InstanceScript::UpdateDynamicHealth)
        if (map->GetDifficulty() == RAID_DIFFICULTY_1025MAN_FLEX)
            return false;

        if (s.disabledMaps.count(map->GetId()))
            return false;

        uint32 const maxPlayers = instanceMap->GetMaxPlayers();
        if (instanceMap->IsHeroic())
        {
            if (maxPlayers <= 5)
                return s.enable5MHeroic;
            if (maxPlayers <= 10)
                return s.enable10MHeroic;
            if (maxPlayers <= 25)
                return s.enable25MHeroic;
            return s.enableOtherHeroic;
        }

        if (maxPlayers <= 5)
            return s.enable5M;
        if (maxPlayers <= 10)
            return s.enable10M;
        if (maxPlayers <= 15)
            return s.enable15M;
        if (maxPlayers <= 20)
            return s.enable20M;
        if (maxPlayers <= 25)
            return s.enable25M;
        if (maxPlayers <= 40)
            return s.enable40M;
        return s.enableOtherNormal;
    }

    // AB ABUtils.cpp:3050-3090: the LFG level range of the map; a heroic that is not in the dungeon finder uses its normal one.
    LFGDungeonEntry const* FindLfgDungeon(Map* map)
    {
        if (LFGDungeonEntry const* dungeon = DBCManager::GetLFGDungeon(map->GetId(), map->GetDifficulty()))
            return dungeon;

        if (!map->IsHeroic())
            return nullptr;

        switch (map->GetDifficulty())
        {
            case DUNGEON_DIFFICULTY_HEROIC:
                return DBCManager::GetLFGDungeon(map->GetId(), DUNGEON_DIFFICULTY_NORMAL);
            case RAID_DIFFICULTY_10MAN_HEROIC:
                return DBCManager::GetLFGDungeon(map->GetId(), RAID_DIFFICULTY_10MAN_NORMAL);
            case RAID_DIFFICULTY_25MAN_HEROIC:
                return DBCManager::GetLFGDungeon(map->GetId(), RAID_DIFFICULTY_25MAN_NORMAL);
            default:
                return nullptr;
        }
    }

    // AB's 85% / 115% window around the LFG level range (ABAllCreatureScript.cpp:470-495, ABUtils.cpp:289-305)
    uint8 WindowLow(uint8 lfgMinLevel) { return uint8((float(lfgMinLevel) * 0.85f) + 0.5f); }
    uint8 WindowHigh(uint8 lfgMaxLevel) { return uint8((float(lfgMaxLevel) * 1.15f) + 0.5f); }

    // Highest and average original level of the hostile-looking spawns of this map and difficulty that fall in the
    // window, from the static spawn data (MoP difference, see the file header). Same level source as
    // Creature::SelectLevel: the difficulty row when there is one, else creature_template minlevel/maxlevel.
    MapProfile GetMapProfile(Map* map, uint8 lfgMinLevel, uint8 lfgMaxLevel)
    {
        uint64 const key = (uint64(map->GetId()) << 16) | uint64(map->GetSpawnMode());
        {
            std::lock_guard<std::mutex> guard(g_profilesLock);
            auto const itr = g_profiles.find(key);
            if (itr != g_profiles.end())
                return itr->second;
        }

        uint8 const low = WindowLow(lfgMinLevel);
        uint8 const high = WindowHigh(lfgMaxLevel);
        uint32 const spawnBit = 1u << map->GetSpawnMode();

        MapProfile profile;
        double sum = 0.0;
        uint32 count = 0;
        for (auto const& pair : sObjectMgr->GetAllCreatureData())
        {
            CreatureData const& data = pair.second;
            if (data.mapId != map->GetId() || !(uint32(data.spawnMask) & spawnBit))
                continue;

            CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(data.id);
            if (!cinfo || cinfo->type == CREATURE_TYPE_CRITTER || (cinfo->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER))
                continue;

            uint8 minLevel = cinfo->minlevel;
            uint8 maxLevel = cinfo->maxlevel;
            if (map->GetDifficulty() > REGULAR_DIFFICULTY)
                if (CreatureDifficultyInfo const* difficultyInfo = sObjectMgr->SelectDifficultyInfo(map, cinfo->Entry))
                {
                    minLevel = difficultyInfo->LevelMin;
                    maxLevel = difficultyInfo->LevelMax;
                }

            uint8 const lo = std::min(minLevel, maxLevel);
            uint8 const hi = std::max(minLevel, maxLevel);
            if (hi < low || hi > high)
                continue;

            profile.highest = std::max(profile.highest, hi);
            sum += (double(lo) + double(hi)) / 2.0;
            ++count;
        }
        profile.average = count ? float(sum / double(count)) : 0.0f;

        TC_LOG_INFO("maps", "AutoBalance: map %u difficulty %u level profile: %u spawns in %u-%u, highest %u, average %.1f",
            map->GetId(), uint32(map->GetSpawnMode()), count, uint32(low), uint32(high), uint32(profile.highest), profile.average);

        std::lock_guard<std::mutex> guard(g_profilesLock);
        g_profiles[key] = profile;
        return profile;
    }

    // creature_classlevelstats base health of a row. A hole (the loader stores a missing basehp as 1, #175) falls back to
    // the column ExpansionColumnForLevel() picks for that level.
    float BaseHealthOf(CreatureBaseStats const* stats, uint32 column, uint8 level)
    {
        uint32 health = stats->BaseHealth[std::min<uint32>(column, MAX_CREATURE_BASE_HP - 1)];
        if (health <= 1)
            health = stats->BaseHealth[std::min<uint32>(AutoBalance::ExpansionColumnForLevel(level), MAX_CREATURE_BASE_HP - 1)];
        return float(health);
    }

    // AB's base damage of a level (CreatureBaseStats::GenerateBaseDamage, ABAllCreatureScript.cpp:1086 and :1095). The damage
    // columns have holes on this core (damage_exp3 is set for levels 80-85 only; above 85 only damage_exp4 is), so a hole
    // takes the nearest lower level of the same column, then the column of the level. 0 when there is none.
    float BaseDamageOf(uint8 level, uint8 unitClass, uint32 column)
    {
        uint32 const columns[2] = { std::min<uint32>(column, MAX_CREATURE_BASE_DAMAGE - 1),
            std::min<uint32>(AutoBalance::ExpansionColumnForLevel(level), MAX_CREATURE_BASE_DAMAGE - 1) };
        for (uint32 const c : columns)
            for (int32 l = int32(level); l >= 1; --l)
            {
                float const damage = sObjectMgr->GetCreatureBaseStats(uint8(l), unitClass)->BaseDamage[c];
                if (damage > 0.0f)
                    return damage;
            }
        return 0.0f;
    }

    // The core's melee damage per hit of a creature with damage modifier 1, as StatSystem.cpp Creature::UpdateDamagePhysical
    // computes it: attack power * attack time / 14 plus the average weapon damage Creature::SelectLevel sets (1.0 - 1.5 x base).
    float NativeMeleePerHit(CreatureBaseStats const* stats, uint32 column, uint32 attackTimeMs)
    {
        return float(stats->AttackPower) * float(attackTimeMs) / 14000.0f +
            1.25f * stats->BaseDamage[std::min<uint32>(column, MAX_CREATURE_BASE_DAMAGE - 1)];
    }

    struct LevelRatios
    {
        float melee = 1.0f;
        float health = 1.0f;
    };

    // #828: the level part of a level-scaled creature's multipliers (see the file header, Melee / Spells).
    LevelRatios ComputeLevelRatios(CreatureTemplate const* cinfo, uint8 unmodifiedLevel, uint8 selectedLevel)
    {
        LevelRatios out;
        CreatureBaseStats const* origStats = sObjectMgr->GetCreatureBaseStats(unmodifiedLevel, cinfo->unit_class);
        CreatureBaseStats const* newStats = sObjectMgr->GetCreatureBaseStats(selectedLevel, cinfo->unit_class);
        uint32 const origColumn = cinfo->expansion;                                      // what SelectLevel reads unscaled
        uint32 const newColumn = AutoBalance::ExpansionColumnForLevel(selectedLevel);    // what it reads scaled

        // AB newHealthMultiplier (ABAllCreatureScript.cpp:707-760): new base health / original base health (ModHealth cancels)
        float const origHealth = BaseHealthOf(origStats, origColumn, unmodifiedLevel);
        float const newHealth = BaseHealthOf(newStats, newColumn, selectedLevel);
        float const healthRatio = (origHealth > 1.0f && newHealth > 1.0f) ? newHealth / origHealth : 1.0f;
        out.health = healthRatio;

        // AB newDamageMultiplier (ABAllCreatureScript.cpp:1081-1114): new base damage / original base damage, applied to melee
        // on top of the damage the core already computes at the scaled level (ABUnitScript.cpp:52-70 -> :438)
        float const origBase = BaseDamageOf(unmodifiedLevel, cinfo->unit_class, origColumn);
        float const newBase = BaseDamageOf(selectedLevel, cinfo->unit_class, newColumn);
        float melee = (origBase > 0.0f && newBase > 0.0f) ? newBase / origBase : 1.0f;

        // never more, relative to the creature's own health, than its unscaled melee: scaled melee per hit
        // (newPerHit x modifier x melee) <= unscaled melee per hit (origPerHit x modifier) x health ratio
        uint32 const attackTime = cinfo->BaseAttackTime ? cinfo->BaseAttackTime : 2000;
        float const origPerHit = NativeMeleePerHit(origStats, origColumn, attackTime);
        float const newPerHit = NativeMeleePerHit(newStats, newColumn, attackTime);
        if (origPerHit > 0.0f && newPerHit > 0.0f)
            melee = std::min(melee, healthRatio * origPerHit / newPerHit);

        out.melee = melee;
        return out;
    }

    // AB ABUtils.cpp:625-665 getDefaultMultiplier: the hyperbolic-tangent curve of the adjusted player count
    float DefaultMultiplier(uint32 maxPlayers, uint32 adjustedPlayerCount, InflectionSettings const& ip)
    {
        float const maxNumberOfPlayers = float(maxPlayers);
        float const diff = (maxNumberOfPlayers / 5) * 1.5f;
        float const curveCeilingAdjustment = ip.curveCeiling /
            (((std::tanh((maxNumberOfPlayers - ip.value) / diff) + 1.0f) / 2.0f) * (ip.curveCeiling - ip.curveFloor) + ip.curveFloor);
        return ((std::tanh((float(adjustedPlayerCount) - ip.value) / diff) + 1.0f) / 2.0f) *
            (ip.curveCeiling * curveCeilingAdjustment - ip.curveFloor) + ip.curveFloor;
    }

    // AB ABUtils.cpp:925-1099 getInflectionPointSettings (value = maxPlayers x InflectionPoint[, x BossModifier])
    InflectionSettings InflectionFor(InstanceState const& state, Settings const& s, bool isBoss)
    {
        InflectionSettings const& base = s.inflection[state.category];
        InflectionSettings out;
        out.value = float(state.maxPlayers) * base.value;
        out.curveFloor = base.curveFloor;
        out.curveCeiling = base.curveCeiling;

        auto const o = s.inflectionOverrides.find(state.mapId);
        if (o != s.inflectionOverrides.end())
        {
            if (o->second.value != -1)
                out.value = float(state.maxPlayers) * o->second.value;
            if (o->second.curveFloor != -1)
                out.curveFloor = o->second.curveFloor;
            if (o->second.curveCeiling != -1)
                out.curveCeiling = o->second.curveCeiling;
        }

        if (isBoss)
        {
            auto const b = s.bossInflectionOverrides.find(state.mapId);
            if (b != s.bossInflectionOverrides.end() && b->second.value != -1)
                out.value *= b->second.value;
            else
                out.value *= base.bossModifier;
        }
        return out;
    }

    void ApplyOverride(StatModifiers& m, StatModifiers const& o)
    {
        if (o.global != -1) m.global = o.global;
        if (o.health != -1) m.health = o.health;
        if (o.mana != -1) m.mana = o.mana;
        if (o.armor != -1) m.armor = o.armor;
        if (o.damage != -1) m.damage = o.damage;
        if (o.ccduration != -1) m.ccduration = o.ccduration;
    }

    // AB ABUtils.cpp:1147-1582 getStatModifiers (creature = nullptr: the map's own, non-boss values)
    StatModifiers StatModifiersFor(InstanceState const& state, Settings const& s, bool hasCreature, bool isBoss, uint32 entry)
    {
        bool const boss = hasCreature && isBoss;
        StatModifiers m = s.statModifiers[state.category][boss ? 1 : 0];

        auto const bossOverride = s.bossStatOverrides.find(state.mapId);
        auto const mapOverride = s.statOverrides.find(state.mapId);
        if (boss && bossOverride != s.bossStatOverrides.end())
            ApplyOverride(m, bossOverride->second);
        else if (mapOverride != s.statOverrides.end())
            ApplyOverride(m, mapOverride->second);

        if (hasCreature)
        {
            auto const creatureOverride = s.creatureStatOverrides.find(entry);
            if (creatureOverride != s.creatureStatOverrides.end())
                ApplyOverride(m, creatureOverride->second);
        }
        return m;
    }

    bool OutsideSkipRange(InstanceState const& state)
    {
        int32 const players = int32(state.highestPlayerLevel);
        return (state.avgCreatureLevel > float(players + state.skipHigher) || state.skipHigher == 0) ||
            (state.avgCreatureLevel < float(players - state.skipLower) || state.skipLower == 0);
    }

    // AB ABUtils.cpp:675-923 getWorldMultiplier + :2981-3001: the multipliers of damage/healing whose source is not an enemy
    // creature (a player hurting itself or a friend, a friendly NPC healing a player). The level part uses the warrior
    // base damage of the map's average creature level and of the highest player level (the "map is a warrior").
    void ComputeWorldMultipliers(InstanceState& state, Settings const& s)
    {
        state.worldHealthMultiplier = 1.0f;
        state.worldDamageHealingMultiplier = 1.0f;
        state.scaledWorldDamageHealingMultiplier = 1.0f;
        uint8 const avgRounded = uint8(state.avgCreatureLevel + 0.5f);
        state.worldMultiplierTargetLevel = avgRounded;

        if (!state.enabled || !state.realPlayerCount || state.avgCreatureLevel == 0.0f || !state.maxPlayers)
            return;

        float const defaultMultiplier = DefaultMultiplier(state.maxPlayers, state.adjustedPlayerCount, InflectionFor(state, s, false));
        StatModifiers const mods = StatModifiersFor(state, s, false, false, 0);
        state.worldHealthMultiplier = defaultMultiplier * mods.global * mods.health;
        float damage = defaultMultiplier * mods.global * mods.damage;
        state.worldDamageHealingMultiplier = damage;

        if (s.levelScaling && state.highestPlayerLevel && OutsideSkipRange(state))
        {
            state.worldMultiplierTargetLevel = state.highestPlayerLevel;
            float const origBase = BaseDamageOf(avgRounded, CLASS_WARRIOR, AutoBalance::ExpansionColumnForLevel(avgRounded));
            float const newBase = BaseDamageOf(state.highestPlayerLevel, CLASS_WARRIOR, AutoBalance::ExpansionColumnForLevel(state.highestPlayerLevel));
            if (origBase > 0.0f && newBase > 0.0f)
                damage *= newBase / origBase;
        }
        state.scaledWorldDamageHealingMultiplier = damage;
    }

    // AB ABUtils.cpp:2419-2617 UpdateMapPlayerStats: player count (GMs ignored), MinPlayers, difficulty offset, lowest and
    // highest player level. Returns true when the creatures must re-scale (adjusted count or highest level changed).
    bool RecountPlayers(InstanceState& state, Map* map, Player const* leaving, Settings const& s)
    {
        uint32 count = 0;
        uint8 highest = 0;
        uint8 lowest = 0;
        Map::PlayerList const& players = map->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            Player* player = itr->GetSource();
            if (!player || player == leaving || player->IsGameMaster())
                continue;
            ++count;
            highest = std::max(highest, player->GetLevel());
            if (!lowest || player->GetLevel() < lowest)
                lowest = player->GetLevel();
        }

        uint32 const oldAdjusted = state.adjustedPlayerCount;
        uint8 const oldHighest = state.highestPlayerLevel;
        bool const firstCount = !state.counted;

        state.counted = true;
        state.realPlayerCount = count;
        state.playerCount = count ? count : 1;
        int32 adjusted = int32(state.playerCount);
        if (adjusted < int32(state.minPlayers))
            adjusted = int32(state.minPlayers);
        adjusted += s.playerCountDifficultyOffset;
        state.adjustedPlayerCount = uint32(std::clamp<int32>(adjusted, 0, 255));

        // nobody left: keep the last levels (nothing re-scales without players, phase 1)
        if (highest)
        {
            state.highestPlayerLevel = highest;
            state.lowestPlayerLevel = lowest;
        }

        bool const changed = firstCount || oldAdjusted != state.adjustedPlayerCount || oldHighest != state.highestPlayerLevel;
        if (changed)
            ComputeWorldMultipliers(state, s);
        return changed && !firstCount;
    }

    // AB ABUtils.cpp:2068 LoadMapSettings + GetMapInfo, once per instance and again after a .reload config.
    void InitInstanceState(InstanceState& state, Map* map, Settings const& s)
    {
        state.generation = s.generation;
        state.enabled = ShouldMapBeEnabled(map, s);
        state.mapId = map->GetId();
        InstanceMap* instanceMap = map->ToInstanceMap();
        state.maxPlayers = instanceMap ? instanceMap->GetMaxPlayers() : 0;
        state.heroic = map->IsHeroic();
        state.category = CategoryOf(state.maxPlayers, state.heroic);

        if (LFGDungeonEntry const* dungeon = FindLfgDungeon(map))
        {
            state.lfgMinLevel = uint8(std::min<uint32>(dungeon->minlevel, 255));
            state.lfgMaxLevel = uint8(std::min<uint32>(dungeon->maxlevel, 255));
            state.lfgTargetLevel = uint8(std::min<uint32>(dungeon->reclevel, 255));
        }
        else
        {
            // AB logs this and leaves the range at 0, which puts every creature outside the window: nothing scales.
            if (state.enabled)
                TC_LOG_INFO("maps", "AutoBalance: map %u difficulty %u has no LFG level range, not scaled", map->GetId(), uint32(map->GetSpawnMode()));
            state.enabled = false;
        }

        bool const heroic = map->IsHeroic();
        bool const raid = map->IsRaid();
        state.skipHigher = s.skipHigher;
        state.skipLower = s.skipLower;
        state.ceiling = raid ? (heroic ? s.ceilingHeroicRaids : s.ceilingRaids) : (heroic ? s.ceilingHeroicDungeons : s.ceilingDungeons);
        state.floor = raid ? (heroic ? s.floorHeroicRaids : s.floorRaids) : (heroic ? s.floorHeroicDungeons : s.floorDungeons);

        auto const o = s.dynamicOverrides.find(map->GetId());
        if (o != s.dynamicOverrides.end())
        {
            if (o->second.skipHigher != -1) state.skipHigher = o->second.skipHigher;
            if (o->second.skipLower != -1) state.skipLower = o->second.skipLower;
            if (o->second.ceiling != -1) state.ceiling = o->second.ceiling;
            if (o->second.floor != -1) state.floor = o->second.floor;
        }

        // AB ABUtils.cpp:2094-2117: MinPlayers by instance id / size / difficulty, never above the instance size
        auto const& perInstance = state.heroic ? s.minPlayersPerHeroicInstance : s.minPlayersPerInstance;
        auto const minItr = perInstance.find(map->GetId());
        if (minItr != perInstance.end())
            state.minPlayers = minItr->second;
        else if (state.maxPlayers <= 5)
            state.minPlayers = state.heroic ? s.minPlayersHeroic : s.minPlayersNormal;
        else
            state.minPlayers = state.heroic ? s.minPlayersRaidHeroic : s.minPlayersRaid;
        if (state.maxPlayers && state.minPlayers > state.maxPlayers)
            state.minPlayers = state.maxPlayers;

        if (state.enabled)
        {
            MapProfile const profile = GetMapProfile(map, state.lfgMinLevel, state.lfgMaxLevel);
            state.highestCreatureLevel = profile.highest;
            state.avgCreatureLevel = profile.average;
            if (!profile.highest)
                state.enabled = false;
        }
    }

    std::shared_ptr<InstanceState> FindInstanceState(uint32 instanceId)
    {
        std::shared_lock<std::shared_mutex> guard(g_instancesLock);
        auto const itr = g_instances.find(instanceId);
        return itr != g_instances.end() ? itr->second : nullptr;
    }

    std::shared_ptr<InstanceState> GetOrCreateInstanceState(Map* map, Settings const& s)
    {
        uint32 const instanceId = map->GetInstanceId();
        if (std::shared_ptr<InstanceState> existing = FindInstanceState(instanceId))
            return existing;

        std::shared_ptr<InstanceState> created = std::make_shared<InstanceState>();
        InitInstanceState(*created, map, s);

        std::unique_lock<std::shared_mutex> guard(g_instancesLock);
        auto const inserted = g_instances.emplace(instanceId, created);   // another thread may have won the race
        AutoBalance::g_hasInstances.store(true, std::memory_order_relaxed);
        if (inserted.second)
            TC_LOG_INFO("maps", "AutoBalance: instance %u (map %u '%s', difficulty %u): %s, LFG %u-%u, creatures highest %u average %.1f, skip %d/%d, ceiling %d floor %d, min players %u",
                instanceId, map->GetId(), map->GetMapName(), uint32(map->GetSpawnMode()), created->enabled ? "level scaling on" : "not scaled",
                uint32(created->lfgMinLevel), uint32(created->lfgMaxLevel), uint32(created->highestCreatureLevel), created->avgCreatureLevel,
                created->skipHigher, created->skipLower, created->ceiling, created->floor, created->minPlayers);
        return inserted.first->second;
    }

    // AB ABWorldScript.cpp:9-15 + ABUtils.cpp:2714-3036 UpdateMapDataIfNeeded: after a .reload config (or .ab setoffset) the
    // instance takes the new settings and every creature re-scales. Caller holds state.lock.
    void RefreshIfStale(InstanceState& state, Map* map)
    {
        if (state.generation == g_generation.load(std::memory_order_relaxed))
            return;

        std::shared_ptr<Settings const> settings = GetSettings();
        bool const wasEnabled = state.enabled;
        InitInstanceState(state, map, *settings);
        state.counted = false;
        RecountPlayers(state, map, nullptr, *settings);
        ++state.stamp;
        TC_LOG_INFO("maps", "AutoBalance: instance %u (map %u): config reloaded, %s -> %s, players %u (adjusted %u)",
            map->GetInstanceId(), map->GetId(), wasEnabled ? "on" : "off", state.enabled ? "on" : "off", state.playerCount, state.adjustedPlayerCount);
    }

    // AB ABUtils.cpp:1644-1721 isBossOrBossSummon
    bool IsBossOrBossSummon(Creature* creature)
    {
        if (!creature)
            return false;
        if (creature->IsDungeonBoss() || creature->isWorldBoss())
            return true;
        if (TempSummon* summon = creature->ToTempSummon())
            if (Unit* summoner = summon->GetSummoner())
                if (Creature* summonerCreature = summoner->ToCreature())
                    return summonerCreature->IsDungeonBoss() || summonerCreature->isWorldBoss();
        return false;
    }

    // AB ABUtils.cpp:1723-1896 isCreatureRelevant + the level window of ABAllCreatureScript.cpp:470-497 and the
    // DisabledID check of :516-522 (the parts that apply when the level is chosen).
    bool IsRelevant(Creature* creature, uint8 unmodifiedLevel, InstanceState const& state, Settings const& s, Map* map)
    {
        // player pets and player-controlled summons are never scaled
        if (creature->IsPet() || ((creature->IsHunterPet() || creature->IsSummon()) && creature->IsControlledByPlayer()))
            return false;

        // a creature a player summoned is scaled only if it can fight the players (ABUtils.cpp:1787-1873)
        if (TempSummon* summon = creature->ToTempSummon())
            if (Unit* summonerUnit = summon->GetSummoner())
                if (Player* summoner = summonerUnit->ToPlayer())
                {
                    bool hostileToAnyValidPlayer = false;
                    Map::PlayerList const& players = map->GetPlayers();
                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    {
                        Player* player = itr->GetSource();
                        if (!player || player->IsGameMaster() || player->IsCharmed() || player->IsHostileToPlayers() ||
                            player->IsHostileTo(summoner) || !player->IsAlive())
                            continue;
                        if (creature->IsGuardian() || creature->IsTotem())
                            continue;
                        if (player->isTargetableForAttack(true))
                        {
                            hostileToAnyValidPlayer = true;
                            break;
                        }
                    }
                    if (!hostileToAnyValidPlayer)
                        return false;
                }

        // tiny critters are ignored (ABUtils.cpp:1877)
        bool const critter = creature->GetCreatureType() == CREATURE_TYPE_CRITTER;
        if (critter && unmodifiedLevel <= 5 && creature->GetMaxHealth() < 100)
            return false;

        // AutoBalance.DisabledID (forced player count 0, ABAllCreatureScript.cpp:516-522)
        auto const forced = s.forcedCreatureIds.find(creature->GetEntry());
        if (forced != s.forcedCreatureIds.end() && forced->second == 0)
            return false;

        // outside the 85%-115% LFG window: untouched, unless a trigger or a substantial critter
        bool const inWindow = unmodifiedLevel >= WindowLow(state.lfgMinLevel) && unmodifiedLevel <= WindowHigh(state.lfgMaxLevel);
        if (!inWindow && !(critter && unmodifiedLevel >= 5 && creature->GetMaxHealth() > 100) && !creature->IsTrigger())
            return false;

        return true;
    }

    // AB ABAllCreatureScript.cpp:546-640: the level a relevant creature gets.
    uint8 SelectScaledLevel(uint8 unmodifiedLevel, InstanceState const& state, Settings const& s)
    {
        int32 const players = int32(state.highestPlayerLevel);
        if (!players)
            return unmodifiedLevel;

        // only scale when the instance's average creature level is outside the skip range (0 = always scale that way)
        if (!OutsideSkipRange(state))
            return unmodifiedLevel;

        int32 selected;
        if (s.method == ScalingMethod::Dynamic)
        {
            selected = (players + state.ceiling) - (int32(state.highestCreatureLevel) - int32(unmodifiedLevel));
            if (selected < players - state.floor)
                selected = players - state.floor;
            if (selected > players + state.ceiling)
                selected = players + state.ceiling;
        }
        else
            selected = players;

        // creature_classlevelstats covers levels 1-100 on this core
        return uint8(std::clamp<int32>(selected, 1, 100));
    }

    // AB ABAllCreatureScript.cpp:513-543 + :645-1409: the player-count (and level) multipliers of a relevant creature.
    void ComputeCreatureMultipliers(CreatureState& cs, CreatureTemplate const* cinfo, InstanceState const& state, Settings const& s)
    {
        cs.healthMultiplier = cs.scaledHealthMultiplier = 1.0f;
        cs.manaMultiplier = cs.scaledManaMultiplier = 1.0f;
        cs.armorMultiplier = cs.scaledArmorMultiplier = 1.0f;
        cs.damageMultiplier = cs.meleeDamageMultiplier = cs.spellDamageMultiplier = 1.0f;
        cs.ccDurationMultiplier = cs.xpModifier = cs.moneyModifier = 1.0f;

        // ForcedID: never scaled for more players than its list says
        uint32 adjustedPlayerCount = state.adjustedPlayerCount;
        auto const forced = s.forcedCreatureIds.find(cinfo->Entry);
        if (forced != s.forcedCreatureIds.end() && forced->second > 0 && adjustedPlayerCount > uint32(forced->second))
            adjustedPlayerCount = uint32(forced->second);
        cs.instancePlayerCount = adjustedPlayerCount;
        if (!adjustedPlayerCount)                                       // no players: attributes unchanged (AB :537-541)
            return;

        LevelRatios ratios;
        if (cs.selectedLevel != cs.unmodifiedLevel)
            ratios = ComputeLevelRatios(cinfo, cs.unmodifiedLevel, cs.selectedLevel);

        float const defaultMultiplier = DefaultMultiplier(state.maxPlayers, adjustedPlayerCount, InflectionFor(state, s, cs.boss));
        StatModifiers const mods = StatModifiersFor(state, s, true, cs.boss, cinfo->Entry);

        float health = defaultMultiplier * mods.global * mods.health;
        if (health <= s.minHPModifier)
            health = s.minHPModifier;
        cs.healthMultiplier = health;
        cs.scaledHealthMultiplier = health * ratios.health;

        float mana = defaultMultiplier * mods.global * mods.mana;
        if (mana <= s.minManaModifier)
            mana = s.minManaModifier;
        cs.manaMultiplier = mana;
        cs.scaledManaMultiplier = mana;                                 // the core already reads the mana of the scaled level

        cs.armorMultiplier = defaultMultiplier * mods.global * mods.armor;
        cs.scaledArmorMultiplier = cs.armorMultiplier;                  // the core already reads the armor of the scaled level

        float damage = defaultMultiplier * mods.global * mods.damage;
        if (damage <= s.minDamageModifier)
            damage = s.minDamageModifier;
        cs.damageMultiplier = damage;
        cs.meleeDamageMultiplier = damage * ratios.melee;
        cs.spellDamageMultiplier = damage * ratios.health;

        if (mods.ccduration != -1.0f)
            cs.ccDurationMultiplier = std::min(std::max(defaultMultiplier * mods.ccduration, s.minCCDurationModifier), s.maxCCDurationModifier);

        // rewards (ABAllCreatureScript.cpp:1334-1409): dynamic = the mean of the scaled health and damage multipliers
        float const average = (cs.scaledHealthMultiplier + cs.meleeDamageMultiplier) / 2.0f;
        if (s.rewardXP)
            cs.xpModifier = s.rewardMethod == ScalingMethod::Fixed ? s.rewardXPModifier : average * s.rewardXPModifier;
        if (s.rewardMoney)
            cs.moneyModifier = s.rewardMethod == ScalingMethod::Fixed ? s.rewardMoneyModifier : average * s.rewardMoneyModifier;
    }

    // rounded, at least 1, at most INT32_MAX (no lround: long is 32-bit on Windows)
    double ScaledAmount(double amount, float multiplier)
    {
        double const scaled = std::floor(amount * double(multiplier) + 0.5);
        return std::max(1.0, std::min(scaled, double(std::numeric_limits<int32>::max())));
    }

    bool HasAuraType(SpellInfo const* spellInfo, AuraType auraType)
    {
        if (!spellInfo)
            return false;
        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (spellInfo->Effects[i].IsAura() && spellInfo->Effects[i].ApplyAuraName == auraType)
                return true;
        return false;
    }

    // 5.4 spells whose SpellEffectScaling multiplier is driven by a SpellScaling class already follow the caster's level
    // (SpellEffectInfo::CalcValue; without a class the gtSpellScaling lookup fails and the base points are used unscaled)
    bool HasNativeLevelScaling(SpellInfo const* spellInfo)
    {
        SpellScalingEntry const* scaling = spellInfo->GetSpellScaling();
        if (!scaling || scaling->ScalingClass == 0)
            return false;
        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (spellInfo->Effects[i].ScalingMultiplier != 0.0f)
                return true;
        return false;
    }

    // Spell effects that deal the caster's weapon damage (Spell::EffectWeaponDmg) take the melee multiplier.
    bool UsesWeaponDamage(SpellInfo const* spellInfo)
    {
        return spellInfo->HasEffect(SPELL_EFFECT_WEAPON_DAMAGE) || spellInfo->HasEffect(SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL) ||
            spellInfo->HasEffect(SPELL_EFFECT_WEAPON_PERCENT_DAMAGE) || spellInfo->HasEffect(SPELL_EFFECT_NORMALIZED_WEAPON_DMG);
    }

    enum class AmountKind : uint8
    {
        Melee,
        Spell,                                                          // direct spell damage or healing
        Periodic                                                        // periodic damage tick
    };

    bool IsPlayerControlledSummon(Unit* unit)
    {
        return (unit->IsHunterPet() || unit->IsPet() || unit->IsSummon()) && unit->IsControlledByPlayer();
    }

    // AB ABUnitScript.cpp:186-459 _Modify_Damage_Healing, as a multiplier (1.0 = unchanged). healing = AB's amount >= 0.
    float DamageHealingMultiplier(Unit* target, Unit* source, bool healing, SpellInfo const* spellInfo, AmountKind kind)
    {
        if (!AutoBalance::IsActive() || !target)
            return 1.0f;
        if (!source)
            source = target;

        Map* map = source->GetMap();
        if (!map || !target->GetMap() || !map->IsDungeon() || !target->GetMap()->IsDungeon() || !map->GetInstanceId())
            return 1.0f;
        // MoP difference: source and target share the instance (AB reads each one's map; they never differ in practice)
        if (target->GetMap() != map || !source->IsInWorld())
            return 1.0f;
        if (spellInfo && InList(SpellIdsToNeverModify, spellInfo->Id))
            return 1.0f;

        bool const sourcePlayer = source->GetTypeId() == TYPEID_PLAYER;
        bool const self = source->GetGUID() == target->GetGUID();
        if (sourcePlayer && self && healing)
            return 1.0f;                                                // a player healing itself
        else if (sourcePlayer && self && !healing)
        {
            if (spellInfo && InList(SpellIdsThatSpendPlayerHealth, spellInfo->Id))
                return 1.0f;                                            // Life Tap and friends
        }
        else if (sourcePlayer && target->IsFriendlyTo(source) && !healing)
        {
            // friendly fire: world multiplier below
        }
        else if (sourcePlayer)
            return 1.0f;                                                // players' damage and healing are never changed
        else if (source->GetTypeId() == TYPEID_UNIT && self && HasAuraType(spellInfo, SPELL_AURA_SHARE_DAMAGE_PCT))
            return 1.0f;                                                // a creature sharing damage with itself

        if (IsPlayerControlledSummon(source))
            return 1.0f;

        std::shared_ptr<InstanceState> state = FindInstanceState(map->GetInstanceId());
        if (!state)
            return 1.0f;

        bool const percentPeriodic = HasAuraType(spellInfo, SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
        std::lock_guard<std::mutex> guard(state->lock);
        if (!state->enabled)
            return 1.0f;

        if (sourcePlayer && self && !healing)
            return percentPeriodic ? state->worldDamageHealingMultiplier : state->scaledWorldDamageHealingMultiplier;
        if (target->GetTypeId() == TYPEID_PLAYER && healing)
            return state->scaledWorldDamageHealingMultiplier;           // a non-player healing a player
        if (target->GetTypeId() == TYPEID_PLAYER && source->GetTypeId() != TYPEID_UNIT && !healing)
            return percentPeriodic ? state->worldDamageHealingMultiplier : state->scaledWorldDamageHealingMultiplier;

        // a creature: its own multiplier (AB ScaledDamageMultiplier; DamageMultiplier for percent-of-health periodic damage)
        auto const itr = state->creatures.find(source->GetGUID());
        if (itr == state->creatures.end())
            return 1.0f;
        CreatureState const& cs = itr->second;
        if (kind == AmountKind::Melee)
            return cs.meleeDamageMultiplier;
        if (percentPeriodic || (spellInfo && HasNativeLevelScaling(spellInfo)))
            return cs.damageMultiplier;
        if (kind == AmountKind::Spell && spellInfo && UsesWeaponDamage(spellInfo))
            return cs.meleeDamageMultiplier;
        return cs.spellDamageMultiplier;
    }

    // AB ABUnitScript.cpp:461-514 _Modifier_CCDuration (+ the refresh guard, see the file header)
    void ModifyCCDuration(Unit* unit, Aura* aura)
    {
        if (!AutoBalance::IsActive() || !unit || !aura)
            return;
        if (!(aura->GetDuration() > 0 || aura->GetMaxDuration() > 0))
            return;
        Unit* caster = aura->GetCaster();
        if (!caster || aura->GetDuration() != aura->GetMaxDuration())
            return;
        if (unit->GetTypeId() != TYPEID_PLAYER || caster->GetTypeId() == TYPEID_PLAYER)
            return;
        Map* map = unit->GetMap();
        if (!map || !map->IsDungeon() || !caster->GetMap() || !caster->GetMap()->IsDungeon() || caster->GetMap() != map)
            return;
        if (IsPlayerControlledSummon(caster))
            return;
        if (!(aura->HasEffectType(SPELL_AURA_MOD_CHARM) || aura->HasEffectType(SPELL_AURA_MOD_CONFUSE) ||
            aura->HasEffectType(SPELL_AURA_MOD_DISARM) || aura->HasEffectType(SPELL_AURA_MOD_FEAR) ||
            aura->HasEffectType(SPELL_AURA_MOD_PACIFY) || aura->HasEffectType(SPELL_AURA_MOD_POSSESS) ||
            aura->HasEffectType(SPELL_AURA_MOD_SILENCE) || aura->HasEffectType(SPELL_AURA_MOD_STUN) ||
            aura->HasEffectType(SPELL_AURA_MOD_SPEED_SLOW_ALL)))
            return;

        std::shared_ptr<InstanceState> state = FindInstanceState(map->GetInstanceId());
        if (!state)
            return;

        int32 const original = aura->GetDuration();
        int32 scaledDuration = 0;
        {
            std::lock_guard<std::mutex> guard(state->lock);
            if (!state->enabled)
                return;
            auto const itr = state->creatures.find(caster->GetGUID());
            if (itr == state->creatures.end() || itr->second.ccDurationMultiplier == 1.0f)
                return;

            auto const key = std::make_tuple(unit->GetGUID(), caster->GetGUID(), aura->GetId());
            auto const done = state->ccScaled.find(key);
            if (done != state->ccScaled.end() && done->second == aura->GetMaxDuration())
                return;                                                 // a refresh of an aura already shortened

            scaledDuration = int32(uint32(float(original) * itr->second.ccDurationMultiplier));
            if (scaledDuration == original)
                return;
            state->ccScaled[key] = scaledDuration;
            ++state->ccEvents;
            state->ccLastBefore = original;
            state->ccLastAfter = scaledDuration;
        }

        aura->SetMaxDuration(scaledDuration);
        aura->SetDuration(scaledDuration);
    }

    void CountHeal(Unit* source, uint32 before, uint32 after)
    {
        if (!source || !source->GetMap() || !source->GetMap()->GetInstanceId())
            return;
        if (std::shared_ptr<InstanceState> state = FindInstanceState(source->GetMap()->GetInstanceId()))
        {
            std::lock_guard<std::mutex> guard(state->lock);
            ++state->healEvents;
            state->healBefore += before;
            state->healAfter += after;
        }
    }

    class AutoBalanceUnitScript : public UnitScript
    {
    public:
        AutoBalanceUnitScript() : UnitScript("AutoBalance_UnitScript") { }

        // AB ABUnitScript.cpp:52-70; this core's hook runs before armor (Unit::CalculateMeleeDamage)
        void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage) override
        {
            if (!AutoBalance::IsActive() || !damage)
                return;
            float const multiplier = DamageHealingMultiplier(target, attacker, false, nullptr, AmountKind::Melee);
            if (multiplier != 1.0f)
                damage = uint32(ScaledAmount(double(damage), multiplier));
        }

        // AB ABUnitScript.cpp:30-50 (a positive spell's amount counts as healing there)
        void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* spellInfo) override
        {
            if (!AutoBalance::IsActive() || damage <= 0 || !spellInfo)
                return;
            float const multiplier = DamageHealingMultiplier(target, attacker, spellInfo->IsPositive(), spellInfo, AmountKind::Spell);
            if (multiplier != 1.0f)
                damage = int32(ScaledAmount(double(damage), multiplier));
        }

        // AB ABUnitScript.cpp:8-28. This hook also runs for periodic heal ticks (SpellAuraEffects.cpp HandlePeriodicHealAurasTick),
        // which ModifyHealReceived scales right after it: damage ticks only here (see the file header).
        void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, int32& damage, SpellInfo const* spellInfo) override
        {
            if (!AutoBalance::IsActive() || damage <= 0 || !spellInfo || spellInfo->IsPositive())
                return;
            float const multiplier = DamageHealingMultiplier(target, attacker, false, spellInfo, AmountKind::Periodic);
            if (multiplier != 1.0f)
                damage = int32(ScaledAmount(double(damage), multiplier));
        }

        // AB ABUnitScript.cpp:72-87 (Unit::HealBySpell passes (target, healer) since #829, as the periodic heal tick does)
        void ModifyHealReceived(Unit* target, Unit* healer, uint32& heal, SpellInfo const* spellInfo) override
        {
            if (!AutoBalance::IsActive() || !heal)
                return;
            float const multiplier = DamageHealingMultiplier(target, healer, true, spellInfo, AmountKind::Spell);
            if (multiplier == 1.0f)
                return;
            uint32 const before = heal;
            heal = uint32(ScaledAmount(double(heal), multiplier));
            CountHeal(healer ? healer : target, before, heal);
        }

        // AB ABUnitScript.cpp:89-114
        void OnAuraApply(Unit* unit, Aura* aura) override
        {
            ModifyCCDuration(unit, aura);
        }
    };

    // AB ABAllMapScript.cpp:75-114 / :196-226 + Message.cpp (English texts)
    void NotifyPlayers(Map* map, Player* subject, bool entered, uint32 playerCount, uint32 adjustedPlayerCount)
    {
        InstanceMap* instanceMap = map->ToInstanceMap();
        if (!instanceMap || !subject)
            return;
        if (!entered && subject->IsGameMaster())
            return;

        char buffer[256];
        Map::PlayerList const& players = map->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        {
            Player* player = itr->GetSource();
            if (!player || !player->GetSession())
                continue;

            if (entered && player == subject)
            {
                std::snprintf(buffer, sizeof(buffer), "|cffc3dbff [AutoBalance]|r|cffFF8000 Welcome to %s (%u-player %s). There are %u player(s) in this instance. Difficulty set to %u player(s).|r",
                    map->GetMapName(), instanceMap->GetMaxPlayers(), instanceMap->IsHeroic() ? "Heroic" : "Normal", playerCount, adjustedPlayerCount);
                ChatHandler(player->GetSession()).SendSysMessage(buffer);
                if (player->IsGameMaster())
                    ChatHandler(player->GetSession()).SendSysMessage("|cffc3dbff [AutoBalance]|r|cffFF8000 Your GM flag is turned on. AutoBalance will ignore you. Please turn GM off and exit/re-enter the instance if you'd like to be considering for AutoBalancing.|r");
            }
            else if (player != subject && !subject->IsGameMaster())
            {
                if (entered)
                    std::snprintf(buffer, sizeof(buffer), "|cffc3dbff [AutoBalance]|r|cffFF8000 %s enters the instance. There are %u player(s) in this instance. Difficulty set to %u player(s).|r",
                        subject->GetName().c_str(), playerCount, adjustedPlayerCount);
                else
                    std::snprintf(buffer, sizeof(buffer), "|cffc3dbff [AutoBalance]|r|cffFF8000 %s left the instance. There are %u player(s) in this instance. Difficulty set to %u player(s).|r",
                        subject->GetName().c_str(), playerCount, adjustedPlayerCount);
                ChatHandler(player->GetSession()).SendSysMessage(buffer);
            }
        }
    }

    // players entering/leaving or levelling: recount, and re-scale the creatures when the count or the highest level changed
    void OnInstancePlayersChanged(Map* map, Player* player, Player const* leaving, bool notify, bool entered)
    {
        std::shared_ptr<Settings const> settings = GetSettings();
        std::shared_ptr<InstanceState> state = entered ? GetOrCreateInstanceState(map, *settings) : FindInstanceState(map->GetInstanceId());
        if (!state)
            return;

        bool enabled = false;
        uint32 playerCount = 0, realPlayerCount = 0, adjusted = 0;
        {
            std::lock_guard<std::mutex> guard(state->lock);
            RefreshIfStale(*state, map);
            uint32 const oldAdjusted = state->adjustedPlayerCount;
            uint8 const oldHighest = state->highestPlayerLevel;
            if (RecountPlayers(*state, map, leaving, *settings) && state->enabled)
            {
                ++state->stamp;
                TC_LOG_INFO("maps", "AutoBalance: instance %u (map %u): players %u (adjusted %u -> %u), highest player level %u -> %u",
                    map->GetInstanceId(), map->GetId(), state->playerCount, oldAdjusted, state->adjustedPlayerCount,
                    uint32(oldHighest), uint32(state->highestPlayerLevel));
            }
            enabled = state->enabled;
            playerCount = state->playerCount;
            realPlayerCount = state->realPlayerCount;
            adjusted = state->adjustedPlayerCount;
        }

        if (notify && enabled && settings->playerChangeNotify)
            NotifyPlayers(map, player, entered, entered ? playerCount : realPlayerCount, adjusted);   // AB :186 (0 when the last one left)
    }

    class AutoBalancePlayerScript : public PlayerScript
    {
    public:
        AutoBalancePlayerScript() : PlayerScript("AutoBalance_PlayerScript") { }

        // AB ABPlayerScript.cpp:12-16
        void OnLogin(Player* player) override
        {
            if (AutoBalance::IsActive() && player && player->GetSession() && GetSettings()->announce)
                ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00AutoBalance |rmodule.");
        }

        // AB ABPlayerScript.cpp:18-37
        void OnLevelChanged(Player* player, uint8 /*oldLevel*/) override
        {
            if (!AutoBalance::IsActive() || !player || player->IsGameMaster())
                return;
            Map* map = player->GetMap();
            if (!map || !map->IsDungeon() || !map->GetInstanceId())
                return;
            OnInstancePlayersChanged(map, player, nullptr, false, false);
        }

        // AB ABPlayerScript.cpp:39-74
        void OnGiveXP(Player* player, uint32& amount, Unit* victim) override
        {
            if (!AutoBalance::IsActive() || !player || !victim)
                return;
            Map* map = player->GetMap();
            if (!map || !map->IsDungeon() || !map->GetInstanceId())
                return;

            std::shared_ptr<Settings const> settings = GetSettings();
            std::shared_ptr<InstanceState> state = FindInstanceState(map->GetInstanceId());
            if (!state || !settings->rewardXP)
                return;

            std::lock_guard<std::mutex> guard(state->lock);
            if (!state->enabled)
                return;

            float modifier = 1.0f;
            auto const itr = state->creatures.find(victim->GetGUID());
            if (itr != state->creatures.end())
                modifier = itr->second.xpModifier;

            uint32 const before = amount;
            if (settings->rewardMethod == ScalingMethod::Dynamic)
                amount = uint32(amount * modifier);
            else if (state->maxPlayers)
                amount = uint32(amount * modifier * (float(state->playerCount) / state->maxPlayers));

            ++state->xpEvents;
            state->xpBefore += before;
            state->xpAfter += amount;
            state->xpLastEntry = victim->GetEntry();
            state->xpLastBefore = before;
            state->xpLastAfter = amount;
            state->xpLastModifier = modifier;
        }
    };

    std::shared_ptr<InstanceState> PlayerInstanceState(ChatHandler* handler, Player*& player)
    {
        player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (!player || !player->GetMap() || !player->GetMap()->IsDungeon() || !player->GetMap()->GetInstanceId())
        {
            handler->SendSysMessage("This command can only be used in a dungeon or raid.|r");
            handler->SetSentErrorMessage(true);
            return nullptr;
        }
        return FindInstanceState(player->GetMap()->GetInstanceId());
    }

    // AB ABCommandScript.h:21-35 / ABCommandScript.cpp:9-186
    class AutoBalanceCommandScript : public CommandScript
    {
    public:
        AutoBalanceCommandScript() : CommandScript("AutoBalance_CommandScript") { }

        ChatCommandTable GetCommands() const override
        {
            static ChatCommandTable abCommandTable =
            {
                { "setoffset",    &HandleSetOffset,     rbac::RBAC_PERM_COMMAND_INSTANCE_SET_BOSS_STATE, Trinity::ChatCommands::Console::Yes },
                { "getoffset",    &HandleGetOffset,     rbac::RBAC_PERM_COMMAND_INSTANCE_STATS,          Trinity::ChatCommands::Console::Yes },
                { "mapstat",      &HandleMapStat,       rbac::RBAC_PERM_COMMAND_INSTANCE_STATS,          Trinity::ChatCommands::Console::No },
                { "creaturestat", &HandleCreatureStat,  rbac::RBAC_PERM_COMMAND_INSTANCE_STATS,          Trinity::ChatCommands::Console::No },
            };
            static ChatCommandTable commandTable =
            {
                { "autobalance", abCommandTable, rbac::RBAC_PERM_COMMAND_INSTANCE_STATS, Trinity::ChatCommands::Console::Yes },
                { "ab",          abCommandTable, rbac::RBAC_PERM_COMMAND_INSTANCE_STATS, Trinity::ChatCommands::Console::Yes },
            };
            return commandTable;
        }

        // AB ABCommandScript.cpp:9-32 (globalConfigTime -> every instance re-processes)
        static bool HandleSetOffset(ChatHandler* handler, char const* args)
        {
            if (!args || !*args)
            {
                handler->SendSysMessage(".autobalance setoffset #");
                handler->SendSysMessage("Set the player difficulty offset for this instance. Usage: .ab offset <number>.|r");
                return false;
            }
            char* end = nullptr;
            long const value = std::strtol(args, &end, 10);
            if (end == args)
            {
                handler->SendSysMessage("Error changing Player Difficulty Offset! Please try again.|r");
                return false;
            }
            std::shared_ptr<Settings> s = std::make_shared<Settings>(*GetSettings());
            s->playerCountDifficultyOffset = int32(std::clamp<long>(value, -255, 255));
            int32 const offset = s->playerCountDifficultyOffset;
            PublishSettings(s);
            handler->PSendSysMessage("Changing Player Difficulty Offset to %d.|r", offset);
            return true;
        }

        static bool HandleGetOffset(ChatHandler* handler, char const* /*args*/)
        {
            handler->PSendSysMessage("Current Player Difficulty Offset = %d.|r", GetSettings()->playerCountDifficultyOffset);
            return true;
        }

        // AB ABCommandScript.cpp:40-126
        static bool HandleMapStat(ChatHandler* handler, char const* /*args*/)
        {
            Player* player = nullptr;
            std::shared_ptr<InstanceState> state = PlayerInstanceState(handler, player);
            if (!player)
                return false;
            Map* map = player->GetMap();
            InstanceMap* instanceMap = map->ToInstanceMap();
            handler->SendSysMessage("---");
            if (!state)
            {
                handler->PSendSysMessage("%s | ID %u-%u | AutoBalance DISABLED", map->GetMapName(), map->GetId(), map->GetInstanceId());
                return true;
            }

            std::shared_ptr<Settings const> s = GetSettings();
            std::lock_guard<std::mutex> guard(state->lock);
            handler->PSendSysMessage("%s (%u-player %s) | ID %u-%u%s", map->GetMapName(), instanceMap ? instanceMap->GetMaxPlayers() : 0,
                state->heroic ? "Heroic" : "Normal", map->GetId(), map->GetInstanceId(), state->enabled ? "" : " | AutoBalance DISABLED");
            handler->PSendSysMessage("Players on map: %u (Lvl %u - %u)", state->playerCount, uint32(state->lowestPlayerLevel), uint32(state->highestPlayerLevel));
            int32 const offset = s->playerCountDifficultyOffset;
            if (state->playerCount < state->minPlayers && !offset)
                handler->PSendSysMessage("Adjusted Player Count: %u (Map Minimum)|r", state->adjustedPlayerCount);
            else if (state->playerCount < state->minPlayers && offset)
                handler->PSendSysMessage("Adjusted Player Count: %u (Map Minimum + Difficulty Offset of %d)|r", state->adjustedPlayerCount, offset);
            else if (offset)
                handler->PSendSysMessage("Adjusted Player Count: %u (Difficulty Offset of %d)|r", state->adjustedPlayerCount, offset);
            else
                handler->PSendSysMessage("Adjusted Player Count: %u|r", state->adjustedPlayerCount);
            handler->PSendSysMessage("LFG Range: Lvl %u - %u (Target: Lvl %u)|r", uint32(state->lfgMinLevel), uint32(state->lfgMaxLevel), uint32(state->lfgTargetLevel));
            bool const levelScaled = state->enabled && s->levelScaling && state->highestPlayerLevel && OutsideSkipRange(*state);
            if (levelScaled)
                handler->PSendSysMessage("Map Level: %u->%u (Level Scaling Enabled)|r", uint32(state->avgCreatureLevel + 0.5f), uint32(state->highestPlayerLevel));
            else
                handler->PSendSysMessage("Map Level: %u (Level Scaling Disabled)|r", uint32(state->avgCreatureLevel + 0.5f));
            handler->PSendSysMessage("World health multiplier: %.3f|r", state->worldHealthMultiplier);
            if (state->worldDamageHealingMultiplier != state->scaledWorldDamageHealingMultiplier)
                handler->PSendSysMessage("World hostile damage and healing multiplier: %.3f -> %.3f|r", state->worldDamageHealingMultiplier, state->scaledWorldDamageHealingMultiplier);
            else
                handler->PSendSysMessage("World hostile damage and healing multiplier: %.3f|r", state->worldDamageHealingMultiplier);
            // MoP difference: the map statistics come from the static spawns (highest + average), not the live creature list
            handler->PSendSysMessage("Original Creature Level Range: up to %u (Avg: %.2f)|r", uint32(state->highestCreatureLevel), state->avgCreatureLevel);
            handler->PSendSysMessage("Scaled creatures in map: %u|r", uint32(state->creatures.size()));
            return true;
        }

        // AB ABCommandScript.cpp:128-186
        static bool HandleCreatureStat(ChatHandler* handler, char const* /*args*/)
        {
            Creature* target = handler->getSelectedCreature();
            if (!target)
            {
                handler->SendSysMessage(LANG_SELECT_CREATURE);
                handler->SetSentErrorMessage(true);
                return false;
            }
            if (!target->GetMap() || !target->GetMap()->IsDungeon() || !target->GetMap()->GetInstanceId())
            {
                handler->SendSysMessage("That target is not in an instance.|r");
                handler->SetSentErrorMessage(true);
                return false;
            }

            CreatureState cs;
            bool tracked = false;
            if (std::shared_ptr<InstanceState> state = FindInstanceState(target->GetMap()->GetInstanceId()))
            {
                std::lock_guard<std::mutex> guard(state->lock);
                auto const itr = state->creatures.find(target->GetGUID());
                if (itr != state->creatures.end())
                {
                    cs = itr->second;
                    tracked = true;
                }
            }
            if (!tracked)
            {
                cs.unmodifiedLevel = cs.selectedLevel = target->GetLevel();
                cs.boss = IsBossOrBossSummon(target);
            }

            handler->SendSysMessage("---");
            if (cs.selectedLevel != cs.unmodifiedLevel)
                handler->PSendSysMessage("%s (%u->%u%s), %s", target->GetName().c_str(), uint32(cs.unmodifiedLevel), uint32(cs.selectedLevel),
                    cs.boss ? " | Boss" : "", tracked ? "Active for Map Stats|r" : "Ignored for Map Stats|r");
            else
                handler->PSendSysMessage("%s (%u%s), %s", target->GetName().c_str(), uint32(cs.unmodifiedLevel), cs.boss ? " | Boss" : "",
                    tracked ? "Active for Map Stats|r" : "Ignored for Map Stats|r");
            handler->PSendSysMessage("Creature difficulty level: %u player(s)|r", cs.instancePlayerCount);
            if (cs.selectedLevel != cs.unmodifiedLevel)
            {
                handler->PSendSysMessage("Health multiplier: %.3f -> %.3f|r", cs.healthMultiplier, cs.scaledHealthMultiplier);
                handler->PSendSysMessage("Mana multiplier: %.3f -> %.3f|r", cs.manaMultiplier, cs.scaledManaMultiplier);
                handler->PSendSysMessage("Armor multiplier: %.3f -> %.3f|r", cs.armorMultiplier, cs.scaledArmorMultiplier);
                handler->PSendSysMessage("Damage multiplier: %.3f -> %.3f|r", cs.damageMultiplier, cs.meleeDamageMultiplier);
            }
            else
            {
                handler->PSendSysMessage("Health multiplier: %.3f|r", cs.healthMultiplier);
                handler->PSendSysMessage("Mana multiplier: %.3f|r", cs.manaMultiplier);
                handler->PSendSysMessage("Armor multiplier: %.3f|r", cs.armorMultiplier);
                handler->PSendSysMessage("Damage multiplier: %.3f|r", cs.damageMultiplier);
            }
            handler->PSendSysMessage("CC Duration multiplier: %.3f|r", cs.ccDurationMultiplier);
            handler->PSendSysMessage("XP multiplier: %.3f  Money multiplier: %.3f|r", cs.xpModifier, cs.moneyModifier);
            return true;
        }
    };

    bool g_scriptMgrReady = false;                                      // world thread only
    bool g_scriptsRegistered = false;

    // Script registration grows ScriptMgr's registries: only from the world thread while the map threads are parked
    // (startup, or a .reload config command), the same contract as the harness' lazy scripts (BotScenarioSpellWatch.cpp).
    void RegisterScriptsIfActive()
    {
        if (!g_scriptMgrReady || g_scriptsRegistered || !AutoBalance::IsActive())
            return;

        g_scriptsRegistered = true;
        new AutoBalanceUnitScript();
        new AutoBalancePlayerScript();
        new AutoBalanceCommandScript();
        ChatHandler::SetLoadCommandTable(true);                         // the .autobalance table is picked up at the next command
        TC_LOG_INFO("server.loading", "AutoBalance: hooks registered (damage, healing, CC, XP, level-up, login) and the .autobalance command");
    }
}

uint32 AutoBalance::ExpansionColumnForLevel(uint8 level)
{
    if (level > 86)
        return EXPANSION_MISTS_OF_PANDARIA;
    if (level > 81)
        return EXPANSION_CATACLYSM;
    if (level > 71)
        return EXPANSION_WRATH_OF_THE_LICH_KING;
    return EXPANSION_CLASSIC;
}

void AutoBalance::LoadConfig()
{
    std::shared_ptr<Settings> s = std::make_shared<Settings>();

    // AB conf/AutoBalance.conf.dist + ABWorldScript.cpp:17-500 (keys and defaults identical, except Enable.Global: 1 in AB, 0 here)
    s->enableGlobal = sConfigMgr->GetBoolDefault("AutoBalance.Enable.Global", false);
    s->enable5M = sConfigMgr->GetBoolDefault("AutoBalance.Enable.5M", true);
    s->enable10M = sConfigMgr->GetBoolDefault("AutoBalance.Enable.10M", true);
    s->enable15M = sConfigMgr->GetBoolDefault("AutoBalance.Enable.15M", true);
    s->enable20M = sConfigMgr->GetBoolDefault("AutoBalance.Enable.20M", true);
    s->enable25M = sConfigMgr->GetBoolDefault("AutoBalance.Enable.25M", true);
    s->enable40M = sConfigMgr->GetBoolDefault("AutoBalance.Enable.40M", true);
    s->enableOtherNormal = sConfigMgr->GetBoolDefault("AutoBalance.Enable.OtherNormal", true);
    s->enable5MHeroic = sConfigMgr->GetBoolDefault("AutoBalance.Enable.5MHeroic", true);
    s->enable10MHeroic = sConfigMgr->GetBoolDefault("AutoBalance.Enable.10MHeroic", true);
    s->enable25MHeroic = sConfigMgr->GetBoolDefault("AutoBalance.Enable.25MHeroic", true);
    s->enableOtherHeroic = sConfigMgr->GetBoolDefault("AutoBalance.Enable.OtherHeroic", true);
    s->disabledMaps = LoadIdSet(sConfigMgr->GetStringDefault("AutoBalance.Disable.PerInstance", ""));

    // ABWorldScript.cpp:29-34 (ForcedID20 is in AB's conf but AB never reads it)
    LoadForcedIds(s->forcedCreatureIds, sConfigMgr->GetStringDefault("AutoBalance.ForcedID40", "", true), 40);
    LoadForcedIds(s->forcedCreatureIds, sConfigMgr->GetStringDefault("AutoBalance.ForcedID25", "", true), 25);
    LoadForcedIds(s->forcedCreatureIds, sConfigMgr->GetStringDefault("AutoBalance.ForcedID10", "", true), 10);
    LoadForcedIds(s->forcedCreatureIds, sConfigMgr->GetStringDefault("AutoBalance.ForcedID5", "", true), 5);
    LoadForcedIds(s->forcedCreatureIds, sConfigMgr->GetStringDefault("AutoBalance.ForcedID2", "", true), 2);
    LoadForcedIds(s->forcedCreatureIds, sConfigMgr->GetStringDefault("AutoBalance.DisabledID", ""), 0);

    // MinPlayers (ABWorldScript.cpp:46-60)
    s->minPlayersNormal = uint32(std::max(0, sConfigMgr->GetIntDefault("AutoBalance.MinPlayers", 1)));
    s->minPlayersHeroic = uint32(std::max(0, sConfigMgr->GetIntDefault("AutoBalance.MinPlayers.Heroic", 1)));
    s->minPlayersRaid = uint32(std::max(0, sConfigMgr->GetIntDefault("AutoBalance.MinPlayers.Raid", int32(s->minPlayersNormal))));
    s->minPlayersRaidHeroic = uint32(std::max(0, sConfigMgr->GetIntDefault("AutoBalance.MinPlayers.RaidHeroic", int32(s->minPlayersHeroic))));
    s->minPlayersPerInstance = LoadMinPlayersPerInstance(sConfigMgr->GetStringDefault("AutoBalance.MinPlayers.PerInstance", "", true));
    s->minPlayersPerHeroicInstance = LoadMinPlayersPerInstance(sConfigMgr->GetStringDefault("AutoBalance.MinPlayers.Heroic.PerInstance", "", true));

    // per-instance overrides (ABWorldScript.cpp:69-93)
    s->inflectionOverrides = LoadInflectionOverrides(sConfigMgr->GetStringDefault("AutoBalance.InflectionPoint.PerInstance", "", true));
    s->bossInflectionOverrides = LoadInflectionOverrides(sConfigMgr->GetStringDefault("AutoBalance.InflectionPoint.Boss.PerInstance", "", true));
    s->statOverrides = LoadStatOverrides(sConfigMgr->GetStringDefault("AutoBalance.StatModifier.PerInstance", "", true));
    s->bossStatOverrides = LoadStatOverrides(sConfigMgr->GetStringDefault("AutoBalance.StatModifier.Boss.PerInstance", "", true));
    s->creatureStatOverrides = LoadStatOverrides(sConfigMgr->GetStringDefault("AutoBalance.StatModifier.PerCreature", "", true));

    // misc (ABWorldScript.cpp:132-138)
    s->playerChangeNotify = sConfigMgr->GetBoolDefault("AutoBalance.PlayerChangeNotify", true);
    s->playerCountDifficultyOffset = sConfigMgr->GetIntDefault("AutoBalance.playerCountDifficultyOffset", 0);

    // InflectionPoint* (ABWorldScript.cpp:148-201): the four broad kinds, then the sizes, blank = the broad kind's value
    for (uint32 c = 0; c < MAX_SIZE_CATEGORY; ++c)
    {
        InflectionSettings const parent = CategoryParent[c] >= 0 ? s->inflection[CategoryParent[c]] : InflectionSettings();
        std::string const stem = InflectionKeys[c];
        s->inflection[c].value = OptionalFloat(stem.c_str(), parent.value);
        s->inflection[c].curveFloor = OptionalFloat((stem + ".CurveFloor").c_str(), parent.curveFloor);
        s->inflection[c].curveCeiling = OptionalFloat((stem + ".CurveCeiling").c_str(), parent.curveCeiling);
        s->inflection[c].bossModifier = OptionalFloat((stem + ".BossModifier").c_str(), parent.bossModifier);
    }

    // StatModifier* (ABWorldScript.cpp:223-415): creature and boss tables, blank = the broad kind's value; CCDuration -1 = unset
    for (uint32 c = 0; c < MAX_SIZE_CATEGORY; ++c)
        for (uint32 boss = 0; boss < 2; ++boss)
        {
            StatModifiers const parent = CategoryParent[c] >= 0 ? s->statModifiers[CategoryParent[c]][boss] : StatModifiers();
            std::string const stem = std::string(StatModifierKeys[c]) + (boss ? ".Boss" : "");
            StatModifiers& m = s->statModifiers[c][boss];
            m.global = OptionalFloat((stem + ".Global").c_str(), parent.global);
            m.health = OptionalFloat((stem + ".Health").c_str(), parent.health);
            m.mana = OptionalFloat((stem + ".Mana").c_str(), parent.mana);
            m.armor = OptionalFloat((stem + ".Armor").c_str(), parent.armor);
            m.damage = OptionalFloat((stem + ".Damage").c_str(), parent.damage);
            m.ccduration = OptionalFloat((stem + ".CCDuration").c_str(), parent.ccduration);
        }

    // Modifier min/max (ABWorldScript.cpp:421-425)
    s->minHPModifier = OptionalFloat("AutoBalance.MinHPModifier", 0.1f);
    s->minManaModifier = OptionalFloat("AutoBalance.MinManaModifier", 0.01f);
    s->minDamageModifier = OptionalFloat("AutoBalance.MinDamageModifier", 0.01f);
    s->minCCDurationModifier = OptionalFloat("AutoBalance.MinCCDurationModifier", 0.25f);
    s->maxCCDurationModifier = OptionalFloat("AutoBalance.MaxCCDurationModifier", 1.0f);

    // LevelScaling.* (ABWorldScript.cpp:431-462; phase 1)
    s->levelScaling = sConfigMgr->GetBoolDefault("AutoBalance.LevelScaling", true);
    s->method = LoadMethod("AutoBalance.LevelScaling.Method");
    s->skipHigher = sConfigMgr->GetIntDefault("AutoBalance.LevelScaling.SkipHigherLevels", 3);
    s->skipLower = sConfigMgr->GetIntDefault("AutoBalance.LevelScaling.SkipLowerLevels", 5);
    s->ceilingDungeons = sConfigMgr->GetIntDefault("AutoBalance.LevelScaling.DynamicLevel.Ceiling.Dungeons", 1);
    s->floorDungeons = sConfigMgr->GetIntDefault("AutoBalance.LevelScaling.DynamicLevel.Floor.Dungeons", 5);
    s->ceilingHeroicDungeons = sConfigMgr->GetIntDefault("AutoBalance.LevelScaling.DynamicLevel.Ceiling.HeroicDungeons", 2);
    s->floorHeroicDungeons = sConfigMgr->GetIntDefault("AutoBalance.LevelScaling.DynamicLevel.Floor.HeroicDungeons", 5);
    s->ceilingRaids = sConfigMgr->GetIntDefault("AutoBalance.LevelScaling.DynamicLevel.Ceiling.Raids", 3);
    s->floorRaids = sConfigMgr->GetIntDefault("AutoBalance.LevelScaling.DynamicLevel.Floor.Raids", 5);
    s->ceilingHeroicRaids = sConfigMgr->GetIntDefault("AutoBalance.LevelScaling.DynamicLevel.Ceiling.HeroicRaids", 3);
    s->floorHeroicRaids = sConfigMgr->GetIntDefault("AutoBalance.LevelScaling.DynamicLevel.Floor.HeroicRaids", 5);
    s->dynamicOverrides = LoadDynamicOverrides(sConfigMgr->GetStringDefault("AutoBalance.LevelScaling.DynamicLevel.PerInstance", "229 -1 -1 1, 230 0 0"));

    // RewardScaling.* (ABWorldScript.cpp:477-493)
    s->rewardMethod = LoadMethod("AutoBalance.RewardScaling.Method");
    s->rewardXP = sConfigMgr->GetBoolDefault("AutoBalance.RewardScaling.XP", true);
    s->rewardXPModifier = OptionalFloat("AutoBalance.RewardScaling.XP.Modifier", 1.0f);
    s->rewardMoney = sConfigMgr->GetBoolDefault("AutoBalance.RewardScaling.Money", true);
    s->rewardMoneyModifier = OptionalFloat("AutoBalance.RewardScaling.Money.Modifier", 1.0f);

    // Announcement (ABWorldScript.cpp:499)
    s->announce = sConfigMgr->GetBoolDefault("AutoBalanceAnnounce.enable", true);

    bool const active = s->enableGlobal;
    ScalingMethod const method = s->method;
    int32 const skipHigher = s->skipHigher, skipLower = s->skipLower;
    bool const levelScaling = s->levelScaling;
    PublishSettings(s);

    if (active)
        TC_LOG_INFO("server.loading", "AutoBalance: ON (level scaling %s, method %s, skip %d/%d). Open instances re-scale with the new settings.",
            levelScaling ? "on" : "off", method == ScalingMethod::Fixed ? "fixed" : "dynamic", skipHigher, skipLower);

    // a .reload config that turns the feature on registers the hooks now (no-op at startup: ScriptMgr is not ready yet)
    RegisterScriptsIfActive();
}

void AutoBalance::RegisterScripts()
{
    g_scriptMgrReady = true;
    RegisterScriptsIfActive();
}

uint8 AutoBalance::OnCreatureSelectLevel(Creature* creature, CreatureTemplate const* cinfo, uint8 level, int32& expansionColumn)
{
    expansionColumn = -1;
    if (!creature || !cinfo)
        return level;

    Map* map = creature->GetMap();
    if (!map || !map->IsDungeon() || !map->GetInstanceId())
        return level;

    std::shared_ptr<Settings const> settings = GetSettings();
    if (!settings->enableGlobal)
        return level;

    bool const boss = IsBossOrBossSummon(creature);                     // before the lock (looks the summoner up)

    std::shared_ptr<InstanceState> state = GetOrCreateInstanceState(map, *settings);
    if (!state)
        return level;

    std::lock_guard<std::mutex> guard(state->lock);
    RefreshIfStale(*state, map);
    if (!state->enabled)
    {
        state->creatures.erase(creature->GetGUID());
        return level;
    }

    // the first grid of a new instance loads inside Map::AddPlayerToMap; the entering player is already linked to the
    // map then (Player::SetMap links m_mapRef), so it is counted
    if (!state->counted)
        RecountPlayers(*state, map, nullptr, *settings);

    // a re-scale keeps the level the creature was rolled at (AB UnmodifiedLevel), unless its entry changed
    uint8 unmodifiedLevel = level;
    auto const existing = state->creatures.find(creature->GetGUID());
    if (existing != state->creatures.end() && existing->second.entry == cinfo->Entry && existing->second.unmodifiedLevel)
        unmodifiedLevel = existing->second.unmodifiedLevel;

    if (!IsRelevant(creature, unmodifiedLevel, *state, *settings, map))
    {
        state->creatures.erase(creature->GetGUID());
        return level;
    }

    uint8 const selectedLevel = settings->levelScaling ? SelectScaledLevel(unmodifiedLevel, *state, *settings) : unmodifiedLevel;

    CreatureState& creatureState = state->creatures[creature->GetGUID()];
    creatureState.entry = cinfo->Entry;
    creatureState.unmodifiedLevel = unmodifiedLevel;
    creatureState.selectedLevel = selectedLevel;
    creatureState.boss = boss;
    creatureState.stamp = state->stamp;
    ComputeCreatureMultipliers(creatureState, cinfo, *state, *settings);

    if (selectedLevel != unmodifiedLevel)
        expansionColumn = int32(ExpansionColumnForLevel(selectedLevel));

    TC_LOG_DEBUG("maps", "AutoBalance: %s (entry %u) in instance %u: level %u -> %u (players %u, adjusted %u), health x%.4f, melee x%.4f, spell x%.4f",
        creature->GetName().c_str(), creature->GetEntry(), map->GetInstanceId(), uint32(unmodifiedLevel), uint32(selectedLevel),
        uint32(state->highestPlayerLevel), creatureState.instancePlayerCount, creatureState.healthMultiplier,
        creatureState.meleeDamageMultiplier, creatureState.spellDamageMultiplier);

    return selectedLevel;
}

void AutoBalance::ModifyCreatureHealthMana(Creature* creature, uint32& health, uint32& mana)
{
    Map* map = creature ? creature->GetMap() : nullptr;
    if (!map || !map->IsDungeon() || !map->GetInstanceId())
        return;

    std::shared_ptr<InstanceState> state = FindInstanceState(map->GetInstanceId());
    if (!state)
        return;

    float healthMultiplier = 1.0f, manaMultiplier = 1.0f, armorMultiplier = 1.0f;
    {
        std::lock_guard<std::mutex> guard(state->lock);
        if (!state->enabled)
            return;
        auto const itr = state->creatures.find(creature->GetGUID());
        if (itr == state->creatures.end())
            return;
        healthMultiplier = itr->second.healthMultiplier;
        manaMultiplier = itr->second.manaMultiplier;
        armorMultiplier = itr->second.armorMultiplier;
    }

    // AB ABAllCreatureScript.cpp:759 / :896 / :1000 (origValue x scaled multiplier = the core's scaled-level value x player multiplier)
    if (healthMultiplier != 1.0f)
        health = uint32(ScaledAmount(double(health), healthMultiplier));
    if (manaMultiplier != 1.0f && mana)
        mana = uint32(ScaledAmount(double(mana), manaMultiplier));

    // armor of the level just selected (UpdateEntry sets it again right after; a respawn only runs SelectLevel)
    if (!creature->IsPet())
        if (CreatureTemplate const* cinfo = creature->GetCreatureTemplate())
            creature->SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE,
                float(sObjectMgr->GetCreatureBaseStats(creature->GetLevel(), cinfo->unit_class)->GenerateArmor(cinfo)) * armorMultiplier);
}

float AutoBalance::ModifyCreatureArmor(Creature* creature, float armor)
{
    Map* map = creature ? creature->GetMap() : nullptr;
    if (!map || !map->IsDungeon() || !map->GetInstanceId())
        return armor;

    std::shared_ptr<InstanceState> state = FindInstanceState(map->GetInstanceId());
    if (!state)
        return armor;

    std::lock_guard<std::mutex> guard(state->lock);
    if (!state->enabled)
        return armor;
    auto const itr = state->creatures.find(creature->GetGUID());
    return itr != state->creatures.end() ? armor * itr->second.armorMultiplier : armor;
}

void AutoBalance::OnCreatureUpdate(Creature* creature)
{
    Map* map = creature->GetMap();
    if (!map || !map->GetInstanceId())
        return;

    std::shared_ptr<InstanceState> state = FindInstanceState(map->GetInstanceId());
    if (!state)
        return;

    {
        std::lock_guard<std::mutex> guard(state->lock);
        RefreshIfStale(*state, map);

        auto const itr = state->creatures.find(creature->GetGUID());
        if (itr == state->creatures.end() || itr->second.stamp == state->stamp)
            return;

        if (!creature->IsAlive() || creature->IsInCombat())
            return;

        if (!state->enabled || !IsActive())
            state->creatures.erase(itr);                                // revert to the native stats below
        else
            itr->second.stamp = state->stamp;
    }

    // same work as UpdateEntry does around SelectLevel: level, health, mana, weapon damage, then armor and the stat update
    float const healthPct = creature->GetHealthPct();
    bool const manaUser = creature->GetPowerType() == POWER_MANA && creature->GetMaxPower(POWER_MANA) > 0;
    float const manaPct = manaUser ? float(creature->GetPower(POWER_MANA)) / float(creature->GetMaxPower(POWER_MANA)) : 1.0f;
    CreatureTemplate const* cinfo = creature->GetCreatureTemplate();
    creature->SelectLevel(cinfo, true);                                 // keeps the roll; re-enters OnCreatureSelectLevel / ModifyCreatureHealthMana
    if (!creature->IsPet())
    {
        float const armor = float(sObjectMgr->GetCreatureBaseStats(creature->GetLevel(), cinfo->unit_class)->GenerateArmor(cinfo));
        creature->SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, ModifyCreatureArmor(creature, armor));
    }
    creature->UpdateAllStats();
    creature->SetHealth(std::max<uint32>(1, creature->CountPctFromMaxHealth(int32(healthPct))));
    if (manaUser && creature->GetPowerType() == POWER_MANA)
        creature->SetPower(POWER_MANA, int32(float(creature->GetMaxPower(POWER_MANA)) * manaPct));
}

void AutoBalance::OnPlayerEnterInstance(Map* map, Player* player)
{
    if (!IsActive() || !map || !map->IsDungeon() || !map->GetInstanceId())
        return;

    // AB ABAllMapScript.cpp:11-115
    OnInstancePlayersChanged(map, player, nullptr, true, true);
}

void AutoBalance::OnPlayerLeaveInstance(Map* map, Player* player)
{
    if (!IsActive() || !map || !map->IsDungeon() || !map->GetInstanceId())
        return;

    // AB ABAllMapScript.cpp:117-227
    OnInstancePlayersChanged(map, player, player, true, false);
}

void AutoBalance::OnInstanceDestroyed(uint32 instanceId)
{
    if (!instanceId)
        return;

    std::unique_lock<std::shared_mutex> guard(g_instancesLock);
    g_instances.erase(instanceId);
    g_hasInstances.store(!g_instances.empty(), std::memory_order_relaxed);
}

void AutoBalance::OnPlayerBeforeLootMoney(Player* player, Loot* loot, ObjectGuid const& source)
{
    // AB ABPlayerScript.cpp:76-123
    if (!IsActive() || !player || !loot || !loot->gold)
        return;
    Map* map = player->GetMap();
    if (!map || !map->IsDungeon() || !map->GetInstanceId())
        return;

    std::shared_ptr<Settings const> settings = GetSettings();
    std::shared_ptr<InstanceState> state = FindInstanceState(map->GetInstanceId());
    if (!state || !settings->rewardMoney)
        return;

    std::lock_guard<std::mutex> guard(state->lock);
    if (!state->enabled || !state->maxPlayers)
        return;

    uint32 const before = loot->gold;
    float const share = float(state->playerCount) / state->maxPlayers;
    if (source.IsCreatureOrVehicle())
    {
        float modifier = 1.0f;
        auto const itr = state->creatures.find(source);
        if (itr != state->creatures.end())
            modifier = itr->second.moneyModifier;
        if (settings->rewardMethod == ScalingMethod::Dynamic)
            loot->gold = uint32(loot->gold * modifier);
        else
            loot->gold = uint32(loot->gold * modifier * share);
    }
    else
        loot->gold = uint32(loot->gold * share);                       // a chest: the share of the instance size

    ++state->moneyEvents;
    state->moneyBefore += before;
    state->moneyAfter += loot->gold;
}

std::string AutoBalance::DescribeMap(Map* map)
{
    char buffer[768];
    if (!map)
        return "ab map=none";

    std::shared_ptr<InstanceState> state = map->GetInstanceId() ? FindInstanceState(map->GetInstanceId()) : nullptr;
    if (!state)
    {
        std::snprintf(buffer, sizeof(buffer), "ab map=%u iid=%u active=%u state=none ab=%u", map->GetId(), map->GetInstanceId(),
            uint32(IsLevelScalingActive()), uint32(IsActive()));
        return buffer;
    }

    int32 const offset = GetSettings()->playerCountDifficultyOffset;
    std::lock_guard<std::mutex> guard(state->lock);
    std::snprintf(buffer, sizeof(buffer),
        "ab map=%u iid=%u active=%u enabled=%u lfg=%u-%u creatureHigh=%u creatureAvg=%.1f players=%u stamp=%u skip=%d/%d ceiling=%d floor=%d tracked=%u"
        " ab=%u gen=%u pc=%u apc=%u minp=%u offset=%d lowest=%u world=%.4f/%.4f->%.4f"
        " xp=%u:%llu->%llu xplast=%u:%u->%u@%.4f money=%u:%llu->%llu heal=%u:%llu->%llu cc=%u:%d->%d",
        map->GetId(), map->GetInstanceId(), uint32(IsLevelScalingActive()), uint32(state->enabled), uint32(state->lfgMinLevel), uint32(state->lfgMaxLevel),
        uint32(state->highestCreatureLevel), state->avgCreatureLevel, uint32(state->highestPlayerLevel), state->stamp,
        state->skipHigher, state->skipLower, state->ceiling, state->floor, uint32(state->creatures.size()),
        uint32(IsActive()), state->generation, state->playerCount, state->adjustedPlayerCount, state->minPlayers, offset,
        uint32(state->lowestPlayerLevel), state->worldHealthMultiplier, state->worldDamageHealingMultiplier, state->scaledWorldDamageHealingMultiplier,
        state->xpEvents, (unsigned long long)state->xpBefore, (unsigned long long)state->xpAfter,
        state->xpLastEntry, state->xpLastBefore, state->xpLastAfter, state->xpLastModifier,
        state->moneyEvents, (unsigned long long)state->moneyBefore, (unsigned long long)state->moneyAfter,
        state->healEvents, (unsigned long long)state->healBefore, (unsigned long long)state->healAfter,
        state->ccEvents, state->ccLastBefore, state->ccLastAfter);
    return buffer;
}

std::string AutoBalance::DescribeCreature(Creature* creature)
{
    char buffer[512];
    if (!creature)
        return "ab creature=none";

    CreatureState cs;
    cs.unmodifiedLevel = creature->GetLevel();
    uint32 tracked = 0;
    Map* map = creature->GetMap();
    if (map && map->GetInstanceId())
        if (std::shared_ptr<InstanceState> state = FindInstanceState(map->GetInstanceId()))
        {
            std::lock_guard<std::mutex> guard(state->lock);
            auto const itr = state->creatures.find(creature->GetGUID());
            if (itr != state->creatures.end())
            {
                cs = itr->second;
                tracked = 1;
            }
        }

    std::snprintf(buffer, sizeof(buffer), "ab entry=%u guid=%u level=%u orig=%u maxhp=%u alive=%u tracked=%u spellmult=%.4f meleemult=%.4f"
        " boss=%u pcount=%u hpmult=%.4f->%.4f manamult=%.4f armormult=%.4f dmgmult=%.4f ccmult=%.4f xpmult=%.4f moneymult=%.4f",
        creature->GetEntry(), creature->GetGUID().GetCounter(), uint32(creature->GetLevel()), uint32(cs.unmodifiedLevel), creature->GetMaxHealth(),
        uint32(creature->IsAlive()), tracked, cs.spellDamageMultiplier, cs.meleeDamageMultiplier,
        uint32(cs.boss), cs.instancePlayerCount, cs.healthMultiplier, cs.scaledHealthMultiplier, cs.manaMultiplier, cs.armorMultiplier,
        cs.damageMultiplier, cs.ccDurationMultiplier, cs.xpModifier, cs.moneyModifier);
    return buffer;
}
