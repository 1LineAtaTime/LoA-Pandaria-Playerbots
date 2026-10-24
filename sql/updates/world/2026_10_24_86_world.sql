-- Sethekk Halls: creature abilities
-- Most of the dungeon's creatures (the Sethekk arakkoa, the Time-Lost Controllers, the Avian Warhawks) had no AI or ran classic
-- rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions
-- or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon
-- (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '18319,18320';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(18319, 0, 0, 0, 1, 0, 100, 7, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Out of Combat - Disable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18319, 0, 8, 9, 3, 1, 100, 7, 0, 15, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Between 0-15% Mana - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18319, 0, 9, 0, 61, 1, 100, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Between 0-15% Mana - Increment Phase By 1 (Phase 1) (No Repeat) (Dungeon)'),
(18319, 0, 10, 0, 9, 1, 100, 7, 25, 80, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Within 25-80 Range - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18319, 0, 11, 0, 9, 1, 100, 7, 5, 15, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Within 5-15 Range - Disable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18319, 0, 12, 0, 9, 1, 100, 7, 0, 5, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Within 0-5 Range - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18319, 0, 13, 0, 3, 0, 100, 6, 30, 100, 100, 100, 23, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Between 30-100% Mana - Decrement Phase By 1 (Phase 1) (No Repeat) (Dungeon)'),
(18319, 0, 14, 0, 14, 0, 100, 2, 5000, 20, 7000, 10000, 11, 17843, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Friendly At 5000 Health - Cast ''Flash Heal'' (Phase 1) (No Repeat) (Dungeon)'),
(18319, 0, 15, 0, 14, 0, 100, 4, 8000, 20, 7000, 10000, 11, 17138, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Friendly At 8000 Health - Cast ''Flash Heal'' (Phase 1) (No Repeat) (Dungeon)'),
(18319, 0, 16, 0, 14, 0, 100, 2, 2000, 20, 8000, 10000, 11, 12160, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Friendly At 2000 Health - Cast ''Rejuvenation'' (Phase 1) (No Repeat) (Dungeon)'),
(18319, 0, 17, 0, 14, 0, 100, 4, 3500, 20, 8000, 10000, 11, 15981, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Friendly At 3500 Health - Cast ''Rejuvenation'' (Phase 1) (No Repeat) (Dungeon)'),
(18319, 0, 18, 0, 1, 0, 100, 0, 1000, 1000, 30000, 30000, 11, 32689, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - Out of Combat - Cast ''Arcane Destruction'''),
(18319, 0, 19, 0, 0, 0, 100, 0, 3100, 5300, 3100, 5300, 11, 32689, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - In Combat - Cast ''Arcane Destruction'''),
(18319, 0, 20, 0, 0, 0, 100, 2, 7800, 13300, 10400, 17700, 11, 22272, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - In Combat - Cast ''Arcane Missiles'' (Normal Dungeon)'),
(18319, 0, 21, 0, 0, 0, 100, 4, 7800, 13300, 10400, 17700, 11, 33988, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Scryer - In Combat - Cast ''Arcane Missiles'' (Heroic Dungeon)'),
(18319, 0, 22, 0, 74, 0, 100, 2, 0, 75, 9600, 15700, 11, 17843, 0, 0, 0, 0, 0, 9, 0, 0, 40, 0, 0, 0, 0, 'Time-Lost Scryer - On Friendly Between 0-75% Health - Cast ''Flash Heal'' (Normal Dungeon)'),
(18319, 0, 23, 0, 74, 0, 100, 4, 0, 75, 9600, 15700, 11, 17138, 0, 0, 0, 0, 0, 9, 0, 0, 40, 0, 0, 0, 0, 'Time-Lost Scryer - On Friendly Between 0-75% Health - Cast ''Flash Heal'' (Heroic Dungeon)'),
(18319, 0, 24, 0, 74, 0, 100, 2, 0, 40, 13200, 21700, 11, 12160, 0, 0, 0, 0, 0, 9, 0, 0, 40, 0, 0, 0, 0, 'Time-Lost Scryer - On Friendly Between 0-40% Health - Cast ''Rejuvenation'' (Normal Dungeon)'),
(18319, 0, 25, 0, 74, 0, 100, 4, 0, 40, 13200, 21700, 11, 15981, 0, 0, 0, 0, 0, 9, 0, 0, 40, 0, 0, 0, 0, 'Time-Lost Scryer - On Friendly Between 0-40% Health - Cast ''Rejuvenation'' (Heroic Dungeon)'),
(18320, 0, 0, 0, 0, 0, 100, 2, 6600, 18600, 9300, 21800, 11, 32675, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Shadowmage - In Combat - Cast ''Shadow Missiles'' (Normal Dungeon)'),
(18320, 0, 1, 0, 0, 0, 100, 4, 6600, 18600, 9300, 21800, 11, 38148, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Shadowmage - In Combat - Cast ''Shadow Missiles'' (Heroic Dungeon)'),
(18320, 0, 2, 0, 0, 0, 100, 2, 4800, 9600, 21700, 33800, 11, 32682, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Shadowmage - In Combat - Cast ''Curse of the Dark Talon'' (Normal Dungeon)'),
(18320, 0, 3, 0, 0, 0, 100, 4, 4800, 9600, 21700, 33800, 11, 38149, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Shadowmage - In Combat - Cast ''Curse of the Dark Talon'' (Heroic Dungeon)');
