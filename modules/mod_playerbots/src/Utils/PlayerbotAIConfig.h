/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERbotAICONFIG_H
#define _PLAYERBOT_PLAYERbotAICONFIG_H

#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "Common.h"
#include "DBCEnums.h"
#include "SharedDefines.h"

enum class HealingManaEfficiency : uint8
{
    VERY_LOW = 1,
    LOW = 2,
    MEDIUM = 4,
    HIGH = 8,
    VERY_HIGH = 16,
    SUPERIOR = 32
};

// Status machine states for the "new rpg" roaming strategy
// (ported from mod-playerbots NewRpgInfo.h / PlayerbotAIConfig.h).
// Enum values are used to index RpgStatusProbWeight[], keep them contiguous.
enum NewRpgStatus : int
{
    // Initial status
    RPG_IDLE = 0,
    // Going to far away place
    RPG_GO_GRIND = 1,
    RPG_GO_CAMP = 2,
    // Exploring nearby
    RPG_WANDER_RANDOM = 3,
    RPG_WANDER_NPC = 4,
    // Do quest (based on quest status) - not implemented yet
    RPG_DO_QUEST = 5,
    // Travel
    RPG_TRAVEL_FLIGHT = 6,
    // Taking a break
    RPG_REST = 7,
    RPG_OUTDOOR_PVP = 8,
    // #201 (#612): bots gather at a configured duel spot and challenge each other there. Modelled on RPG_OUTDOOR_PVP
    // (AC NewRpgInfo.h:68-71,114 pattern); AC dormant duel code is called unchanged by NewRpgDuelSpotAction.
    RPG_DUEL_SPOT = 9,
    RPG_STATUS_END = 10
};

// #201 (#612): one configured duel spot (AiPlayerbot.DuelSpot.<Name>.*). Index in PlayerbotAIConfig::duelSpots is the
// spot index used by NewRpgInfo::DuelSpot and DuelSpotMgr. team: 0 both, 1 Alliance, 2 Horde.
struct DuelSpotInfo
{
    std::string name;
    uint32 mapId = 0;
    float x = 0.0f, y = 0.0f, z = 0.0f, radius = 0.0f;
    uint8 team = 0;
    uint32 minLevel = 0, maxLevel = 0, bots = 0;
    bool alwaysActive = false, allowPvpProhibitedZone = false;
};

class PlayerbotAIConfig
{
public:
    PlayerbotAIConfig()
    {
        for (int i = 0; i < RPG_STATUS_END; ++i)
            RpgStatusProbWeight[i] = 0;
    };
    static PlayerbotAIConfig* instance()
    {
        static PlayerbotAIConfig instance;
        return &instance;
    }

    bool Initialize();
    
    bool IsPvpProhibited(uint32 zoneId, uint32 areaId);
    bool IsInPvpProhibitedZone(uint32 id);
    bool IsInPvpProhibitedArea(uint32 id);

    // #201 (#612): drops duel spots whose map is not in sMapStore. Initialize() runs before the DBCs are loaded on a
    // fresh start, so mod_playerbots.cpp OnStartup() calls this again once they are. Idempotent.
    void ValidateDuelSpotMaps();

    bool enabled, perfMonEnabled;
    uint32 globalCoolDown, reactDelay, maxWaitForMove, disableMoveSplinePath, maxMovementSearchTime, expireActionTime,
        dispelAuraDuration, passiveDelay, repeatDelay, errorDelay, rpgDelay, sitDelay, returnDelay, lootDelay;
    bool dynamicReactDelay;
    float sightDistance, spellDistance, reactDistance, grindDistance, lootDistance, shootDistance, fleeDistance,
        tooCloseDistance, meleeDistance, followDistance, whisperDistance, contactDistance, aoeRadius, rpgDistance,
        targetPosRecalcDistance, farDistance, healDistance, aggroDistance;
    uint32 criticalHealth, lowHealth, mediumHealth, almostFullHealth;
    uint32 lowMana, mediumMana, highMana;
    bool autoSaveMana;
    uint32 saveManaThreshold;
    // #173 (AC PlayerbotAIConfig.h:189): TellMasterNoFacing's masterless-bot fallback.
    bool randomBotSayWithoutMaster;
    bool autoAvoidAoe;
    float maxAoeAvoidRadius;
    std::set<uint32> aoeAvoidSpellWhitelist;
    bool tellWhenAvoidAoe;

    uint32 openGoSpell;

    // #134 mount/riding (AC parity): the level a random bot naturally trains each riding tier at,
    // matching AC's PlayerbotAIConfig.h/.cpp (Bot/Factory/PlayerbotFactory.cpp InitSkills reads these
    // same four keys/defaults). Consumed by BotFactory::InitMountSpells().
    uint32 useGroundMountAtMinLevel;
    uint32 useFastGroundMountAtMinLevel;
    uint32 useFlyMountAtMinLevel;
    uint32 useFastFlyMountAtMinLevel;
    // #134 mount/riding (AC parity): extra spells every random bot learns unconditionally on
    // randomize/refresh (AC default "54197" = Cold Weather Flying, needed to fly in Northrend on
    // 5.4.8 too - see Player::IsKnowHowFlyIn()). AC applies this in PlayerbotFactory::InitSpecialSpells.
    std::vector<uint32> randomBotSpellIds;

    bool randomBotAutologin, disableRandomLevels, downgradeMaxLevelBot;
    uint32 randomBotMaxLevel, randomBotMinLevel, randombotStartingLevel;
    float randomBotMaxLevelChance;
    uint32 minRandomBots, maxRandomBots, maxAddedBotsPerClass, maxAddedBots;
    uint32 randomBotUpdateInterval, randomBotCountChangeMinInterval, randomBotCountChangeMaxInterval;
    uint32 minRandomBotRandomizeTime, maxRandomBotRandomizeTime;
    uint32 minRandomBotTeleportInterval, maxRandomBotTeleportInterval;
    uint32 minRandomBotInWorldTime, maxRandomBotInWorldTime;
    uint32 randomBotsPerInterval;
    // #772 (AC PlayerbotAIConfig.h:184,326-327,370-372): AddRandomBots selection keys.
    uint32 permanentlyInWorldTime;
    bool enablePeriodicOnlineOffline;
    float periodicOnlineOfflineRatio;
    uint32 randomBotAllianceRatio, randomBotHordeRatio;
    bool disableDeathKnightLogin;
    float randomChangeMultiplier;
    bool randomBotLoginAtStartup;
    std::string randomBotAccountPrefix;
    uint32 randomBotAccountCount;
    bool randomBotRandomPassword;
    std::vector<uint32> randomBotAccounts;

    // #734: indexed [class][ChrSpecialization TabPage]. premadeSpecLink holds one Talent.dbc column (0-2) per 5.4.8
    // talent tier (levels 15/30/45/60/75/90), PREMADE_SPEC_NO_CHOICE = no preference; premadeSpecGlyph holds glyph
    // item ids (AC AiPlayerbot.PremadeSpecGlyph, parsedSpecGlyph).
    static constexpr uint16 PREMADE_SPEC_NO_CHOICE = 0xFFFF;
    std::string premadeSpecName[MAX_CLASSES][MAX_SPECIALIZATIONS - 1];
    std::vector<uint16> premadeSpecLink[MAX_CLASSES][MAX_SPECIALIZATIONS - 1];
    std::vector<uint32> premadeSpecGlyph[MAX_CLASSES][MAX_SPECIALIZATIONS - 1];

    // #739: AC AiPlayerbot.RandomClassSpecProb/RandomClassSpecIndex.<class>.<specno> (AC PlayerbotAIConfig.h:306-307,
    // PlayerbotAIConfig.cpp:526-543). <specno> is the ChrSpecialization TabPage (druid has 4: balance, feral,
    // guardian, restoration). Prob = weight of the random spec pick (BotFactory::InitTalentsTree; AC
    // PlayerbotFactory::InitTalentsTree, PlayerbotFactory.cpp:1696-1721); Index = the PremadeSpec* entry that spec uses.
    static constexpr uint32 MAX_SPECNO = MAX_SPECIALIZATIONS - 1;
    uint32 randomClassSpecProb[MAX_CLASSES][MAX_SPECNO];
    uint32 randomClassSpecIndex[MAX_CLASSES][MAX_SPECNO];

    uint32 iterationsPerTick;

    std::string combatStrategies, nonCombatStrategies;
    std::string randomBotCombatStrategies, randomBotNonCombatStrategies;
    std::vector<uint32> pvpProhibitedZoneIds;
    std::vector<uint32> pvpProhibitedAreaIds;

    // AC parity (issue #61): maps whose creature spawns feed the GO_GRIND cell cache, and the level window a
    // cell is offered in ([cellLevel - lower, cellLevel + higher]).
    std::vector<uint32> randomBotMaps;
    uint32 randomBotTeleLowerLevel, randomBotTeleHigherLevel;

    // "new rpg" runtime roaming strategy
    // AC PlayerbotAIConfig.h:392 (issue #610): AiPlayerbot.AutoDoQuests, read by the AiFactory "rpg" branch
    bool autoDoQuests;
    bool enableNewRpgStrategy;
    // Chance (0.0-1.0) a random bot picks an rpg status (go camp) instead of grinding
    float randomBotRpgChance;
    // Per-status selection probability weights (indexed by NewRpgStatus)
    int32 RpgStatusProbWeight[RPG_STATUS_END];
    // #843 (MoP/Cata extension, owner 2026-10-16, BEYOND AC): with no doable quest in the log, an IDLE bot's DO_QUEST
    // pick travels to the nearest same-zone quest giver it can take a quest from (NewRpgBaseAction.cpp QuestHubSearch).
    bool rpgQuestHubSearchEnable = true;
    float rpgQuestHubSearchRadius = 2500.0f;

    // #201 (#612 #614 #615): duel spots. Keys AiPlayerbot.DuelSpot*; per-spot keys AiPlayerbot.DuelSpot.<Name>.*.
    bool duelSpotEnable = false;
    std::vector<DuelSpotInfo> duelSpots;
    uint32 duelSpotSelectDistance = 1500, duelSpotMinLevel = 5, duelSpotMaxDuels = 4;
    uint32 duelSpotRematchDelayMin = 25, duelSpotRematchDelayMax = 35;
    uint32 duelSpotScanIntervalMin = 10000, duelSpotScanIntervalMax = 20000;
    uint32 duelSpotMaxLevelDiff = 5;
    bool duelSpotMaxLevelPairsOnly = true, duelSpotSameTeamOnly = true;
    bool duelSpotChallengeRealPlayers = false;
    uint32 duelSpotPlayerChallengeChance = 25, duelSpotPlayerCooldown = 900;
    bool duelSpotReset = false, duelSpotSeedEnable = false;
    uint32 duelSpotSeedInterval = 5000, duelSpotSeedPerTick = 2, duelSpotSeedMaxShare = 25;
    bool duelSpotSeedRelevel = true, duelSpotSeedRegear = true;
    uint32 duelSpotActiveDistance = 1500;

    // Diagnostic (issue #61): bot names whose roaming/AI state is dumped to the "playerbots"
    // logger every 10 s, so a bot that is parked (or stuck in combat, or whose moves fail) is
    // visible in Server.log instead of silent. Empty (default) disables it.
    std::set<std::string> debugBotNames;

    // Battlegrounds (issue #33). Random bots queue for and play battlegrounds when enabled.
    bool randomBotJoinBG;
    // How often (seconds) the battleground queue census runs. Bots only decide whether to fill a
    // queue from that census, so this is also the worst-case delay before a waiting player's queue
    // starts filling. Upstream AC uses 35; the default here stays at the previous 30.
    uint32 bgCheckInterval;
    // Open battleground queues by themselves, so matches form on a realm with no human players.
    // Upstream drives this by seeding specific (battleground, bracket) pairs as "active" - see
    // RandomPlayerbotMgr::CheckBgQueue - which also concentrates bots into the same brackets
    // instead of spreading them thin across every bracket at once.
    bool randomBotAutoJoinBG;
    // Comma-separated bracket ids to auto-fill per battleground, and how many instances of each
    // to keep going. Bracket ids are PvPDifficultyEntry::RangeIndex (0 = the lowest bracket).
    std::vector<uint32> randomBotAutoJoinWSBrackets;
    std::vector<uint32> randomBotAutoJoinTPBrackets;
    std::vector<uint32> randomBotAutoJoinABBrackets;
    std::vector<uint32> randomBotAutoJoinEYBrackets;
    uint32 randomBotAutoJoinWSCount;
    uint32 randomBotAutoJoinTPCount;
    uint32 randomBotAutoJoinABCount;
    uint32 randomBotAutoJoinEYCount;
    // Shorter AI think delay while inside a battleground.
    bool fastReactInBG;

    // Travel-node graph (issue #32). When enabled the server loads - and, when the stored data is
    // missing or stale, regenerates - the long-distance travel graph at startup, and bots route
    // along it instead of walking straight at far destinations.
    bool enableTravelNodes;
    // Distance beyond which a bot routes through the travel graph rather than straight at the
    // destination. Below this, a plain PathGenerator move is both cheaper and better.
    float travelNodeRouteDistance;

    // #173 header batch: every grouped AiPlayerbot.* key from group-functionality-parity.md section
    // 3.6, with AC defaults. Fields only in this pass; consumers land with the batch that needs them
    // (see the #173 report for which keys are already read and which are just reserved).
    //
    // Security (PlayerbotSecurity).
    int32 groupInvitationPermission;
    bool gearscorecheck;
    bool allowAccountBots;
    bool allowGuildBots;
    bool allowTrustedAccountBots;
    int32 selfBotLevel;

    // Bot add (.playerbots bot / .npcbot addclass; maxAddedBots already existed above).
    int32 addClassCommand;
    int32 addClassAccountPoolSize;
    bool autoInitOnly;

    // Chat (PlayerbotAI::HandleCommand).
    std::string commandPrefix;
    std::string commandSeparator;
    bool enableAutoTradeOnItemMention;

    // Grouping.
    bool randomBotGroupNearby;
    bool inviteChat;
    bool summonWhenGroup;
    bool keepAltsInGroup;
    bool disabledWithoutRealPlayer;
    int32 disabledWithoutRealPlayerLoginDelay;
    int32 disabledWithoutRealPlayerLogoutDelay;
    bool syncLevelWithPlayers;

    // Summon.
    bool allowSummonInCombat;
    bool allowSummonWhenMasterIsDead;
    bool allowSummonWhenBotIsDead;
    int32 reviveBotWhenSummoned;
    bool followTeleport;           // beyond AC (#985), default off
    bool respectCrowdControl;      // beyond AC (#985), default off: AttackersValue skips breakable-CC targets
    uint32 followTeleportDelay;
    bool botRepairWhenSummon;
    bool summonAtInnkeepersEnabled;
    // AC PlayerbotAIConfig.h:287 / .cpp:407-408 (#530 item 6): per-bot diagnostic logs only for bots with a real-player master.
    bool logInGroupOnly = true, logValuesPerTick = false;

    // Taxi (GetGroupSlotIndex stagger, plan batch S5). GapMs/GapJitterMs are a MoP-only extension
    // (AC only has Min/Max); no AC default to match.
    uint32 botTaxiDelayMin;
    uint32 botTaxiDelayMax;
    uint32 botTaxiDelayGapMs;
    uint32 botTaxiDelayGapJitterMs;

    // Loot/gear (plan batch S6).
    int32 lootNeedRollLevel;
    bool lootGreedRollLevel;
    bool lootRollRecipe;
    bool lootRollDisenchant;
    bool autoEquipUpgradeLoot;
    int32 autoGearQualityLimit;
    int32 autoGearScoreLimit;
    int32 autoGearCommand;
    int32 autoGearCommandAltBots;

    // Trade (plan batch S8).
    int32 enableRandomBotTrading;
    std::vector<std::string> tradeActionExcludedPrefixes;

    // Quests (plan batch S7).
    bool syncQuestWithPlayer;
    bool syncQuestForPlayer;
    bool dropObsoleteQuests;
    std::string autoPickReward;

    // Combat (autoSaveMana already existed above).
    int32 defaultPetStance;
    bool applyInstanceStrategies;
    bool forceRebuffOnReadyCheck;
    uint32 forceRebuffMarginSecs;  // AC PlayerbotAIConfig.h:139 (#628)

    // Activity.
    int32 botActiveAlone;
    uint32 botActiveAloneDurationSeconds;
    uint32 botActiveAloneForceWhenInRadius;
    bool botActiveAloneForceWhenInZone;
    bool botActiveAloneForceWhenInMap;
    bool botActiveAloneForceWhenIsFriend;
    bool botActiveAloneForceWhenInGuild;
    // #72: AC PlayerbotAIConfig.h:381-385 (read by the ported AllowActive / AutoScaleActivity)
    bool botActiveAloneSmartScale;
    uint32 botActiveAloneSmartScaleDiffLimitfloor;
    uint32 botActiveAloneSmartScaleDiffLimitCeiling;
    uint32 botActiveAloneSmartScaleWhenMinLevel;
    uint32 botActiveAloneSmartScaleWhenMaxLevel;
    bool enableGreet;
    bool autoPickTalents;
    bool autoUpgradeEquip;
    bool autoLearnTrainerSpells;
    bool autoTeleportForLevel;
    bool twoRoundsGearInit;
    bool limitTalentsExpansion;

    // LFG (plan batch S10).
    bool randomBotJoinLfg;

    // NewRpgStrategy teleport brackets by zone. Loaded from
    // "AiPlayerbot.ZoneBracket.<zoneId>" = "<minLevel>,<maxLevel>"; empty/unset zones are absent from
    // the map. Bundled into this header pass to avoid a second long build; NewRpgStrategy/
    // RandomPlayerbotMgr consumption is out of scope for #173 (RandomPlayerbotMgr.cpp is being edited
    // in parallel - see the #173 report).
    std::map<uint32, std::pair<uint32, uint32>> zoneBrackets;

    std::mutex m_logMtx;
    std::vector<std::string> allowedLogFiles;
    std::unordered_map<std::string, std::pair<FILE*, bool>> logFiles;

    // METHODS
    bool IsInRandomAccountList(uint32 id);

    std::string const GetTimestampStr();
    bool hasLog(std::string const fileName)
    {
        return std::find(allowedLogFiles.begin(), allowedLogFiles.end(), fileName) != allowedLogFiles.end();
    };
    bool openLog(std::string const fileName, char const* mode = "a");
    bool isLogOpen(std::string const fileName)
    {
        auto it = logFiles.find(fileName);
        return it != logFiles.end() && it->second.second;
    }
    void log(std::string const fileName, const char* str, ...);
};

#define sPlayerbotAIConfig PlayerbotAIConfig::instance()

#endif
