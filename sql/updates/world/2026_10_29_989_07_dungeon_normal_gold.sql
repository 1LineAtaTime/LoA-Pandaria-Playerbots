-- #989 (Refs #989): normal-mode dungeon gold (standalone; folds the PR's 2026_10_27_88 + 27_89 + 27_92)
-- Source: DeltaTho playerbots-fork PR 15. Pairs with the core lane's C++ 6fd73ea6 (Unit::Kill reads creature_template_difficulty.mingold/maxgold
--   when the row sets them; NULL = use creature_template gold).
-- The Cataclysm/MoP dungeon revamps share one template between normal (levels 15-43) and heroic (85-92) and the template carries the heroic
--   gold, so normal-mode mobs dropped 20 silver - 30 gold. 27_92 puts the level-scaled normal gold on the DUNGEON_NORMAL difficulty rows and
--   leaves the templates at the heroic amount (our DB already has the heroic template values, so those UPDATEs below are no-ops here).
-- 27_88/27_89 (Scarlet Halls / Monastery template gold scaled to normal) are superseded by 27_92's DUNGEON_NORMAL rows + template restore, except
--   the two creatures without difficulty rows, Mining Monkey 48278 (17) and Master Archer 59175 (107), which keep the scaled value (set at the end).
-- 27_86/87/90/91 (HP workaround and revert, net no-op) are intentionally NOT ported: the fix is C++ 3c866f4a.
-- Design check 2026-10-29: Wowpedia Deadmines (MediaWiki API): separate normal ("previous instance level: 15 - 21", patch 7.3.5 note) and heroic (level 85)
--   modes of one instance, i.e. the PR two-band design; gold scaling is the PR own formula (heroic gold * open-world normal average at the level /
--   average at the heroic level), not a retail value; nothing in the APIs states retail normal gold, so values are PR-derived.
-- Re-runnable: the ADD COLUMN is guarded by information_schema (PREPARE); the UPDATEs set absolute values.
-- Undo: ALTER TABLE `creature_template_difficulty` DROP COLUMN `maxgold`, DROP COLUMN `mingold`;
SET @has := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'creature_template_difficulty' AND COLUMN_NAME = 'mingold');
SET @ddl := IF(@has = 0, 'ALTER TABLE `creature_template_difficulty` ADD COLUMN `mingold` int unsigned DEFAULT NULL AFTER `level_max`, ADD COLUMN `maxgold` int unsigned DEFAULT NULL AFTER `mingold`', 'DO 0');
PREPARE stmt FROM @ddl;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Move the normal-mode gold of 2026_10_27_86-90 (the template gold scaled to the creature's normal level) into the DUNGEON_NORMAL
-- rows, and give the templates back their heroic gold (the original world dump, world_548_20240722). Mining Monkey 48278 and
-- Master Archer 59175 have no difficulty rows, so they are the same level in every mode and keep the scaled gold.
UPDATE `creature_template_difficulty` SET `mingold` = 65, `maxgold` = 65 WHERE `id` = 48420 AND `difficulty` = 'DUNGEON_NORMAL'; -- Defias Digger
UPDATE `creature_template_difficulty` SET `mingold` = 65, `maxgold` = 65 WHERE `id` = 48419 AND `difficulty` = 'DUNGEON_NORMAL'; -- Defias Miner
UPDATE `creature_template_difficulty` SET `mingold` = 226, `maxgold` = 226 WHERE `id` = 43778 AND `difficulty` = 'DUNGEON_NORMAL'; -- Foe Reaper 5000
UPDATE `creature_template_difficulty` SET `mingold` = 147, `maxgold` = 147 WHERE `id` = 47626 AND `difficulty` = 'DUNGEON_NORMAL'; -- Admiral Ripsnarl
UPDATE `creature_template_difficulty` SET `mingold` = 69, `maxgold` = 69 WHERE `id` = 48230 AND `difficulty` = 'DUNGEON_NORMAL'; -- Ogre Henchman
UPDATE `creature_template_difficulty` SET `mingold` = 63, `maxgold` = 63 WHERE `id` = 48229 AND `difficulty` = 'DUNGEON_NORMAL'; -- Kobold Digger
UPDATE `creature_template_difficulty` SET `mingold` = 47, `maxgold` = 47 WHERE `id` = 48262 AND `difficulty` = 'DUNGEON_NORMAL'; -- Ogre Bodyguard
UPDATE `creature_template_difficulty` SET `mingold` = 68, `maxgold` = 68 WHERE `id` = 48279 AND `difficulty` = 'DUNGEON_NORMAL'; -- Goblin Overseer
UPDATE `creature_template_difficulty` SET `mingold` = 31, `maxgold` = 31 WHERE `id` = 48445 AND `difficulty` = 'DUNGEON_NORMAL'; -- Oaf Lackey
UPDATE `creature_template_difficulty` SET `mingold` = 74, `maxgold` = 74 WHERE `id` = 48502 AND `difficulty` = 'DUNGEON_NORMAL'; -- Defias Enforcer
UPDATE `creature_template_difficulty` SET `mingold` = 72, `maxgold` = 72 WHERE `id` = 48505 AND `difficulty` = 'DUNGEON_NORMAL'; -- Defias Shadowguard
UPDATE `creature_template_difficulty` SET `mingold` = 83, `maxgold` = 83 WHERE `id` = 48417 AND `difficulty` = 'DUNGEON_NORMAL'; -- Defias Blood Wizard
UPDATE `creature_template_difficulty` SET `mingold` = 58, `maxgold` = 58 WHERE `id` = 48521 AND `difficulty` = 'DUNGEON_NORMAL'; -- Defias Squallshaper
UPDATE `creature_template_difficulty` SET `mingold` = 59, `maxgold` = 59 WHERE `id` = 48522 AND `difficulty` = 'DUNGEON_NORMAL'; -- Defias Pirate
UPDATE `creature_template_difficulty` SET `mingold` = 83, `maxgold` = 83 WHERE `id` = 48418 AND `difficulty` = 'DUNGEON_NORMAL'; -- Defias Envoker
UPDATE `creature_template_difficulty` SET `mingold` = 70, `maxgold` = 70 WHERE `id` = 48421 AND `difficulty` = 'DUNGEON_NORMAL'; -- Defias Overseer
UPDATE `creature_template_difficulty` SET `mingold` = 19, `maxgold` = 19 WHERE `id` = 48441 AND `difficulty` = 'DUNGEON_NORMAL'; -- Mining Monkey
UPDATE `creature_template_difficulty` SET `mingold` = 148, `maxgold` = 148 WHERE `id` = 47162 AND `difficulty` = 'DUNGEON_NORMAL'; -- Glubtok
UPDATE `creature_template_difficulty` SET `mingold` = 80, `maxgold` = 80 WHERE `id` = 48338 AND `difficulty` = 'DUNGEON_NORMAL'; -- Mine Bunny
UPDATE `creature_template_difficulty` SET `mingold` = 141, `maxgold` = 141 WHERE `id` = 47739 AND `difficulty` = 'DUNGEON_NORMAL'; -- "Captain" Cookie
UPDATE `creature_template_difficulty` SET `mingold` = 195, `maxgold` = 195 WHERE `id` = 3864 AND `difficulty` = 'DUNGEON_NORMAL'; -- Fel Steed
UPDATE `creature_template_difficulty` SET `mingold` = 177, `maxgold` = 177 WHERE `id` = 3865 AND `difficulty` = 'DUNGEON_NORMAL'; -- Shadow Charger
UPDATE `creature_template_difficulty` SET `mingold` = 189, `maxgold` = 189 WHERE `id` = 3869 AND `difficulty` = 'DUNGEON_NORMAL'; -- Lesser Gargoyle
UPDATE `creature_template_difficulty` SET `mingold` = 189, `maxgold` = 189 WHERE `id` = 3870 AND `difficulty` = 'DUNGEON_NORMAL'; -- Stone Sleeper
UPDATE `creature_template_difficulty` SET `mingold` = 156, `maxgold` = 156 WHERE `id` = 3873 AND `difficulty` = 'DUNGEON_NORMAL'; -- Tormented Officer
UPDATE `creature_template_difficulty` SET `mingold` = 103, `maxgold` = 103 WHERE `id` = 3875 AND `difficulty` = 'DUNGEON_NORMAL'; -- Haunted Servitor
UPDATE `creature_template_difficulty` SET `mingold` = 94, `maxgold` = 94 WHERE `id` = 3877 AND `difficulty` = 'DUNGEON_NORMAL'; -- Wailing Guardsman
UPDATE `creature_template_difficulty` SET `mingold` = 214, `maxgold` = 214 WHERE `id` = 3887 AND `difficulty` = 'DUNGEON_NORMAL'; -- Baron Silverlaine
UPDATE `creature_template_difficulty` SET `mingold` = 175, `maxgold` = 175 WHERE `id` = 4278 AND `difficulty` = 'DUNGEON_NORMAL'; -- Commander Springvale
UPDATE `creature_template_difficulty` SET `mingold` = 223, `maxgold` = 223 WHERE `id` = 46962 AND `difficulty` = 'DUNGEON_NORMAL'; -- Baron Ashbury
UPDATE `creature_template_difficulty` SET `mingold` = 226, `maxgold` = 226 WHERE `id` = 46963 AND `difficulty` = 'DUNGEON_NORMAL'; -- Lord Walden
UPDATE `creature_template_difficulty` SET `mingold` = 231, `maxgold` = 231 WHERE `id` = 46964 AND `difficulty` = 'DUNGEON_NORMAL'; -- Lord Godfrey
UPDATE `creature_template_difficulty` SET `mingold` = 118, `maxgold` = 118 WHERE `id` = 47131 AND `difficulty` = 'DUNGEON_NORMAL'; -- Frantic Geist
UPDATE `creature_template_difficulty` SET `mingold` = 139, `maxgold` = 139 WHERE `id` = 47132 AND `difficulty` = 'DUNGEON_NORMAL'; -- Dark Creeper
UPDATE `creature_template_difficulty` SET `mingold` = 115, `maxgold` = 115 WHERE `id` = 47134 AND `difficulty` = 'DUNGEON_NORMAL'; -- Corpse Eater
UPDATE `creature_template_difficulty` SET `mingold` = 84, `maxgold` = 84 WHERE `id` = 47135 AND `difficulty` = 'DUNGEON_NORMAL'; -- Fetid Ghoul
UPDATE `creature_template_difficulty` SET `mingold` = 135, `maxgold` = 135 WHERE `id` = 47136 AND `difficulty` = 'DUNGEON_NORMAL'; -- Unstable Ravager
UPDATE `creature_template_difficulty` SET `mingold` = 110, `maxgold` = 110 WHERE `id` = 47137 AND `difficulty` = 'DUNGEON_NORMAL'; -- Mindless Horror
UPDATE `creature_template_difficulty` SET `mingold` = 106, `maxgold` = 106 WHERE `id` = 47138 AND `difficulty` = 'DUNGEON_NORMAL'; -- Pustulant Monstrosity
UPDATE `creature_template_difficulty` SET `mingold` = 104, `maxgold` = 104 WHERE `id` = 47140 AND `difficulty` = 'DUNGEON_NORMAL'; -- Sorcerous Skeleton
UPDATE `creature_template_difficulty` SET `mingold` = 135, `maxgold` = 135 WHERE `id` = 47141 AND `difficulty` = 'DUNGEON_NORMAL'; -- Dread Scryer
UPDATE `creature_template_difficulty` SET `mingold` = 94, `maxgold` = 94 WHERE `id` = 47143 AND `difficulty` = 'DUNGEON_NORMAL'; -- Spitebone Skeleton
UPDATE `creature_template_difficulty` SET `mingold` = 92, `maxgold` = 92 WHERE `id` = 47145 AND `difficulty` = 'DUNGEON_NORMAL'; -- Spitebone Guardian
UPDATE `creature_template_difficulty` SET `mingold` = 135, `maxgold` = 135 WHERE `id` = 47146 AND `difficulty` = 'DUNGEON_NORMAL'; -- Spitebone Flayer
UPDATE `creature_template_difficulty` SET `mingold` = 103, `maxgold` = 103 WHERE `id` = 47231 AND `difficulty` = 'DUNGEON_NORMAL'; -- Shadowy Attendant
UPDATE `creature_template_difficulty` SET `mingold` = 101, `maxgold` = 101 WHERE `id` = 47232 AND `difficulty` = 'DUNGEON_NORMAL'; -- Ghostly Cook
UPDATE `creature_template_difficulty` SET `mingold` = 7767, `maxgold` = 7767 WHERE `id` = 58632 AND `difficulty` = 'DUNGEON_NORMAL'; -- Armsmaster Harlan
UPDATE `creature_template_difficulty` SET `mingold` = 375, `maxgold` = 375 WHERE `id` = 58674 AND `difficulty` = 'DUNGEON_NORMAL'; -- Angry Hound
UPDATE `creature_template_difficulty` SET `mingold` = 129, `maxgold` = 129 WHERE `id` = 58676 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Defender
UPDATE `creature_template_difficulty` SET `mingold` = 129, `maxgold` = 129 WHERE `id` = 58683 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Myrmidon
UPDATE `creature_template_difficulty` SET `mingold` = 124, `maxgold` = 124 WHERE `id` = 58684 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Scourge Hewer
UPDATE `creature_template_difficulty` SET `mingold` = 124, `maxgold` = 124 WHERE `id` = 58685 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Evangelist
UPDATE `creature_template_difficulty` SET `mingold` = 132, `maxgold` = 132 WHERE `id` = 58756 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Evoker
UPDATE `creature_template_difficulty` SET `mingold` = 264, `maxgold` = 264 WHERE `id` = 58876 AND `difficulty` = 'DUNGEON_NORMAL'; -- Starving Hound
UPDATE `creature_template_difficulty` SET `mingold` = 85, `maxgold` = 85 WHERE `id` = 58898 AND `difficulty` = 'DUNGEON_NORMAL'; -- Vigilant Watchman
UPDATE `creature_template_difficulty` SET `mingold` = 9971, `maxgold` = 9971 WHERE `id` = 59150 AND `difficulty` = 'DUNGEON_NORMAL'; -- Flameweaver Koegler
UPDATE `creature_template_difficulty` SET `mingold` = 153, `maxgold` = 153 WHERE `id` = 59240 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Hall Guardian
UPDATE `creature_template_difficulty` SET `mingold` = 152, `maxgold` = 152 WHERE `id` = 59241 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Treasurer
UPDATE `creature_template_difficulty` SET `mingold` = 8110, `maxgold` = 8110 WHERE `id` = 59303 AND `difficulty` = 'DUNGEON_NORMAL'; -- Houndmaster Braun
UPDATE `creature_template_difficulty` SET `mingold` = 153, `maxgold` = 153 WHERE `id` = 59372 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Scholar
UPDATE `creature_template_difficulty` SET `mingold` = 142, `maxgold` = 142 WHERE `id` = 59373 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Pupil
UPDATE `creature_template_difficulty` SET `mingold` = 2431, `maxgold` = 2431 WHERE `id` = 3977 AND `difficulty` = 'DUNGEON_NORMAL'; -- High Inquisitor Whitemane
UPDATE `creature_template_difficulty` SET `mingold` = 173, `maxgold` = 173 WHERE `id` = 58555 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Fanatic
UPDATE `creature_template_difficulty` SET `mingold` = 173, `maxgold` = 173 WHERE `id` = 58569 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Purifier
UPDATE `creature_template_difficulty` SET `mingold` = 163, `maxgold` = 163 WHERE `id` = 58590 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Zealot
UPDATE `creature_template_difficulty` SET `mingold` = 149, `maxgold` = 149 WHERE `id` = 58605 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Judicator
UPDATE `creature_template_difficulty` SET `mingold` = 93, `maxgold` = 93 WHERE `id` = 58783 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Initiate
UPDATE `creature_template_difficulty` SET `mingold` = 11884, `maxgold` = 11884 WHERE `id` = 59223 AND `difficulty` = 'DUNGEON_NORMAL'; -- Brother Korloff
UPDATE `creature_template_difficulty` SET `mingold` = 123, `maxgold` = 123 WHERE `id` = 59705 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Flamethrower
UPDATE `creature_template_difficulty` SET `mingold` = 123, `maxgold` = 123 WHERE `id` = 59746 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scarlet Centurion
UPDATE `creature_template_difficulty` SET `mingold` = 10395, `maxgold` = 10395 WHERE `id` = 59789 AND `difficulty` = 'DUNGEON_NORMAL'; -- Thalnos the Soulrender
UPDATE `creature_template_difficulty` SET `mingold` = 4484, `maxgold` = 4484 WHERE `id` = 60040 AND `difficulty` = 'DUNGEON_NORMAL'; -- Commander Durand
UPDATE `creature_template_difficulty` SET `mingold` = 27554, `maxgold` = 27554 WHERE `id` = 58664 AND `difficulty` = 'DUNGEON_NORMAL'; -- Instructor Chillheart's Phylactery
UPDATE `creature_template_difficulty` SET `mingold` = 281, `maxgold` = 281 WHERE `id` = 58757 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scholomance Acolyte
UPDATE `creature_template_difficulty` SET `mingold` = 283, `maxgold` = 283 WHERE `id` = 58823 AND `difficulty` = 'DUNGEON_NORMAL'; -- Scholomance Neophyte
UPDATE `creature_template_difficulty` SET `mingold` = 23489, `maxgold` = 23489 WHERE `id` = 59080 AND `difficulty` = 'DUNGEON_NORMAL'; -- Darkmaster Gandling
UPDATE `creature_template_difficulty` SET `mingold` = 20334, `maxgold` = 20334 WHERE `id` = 59153 AND `difficulty` = 'DUNGEON_NORMAL'; -- Rattlegore
UPDATE `creature_template_difficulty` SET `mingold` = 20301, `maxgold` = 20301 WHERE `id` = 59184 AND `difficulty` = 'DUNGEON_NORMAL'; -- Jandice Barov
UPDATE `creature_template_difficulty` SET `mingold` = 348, `maxgold` = 348 WHERE `id` = 59193 AND `difficulty` = 'DUNGEON_NORMAL'; -- Boneweaver
UPDATE `creature_template_difficulty` SET `mingold` = 356, `maxgold` = 356 WHERE `id` = 59368 AND `difficulty` = 'DUNGEON_NORMAL'; -- Krastinovian Carver
UPDATE `creature_template_difficulty` SET `mingold` = 354, `maxgold` = 354 WHERE `id` = 59467 AND `difficulty` = 'DUNGEON_NORMAL'; -- Candlestick Mage
UPDATE `creature_template_difficulty` SET `mingold` = 513, `maxgold` = 513 WHERE `id` = 59613 AND `difficulty` = 'DUNGEON_NORMAL'; -- Professor Slate
UPDATE `creature_template_difficulty` SET `mingold` = 392, `maxgold` = 392 WHERE `id` = 59614 AND `difficulty` = 'DUNGEON_NORMAL'; -- Bored Student

UPDATE `creature_template` SET `mingold` = 7903, `maxgold` = 7903 WHERE `entry` = 48420; -- Defias Digger
UPDATE `creature_template` SET `mingold` = 7906, `maxgold` = 7906 WHERE `entry` = 48419; -- Defias Miner
UPDATE `creature_template` SET `mingold` = 20008, `maxgold` = 20008 WHERE `entry` = 43778; -- Foe Reaper 5000
UPDATE `creature_template` SET `mingold` = 13029, `maxgold` = 13029 WHERE `entry` = 47626; -- Admiral Ripsnarl
UPDATE `creature_template` SET `mingold` = 7084, `maxgold` = 7084 WHERE `entry` = 48230; -- Ogre Henchman
UPDATE `creature_template` SET `mingold` = 7684, `maxgold` = 7684 WHERE `entry` = 48229; -- Kobold Digger
UPDATE `creature_template` SET `mingold` = 4819, `maxgold` = 4819 WHERE `entry` = 48262; -- Ogre Bodyguard
UPDATE `creature_template` SET `mingold` = 6975, `maxgold` = 6975 WHERE `entry` = 48279; -- Goblin Overseer
UPDATE `creature_template` SET `mingold` = 7141, `maxgold` = 7141 WHERE `entry` = 48445; -- Oaf Lackey
UPDATE `creature_template` SET `mingold` = 7576, `maxgold` = 7576 WHERE `entry` = 48502; -- Defias Enforcer
UPDATE `creature_template` SET `mingold` = 7403, `maxgold` = 7403 WHERE `entry` = 48505; -- Defias Shadowguard
UPDATE `creature_template` SET `mingold` = 8562, `maxgold` = 8562 WHERE `entry` = 48417; -- Defias Blood Wizard
UPDATE `creature_template` SET `mingold` = 5920, `maxgold` = 5920 WHERE `entry` = 48521; -- Defias Squallshaper
UPDATE `creature_template` SET `mingold` = 6028, `maxgold` = 6028 WHERE `entry` = 48522; -- Defias Pirate
UPDATE `creature_template` SET `mingold` = 8541, `maxgold` = 8541 WHERE `entry` = 48418; -- Defias Envoker
UPDATE `creature_template` SET `mingold` = 7165, `maxgold` = 7165 WHERE `entry` = 48421; -- Defias Overseer
UPDATE `creature_template` SET `mingold` = 2285, `maxgold` = 2285 WHERE `entry` = 48441; -- Mining Monkey
UPDATE `creature_template` SET `mingold` = 13158, `maxgold` = 13158 WHERE `entry` = 47162; -- Glubtok
UPDATE `creature_template` SET `mingold` = 8244, `maxgold` = 8244 WHERE `entry` = 48338; -- Mine Bunny
UPDATE `creature_template` SET `mingold` = 12490, `maxgold` = 12490 WHERE `entry` = 47739; -- "Captain" Cookie
UPDATE `creature_template` SET `mingold` = 14077, `maxgold` = 14077 WHERE `entry` = 3864; -- Fel Steed
UPDATE `creature_template` SET `mingold` = 13801, `maxgold` = 13801 WHERE `entry` = 3865; -- Shadow Charger
UPDATE `creature_template` SET `mingold` = 12163, `maxgold` = 12163 WHERE `entry` = 3869; -- Lesser Gargoyle
UPDATE `creature_template` SET `mingold` = 12190, `maxgold` = 12190 WHERE `entry` = 3870; -- Stone Sleeper
UPDATE `creature_template` SET `mingold` = 10030, `maxgold` = 10030 WHERE `entry` = 3873; -- Tormented Officer
UPDATE `creature_template` SET `mingold` = 7431, `maxgold` = 7431 WHERE `entry` = 3875; -- Haunted Servitor
UPDATE `creature_template` SET `mingold` = 6770, `maxgold` = 6770 WHERE `entry` = 3877; -- Wailing Guardsman
UPDATE `creature_template` SET `mingold` = 12181, `maxgold` = 12181 WHERE `entry` = 3887; -- Baron Silverlaine
UPDATE `creature_template` SET `mingold` = 9945, `maxgold` = 9945 WHERE `entry` = 4278; -- Commander Springvale
UPDATE `creature_template` SET `mingold` = 12683, `maxgold` = 12683 WHERE `entry` = 46962; -- Baron Ashbury
UPDATE `creature_template` SET `mingold` = 12829, `maxgold` = 12829 WHERE `entry` = 46963; -- Lord Walden
UPDATE `creature_template` SET `mingold` = 13127, `maxgold` = 13127 WHERE `entry` = 46964; -- Lord Godfrey
UPDATE `creature_template` SET `mingold` = 8490, `maxgold` = 8490 WHERE `entry` = 47131; -- Frantic Geist
UPDATE `creature_template` SET `mingold` = 8930, `maxgold` = 8930 WHERE `entry` = 47132; -- Dark Creeper
UPDATE `creature_template` SET `mingold` = 8286, `maxgold` = 8286 WHERE `entry` = 47134; -- Corpse Eater
UPDATE `creature_template` SET `mingold` = 6075, `maxgold` = 6075 WHERE `entry` = 47135; -- Fetid Ghoul
UPDATE `creature_template` SET `mingold` = 8687, `maxgold` = 8687 WHERE `entry` = 47136; -- Unstable Ravager
UPDATE `creature_template` SET `mingold` = 7080, `maxgold` = 7080 WHERE `entry` = 47137; -- Mindless Horror
UPDATE `creature_template` SET `mingold` = 6824, `maxgold` = 6824 WHERE `entry` = 47138; -- Pustulant Monstrosity
UPDATE `creature_template` SET `mingold` = 7528, `maxgold` = 7528 WHERE `entry` = 47140; -- Sorcerous Skeleton
UPDATE `creature_template` SET `mingold` = 8675, `maxgold` = 8675 WHERE `entry` = 47141; -- Dread Scryer
UPDATE `creature_template` SET `mingold` = 6799, `maxgold` = 6799 WHERE `entry` = 47143; -- Spitebone Skeleton
UPDATE `creature_template` SET `mingold` = 6662, `maxgold` = 6662 WHERE `entry` = 47145; -- Spitebone Guardian
UPDATE `creature_template` SET `mingold` = 8679, `maxgold` = 8679 WHERE `entry` = 47146; -- Spitebone Flayer
UPDATE `creature_template` SET `mingold` = 7463, `maxgold` = 7463 WHERE `entry` = 47231; -- Shadowy Attendant
UPDATE `creature_template` SET `mingold` = 7302, `maxgold` = 7302 WHERE `entry` = 47232; -- Ghostly Cook
UPDATE `creature_template` SET `mingold` = 231024, `maxgold` = 231024 WHERE `entry` = 58632; -- Armsmaster Harlan
UPDATE `creature_template` SET `mingold` = 12373, `maxgold` = 12373 WHERE `entry` = 58674; -- Angry Hound
UPDATE `creature_template` SET `mingold` = 4991, `maxgold` = 4991 WHERE `entry` = 58676; -- Scarlet Defender
UPDATE `creature_template` SET `mingold` = 4971, `maxgold` = 4971 WHERE `entry` = 58683; -- Scarlet Myrmidon
UPDATE `creature_template` SET `mingold` = 4784, `maxgold` = 4784 WHERE `entry` = 58684; -- Scarlet Scourge Hewer
UPDATE `creature_template` SET `mingold` = 4766, `maxgold` = 4766 WHERE `entry` = 58685; -- Scarlet Evangelist
UPDATE `creature_template` SET `mingold` = 5084, `maxgold` = 5084 WHERE `entry` = 58756; -- Scarlet Evoker
UPDATE `creature_template` SET `mingold` = 10197, `maxgold` = 10197 WHERE `entry` = 58876; -- Starving Hound
UPDATE `creature_template` SET `mingold` = 2817, `maxgold` = 2817 WHERE `entry` = 58898; -- Vigilant Watchman
UPDATE `creature_template` SET `mingold` = 296577, `maxgold` = 296577 WHERE `entry` = 59150; -- Flameweaver Koegler
UPDATE `creature_template` SET `mingold` = 5918, `maxgold` = 5918 WHERE `entry` = 59240; -- Scarlet Hall Guardian
UPDATE `creature_template` SET `mingold` = 5846, `maxgold` = 5846 WHERE `entry` = 59241; -- Scarlet Treasurer
UPDATE `creature_template` SET `mingold` = 241231, `maxgold` = 241231 WHERE `entry` = 59303; -- Houndmaster Braun
UPDATE `creature_template` SET `mingold` = 5897, `maxgold` = 5897 WHERE `entry` = 59372; -- Scarlet Scholar
UPDATE `creature_template` SET `mingold` = 5472, `maxgold` = 5472 WHERE `entry` = 59373; -- Scarlet Pupil
UPDATE `creature_template` SET `mingold` = 62200, `maxgold` = 62200 WHERE `entry` = 3977; -- High Inquisitor Whitemane
UPDATE `creature_template` SET `mingold` = 5052, `maxgold` = 5052 WHERE `entry` = 58555; -- Scarlet Fanatic
UPDATE `creature_template` SET `mingold` = 5058, `maxgold` = 5058 WHERE `entry` = 58569; -- Scarlet Purifier
UPDATE `creature_template` SET `mingold` = 4772, `maxgold` = 4772 WHERE `entry` = 58590; -- Scarlet Zealot
UPDATE `creature_template` SET `mingold` = 4338, `maxgold` = 4338 WHERE `entry` = 58605; -- Scarlet Judicator
UPDATE `creature_template` SET `mingold` = 3079, `maxgold` = 3079 WHERE `entry` = 58783; -- Scarlet Initiate
UPDATE `creature_template` SET `mingold` = 304103, `maxgold` = 304103 WHERE `entry` = 59223; -- Brother Korloff
UPDATE `creature_template` SET `mingold` = 3671, `maxgold` = 3671 WHERE `entry` = 59705; -- Scarlet Flamethrower
UPDATE `creature_template` SET `mingold` = 3670, `maxgold` = 3670 WHERE `entry` = 59746; -- Scarlet Centurion
UPDATE `creature_template` SET `mingold` = 289664, `maxgold` = 289664 WHERE `entry` = 59789; -- Thalnos the Soulrender
UPDATE `creature_template` SET `mingold` = 114737, `maxgold` = 114737 WHERE `entry` = 60040; -- Commander Durand
UPDATE `creature_template` SET `mingold` = 409121, `maxgold` = 409121 WHERE `entry` = 58664; -- Instructor Chillheart's Phylactery
UPDATE `creature_template` SET `mingold` = 4830, `maxgold` = 4830 WHERE `entry` = 58757; -- Scholomance Acolyte
UPDATE `creature_template` SET `mingold` = 4878, `maxgold` = 4878 WHERE `entry` = 58823; -- Scholomance Neophyte
UPDATE `creature_template` SET `mingold` = 348770, `maxgold` = 348770 WHERE `entry` = 59080; -- Darkmaster Gandling
UPDATE `creature_template` SET `mingold` = 301916, `maxgold` = 301916 WHERE `entry` = 59153; -- Rattlegore
UPDATE `creature_template` SET `mingold` = 301436, `maxgold` = 301436 WHERE `entry` = 59184; -- Jandice Barov
UPDATE `creature_template` SET `mingold` = 5697, `maxgold` = 5697 WHERE `entry` = 59193; -- Boneweaver
UPDATE `creature_template` SET `mingold` = 6124, `maxgold` = 6124 WHERE `entry` = 59368; -- Krastinovian Carver
UPDATE `creature_template` SET `mingold` = 6094, `maxgold` = 6094 WHERE `entry` = 59467; -- Candlestick Mage
UPDATE `creature_template` SET `mingold` = 8396, `maxgold` = 8396 WHERE `entry` = 59613; -- Professor Slate
UPDATE `creature_template` SET `mingold` = 6749, `maxgold` = 6749 WHERE `entry` = 59614; -- Bored Student

-- From 27_88 / 27_86: no difficulty row, so the template carries the scaled normal gold
UPDATE `creature_template` SET `mingold` = 17, `maxgold` = 17 WHERE `entry` = 48278; -- Mining Monkey (was 2137)
UPDATE `creature_template` SET `mingold` = 107, `maxgold` = 107 WHERE `entry` = 59175; -- Master Archer (was 4121)
