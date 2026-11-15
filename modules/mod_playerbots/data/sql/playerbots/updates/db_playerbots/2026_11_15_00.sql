-- Best-in-slot table for the "autogear bis" chat command (BisListMgr).
-- Lookup keys: (class, tab, slot, faction, auto_gear_score_limit).
-- faction: 0=Both, 1=Alliance, 2=Horde. Faction-specific rows override Both.
-- auto_gear_score_limit: matched against the item level the command asks for (AiPlayerbot.AutoGearScoreLimit by default).
-- slot: EquipmentSlots enum (head=0, neck=1, shoulders=2, chest=4, waist=5, legs=6, feet=7, wrists=8, hands=9,
--       finger1=10, finger2=11, trinket1=12, trinket2=13, back=14, mainhand=15, offhand=16, ranged=17).
-- tab: specialization tab of the class (0-3).
-- The table ships empty: with no row for a class/spec/item level the command falls back to a normal autogear.
-- Re-runnable. Undo: DROP TABLE `playerbots_bis_gear`;

CREATE TABLE IF NOT EXISTS `playerbots_bis_gear` (
    `class`                 TINYINT UNSIGNED NOT NULL,
    `tab`                   TINYINT UNSIGNED NOT NULL,
    `slot`                  TINYINT UNSIGNED NOT NULL,
    `faction`               TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `auto_gear_score_limit` SMALLINT UNSIGNED NOT NULL,
    `item_id`               INT UNSIGNED NOT NULL,
    `phase`                 VARCHAR(32) NOT NULL DEFAULT '',
    `class_name`            VARCHAR(16) NOT NULL,
    `spec_name`             VARCHAR(32) NOT NULL,
    `slot_name`             VARCHAR(16) NOT NULL,
    `faction_name`          VARCHAR(8)  NOT NULL DEFAULT 'Both',
    `item_name`             VARCHAR(96) NOT NULL,
    PRIMARY KEY (`class`, `tab`, `slot`, `faction`, `auto_gear_score_limit`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
