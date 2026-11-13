-- Canonical LoA gossip keys must be the primary keys used by upstream INSERT/REPLACE.
-- The prior SF compatibility aliases are retained for rollback/readers, but are no longer keys.
-- Precondition: modern/legacy keys match and modern pairs are unique (verified before deployment).
ALTER TABLE gossip_menu DROP PRIMARY KEY, ADD PRIMARY KEY (MenuID,TextID);
ALTER TABLE gossip_menu_option DROP PRIMARY KEY, ADD PRIMARY KEY (MenuID,OptionID);
ALTER TABLE gossip_menu ADD COLUMN IF NOT EXISTS VerifiedBuild int NOT NULL DEFAULT 0;
ALTER TABLE gossip_menu_option ADD COLUMN IF NOT EXISTS VerifiedBuild int NOT NULL DEFAULT 0;

-- Matching normalized locale schema; source SFDB27 schema only, no world replacement.
CREATE TABLE IF NOT EXISTS `gossip_menu_option_locale` (
  `MenuID` int unsigned NOT NULL DEFAULT '0',
  `OptionID` int unsigned NOT NULL DEFAULT '0',
  `Locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `OptionText` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `BoxText` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  PRIMARY KEY (`MenuID`,`OptionID`,`Locale`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Normalized locale schemas from original LoA world reference; legacy tables retained.
CREATE TABLE IF NOT EXISTS `achievement_reward_locale` (
  `ID` mediumint unsigned NOT NULL DEFAULT '0',
  `Locale` varchar(4) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `Subject` text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Body` text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  PRIMARY KEY (`ID`,`Locale`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `creature_template_locale` (
  `entry` mediumint unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL,
  `Name` text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `FemaleName` text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Title` text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `VerifiedBuild` int DEFAULT '0',
  PRIMARY KEY (`entry`,`locale`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `gameobject_template_locale` (
  `entry` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `name` text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `castBarCaption` text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `VerifiedBuild` smallint DEFAULT '0',
  PRIMARY KEY (`entry`,`locale`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `item_template_locale` (
  `ID` mediumint unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `Name` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `Description` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci,
  `VerifiedBuild` smallint DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `npc_text_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `Locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `Text0_0` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text0_1` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text1_0` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text1_1` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text2_0` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text2_1` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text3_0` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text3_1` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text4_0` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text4_1` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text5_0` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text5_1` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text6_0` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text6_1` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text7_0` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `Text7_1` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  PRIMARY KEY (`ID`,`Locale`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `page_text_locale` (
  `ID` mediumint unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `Text` text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `VerifiedBuild` int DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `points_of_interest_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `Name` text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `VerifiedBuild` int DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `quest_objectives_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `QuestId` int unsigned NOT NULL DEFAULT '0',
  `StorageIndex` tinyint NOT NULL DEFAULT '0',
  `Description` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `quest_offer_reward_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `RewardText` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `quest_request_items_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `CompletionText` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `quest_template_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `LogTitle` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `LogDescription` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `QuestDescription` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `AreaDescription` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `PortraitGiverText` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `PortraitGiverName` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `PortraitTurnInText` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `PortraitTurnInName` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `QuestCompletionLog` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_bin,
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin ROW_FORMAT=DYNAMIC;

-- LoA permits unspecified waypoint orientation.
ALTER TABLE waypoint_data MODIFY COLUMN orientation float NULL DEFAULT 0;

-- Missing metadata columns matched against original LoA reference schema.
ALTER TABLE `bad_word` ADD COLUMN IF NOT EXISTS `convert` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '';
ALTER TABLE `creature` ADD COLUMN IF NOT EXISTS `VerifiedBuild` int NOT NULL DEFAULT '0';
ALTER TABLE `creature_addon` ADD COLUMN IF NOT EXISTS `MountCreatureID` int unsigned NOT NULL DEFAULT '0';
ALTER TABLE `creature_equip_template` ADD COLUMN IF NOT EXISTS `VerifiedBuild` smallint DEFAULT '0';
ALTER TABLE `creature_summon_groups` ADD COLUMN IF NOT EXISTS `Comment` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '';
ALTER TABLE `creature_template_addon` ADD COLUMN IF NOT EXISTS `MountCreatureID` int unsigned NOT NULL DEFAULT '0';
ALTER TABLE `gameobject` ADD COLUMN IF NOT EXISTS `VerifiedBuild` int NOT NULL DEFAULT '0';
ALTER TABLE `points_of_interest` ADD COLUMN IF NOT EXISTS `VerifiedBuild` smallint DEFAULT '0';
ALTER TABLE `quest_poi` ADD COLUMN IF NOT EXISTS `VerifiedBuild` smallint DEFAULT '0';
ALTER TABLE `quest_poi_points` ADD COLUMN IF NOT EXISTS `VerifiedBuild` smallint DEFAULT '0';
ALTER TABLE `waypoint_data` ADD COLUMN IF NOT EXISTS `entry` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci;
ALTER TABLE `waypoint_scripts` ADD COLUMN IF NOT EXISTS `Comment` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '';
ALTER TABLE `world_drop_loot_template` ADD COLUMN IF NOT EXISTS `info` text CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci;
