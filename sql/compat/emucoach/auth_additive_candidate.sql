-- REVIEW CANDIDATE ONLY. Tested on empty schema-only fixture, not production data.
-- Do not run until data semantics, indexes, backups and migration order are reviewed.
-- Core automatic updates must remain disabled during compatibility testing.

-- Additive schema-only compatibility candidates for mop_auth.

-- Generated from the fork baseline. Review before any production use.

ALTER TABLE `account` ADD COLUMN `battlenet_account` varchar(32) NOT NULL DEFAULT '';

ALTER TABLE `account` ADD COLUMN `totp_secret` varbinary(128) DEFAULT NULL;

ALTER TABLE `account` ADD COLUMN `lock_country` varchar(2) NOT NULL DEFAULT '00';

ALTER TABLE `account` ADD COLUMN `project_member_id` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `account` ADD COLUMN `project_verified` tinyint(1) NOT NULL DEFAULT '0';

ALTER TABLE `account` ADD COLUMN `project_is_free` tinyint(1) NOT NULL DEFAULT '0';

ALTER TABLE `account` ADD COLUMN `project_is_temp` tinyint(1) NOT NULL DEFAULT '0';

ALTER TABLE `account` ADD COLUMN `project_unban_count` tinyint NOT NULL DEFAULT '0';

ALTER TABLE `account` ADD COLUMN `project_antierror` int unsigned DEFAULT NULL;

ALTER TABLE `account` ADD COLUMN `project_attached` int unsigned DEFAULT NULL;

ALTER TABLE `account` ADD COLUMN `project_passchange` int unsigned NOT NULL DEFAULT '0';

ALTER TABLE `account` ADD COLUMN `project_vote_time` bigint NOT NULL DEFAULT '0';

ALTER TABLE `account` ADD COLUMN `project_hwid` varchar(40) NOT NULL DEFAULT '';

ALTER TABLE `battlenet_accounts` ADD COLUMN `project_member_id` int UNSIGNED NULL DEFAULT NULL;

ALTER TABLE `battlenet_accounts` ADD COLUMN `project_is_temp` tinyint(1) NULL DEFAULT 0 COMMENT 'nighthold';

CREATE TABLE IF NOT EXISTS `battlepay_log`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `accountId` int UNSIGNED NOT NULL,
  `characterGuid` int NOT NULL DEFAULT 0,
  `realm` int UNSIGNED NOT NULL,
  `item` int NOT NULL DEFAULT 0,
  `price` int UNSIGNED NOT NULL,
  `date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

CREATE TABLE IF NOT EXISTS `boost_promotion_executed`  (
  `member_id` int NOT NULL
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

CREATE TABLE IF NOT EXISTS `rbac_account_groups`  (
  `accountId` int UNSIGNED NOT NULL COMMENT 'Account id',
  `groupId` int UNSIGNED NOT NULL COMMENT 'Group id',
  `realmId` int NOT NULL DEFAULT -1 COMMENT 'Realm Id, -1 means all',
  PRIMARY KEY (`accountId`, `groupId`, `realmId`) USING BTREE,
  INDEX `fk__rbac_account_groups__rbac_groups`(`groupId`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin COMMENT = 'Account-Group relation' ROW_FORMAT = Dynamic;

CREATE TABLE IF NOT EXISTS `rbac_account_permissions`  (
  `accountId` int UNSIGNED NOT NULL COMMENT 'Account id',
  `permissionId` int UNSIGNED NOT NULL COMMENT 'Permission id',
  `granted` tinyint(1) NOT NULL DEFAULT 1 COMMENT 'Granted = 1, Denied = 0',
  `realmId` int NOT NULL DEFAULT -1 COMMENT 'Realm Id, -1 means all',
  PRIMARY KEY (`accountId`, `permissionId`, `realmId`) USING BTREE,
  INDEX `fk__rbac_account_roles__rbac_permissions`(`permissionId`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin COMMENT = 'Account-Permission relation' ROW_FORMAT = Dynamic;

CREATE TABLE IF NOT EXISTS `rbac_default_permissions`  (
  `secId` int UNSIGNED NOT NULL COMMENT 'Security Level id',
  `permissionId` int UNSIGNED NOT NULL COMMENT 'permission id',
  `realmId` int NOT NULL DEFAULT -1 COMMENT 'Realm Id, -1 means all',
  PRIMARY KEY (`secId`, `permissionId`, `realmId`) USING BTREE,
  INDEX `fk__rbac_default_permissions__rbac_permissions`(`permissionId`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin COMMENT = 'Default permission to assign to different account security levels' ROW_FORMAT = Dynamic;

CREATE TABLE IF NOT EXISTS `rbac_linked_permissions`  (
  `id` int UNSIGNED NOT NULL COMMENT 'Permission id',
  `linkedId` int UNSIGNED NOT NULL COMMENT 'Linked Permission id',
  PRIMARY KEY (`id`, `linkedId`) USING BTREE,
  INDEX `fk__rbac_linked_permissions__rbac_permissions1`(`id`) USING BTREE,
  INDEX `fk__rbac_linked_permissions__rbac_permissions2`(`linkedId`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin COMMENT = 'Permission - Linked Permission relation' ROW_FORMAT = Dynamic;

CREATE TABLE IF NOT EXISTS `rbac_permissions`  (
  `id` int UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Permission id',
  `name` varchar(100) NOT NULL COMMENT 'Permission name',
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin COMMENT = 'Permission List' ROW_FORMAT = Dynamic;

ALTER TABLE `realmlist` ADD COLUMN `localAddress` varchar(255) NOT NULL DEFAULT '127.0.0.1';

ALTER TABLE `realmlist` ADD COLUMN `localSubnetMask` varchar(255) NOT NULL DEFAULT '255.255.255.0';

CREATE TABLE IF NOT EXISTS `build_info`  (
  `build` int NOT NULL,
  `majorVersion` int NULL DEFAULT NULL,
  `minorVersion` int NULL DEFAULT NULL,
  `bugfixVersion` int NULL DEFAULT NULL,
  `hotfixVersion` varchar(3) NULL DEFAULT NULL,
  `winAuthSeed` varchar(32) NULL DEFAULT NULL,
  `win64AuthSeed` varchar(32) NULL DEFAULT NULL,
  `mac64AuthSeed` varchar(32) NULL DEFAULT NULL,
  `winChecksumSeed` varchar(40) NULL DEFAULT NULL,
  `macChecksumSeed` varchar(40) NULL DEFAULT NULL,
  PRIMARY KEY (`build`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = DYNAMIC;

CREATE TABLE IF NOT EXISTS `wow_token`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `accountId` int UNSIGNED NOT NULL,
  `characterGuid` int NOT NULL DEFAULT 0,
  `realm` int UNSIGNED NOT NULL,
  `coins` int UNSIGNED NOT NULL,
  `date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

CREATE TABLE IF NOT EXISTS `updates` (
  `name` varchar(200) NOT NULL COMMENT 'filename with extension of the update.',
  `hash` char(40) DEFAULT '' COMMENT 'sha1 hash of the sql file.',
  `state` enum('RELEASED','ARCHIVED','CUSTOM') NOT NULL DEFAULT 'RELEASED' COMMENT 'defines if an update is released or archived.',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'timestamp when the query was applied.',
  `speed` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'time the query takes to apply in ms.',
  PRIMARY KEY (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='List of all applied updates in this database.' ROW_FORMAT=DYNAMIC;

CREATE TABLE IF NOT EXISTS `updates_include` (
  `path` varchar(200) NOT NULL COMMENT 'directory to include. $ means relative to the source directory.',
  `state` enum('RELEASED','ARCHIVED') NOT NULL DEFAULT 'RELEASED' COMMENT 'defines if the directory contains released or archived updates.',
  PRIMARY KEY (`path`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin COMMENT='List of directories where we want to include sql updates.' ROW_FORMAT=DYNAMIC;
