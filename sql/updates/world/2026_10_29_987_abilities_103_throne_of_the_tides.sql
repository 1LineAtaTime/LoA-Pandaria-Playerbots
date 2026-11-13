-- #987 wave 1 (pure additions): Throne of the Tides creature abilities, imported from DeltaTho PR 15 file 2026_10_25_25_world.sql.
-- 2 creatures, 4 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '40584,40943';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(40584, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 76790, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Naz''jar Invader - In Combat - Cast ''Bubble Shield'''),
(40584, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 76807, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Naz''jar Invader - In Combat - Cast ''Lacerate'''),
(40943, 0, 0, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 76560, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gilgoblin Aquamage - Within 0-5 Range - Cast ''Tsunami'''),
(40943, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 76535, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gilgoblin Aquamage - In Combat - Cast ''Water Bolt''');
