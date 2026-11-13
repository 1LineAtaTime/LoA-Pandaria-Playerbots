-- #987 wave 1 (pure additions): Kezan creature abilities, imported from DeltaTho PR 15 file 2026_10_24_14_world.sql.
-- 10 creatures, 12 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '34865,35304,35609,35613,35623,37054,37056,37057,37683,49218';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(34865, 0, 0, 0, 0, 0, 100, 0, 3500, 7700, 13200, 16700, 11, 32738, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Tunneling Worm - In Combat - Cast ''Bore'''),
(35304, 0, 0, 0, 1, 0, 100, 1, 500, 1000, 0, 0, 11, 76136, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Brute Enforcer - Out of Combat - Cast ''Acidic Sweat'' (No Repeat)'),
(35304, 0, 1, 0, 1, 0, 100, 0, 10000, 20000, 40000, 50000, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Brute Enforcer - Out of Combat - Say Line 0'),
(35609, 0, 0, 0, 1, 0, 100, 1, 500, 1000, 0, 0, 11, 76136, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Villa Mook - Out of Combat - Cast ''Acidic Sweat'' (No Repeat)'),
(35609, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 32064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Villa Mook - On Aggro - Cast ''Battle Shout'' (No Repeat)'),
(35613, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 3385, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Keensnout Potbelly - On Aggro - Cast ''Boar Charge'' (No Repeat)'),
(35623, 0, 0, 0, 1, 0, 100, 1, 500, 1000, 0, 0, 11, 76136, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Villa Mook - Out of Combat - Cast ''Acidic Sweat'' (No Repeat)'),
(37054, 0, 0, 0, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 11, 51733, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Goblin Supermodel - Out of Combat - Cast ''Shadow Channelling'' (No Repeat)'),
(37056, 0, 0, 0, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 11, 51733, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Goblin Supermodel - Out of Combat - Cast ''Shadow Channelling'' (No Repeat)'),
(37057, 0, 0, 0, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 11, 51733, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Goblin Supermodel - Out of Combat - Cast ''Shadow Channelling'' (No Repeat)'),
(37683, 0, 0, 0, 1, 0, 100, 1, 500, 1000, 0, 0, 11, 76136, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Villa Mook - Out of Combat - Cast ''Acidic Sweat'' (No Repeat)'),
(49218, 0, 0, 0, 1, 0, 100, 1, 500, 1000, 0, 0, 11, 76136, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Villa Mook - Out of Combat - Cast ''Acidic Sweat'' (No Repeat)');
