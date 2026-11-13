-- #987 wave 1 (pure additions): Westfall creature abilities, imported from DeltaTho PR 15 file 2026_10_24_17_world.sql.
-- 18 creatures, 30 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '487,488,489,490,596,831,869,870,874,876,1216,42342,42357,42407,42653,42669,42677,51915';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(487, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Protector Bialon - In Combat - Cast ''Shoot'''),
(488, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Protector Weaver - In Combat - Cast ''Shoot'''),
(489, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Protector Dutfield - In Combat - Cast ''Shoot'''),
(490, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Protector Gariel - In Combat - Cast ''Shoot'''),
(596, 0, 0, 0, 0, 0, 100, 0, 2300, 4100, 7400, 9800, 11, 20883, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Brainwashed Noble - In Combat - Cast ''Arcane Blast'''),
(596, 0, 1, 0, 2, 0, 100, 1, 30, 60, 0, 0, 11, 6117, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Brainwashed Noble - Between 30-60% Health - Cast ''Mage Armor'' (No Repeat)'),
(831, 0, 0, 0, 0, 0, 100, 0, 4000, 7200, 13500, 18900, 11, 80380, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sea Crawler - In Combat - Cast ''Sand Breath'''),
(831, 0, 1, 0, 0, 0, 100, 0, 8500, 12300, 22000, 34000, 11, 78744, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sea Crawler - In Combat - Cast ''Bubblebeam'''),
(869, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Protector Dorana - In Combat - Cast ''Shoot'''),
(870, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Protector Deni - In Combat - Cast ''Shoot'''),
(874, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Protector Korelor - In Combat - Cast ''Shoot'''),
(876, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Protector Leick - In Combat - Cast ''Shoot'''),
(1216, 0, 0, 0, 0, 0, 100, 0, 4000, 7200, 13500, 18900, 11, 80380, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shore Crawler - In Combat - Cast ''Sand Breath'''),
(1216, 0, 1, 0, 0, 0, 100, 0, 8500, 12300, 22000, 34000, 11, 78744, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shore Crawler - In Combat - Cast ''Bubblebeam'''),
(1216, 0, 2, 0, 2, 0, 100, 1, 0, 42, 0, 0, 11, 79840, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shore Crawler - Between 0-42% Health - Cast ''Harden'' (No Repeat)'),
(42342, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 15000, 20000, 11, 7342, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Energized Harvest Reaper - In Combat - Cast ''Wide Slash'''),
(42342, 0, 1, 0, 2, 0, 100, 0, 0, 40, 25000, 29000, 11, 80572, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Energized Harvest Reaper - Between 0-40% Health - Cast ''Energize!'''),
(42357, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hulking Goretusk - On Aggro - Cast ''Rushing Charge'' (No Repeat)'),
(42357, 0, 1, 0, 0, 0, 100, 0, 5500, 7800, 18500, 26300, 11, 32019, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hulking Goretusk - In Combat - Cast ''Gore'''),
(42407, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sentinel Hill Guard - In Combat - Cast ''Revenge'''),
(42407, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 12169, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sentinel Hill Guard - In Combat - Cast ''Shield Block'''),
(42407, 0, 2, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sentinel Hill Guard - In Combat - Cast ''Shoot'''),
(42653, 0, 0, 0, 2, 0, 100, 0, 0, 35, 18000, 25000, 11, 79853, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Jango Spothide - Between 0-35% Health - Cast ''Evasion'''),
(42653, 0, 1, 0, 0, 0, 100, 0, 3500, 9000, 24000, 28000, 11, 3136, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Jango Spothide - In Combat - Cast ''Frenzied Command'''),
(42669, 0, 0, 0, 2, 0, 100, 1, 0, 40, 0, 0, 11, 80570, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Chasm Slime - Between 0-40% Health - Cast ''Mimic'' (No Repeat)'),
(42677, 0, 0, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Moonbrook Thug - Between 0-15% Health - Flee For Assist (No Repeat)'),
(42677, 0, 1, 0, 0, 0, 100, 0, 5500, 7800, 18500, 21200, 11, 80146, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Moonbrook Thug - In Combat - Cast ''Bonk'''),
(51915, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Westfall Brigade Guard - In Combat - Cast ''Revenge'''),
(51915, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 12169, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Westfall Brigade Guard - In Combat - Cast ''Shield Block'''),
(51915, 0, 2, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Westfall Brigade Guard - In Combat - Cast ''Shoot''');
