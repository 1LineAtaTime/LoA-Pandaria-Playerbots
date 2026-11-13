-- #987 wave 1 (pure additions): Zul'Gurub creature abilities, imported from DeltaTho PR 15 file 2026_10_25_29_world.sql.
-- 7 creatures, 13 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '52081,52082,52088,52089,52322,52339,52417';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(52081, 0, 0, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 96413, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gurubashi Cauldron-Mixer - In Combat - Cast ''Gout of Flame'''),
(52082, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 96487, 64, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro - Self: Cast spell 96487 on Self // '),
(52082, 0, 1, 0, 0, 0, 100, 0, 5000, 6000, 2500, 2500, 11, 96804, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 5000 and 6000 ms (and later repeats every 2500 and 2500 ms) - Self: Cast spell 96804 on Victim // '),
(52082, 0, 2, 0, 0, 0, 100, 0, 21000, 21000, 20000, 20500, 11, 96413, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 21000 and 21000 ms (and later repeats every 20000 and 20500 ms) - Self: Cast spell 96413 on Random hostile // '),
(52088, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 96488, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro - Self: Cast spell 96488 on Self // '),
(52088, 0, 1, 0, 0, 0, 100, 1, 5000, 5000, 0, 0, 11, 96449, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 5000 and 5000 ms (and later repeats every 0 and 0 ms) - Self: Cast spell 96449 on Random hostile // '),
(52088, 0, 2, 0, 0, 0, 100, 0, 21000, 21000, 20000, 20500, 11, 96413, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 21000 and 21000 ms (and later repeats every 20000 and 20500 ms) - Self: Cast spell 96413 on Random hostile // '),
(52089, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 96332, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gurubashi Worker - Out of Combat - Cast ''Hypothermia'''),
(52322, 0, 0, 1, 4, 0, 100, 0, 0, 0, 0, 0, 11, 97016, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro - Self: Cast spell 97016 on Self // '),
(52322, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Talk 0 // '),
(52339, 0, 0, 0, 0, 0, 100, 0, 10000, 12000, 32000, 33000, 11, 96790, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 10000 and 12000 ms (and later repeats every 32000 and 33000 ms) - Self: Cast spell 96790 on Self // '),
(52339, 0, 1, 0, 0, 0, 100, 0, 9000, 10000, 30000, 31000, 11, 96849, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 9000 and 10000 ms (and later repeats every 30000 and 31000 ms) - Self: Cast spell 96849 on Self // '),
(52417, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 12097, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shredtooth Frenzy - In Combat - Cast ''Pierce Armor''');
