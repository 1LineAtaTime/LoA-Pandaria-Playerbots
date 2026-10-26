-- AzerothCore's playerbots_account_type: random bot accounts (1) and addclass accounts (2)
CREATE TABLE IF NOT EXISTS `playerbots_account_type` (
    `account_id` int unsigned NOT NULL,
    `account_type` tinyint unsigned NOT NULL DEFAULT 0 COMMENT '0 = unassigned, 1 = RNDbot, 2 = AddClass',
    `assignment_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Playerbot account type assignments';
