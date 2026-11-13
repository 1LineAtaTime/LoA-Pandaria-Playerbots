-- #987 wave 1 (pure additions): Zul'Farrak creature abilities, imported from DeltaTho PR 15 file 2026_10_24_67_world.sql.
-- 3 creatures, 8 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '7786,10080,10082';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(7786, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 11998, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skeleton of Zum''rah - In Combat - Cast ''Strike'''),
(7786, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 18663, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skeleton of Zum''rah - In Combat - Cast ''Shadow Shock'''),
(10080, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 13730, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sandarr Dunereaver - On Aggro - Cast ''Demoralizing Shout'''),
(10080, 0, 1, 0, 0, 0, 100, 0, 2700, 3100, 9600, 12300, 11, 14516, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sandarr Dunereaver - In Combat - Cast ''Strike'''),
(10080, 0, 2, 0, 13, 0, 100, 0, 19600, 19600, 0, 0, 11, 15615, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sandarr Dunereaver - In Combat - Cast ''Pummel'''),
(10082, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 6533, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Zerillis - On Aggro - Cast ''Net'''),
(10082, 0, 1, 0, 0, 0, 100, 0, 2100, 2500, 8600, 9400, 11, 15547, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Zerillis - In Combat - Cast ''Shoot'''),
(10082, 0, 2, 0, 2, 0, 100, 1, 20, 40, 0, 0, 11, 12551, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Zerillis - Between 20-40% Health - Cast ''Frost Shot'' (No Repeat)');
