/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
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

#include "PlayerbotAIConfig.h"
#include <sstream>

#include <iostream>

#include "Config.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"  // #201 (#612): sMapStore for the duel-spot map check
#include "Helper.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "Factory/RandomPlayerbotFactory.h"
#include "World.h"  // #201 (#612): CONFIG_MAX_PLAYER_LEVEL for the duel-spot level clamp

template <class T>
void LoadList(std::string const value, T& list)
{
    std::vector<std::string> ids = split(value, ',');
    for (std::vector<std::string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        uint32 id = atoi((*i).c_str());
        // if (!id)
        //     continue;
        list.push_back(id);
    }
}

template <class T>
void LoadSet(std::string const value, T& set)
{
    std::vector<std::string> ids = split(value, ',');
    for (std::vector<std::string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        uint32 id = atoi((*i).c_str());
        // if (!id)
        //     continue;
        set.insert(id);
    }
}

template <class T>
void LoadListString(std::string const value, T& list)
{
    std::vector<std::string> strings = split(value, ',');
    for (std::vector<std::string>::iterator i = strings.begin(); i != strings.end(); i++)
    {
        std::string const string = *i;
        if (string.empty())
            continue;

        list.push_back(string);
    }
}

bool PlayerbotAIConfig::Initialize()
{
    TC_LOG_INFO("server.loading", "Initializing AI Playerbots");
    enabled = sConfigMgr->GetBoolDefault("AiPlayerbot.Enabled", true);
    if (!enabled)
    {
        TC_LOG_INFO("server.loading", "Playerbots Module is disabled in playerbots.conf");
        return false;
    }

    perfMonEnabled = sConfigMgr->GetBoolDefault("AiPlayerbot.PerfMonEnabled", false);
    globalCoolDown = sConfigMgr->GetIntDefault("AiPlayerbot.GlobalCooldown", 1500);
    maxWaitForMove = sConfigMgr->GetIntDefault("AiPlayerbot.MaxWaitForMove", 5000);
    disableMoveSplinePath = sConfigMgr->GetIntDefault("AiPlayerbot.DisableMoveSplinePath", 0);
    maxMovementSearchTime = sConfigMgr->GetIntDefault("AiPlayerbot.MaxMovementSearchTime", 3);
    expireActionTime = sConfigMgr->GetIntDefault("AiPlayerbot.ExpireActionTime", 5000);
    dispelAuraDuration = sConfigMgr->GetIntDefault("AiPlayerbot.DispelAuraDuration", 7000);
    reactDelay = sConfigMgr->GetIntDefault("AiPlayerbot.ReactDelay", 100);
    dynamicReactDelay = sConfigMgr->GetBoolDefault("AiPlayerbot.DynamicReactDelay", true);
    passiveDelay = sConfigMgr->GetIntDefault("AiPlayerbot.PassiveDelay", 10000);
    repeatDelay = sConfigMgr->GetIntDefault("AiPlayerbot.RepeatDelay", 2000);
    errorDelay = sConfigMgr->GetIntDefault("AiPlayerbot.ErrorDelay", 5000);
    rpgDelay = sConfigMgr->GetIntDefault("AiPlayerbot.RpgDelay", 10000);
    sitDelay = sConfigMgr->GetIntDefault("AiPlayerbot.SitDelay", 30000);
    returnDelay = sConfigMgr->GetIntDefault("AiPlayerbot.ReturnDelay", 7000);
    lootDelay = sConfigMgr->GetIntDefault("AiPlayerbot.LootDelay", 1000);

    farDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.FarDistance", 20.0f);
    // issue #86: was declared but never read, so it stayed 0 and TravelNodeMap::getRoute's
    // "can the bot walk to the start node" check (cropPathTo/isPathTo with this as the max
    // distance) always failed - every travel-graph route was found by the A* and then discarded.
    targetPosRecalcDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.TargetPosRecalcDistance", 100.0f);
    sightDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.SightDistance", 100.0f);
    spellDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.SpellDistance", 25.0f);
    shootDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.ShootDistance", 25.0f);
    healDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.HealDistance", 25.0f);
    lootDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.LootDistance", 15.0f);
    fleeDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.FleeDistance", 7.5f);
    aggroDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.AggroDistance", 22.0f);
    tooCloseDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.TooCloseDistance", 5.0f);
    meleeDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.MeleeDistance", 0.75f);
    followDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.FollowDistance", 1.5f);
    whisperDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.WhisperDistance", 6000.0f);
    contactDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.ContactDistance", 0.5f);
    aoeRadius = sConfigMgr->GetFloatDefault("AiPlayerbot.AoeRadius", 5.0f);
    rpgDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.RpgDistance", 200.0f);
    grindDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.GrindDistance", 75.0f);
    reactDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.ReactDistance", 150.0f);

    criticalHealth = sConfigMgr->GetIntDefault("AiPlayerbot.CriticalHealth", 20);
    lowHealth = sConfigMgr->GetIntDefault("AiPlayerbot.LowHealth", 45);
    mediumHealth = sConfigMgr->GetIntDefault("AiPlayerbot.MediumHealth", 65);
    almostFullHealth = sConfigMgr->GetIntDefault("AiPlayerbot.AlmostFullHealth", 85);
    lowMana = sConfigMgr->GetIntDefault("AiPlayerbot.LowMana", 15);
    mediumMana = sConfigMgr->GetIntDefault("AiPlayerbot.MediumMana", 40);
    highMana = sConfigMgr->GetIntDefault("AiPlayerbot.HighMana", 65);
    autoSaveMana = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoSaveMana", true);
    saveManaThreshold = sConfigMgr->GetIntDefault("AiPlayerbot.SaveManaThreshold", 60);
    randomBotSayWithoutMaster = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotSayWithoutMaster", false);
    autoAvoidAoe = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoAvoidAoe", true);
    maxAoeAvoidRadius = sConfigMgr->GetFloatDefault("AiPlayerbot.MaxAoeAvoidRadius", 15.0f);
    LoadSet<std::set<uint32>>(sConfigMgr->GetStringDefault("AiPlayerbot.AoeAvoidSpellWhitelist", "50759,57491,13810"),
        aoeAvoidSpellWhitelist);
    tellWhenAvoidAoe = sConfigMgr->GetBoolDefault("AiPlayerbot.TellWhenAvoidAoe", false);

    iterationsPerTick = sConfigMgr->GetIntDefault("AiPlayerbot.IterationsPerTick", 100);

    LoadList<std::vector<uint32>>(
        sConfigMgr->GetStringDefault("AiPlayerbot.PvpProhibitedZoneIds",
            "2255,656,2361,2362,2363,976,35,2268,3425,392,541,1446,3828,3712,3738,3565,"
            "3539,3623,4152,3988,4658,4284,4418,4436,4275,4323,4395"),
        pvpProhibitedZoneIds);
    LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault("AiPlayerbot.PvpProhibitedAreaIds", "976,35"),
        pvpProhibitedAreaIds);

    // AC PlayerbotAIConfig.cpp:192/418-419. AC's default is its four continents; this fork adds the
    // Cataclysm/Mists continents (646 Deepholm, 648 Lost Isles, 654 Gilneas, 860 Wandering Isle,
    // 870 Pandaria, 1064 Isle of Thunder) so MoP-only zones get grind cells.
    randomBotMaps.clear();
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotMaps", "0,1,530,571,646,648,654,860,870,1064"),
        randomBotMaps);
    randomBotTeleLowerLevel = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotTeleLowerLevel", 1);
    randomBotTeleHigherLevel = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotTeleHigherLevel", 3);

    randomBotAutologin = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotAutologin", true);
    randomBotAccountPrefix = sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotAccountPrefix", "rndbot");
    randomBotAccountCount = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAccountCount", 0);
    randomBotAccountSlackPct = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAccountSlackPct", 0);
    maxAddedBotsPerClass = sConfigMgr->GetIntDefault("AiPlayerbot.MaxAddedBotsPerClass", 50);
    maxAddedBots = sConfigMgr->GetIntDefault("AiPlayerbot.MaxAddedBots", 200);
    minRandomBots = sConfigMgr->GetIntDefault("AiPlayerbot.MinRandomBots", 50);
    maxRandomBots = sConfigMgr->GetIntDefault("AiPlayerbot.MaxRandomBots", 200);
    // issue #136: AC defaults (PlayerbotAIConfig.cpp:628/630/698 on the reference).
    disableRandomLevels = sConfigMgr->GetBoolDefault("AiPlayerbot.DisableRandomLevels", false);
    randombotStartingLevel = sConfigMgr->GetIntDefault("AiPlayerbot.RandombotStartingLevel", 1);
    downgradeMaxLevelBot = sConfigMgr->GetBoolDefault("AiPlayerbot.DowngradeMaxLevelBot", true);
    randomBotMaxLevel = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotMaxLevel", 90);
    randomBotMinLevel = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotMinLevel", 1);
    randomBotMaxLevelChance = sConfigMgr->GetFloatDefault("AiPlayerbot.RandomBotMaxLevelChance", 0.25f);
    randomBotsPerInterval = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotsPerInterval", 60);
    // #772 (AC PlayerbotAIConfig.cpp:265-266,677-679,700-702): same names and defaults. YEAR = 31104000 s, the value
    // AddRandomBots hard-coded before.
    permanentlyInWorldTime = sConfigMgr->GetIntDefault("AiPlayerbot.PermanentlyInWorldTime", 1 * YEAR);
    randomBotAllianceRatio = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAllianceRatio", 50);
    randomBotHordeRatio = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotHordeRatio", 50);
    disableDeathKnightLogin = sConfigMgr->GetBoolDefault("AiPlayerbot.DisableDeathKnightLogin", false);
    enablePeriodicOnlineOffline = sConfigMgr->GetBoolDefault("AiPlayerbot.EnablePeriodicOnlineOffline", false);
    periodicOnlineOfflineRatio = sConfigMgr->GetFloatDefault("AiPlayerbot.PeriodicOnlineOfflineRatio", 2.0f);
    // The ratio divides the account count in AddRandomBots (AC documents "must be greater than 1.0" but does not
    // enforce it); anything below 1 would divide by zero or log in more accounts than exist.
    if (periodicOnlineOfflineRatio < 1.0f)
        periodicOnlineOfflineRatio = 1.0f;
    randomBotUpdateInterval = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotUpdateInterval", 20);
    randomBotCountChangeMinInterval =
        sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotCountChangeMinInterval", 30 * MINUTE);
    randomBotCountChangeMaxInterval =
        sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotCountChangeMaxInterval", 2 * HOUR);
    minRandomBotInWorldTime = sConfigMgr->GetIntDefault("AiPlayerbot.MinRandomBotInWorldTime", 2 * HOUR);
    maxRandomBotInWorldTime = sConfigMgr->GetIntDefault("AiPlayerbot.MaxRandomBotInWorldTime", 14 * 24 * HOUR);
    minRandomBotRandomizeTime = sConfigMgr->GetIntDefault("AiPlayerbot.MinRandomBotRandomizeTime", 2 * HOUR);
    maxRandomBotRandomizeTime = sConfigMgr->GetIntDefault("AiPlayerbot.MaxRandomBotRandomizeTime", 14 * 24 * HOUR);
    minRandomBotTeleportInterval = sConfigMgr->GetIntDefault("AiPlayerbot.MinRandomBotTeleportInterval", 1 * HOUR);
    maxRandomBotTeleportInterval = sConfigMgr->GetIntDefault("AiPlayerbot.MaxRandomBotTeleportInterval", 5 * HOUR);

    randomBotLoginAtStartup = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotLoginAtStartup", true);
    openGoSpell = sConfigMgr->GetIntDefault("AiPlayerbot.OpenGoSpell", 6477);

    // #134 mount/riding (AC parity): same keys/defaults as AC PlayerbotAIConfig.cpp:471-474; already
    // documented in playerbots.conf.dist but never loaded, so random bots never actually got riding.
    useGroundMountAtMinLevel = sConfigMgr->GetIntDefault("AiPlayerbot.UseGroundMountAtMinLevel", 20);
    useFastGroundMountAtMinLevel = sConfigMgr->GetIntDefault("AiPlayerbot.UseFastGroundMountAtMinLevel", 40);
    useFlyMountAtMinLevel = sConfigMgr->GetIntDefault("AiPlayerbot.UseFlyMountAtMinLevel", 60);
    useFastFlyMountAtMinLevel = sConfigMgr->GetIntDefault("AiPlayerbot.UseFastFlyMountAtMinLevel", 70);
    randomBotSpellIds.clear();
    LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotSpellIds", "54197"),
                                  randomBotSpellIds);

    randomChangeMultiplier = sConfigMgr->GetFloatDefault("AiPlayerbot.RandomChangeMultiplier", 1.0);

    randomBotCombatStrategies = sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotCombatStrategies", "");
    randomBotNonCombatStrategies = sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotNonCombatStrategies", "");
    combatStrategies = sConfigMgr->GetStringDefault("AiPlayerbot.CombatStrategies", "");
    nonCombatStrategies = sConfigMgr->GetStringDefault("AiPlayerbot.NonCombatStrategies", "");

    // "new rpg" runtime roaming strategy
    // AC PlayerbotAIConfig.cpp:727 (issue #610)
    autoDoQuests = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoDoQuests", true);
    enableNewRpgStrategy = sConfigMgr->GetBoolDefault("AiPlayerbot.EnableNewRpgStrategy", true);
    randomBotRpgChance = sConfigMgr->GetFloatDefault("AiPlayerbot.RandomBotRpgChance", 0.20f);
    RpgStatusProbWeight[RPG_WANDER_RANDOM] = sConfigMgr->GetIntDefault("AiPlayerbot.RpgStatusProbWeight.WanderRandom", 15);
    RpgStatusProbWeight[RPG_WANDER_NPC] = sConfigMgr->GetIntDefault("AiPlayerbot.RpgStatusProbWeight.WanderNpc", 20);
    RpgStatusProbWeight[RPG_GO_GRIND] = sConfigMgr->GetIntDefault("AiPlayerbot.RpgStatusProbWeight.GoGrind", 15);
    RpgStatusProbWeight[RPG_GO_CAMP] = sConfigMgr->GetIntDefault("AiPlayerbot.RpgStatusProbWeight.GoCamp", 10);
    RpgStatusProbWeight[RPG_DO_QUEST] = sConfigMgr->GetIntDefault("AiPlayerbot.RpgStatusProbWeight.DoQuest", 60);
    RpgStatusProbWeight[RPG_TRAVEL_FLIGHT] = sConfigMgr->GetIntDefault("AiPlayerbot.RpgStatusProbWeight.TravelFlight", 15);
    RpgStatusProbWeight[RPG_REST] = sConfigMgr->GetIntDefault("AiPlayerbot.RpgStatusProbWeight.Rest", 5);
    RpgStatusProbWeight[RPG_OUTDOOR_PVP] = sConfigMgr->GetIntDefault("AiPlayerbot.RpgStatusProbWeight.OutdoorPvp", 10);
    // #843 (MoP/Cata extension, owner 2026-10-16, BEYOND AC; AC has no such key). Default on, in code and conf.dist.
    rpgQuestHubSearchEnable = sConfigMgr->GetBoolDefault("AiPlayerbot.RpgQuestHubSearchEnable", true);
    rpgQuestHubSearchRadius = std::max(0.0f, sConfigMgr->GetFloatDefault("AiPlayerbot.RpgQuestHubSearchRadius", 2500.0f));

    // #201 (#612 #614 #615 #618): duel spots (AC has no equivalent; follows AC's RpgStatusProbWeight pattern above,
    // AC PlayerbotAIConfig.cpp:737). Weight is 0 in code (off); the MoP conf.dist sets 10. SPEC section 1.
    RpgStatusProbWeight[RPG_DUEL_SPOT] = sConfigMgr->GetIntDefault("AiPlayerbot.RpgStatusProbWeight.DuelSpot", 0);
    duelSpotEnable = sConfigMgr->GetBoolDefault("AiPlayerbot.DuelSpotEnable", false);
    duelSpotSelectDistance = sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotSelectDistance", 1500);
    duelSpotMinLevel = sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotMinLevel", 5);
    duelSpotMaxDuels = sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotMaxDuels", 6);
    duelSpotRematchDelayMin = sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotRematchDelayMin", 25);
    duelSpotRematchDelayMax = std::max<uint32>(duelSpotRematchDelayMin,
        sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotRematchDelayMax", 35));
    duelSpotScanIntervalMin = sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotScanIntervalMin", 10000);
    duelSpotScanIntervalMax = std::max<uint32>(duelSpotScanIntervalMin,
        sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotScanIntervalMax", 20000));
    duelSpotMaxLevelDiff = sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotMaxLevelDiff", 5);
    duelSpotMaxLevelPairsOnly = sConfigMgr->GetBoolDefault("AiPlayerbot.DuelSpotMaxLevelPairsOnly", true);
    duelSpotSameTeamOnly = sConfigMgr->GetBoolDefault("AiPlayerbot.DuelSpotSameTeamOnly", true);
    duelSpotChallengeRealPlayers = sConfigMgr->GetBoolDefault("AiPlayerbot.DuelSpotChallengeRealPlayers", false);
    duelSpotPlayerChallengeChance = sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotPlayerChallengeChance", 25);
    duelSpotPlayerCooldown = sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotPlayerCooldown", 900);
    duelSpotReset = sConfigMgr->GetBoolDefault("AiPlayerbot.DuelSpotReset", false);
    duelSpotSeedEnable = sConfigMgr->GetBoolDefault("AiPlayerbot.DuelSpotSeedEnable", false);
    duelSpotSeedInterval = std::max<uint32>(100, sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotSeedInterval", 5000));
    duelSpotSeedPerTick = sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotSeedPerTick", 2);
    duelSpotSeedMaxShare = sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotSeedMaxShare", 25);
    duelSpotSeedRelevel = sConfigMgr->GetBoolDefault("AiPlayerbot.DuelSpotSeedRelevel", true);
    duelSpotSeedRegear = sConfigMgr->GetBoolDefault("AiPlayerbot.DuelSpotSeedRegear", true);
    duelSpotActiveDistance = sConfigMgr->GetIntDefault("AiPlayerbot.DuelSpotActiveDistance", 1500);

    duelSpots.clear();
    {
        // Code defaults for the shipped spots (same values as conf.dist). Owner 2026-10-20: city spots are max level only
        // (min/max 0 = server max) with 12 bots = 6 pairs; OrgrimmarGateMax folded into OrgrimmarGate.
        struct DuelSpotDefault
        {
            char const* name;
            uint32 mapId;
            float x, y, z, radius;
            uint8 team;
            uint32 minLevel, maxLevel, bots;
            bool alwaysActive, allowPvpProhibitedZone;
        };
        static DuelSpotDefault const defaults[] =
        {
            {"StormwindGate", 0, -9082.0f, 341.0f, 93.0f, 25.0f, 1, 0, 0, 12, true, false},
            {"OrgrimmarGate", 1, 1318.33f, -4386.95f, 26.14f, 25.0f, 2, 0, 0, 12, true, false},
            {"OrgrimmarGateMax", 1, 1318.33f, -4386.95f, 26.14f, 25.0f, 2, 0, 0, 15, true, false},   // owner 2026-10-20: folded into OrgrimmarGate (not in the default list)
            {"RazorHill", 1, 310.0f, -4736.0f, 9.4f, 15.0f, 2, 5, 9, 10, true, false},
            {"SenjinVillage", 1, -816.95f, -4896.27f, 19.17f, 10.0f, 2, 5, 7, 5, true, false},
            {"Goldshire", 0, -9448.5f, 68.2f, 56.3f, 20.0f, 1, 5, 12, 10, true, false},
            {"IronforgeGate", 0, -5040.5f, -814.8f, 495.1f, 20.0f, 1, 0, 0, 12, false, false},
            {"ShrineSevenStars", 870, 886.29f, 299.78f, 503.12f, 15.0f, 1, 0, 0, 12, false, false},
            {"ShrineTwoMoons", 870, 1598.28f, 920.59f, 470.63f, 15.0f, 2, 0, 0, 12, false, false},
            {"DalaranCircleOfWills", 571, 5803.4f, 642.0f, 610.1f, 15.0f, 0, 0, 0, 12, false, true},
            {"Crossroads", 1, -452.8f, -2650.8f, 95.5f, 20.0f, 2, 10, 25, 10, false, false},
            {"UndercityGate", 0, 1923.7f, 234.2f, 49.1f, 20.0f, 2, 0, 0, 12, false, false},
            {"ThunderBluffLifts", 1, -881.9f, -156.2f, -0.8f, 20.0f, 2, 0, 0, 12, false, false},
            {"SilvermoonGate", 530, 9390.0f, -7278.0f, 14.3f, 20.0f, 2, 0, 0, 12, false, false},
            {"ExodarGate", 530, -4080.0f, -12045.0f, 0.0f, 20.0f, 1, 0, 0, 12, false, false},
            {"BootyBay", 0, -14297.2f, 531.0f, 8.8f, 15.0f, 0, 30, 0, 6, false, false},
            {"GurubashiArena", 0, -13277.4f, 127.4f, 26.1f, 20.0f, 0, 30, 0, 6, false, false},
            {"Halaa", 530, -1563.1f, 7945.7f, -22.6f, 20.0f, 0, 60, 0, 6, false, false},
            {"Halfhill", 870, -202.6f, 482.4f, 187.2f, 20.0f, 0, 85, 90, 10, false, false},
        };

        uint32 const serverMax = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
        std::string const spotList = sConfigMgr->GetStringDefault("AiPlayerbot.DuelSpots",
            "StormwindGate,OrgrimmarGate,RazorHill,SenjinVillage,Goldshire,IronforgeGate,ShrineSevenStars,ShrineTwoMoons,DalaranCircleOfWills,Crossroads,UndercityGate,ThunderBluffLifts,SilvermoonGate,ExodarGate,BootyBay,GurubashiArena,Halaa,Halfhill");
        std::stringstream spotStream(spotList);
        std::string spotName;
        while (std::getline(spotStream, spotName, ','))
        {
            size_t const begin = spotName.find_first_not_of(" \t");
            size_t const end = spotName.find_last_not_of(" \t");
            if (begin == std::string::npos)
                continue;
            spotName = spotName.substr(begin, end - begin + 1);

            bool duplicate = false;
            for (DuelSpotInfo const& known : duelSpots)
                duplicate = duplicate || known.name == spotName;
            if (duplicate)
                continue;

            static DuelSpotDefault const none{"", 0, 0.0f, 0.0f, 0.0f, 20.0f, 0, 5, 0, 0, false, false};  // radius 20, MinLevel 5 as AC
            DuelSpotDefault const* def = &none;
            for (DuelSpotDefault const& d : defaults)
                if (spotName == d.name)
                    def = &d;

            std::string const prefix = "AiPlayerbot.DuelSpot." + spotName + ".";
            if (!sConfigMgr->GetBoolDefault(prefix + "Enable", true))
                continue;

            DuelSpotInfo spot;
            spot.name = spotName;
            spot.mapId = sConfigMgr->GetIntDefault(prefix + "Map", def->mapId);
            spot.x = sConfigMgr->GetFloatDefault(prefix + "X", def->x);
            spot.y = sConfigMgr->GetFloatDefault(prefix + "Y", def->y);
            spot.z = sConfigMgr->GetFloatDefault(prefix + "Z", def->z);
            spot.radius = sConfigMgr->GetFloatDefault(prefix + "Radius", def->radius);
            spot.team = uint8(std::min<uint32>(2, sConfigMgr->GetIntDefault(prefix + "Team", def->team)));
            spot.minLevel = sConfigMgr->GetIntDefault(prefix + "MinLevel", def->minLevel);
            uint32 const rawMax = sConfigMgr->GetIntDefault(prefix + "MaxLevel", def->maxLevel);
            spot.bots = sConfigMgr->GetIntDefault(prefix + "Bots", def->bots);
            spot.alwaysActive = sConfigMgr->GetBoolDefault(prefix + "AlwaysActive", def->alwaysActive);
            spot.allowPvpProhibitedZone =
                sConfigMgr->GetBoolDefault(prefix + "AllowPvpProhibitedZone", def->allowPvpProhibitedZone);

            // MaxLevel 0 = server max; clamp to the server max. MinLevel 0 together with MaxLevel 0 = "server max only"
            // (levels 90-90).
            spot.maxLevel = (rawMax == 0 || rawMax > serverMax) ? serverMax : rawMax;
            if (spot.minLevel == 0)  // #201 (SPEC amendment A9): same as AC - MinLevel 0 = server max, whatever MaxLevel is
                spot.minLevel = serverMax;
            if (spot.radius <= 0.0f)
                continue;

            duelSpots.push_back(spot);
        }
    }
    ValidateDuelSpotMaps();
    TC_LOG_INFO("playerbots", "[DUELSPOT] config: enable=%u spots=%u", uint32(duelSpotEnable), uint32(duelSpots.size()));

    // AiPlayerbot.DebugBotNames = "Arkond,Tariney" - comma-separated watch list for the
    // periodic [RPGSTAT] roaming/AI state dump (issue #61).
    debugBotNames.clear();
    {
        std::string const names = sConfigMgr->GetStringDefault("AiPlayerbot.DebugBotNames", "");
        std::stringstream stream(names);
        std::string name;
        while (std::getline(stream, name, ','))
        {
            size_t const begin = name.find_first_not_of(" \t");
            size_t const end = name.find_last_not_of(" \t");
            if (begin == std::string::npos)
                continue;
            debugBotNames.insert(name.substr(begin, end - begin + 1));
        }
    }

    randomBotJoinBG = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotJoinBG", false);
    bgCheckInterval = sConfigMgr->GetIntDefault("AiPlayerbot.BgCheckInterval", 30);
    randomBotAutoJoinBG = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotAutoJoinBG", false);
    // #304: code defaults = AC's (PlayerbotAIConfig.cpp:389-392: WS "7", AB "6", EY "2", the level-80 brackets)
    // translated to 5.4.8 per-map ids (80-84: WS 14, AB 14, EY 9), the same values playerbots.conf.dist ships.
    // TP (MoP-only) has no 80-84 bracket and stays closed unless configured.
    LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotAutoJoinWSBrackets", "14"),
                                  randomBotAutoJoinWSBrackets);
    LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotAutoJoinTPBrackets", ""),
                                  randomBotAutoJoinTPBrackets);
    LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotAutoJoinABBrackets", "14"),
                                  randomBotAutoJoinABBrackets);
    LoadList<std::vector<uint32>>(sConfigMgr->GetStringDefault("AiPlayerbot.RandomBotAutoJoinEYBrackets", "9"),
                                  randomBotAutoJoinEYBrackets);
    // #304: AC's key names are RandomBotAutoJoinBG<bg>Count (AC PlayerbotAIConfig.cpp:395-398, default 1); this
    // fork read RandomBotAutoJoin<bg>Count, so the AC keys in playerbots.conf.dist were ignored. The AC key wins;
    // the old spelling stays as its fallback so existing confs keep working. TP (MoP-only) takes AC's spelling.
    randomBotAutoJoinWSCount = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGWSCount",
                                                         sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinWSCount", 1));
    randomBotAutoJoinTPCount = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGTPCount",
                                                         sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinTPCount", 1));
    randomBotAutoJoinABCount = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGABCount",
                                                         sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinABCount", 1));
    randomBotAutoJoinEYCount = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGEYCount",
                                                         sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinEYCount", 1));
    fastReactInBG = sConfigMgr->GetBoolDefault("AiPlayerbot.FastReactInBG", true);

    enableTravelNodes = sConfigMgr->GetBoolDefault("AiPlayerbot.EnableTravelNodes", false);
    travelNodeRouteDistance = sConfigMgr->GetFloatDefault("AiPlayerbot.TravelNodeRouteDistance", 500.0f);


    // Before CreateRandomBots, which sizes and assigns the addclass accounts
    addClassCommand = sConfigMgr->GetIntDefault("AiPlayerbot.AddClassCommand", 1);
    addClassAccountPoolSize = sConfigMgr->GetIntDefault("AiPlayerbot.AddClassAccountPoolSize", 50);
    autoInitEquipLevelLimitRatio = sConfigMgr->GetFloatDefault("AiPlayerbot.AutoInitEquipLevelLimitRatio", 1.0f);

    RandomPlayerbotFactory::CreateRandomBots();
    if (World::IsStopped())
    {
        return true;
    }

    // Assign account types after the accounts and characters are created (AC PlayerbotAIConfig.cpp:913)
    sRandomPlayerbotMgr->AssignAccountTypes();

    // As AzerothCore (RandomPlayerbotMgr::Init)
    if (addClassCommand)
        sRandomPlayerbotMgr->PrepareAddclassCache();
    sRandomPlayerbotMgr->PrepareTeleportCache();
    sRandomPlayerbotMgr->PrepareBattleMastersCache();

    // #746 (AC RandomPlayerbotMgr::Init, Bot/RandomPlayerbotMgr.cpp:1788-1790): every boot starts with no 'add' rows, so
    // GetBots() finds nothing and AddRandomBots() picks a fresh bot_count set. Without it MoP kept 2196 stale 'add'
    // rows (stress ramps + harness rows) and loaded the first 1000 by id. DirectExecute instead of AC's async Execute:
    // the first RandomPlayerbotMgr tick reads the table, so the delete must be finished before it.
    if (enabled)
        PlayerbotsDatabase.DirectExecute("DELETE FROM playerbots_random_bots WHERE `event` = 'add'");

    TC_LOG_INFO("server.loading", "---------------------------------------");
    TC_LOG_INFO("server.loading", "          Loading TalentSpecs          ");
    TC_LOG_INFO("server.loading", "---------------------------------------");
    // #734: the 5.4.8 counterpart of AC's PlayerbotAIConfig premade spec loader (AC PlayerbotAIConfig.cpp:486-515,
    // AiPlayerbot.PremadeSpecName/Link/Glyph.<class>.<specno>). The old loader read "PremadeSpecLink.<class>.<spec>"
    // as a comma list of Talent ids while playerbots.conf.dist only carried AC's WotLK keys
    // "PremadeSpecLink.<class>.<spec>.<level>", so every link loaded empty ("Premade spec loaded: prot pve - ") and
    // BotFactory::InitTalentsTree returned before learning anything (0 character_talent rows for every bot).
    // <specno> is the ChrSpecialization TabPage (the CMSG_SET_PRIMARY_TALENT_TREE tab). The link is one character
    // per talent tier (levels 15/30/45/60/75/90): '0'-'2' = Talent.dbc ColumnIndex, any other character = no
    // preference (InitTalentsTree then picks at random, as AC's InitTalents spends every free point). The loop bound
    // is MAX_SPECIALIZATIONS - 1, the arrays' size (it was MAX_SPECIALIZATIONS, one past the end).
    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        for (uint32 spec = 0; spec < MAX_SPECIALIZATIONS - 1; ++spec)
        {
            premadeSpecName[cls][spec].clear();
            premadeSpecLink[cls][spec].clear();
            premadeSpecGlyph[cls][spec].clear();

            std::ostringstream os;
            os << "AiPlayerbot.PremadeSpecName." << cls << "." << spec;
            std::string const loadSpec = sConfigMgr->GetStringDefault(os.str().c_str(), "", true);
            if (loadSpec.empty())
                continue;

            premadeSpecName[cls][spec] = loadSpec;

            os.str("");
            os.clear();
            os << "AiPlayerbot.PremadeSpecLink." << cls << "." << spec;
            std::string const link = sConfigMgr->GetStringDefault(os.str().c_str(), "", true);
            for (char c : link)
            {
                if (premadeSpecLink[cls][spec].size() >= 6)
                    break;
                premadeSpecLink[cls][spec].push_back((c >= '0' && c <= '2') ? uint16(c - '0') : PREMADE_SPEC_NO_CHOICE);
            }

            os.str("");
            os.clear();
            os << "AiPlayerbot.PremadeSpecGlyph." << cls << "." << spec;
            std::string const glyphs = sConfigMgr->GetStringDefault(os.str().c_str(), "", true);
            for (std::string const& id : split(glyphs, ','))
                if (uint32 itemId = uint32(atoi(id.c_str())))
                    premadeSpecGlyph[cls][spec].push_back(itemId);

            TC_LOG_DEBUG("playerbots", "Premade spec loaded: %u.%u %s - link '%s' glyphs '%s'", cls, spec,
                         loadSpec.c_str(), link.c_str(), glyphs.c_str());
        }

        // #739: AC PlayerbotAIConfig.cpp:526-543 - AiPlayerbot.RandomClassSpecProb/Index.<class>.<specno>. AC defaults:
        // 33/33/34 for specno 0/1/2 and 0 for the rest; the Index default is the specno itself.
        for (uint32 spec = 0; spec < MAX_SPECNO; ++spec)
        {
            std::ostringstream os;
            os << "AiPlayerbot.RandomClassSpecProb." << cls << "." << spec;
            uint32 def;
            if (spec <= 1)
                def = 33;
            else if (spec == 2)
                def = 34;
            else
                def = 0;
            randomClassSpecProb[cls][spec] = sConfigMgr->GetIntDefault(os.str().c_str(), def);
            os.str("");
            os.clear();
            os << "AiPlayerbot.RandomClassSpecIndex." << cls << "." << spec;
            randomClassSpecIndex[cls][spec] = sConfigMgr->GetIntDefault(os.str().c_str(), spec);
        }
    }


    // #173 header batch: group-functionality-parity.md section 3.6, AC defaults (sConfigMgr->GetOption<T>
    // on AC -> the Get*Default overloads this fork's Config.h actually has).
    groupInvitationPermission = sConfigMgr->GetIntDefault("AiPlayerbot.GroupInvitationPermission", 1);
    gearscorecheck = sConfigMgr->GetBoolDefault("AiPlayerbot.GearScoreCheck", false);
    allowAccountBots = sConfigMgr->GetBoolDefault("AiPlayerbot.AllowAccountBots", true);
    allowGuildBots = sConfigMgr->GetBoolDefault("AiPlayerbot.AllowGuildBots", true);
    allowTrustedAccountBots = sConfigMgr->GetBoolDefault("AiPlayerbot.AllowTrustedAccountBots", true);
    selfBotLevel = sConfigMgr->GetIntDefault("AiPlayerbot.SelfBotLevel", 1);

    autoInitOnly = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoInitOnly", false);

    commandPrefix = sConfigMgr->GetStringDefault("AiPlayerbot.CommandPrefix", "");
    commandSeparator = sConfigMgr->GetStringDefault("AiPlayerbot.CommandSeparator", "\\\\");
    enableAutoTradeOnItemMention = sConfigMgr->GetBoolDefault("AiPlayerbot.EnableAutoTradeOnItemMention", true);

    randomBotGroupNearby = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotGroupNearby", false);
    inviteChat = sConfigMgr->GetBoolDefault("AiPlayerbot.InviteChat", false);
    summonWhenGroup = sConfigMgr->GetBoolDefault("AiPlayerbot.SummonWhenGroup", true);
    keepAltsInGroup = sConfigMgr->GetBoolDefault("AiPlayerbot.KeepAltsInGroup", false);
    disabledWithoutRealPlayer = sConfigMgr->GetBoolDefault("AiPlayerbot.DisabledWithoutRealPlayer", false);
    disabledWithoutRealPlayerLoginDelay = sConfigMgr->GetIntDefault("AiPlayerbot.DisabledWithoutRealPlayerLoginDelay", 30);
    disabledWithoutRealPlayerLogoutDelay = sConfigMgr->GetIntDefault("AiPlayerbot.DisabledWithoutRealPlayerLogoutDelay", 300);
    syncLevelWithPlayers = sConfigMgr->GetBoolDefault("AiPlayerbot.SyncLevelWithPlayers", false);

    allowSummonInCombat = sConfigMgr->GetBoolDefault("AiPlayerbot.AllowSummonInCombat", true);
    allowSummonWhenMasterIsDead = sConfigMgr->GetBoolDefault("AiPlayerbot.AllowSummonWhenMasterIsDead", true);
    allowSummonWhenBotIsDead = sConfigMgr->GetBoolDefault("AiPlayerbot.AllowSummonWhenBotIsDead", true);
    reviveBotWhenSummoned = sConfigMgr->GetIntDefault("AiPlayerbot.ReviveBotWhenSummoned", 1);
    // Beyond AC (#985): off by default
    followTeleport = sConfigMgr->GetBoolDefault("AiPlayerbot.FollowTeleport", false);
    followTeleportDelay = sConfigMgr->GetIntDefault("AiPlayerbot.FollowTeleportDelay", 2000);
    respectCrowdControl = sConfigMgr->GetBoolDefault("AiPlayerbot.RespectCrowdControl", false);
    botRepairWhenSummon = sConfigMgr->GetBoolDefault("AiPlayerbot.BotRepairWhenSummon", true);
    summonAtInnkeepersEnabled = sConfigMgr->GetBoolDefault("AiPlayerbot.SummonAtInnkeepersEnabled", true);
    logInGroupOnly = sConfigMgr->GetBoolDefault("AiPlayerbot.LogInGroupOnly", true);
    logValuesPerTick = sConfigMgr->GetBoolDefault("AiPlayerbot.LogValuesPerTick", false);

    botTaxiDelayMin = sConfigMgr->GetIntDefault("AiPlayerbot.BotTaxiDelayMinMs", 350);
    botTaxiDelayMax = sConfigMgr->GetIntDefault("AiPlayerbot.BotTaxiDelayMaxMs", 5000);
    // #136: AC PlayerbotAIConfig.cpp:480-481 names these keys BotTaxiGapMs / BotTaxiGapJitterMs with defaults 200 / 100
    // (conf/playerbots.conf.dist:683-686); this fork had BotTaxiDelayGapMs / BotTaxiDelayGapJitterMs with 400 / 150.
    botTaxiDelayGapMs = sConfigMgr->GetIntDefault("AiPlayerbot.BotTaxiGapMs", 200);
    botTaxiDelayGapJitterMs = sConfigMgr->GetIntDefault("AiPlayerbot.BotTaxiGapJitterMs", 100);

    lootNeedRollLevel = sConfigMgr->GetIntDefault("AiPlayerbot.LootNeedRollLevel", 1);
    lootGreedRollLevel = sConfigMgr->GetBoolDefault("AiPlayerbot.LootGreedRollLevel", false);
    lootRollRecipe = sConfigMgr->GetBoolDefault("AiPlayerbot.LootRollRecipe", false);
    lootRollDisenchant = sConfigMgr->GetBoolDefault("AiPlayerbot.LootRollDisenchant", false);
    autoEquipUpgradeLoot = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoEquipUpgradeLoot", true);
    autoGearQualityLimit = sConfigMgr->GetIntDefault("AiPlayerbot.AutoGearQualityLimit", 3);
    autoGearScoreLimit = sConfigMgr->GetIntDefault("AiPlayerbot.AutoGearScoreLimit", 0);
    autoGearCommand = sConfigMgr->GetIntDefault("AiPlayerbot.AutoGearCommand", 1);
    autoGearCommandAltBots = sConfigMgr->GetIntDefault("AiPlayerbot.AutoGearCommandAltBots", 1);

    enableRandomBotTrading = sConfigMgr->GetIntDefault("AiPlayerbot.EnableRandomBotTrading", 1);
    tradeActionExcludedPrefixes.clear();
    LoadListString<std::vector<std::string>>(
        sConfigMgr->GetStringDefault("AiPlayerbot.TradeActionExcludedPrefixes", ""), tradeActionExcludedPrefixes);

    syncQuestWithPlayer = sConfigMgr->GetBoolDefault("AiPlayerbot.SyncQuestWithPlayer", true);
    syncQuestForPlayer = sConfigMgr->GetBoolDefault("AiPlayerbot.SyncQuestForPlayer", false);
    dropObsoleteQuests = sConfigMgr->GetBoolDefault("AiPlayerbot.DropObsoleteQuests", true);
    autoPickReward = sConfigMgr->GetStringDefault("AiPlayerbot.AutoPickReward", "yes");

    defaultPetStance = sConfigMgr->GetIntDefault("AiPlayerbot.DefaultPetStance", 1);
    applyInstanceStrategies = sConfigMgr->GetBoolDefault("AiPlayerbot.ApplyInstanceStrategies", true);
    forceRebuffOnReadyCheck = sConfigMgr->GetBoolDefault("AiPlayerbot.ForceRebuffOnReadyCheck", false);
    forceRebuffMarginSecs = std::min<uint32>(sConfigMgr->GetIntDefault("AiPlayerbot.ForceRebuffMarginSecs", 60), 3600u);  // AC PlayerbotAIConfig.cpp:166 (#628)

    botActiveAlone = sConfigMgr->GetIntDefault("AiPlayerbot.BotActiveAlone", 10);
    botActiveAloneDurationSeconds = sConfigMgr->GetIntDefault("AiPlayerbot.BotActiveAloneDurationSeconds", 30);
    botActiveAloneForceWhenInRadius = sConfigMgr->GetIntDefault("AiPlayerbot.BotActiveAloneForceWhenInRadius", 150);
    botActiveAloneForceWhenInZone = sConfigMgr->GetBoolDefault("AiPlayerbot.BotActiveAloneForceWhenInZone", true);
    botActiveAloneForceWhenInMap = sConfigMgr->GetBoolDefault("AiPlayerbot.BotActiveAloneForceWhenInMap", false);
    botActiveAloneForceWhenIsFriend = sConfigMgr->GetBoolDefault("AiPlayerbot.BotActiveAloneForceWhenIsFriend", false);
    botActiveAloneForceWhenInGuild = sConfigMgr->GetBoolDefault("AiPlayerbot.BotActiveAloneForceWhenInGuild", true);
    // #72: AC PlayerbotAIConfig.cpp:688-692
    botActiveAloneSmartScale = sConfigMgr->GetBoolDefault("AiPlayerbot.botActiveAloneSmartScale", true);
    botActiveAloneSmartScaleDiffLimitfloor = sConfigMgr->GetIntDefault("AiPlayerbot.botActiveAloneSmartScaleDiffLimitfloor", 50);
    botActiveAloneSmartScaleDiffLimitCeiling = sConfigMgr->GetIntDefault("AiPlayerbot.botActiveAloneSmartScaleDiffLimitCeiling", 200);
    botActiveAloneSmartScaleWhenMinLevel = sConfigMgr->GetIntDefault("AiPlayerbot.botActiveAloneSmartScaleWhenMinLevel", 1);
    botActiveAloneSmartScaleWhenMaxLevel = sConfigMgr->GetIntDefault("AiPlayerbot.botActiveAloneSmartScaleWhenMaxLevel", 90);
    enableGreet = sConfigMgr->GetBoolDefault("AiPlayerbot.EnableGreet", true);
    autoPickTalents = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoPickTalents", true);
    autoUpgradeEquip = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoUpgradeEquip", true);
    autoLearnTrainerSpells = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoLearnTrainerSpells", true);
    autoTeleportForLevel = sConfigMgr->GetBoolDefault("AiPlayerbot.AutoTeleportForLevel", false);
    twoRoundsGearInit = sConfigMgr->GetBoolDefault("AiPlayerbot.TwoRoundsGearInit", false);
    limitTalentsExpansion = sConfigMgr->GetBoolDefault("AiPlayerbot.LimitTalentsExpansion", false);

    randomBotJoinLfg = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotJoinLfg", true);

    // NewRpgStrategy teleport brackets, same zone list AC ships.
    zoneBrackets.clear();
    {
        std::vector<uint32> zoneBracketZoneIds = {
            1, 12, 14, 85, 141, 215, 3430, 3524,
            17, 38, 40, 130, 148, 3433, 3525,
            10, 11, 44, 267, 331, 400, 406,
            3, 8, 15, 16, 33, 45, 47, 51, 357, 405, 440,
            4, 28, 46, 139, 361, 490, 618, 1377,
            3483, 3518, 3519, 3520, 3521, 3522, 3523, 4080,
            65, 66, 67, 210, 394, 495, 2817, 3537, 3711, 4197
        };

        for (uint32 zoneId : zoneBracketZoneIds)
        {
            std::string setting = "AiPlayerbot.ZoneBracket." + std::to_string(zoneId);
            std::string value = sConfigMgr->GetStringDefault(setting.c_str(), "");
            if (value.empty())
                continue;

            size_t commaPos = value.find(',');
            if (commaPos == std::string::npos)
                continue;

            uint32 minLevel = atoi(value.substr(0, commaPos).c_str());
            uint32 maxLevel = atoi(value.substr(commaPos + 1).c_str());
            zoneBrackets[zoneId] = std::make_pair(minLevel, maxLevel);
        }
    }

    TC_LOG_INFO("server.loading", "---------------------------------------");
    TC_LOG_INFO("server.loading", "        AI Playerbots initialized       ");
    TC_LOG_INFO("server.loading", "---------------------------------------");

    return true;
}

// #201 (#612): "A spot whose .Map is not in sMapStore ... is kept out of the runtime vector". Skipped while the DBCs are
// not loaded yet (a fresh start runs Initialize() from OnConfigLoad); mod_playerbots.cpp OnStartup() calls it again.
void PlayerbotAIConfig::ValidateDuelSpotMaps()
{
    if (!sMapStore.GetNumRows())
        return;

    for (auto it = duelSpots.begin(); it != duelSpots.end();)
    {
        if (!sMapStore.LookupEntry(it->mapId))
        {
            TC_LOG_INFO("playerbots", "[DUELSPOT] spot %s skipped: map %u does not exist", it->name.c_str(), it->mapId);  // #638: same text as AC
            it = duelSpots.erase(it);
        }
        else
            ++it;
    }
}

bool PlayerbotAIConfig::IsPvpProhibited(uint32 zoneId, uint32 areaId)
{
    return IsInPvpProhibitedZone(zoneId) || IsInPvpProhibitedArea(areaId) || IsInPvpProhibitedZone(areaId);
}

bool PlayerbotAIConfig::IsInPvpProhibitedZone(uint32 id)
{
    return std::find(pvpProhibitedZoneIds.begin(), pvpProhibitedZoneIds.end(), id) != pvpProhibitedZoneIds.end();
}

bool PlayerbotAIConfig::IsInPvpProhibitedArea(uint32 id)
{
    return std::find(pvpProhibitedAreaIds.begin(), pvpProhibitedAreaIds.end(), id) != pvpProhibitedAreaIds.end();
}

std::string const PlayerbotAIConfig::GetTimestampStr()
{
    time_t t = time(nullptr);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d %02d-%02d-%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour,
        aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

bool PlayerbotAIConfig::openLog(std::string const fileName, char const* mode)
{
    if (!hasLog(fileName))
        return false;

    auto logFileIt = logFiles.find(fileName);
    if (logFileIt == logFiles.end())
    {
        logFiles.insert(std::make_pair(fileName, std::make_pair(nullptr, false)));
        logFileIt = logFiles.find(fileName);
    }

    FILE* file = logFileIt->second.first;
    bool fileOpen = logFileIt->second.second;

    if (fileOpen)  // close log file
        fclose(file);

    std::string m_logsDir = sConfigMgr->GetStringDefault("LogsDir", "");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }

    file = fopen((m_logsDir + fileName).c_str(), mode);
    fileOpen = true;

    logFileIt->second.first = file;
    logFileIt->second.second = fileOpen;

    return true;
}

void PlayerbotAIConfig::log(std::string const fileName, char const* str, ...)
{
    if (!str)
        return;

    std::lock_guard<std::mutex> guard(m_logMtx);

    if (!isLogOpen(fileName) && !openLog(fileName, "a"))
        return;

    FILE* file = logFiles.find(fileName)->second.first;

    va_list ap;
    va_start(ap, str);
    vfprintf(file, str, ap);
    fprintf(file, "\n");
    va_end(ap);
    fflush(file);

    fflush(stdout);
}

bool PlayerbotAIConfig::IsInRandomAccountList(uint32 id)
{
    return std::find(randomBotAccounts.begin(), randomBotAccounts.end(), id) != randomBotAccounts.end();
}