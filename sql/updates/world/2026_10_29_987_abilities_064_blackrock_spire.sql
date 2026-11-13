-- #987 wave 1 (pure additions): Blackrock Spire creature abilities, imported from DeltaTho PR 15 file 2026_10_24_70_world.sql.
-- 8 creatures, 13 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '9218,9219,9416,9596,9701,9718,10177,10264';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(9218, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 10101, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spirestone Battle Lord - In Combat - Cast ''Knock Away'''),
(9218, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 26141, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spirestone Battle Lord - In Combat - Cast ''Hamstring'''),
(9218, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 16856, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spirestone Battle Lord - In Combat - Cast ''Mortal Strike'''),
(9218, 0, 3, 0, 0, 0, 100, 0, 10000, 12000, 16000, 20000, 11, 15655, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spirestone Battle Lord - In Combat - Cast ''Shield Slam'''),
(9219, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 16856, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spirestone Butcher - In Combat - Cast ''Mortal Strike'''),
(9219, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 26141, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spirestone Butcher - In Combat - Cast ''Hamstring'''),
(9416, 0, 0, 0, 2, 0, 100, 1, 40, 80, 0, 0, 11, 3604, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Scarshield Worg - Between 40-80% Health - Cast ''Tendon Rip'' (No Repeat)'),
(9596, 0, 0, 0, 0, 0, 100, 0, 2300, 3200, 9500, 11000, 11, 6466, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Every 9.5 - 11 seconds (2.3 - 3.2s initially)  - Self: Cast spell Axe Toss (6466) on Random hostile'),
(9701, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 25497, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spire Scorpid - In Combat - Cast ''Venom Spit'''),
(9718, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 16856, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ghok Bashguud - In Combat - Cast ''Mortal Strike'''),
(9718, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 24671, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ghok Bashguud - In Combat - Cast ''Snap Kick'''),
(10177, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 25025, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spire Scarab - In Combat - Cast ''Earth Shock'''),
(10264, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 16727, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Solakar Flamewreath - Within 0-8 Range - Cast ''War Stomp''');
