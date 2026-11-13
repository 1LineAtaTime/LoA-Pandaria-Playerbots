-- #987 wave 1 (pure additions): Razorfen Downs creature abilities, imported from DeltaTho PR 15 file 2026_10_24_65_world.sql.
-- 3 creatures, 6 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '7357,8567,14686';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(7357, 0, 0, 0, 0, 0, 100, 0, 0, 1000, 3000, 3500, 11, 12466, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mordresh Fire Eye - In Combat - Cast Fireball'),
(7357, 0, 1, 0, 9, 0, 100, 0, 0, 9, 10000, 16000, 11, 12470, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mordresh Fire Eye - Within Range 0-9yd - Cast Fire Nova'),
(8567, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 12627, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Glutton - On Reset - Cast Disease Cloud'),
(14686, 0, 0, 0, 37, 0, 100, 0, 0, 0, 0, 0, 11, 28533, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lady Falther''ess - On Initialize - Cast ''Transform'' (Salma Saldean)'),
(14686, 0, 1, 0, 0, 0, 100, 0, 2500, 8000, 10000, 18000, 11, 22743, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Lady Falther''ess - In Combat - Cast ''Ribbon of Souls'''),
(14686, 0, 2, 0, 0, 0, 100, 0, 17500, 20000, 19000, 22000, 11, 17105, 0, 0, 0, 0, 0, 5, 30, 1, 2, 0, 0, 0, 0, 'Lady Falther''ess - In Combat - Cast ''Banshee Curse''');
