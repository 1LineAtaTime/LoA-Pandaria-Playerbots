-- #987 wave 1 (pure additions): The Veiled Stair creature abilities, imported from DeltaTho PR 15 file 2026_10_25_39_world.sql.
-- 2 creatures, 3 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '62902,62926';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(62902, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 16636, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Darkhatched Throatripper - Within 8-25 Range - Cast ''Berserker Charge'''),
(62926, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 18000, 24000, 11, 12024, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Exchange Guard - In Combat - Cast ''Net'''),
(62926, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 23337, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Exchange Guard - In Combat - Cast ''Shoot''');
