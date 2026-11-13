-- REVIEW CANDIDATE ONLY. Tested on empty schema-only fixture, not production data.
-- Do not run until data semantics, indexes, backups and migration order are reviewed.
-- Core automatic updates must remain disabled during compatibility testing.

-- Additive schema-only compatibility candidates for mop_characters.

-- Generated from the fork baseline. Review before any production use.

CREATE TABLE IF NOT EXISTS `auctionbidders` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `bidderguid` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`,`bidderguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `auctionhouse` ADD COLUMN `houseid` tinyint(3) unsigned NOT NULL DEFAULT '7';

ALTER TABLE `characters` ADD COLUMN `project_transfer` int(10) unsigned DEFAULT NULL;

ALTER TABLE `characters` ADD COLUMN `project_original_cloned` int(10) unsigned DEFAULT NULL;

ALTER TABLE `characters` ADD COLUMN `project_original_migrated` int(10) unsigned DEFAULT NULL;

ALTER TABLE `characters` ADD COLUMN `project_original_realm_id` int(10) unsigned DEFAULT NULL;

ALTER TABLE `characters` ADD COLUMN `project_original_guid` int(10) unsigned DEFAULT NULL;

ALTER TABLE `characters` ADD COLUMN `project_clone_created` int(10) unsigned DEFAULT NULL;

ALTER TABLE `characters` ADD COLUMN `project_clone_realm_id` int(10) unsigned DEFAULT NULL;

ALTER TABLE `characters` ADD COLUMN `project_clone_guid` int(10) unsigned DEFAULT NULL;

ALTER TABLE `characters` ADD COLUMN `project_migrant_created` int(10) unsigned DEFAULT NULL;

ALTER TABLE `characters` ADD COLUMN `project_migrant_realm_id` int(10) unsigned DEFAULT NULL;

ALTER TABLE `characters` ADD COLUMN `project_migrant_guid` int(10) unsigned DEFAULT NULL;

CREATE TABLE IF NOT EXISTS `custom_solocraft_character_stats` (
  `guid` int unsigned NOT NULL,
  `Difficulty` float NOT NULL,
  `GroupSize` int(11) NOT NULL,
  `SpellPower` int(10) unsigned NOT NULL DEFAULT '0',
  `Stats` float NOT NULL DEFAULT '100',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

CREATE TABLE IF NOT EXISTS `fake_items` (
  `guid` int(11) NOT NULL,
  `fakeEntry` int(11) NOT NULL,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `updates` (
  `name` varchar(200) COLLATE utf8mb4_unicode_ci NOT NULL COMMENT 'filename with extension of the update.',
  `hash` char(40) COLLATE utf8mb4_unicode_ci DEFAULT '' COMMENT 'sha1 hash of the sql file.',
  `state` enum('RELEASED','ARCHIVED') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'RELEASED' COMMENT 'defines if an update is released or archived.',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'timestamp when the query was applied.',
  `speed` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'time the query takes to apply in ms.',
  PRIMARY KEY (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='List of all applied updates in this database.' ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `updates_include` (
  `path` varchar(200) COLLATE utf8mb4_unicode_ci NOT NULL COMMENT 'directory to include. $ means relative to the source directory.',
  `state` enum('RELEASED','ARCHIVED') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'RELEASED' COMMENT 'defines if the directory contains released or archived updates.',
  PRIMARY KEY (`path`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='List of directories where we want to include sql updates.' ROW_FORMAT=DYNAMIC;
