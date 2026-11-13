-- #987 wave 1 (pure additions): Swamp of Sorrows creature abilities, imported from DeltaTho PR 15 file 2026_10_24_54_world.sql.
-- 4 creatures, 10 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '14445,46368,51789,51918';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(14445, 0, 0, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 12533, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Captain Wyrmak - Within 0-5 Range - Cast ''Acid Breath'''),
(14445, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Captain Wyrmak - In Combat - Cast ''Cleave'''),
(46368, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 6951, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Enthralled Makrura - In Combat - Cast ''Decayed Strength'''),
(51789, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 80012, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marshtide Sentry - In Combat - Cast ''Chimera Shot'''),
(51789, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 22910, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marshtide Sentry - In Combat - Cast ''Immolation Trap'''),
(51789, 0, 2, 0, 0, 0, 100, 0, 10000, 13000, 15000, 19000, 11, 80009, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marshtide Sentry - In Combat - Cast ''Serpent Sting'''),
(51789, 0, 3, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marshtide Sentry - In Combat - Cast ''Shoot'''),
(51918, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stonard Grunt - In Combat - Cast ''Revenge'''),
(51918, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 12169, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonard Grunt - In Combat - Cast ''Shield Block'''),
(51918, 0, 2, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stonard Grunt - In Combat - Cast ''Shoot''');
