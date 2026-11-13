-- #987 wave 1 (pure additions): The Botanica creature abilities, imported from DeltaTho PR 15 file 2026_10_24_90_world.sql.
-- 5 creatures, 5 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '18587,19513,19557,19598,19608';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(18587, 0, 0, 0, 1, 0, 100, 1, 500, 500, 0, 0, 11, 34201, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frayer - Out of Combat - Cast Shift Form'),
(19513, 0, 0, 0, 0, 0, 100, 0, 8000, 24000, 28000, 36000, 11, 30584, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Mutate Fear-Shrieker - In Combat - Cast ''Fear'''),
(19557, 0, 0, 0, 0, 0, 100, 0, 3950, 12050, 1200, 13600, 11, 34644, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Greater Frayer - In Combat - Cast ''Lash'''),
(19598, 0, 0, 0, 0, 0, 100, 0, 4850, 13350, 1100, 17900, 11, 34351, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mutate Fleshlasher - In Combat - Cast ''Vicious Bite'''),
(19608, 0, 0, 0, 0, 0, 100, 0, 3850, 11250, 1050, 14350, 11, 34644, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frayer Wildling - In Combat - Cast ''Lash''');
