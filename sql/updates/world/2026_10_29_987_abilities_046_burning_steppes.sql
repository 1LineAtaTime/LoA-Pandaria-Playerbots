-- #987 wave 1 (pure additions): Burning Steppes creature abilities, imported from DeltaTho PR 15 file 2026_10_24_51_world.sql.
-- 20 creatures, 36 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '7031,7032,7055,8917,47782,47805,48100,48118,48120,48153,48156,48287,48288,48289,48291,48316,48413,48432,51539,51540';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(7031, 0, 0, 0, 2, 0, 100, 1, 0, 40, 0, 0, 11, 9941, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Obsidian Reflection at 40% HP'),
(7032, 0, 0, 0, 2, 0, 100, 1, 0, 40, 0, 0, 11, 9941, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Obsidian Reflection at 40% HP'),
(7055, 0, 0, 0, 0, 0, 100, 0, 5000, 7000, 12000, 13000, 11, 32919, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Snarl'),
(8917, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 5000, 8000, 11, 14516, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Quarry Slave - In Combat - Cast Strike'),
(8917, 0, 1, 0, 0, 0, 100, 0, 7000, 9000, 13000, 16000, 11, 9080, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Quarry Slave - In Combat - Cast Hamstring'),
(47782, 0, 0, 0, 2, 0, 100, 1, 0, 40, 0, 0, 11, 82836, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Blood Fury at 40% HP'),
(47782, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 13000, 15000, 11, 11971, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Sunder Armor'),
(47805, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Shadow Bolt'),
(47805, 0, 1, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 11, 79934, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Demon Armor on Spawn'),
(48100, 0, 0, 0, 0, 0, 100, 0, 9000, 9000, 24000, 27000, 11, 89459, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Cast Hex'),
(48100, 0, 1, 0, 0, 0, 100, 0, 5000, 5000, 27000, 32000, 11, 15802, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Shrink'),
(48100, 0, 2, 0, 2, 0, 100, 0, 0, 40, 22000, 25000, 11, 17173, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Drain Life at 40% HP'),
(48100, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 21067, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Poison Bolt'),
(48118, 0, 0, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 11, 12550, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Lightning Shield on Spawn'),
(48118, 0, 1, 0, 16, 0, 100, 0, 12550, 1, 15000, 30000, 11, 12550, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Lightning Shield on Repeat'),
(48118, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 15039, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Flame Shock'),
(48120, 0, 0, 0, 4, 0, 20, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Aggro'),
(48120, 0, 1, 0, 0, 0, 100, 0, 7000, 7000, 13000, 17000, 11, 80182, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Uppercut'),
(48153, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 3248, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Goblin Flamethrower - In Combat - Cast ''Improved Blocking'''),
(48156, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 80549, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Gout of Flame'),
(48287, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 74613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Shoot'),
(48287, 0, 1, 0, 0, 0, 100, 0, 7000, 8000, 14000, 15000, 11, 83014, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 'Cast Rapid Shot'),
(48287, 0, 2, 0, 2, 0, 100, 1, 0, 55, 0, 0, 11, 91297, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Summon Whelps at 55% HP'),
(48288, 0, 0, 0, 0, 0, 100, 0, 6500, 8000, 18000, 19500, 11, 18543, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Flame Lash'),
(48289, 0, 0, 0, 0, 0, 100, 0, 7000, 8000, 17000, 18000, 11, 8873, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Flame Breath'),
(48289, 0, 1, 0, 0, 0, 100, 0, 5000, 5500, 12000, 13500, 11, 32914, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Wing Buffet'),
(48291, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 22120, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Charge on Aggro'),
(48291, 0, 1, 0, 0, 0, 100, 0, 8000, 8000, 17000, 22000, 11, 91294, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Whirling Blade'),
(48316, 0, 0, 0, 0, 0, 100, 0, 5000, 6000, 12000, 13000, 11, 32736, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Mortal Strike'),
(48316, 0, 1, 0, 0, 0, 100, 0, 8000, 8000, 14000, 18000, 11, 9080, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Hamstring'),
(48413, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Shadow Bolt'),
(48432, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Shoot'),
(51539, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Flame Crest Bruiser - In Combat - Cast ''Cleave'''),
(51539, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Flame Crest Bruiser - In Combat - Cast ''Shoot'''),
(51540, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Vigil Guard - In Combat - Cast ''Cleave'''),
(51540, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Vigil Guard - In Combat - Cast ''Shoot''');
