-- #987 wave 1 (pure additions): The Temple of Atal'Hakkar creature abilities, imported from DeltaTho PR 15 file 2026_10_24_69_world.sql.
-- 6 creatures, 10 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '5399,5400,8319,46068,46424,46623';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(5399, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Veyzhak the Cannibal - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(5400, 0, 0, 0, 0, 0, 100, 0, 0, 5000, 60000, 60000, 11, 8282, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Every 60 - 60 seconds (0 - 5s initially) - Self: Cast spell Curse of Blood (8282) on Victim'),
(5400, 0, 1, 0, 0, 0, 100, 0, 5000, 10000, 120000, 120000, 11, 7102, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Every 120 - 120 seconds (5 - 10s initially) - Self: Cast spell Contagion of Rot (7102) on Victim'),
(8319, 0, 0, 0, 0, 0, 100, 0, 2400, 4900, 11800, 17200, 11, 15653, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Nightmare Whelp - In Combat - Cast ''Acid Spit'''),
(46068, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 81109, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Guardian - In Combat - Cast ''Poison Bolt'''),
(46068, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 11639, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Guardian - In Combat - Cast ''Shadow Word: Pain'''),
(46424, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 18266, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Priestess Udum''bra - In Combat - Cast ''Curse of Agony'''),
(46424, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Priestess Udum''bra - In Combat - Cast ''Shadow Bolt'''),
(46623, 0, 0, 0, 0, 0, 100, 0, 2000, 5000, 20000, 25000, 11, 79893, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gomora the Bloodletter - In Combat - Cast ''Bloodworm'''),
(46623, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 88844, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gomora the Bloodletter - In Combat - Cast ''Cauterizing Strike''');
