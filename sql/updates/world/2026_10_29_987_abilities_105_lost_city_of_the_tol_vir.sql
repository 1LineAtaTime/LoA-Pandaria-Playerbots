-- #987 wave 1 (pure additions): Lost City of the Tol'vir creature abilities, imported from DeltaTho PR 15 file 2026_10_25_28_world.sql.
-- 2 creatures, 2 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '44261,45065';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(44261, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 89212, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sharptalon Eagle - In Combat - Cast ''Eagle Claw'''),
(45065, 0, 0, 0, 0, 0, 100, 0, 1, 1, 1000, 1000, 11, 84144, 64, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 1 and 1 ms (and later repeats every 1000 and 1000 ms) - Self: Cast spell 84144 on Random hostile // ');
