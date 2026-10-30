-- #987 wave 1 (pure additions): Arathi Highlands creature abilities, imported from DeltaTho PR 15 file 2026_10_24_33_world.sql.
-- 13 creatures, 23 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '2559,2581,2582,2618,2621,2773,2779,19905,24477,41295,42090,51819,51899';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(2559, 0, 0, 0, 0, 0, 100, 0, 4000, 6000, 12000, 15000, 11, 13445, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Rend'),
(2559, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Rushing Charge on Aggro'),
(2581, 0, 0, 0, 13, 0, 100, 0, 20000, 30000, 0, 0, 11, 82800, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Shield Bash on Player Spell Cast'),
(2582, 0, 0, 0, 0, 0, 100, 0, 8000, 8000, 22000, 23000, 11, 80382, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 'Cast Dirt Toss'),
(2582, 0, 1, 0, 0, 0, 100, 0, 5000, 5000, 12000, 12500, 11, 6016, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Pierce Armor'),
(2618, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 15572, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Sunder Armor'),
(2621, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2000, 4000, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hammerfall Guardian - In Combat - Cast ''Shoot'''),
(2621, 0, 1, 0, 9, 0, 100, 0, 0, 5, 11000, 14000, 11, 12169, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hammerfall Guardian - Within 0-5 Range - Cast ''Shield Block'''),
(2621, 0, 2, 0, 0, 0, 100, 0, 5000, 9000, 8000, 13000, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hammerfall Guardian - In Combat - Cast ''Revenge'''),
(2773, 0, 0, 0, 9, 0, 100, 0, 10, 40, 15000, 25000, 11, 92072, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Impaling Pull on Close'),
(2773, 0, 1, 0, 9, 0, 100, 0, 0, 8, 15000, 25000, 11, 84709, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Stomp on Close'),
(2779, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 43107, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Prince Nazjak - On Aggro - Cast ''Spear Throw'' (No Repeat)'),
(2779, 0, 1, 0, 0, 0, 100, 0, 3800, 5400, 10900, 13100, 11, 17687, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Prince Nazjak - In Combat - Cast ''Flurry'''),
(19905, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 15284, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Cleave'),
(24477, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 7159, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Syndicate Thief - In Combat - Cast ''Backstab'''),
(24477, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 6713, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Syndicate Thief - In Combat - Cast ''Disarm'''),
(24477, 0, 2, 0, 0, 0, 100, 0, 10000, 12000, 16000, 20000, 11, 744, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Syndicate Thief - In Combat - Cast ''Poison'''),
(41295, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 79175, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ocean Crawler - In Combat - Cast ''Slap & Chop'''),
(42090, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Arathi Deathstalker - In Combat - Cast ''Shoot'''),
(51819, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Arathi Deathstalker - In Combat - Cast ''Shoot'''),
(51899, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hammerfall Guardian - In Combat - Cast ''Revenge'''),
(51899, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 12169, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hammerfall Guardian - In Combat - Cast ''Shield Block'''),
(51899, 0, 2, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hammerfall Guardian - In Combat - Cast ''Shoot''');
