-- The core version string now carries the release tag, commit count and hash, date and branch (as AzerothCore's
-- .server info), which no longer fits 120 characters: widen core_version to 255 as AzerothCore's version table.
ALTER TABLE `version` MODIFY `core_version` varchar(255) CHARACTER SET utf8mb3 COLLATE utf8mb3_general_ci NOT NULL DEFAULT '' COMMENT 'Core revision dumped at startup.';
