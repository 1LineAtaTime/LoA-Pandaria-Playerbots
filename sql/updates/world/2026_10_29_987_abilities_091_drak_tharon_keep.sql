-- #987 wave 1 (pure additions): Drak'Tharon Keep creature abilities, imported from DeltaTho PR 15 file 2026_10_25_06_world.sql.
-- 1 creatures, 4 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '27431';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(27431, 0, 0, 0, 0, 0, 100, 0, 0, 0, 30000, 30000, 11, 49724, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Commander - In Combat - Cast Battle Shout'),
(27431, 0, 1, 0, 0, 0, 100, 2, 4000, 7000, 8000, 10000, 11, 49807, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Commander - In Combat - Cast Whirlwind'),
(27431, 0, 2, 0, 0, 0, 100, 4, 4000, 7000, 8000, 10000, 11, 24236, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Commander - In Combat - Cast Whirlwind'),
(27431, 0, 3, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8269, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Drakkari Commander - Between Health 0-30% - Cast Frenzy');
