-- #988 wave 2 (merge): Mount Hyjal creature abilities from DeltaTho PR 15 file 2026_10_25_14_world.sql, ADDED to creatures that already have smart_scripts.
-- 3 creatures, 6 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 38926:1 39436:1 46925:5

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 38926 AND `id` >= 1)
  OR (`entryorguid` = 39436 AND `id` >= 1)
  OR (`entryorguid` = 46925 AND `id` >= 5));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38926, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 15536, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 0 and 0 ms (and later repeats every 3400 and 4700 ms) - Self: Cast spell 15536 on Victim // Twilight Flamecaller - In Combat - Cast ''Fireball'''),
(38926, 0, 2, 0, 0, 0, 100, 0, 3000, 6000, 11000, 12000, 0, 11, 20794, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 3000 and 6000 ms (and later repeats every 11000 and 12000 ms) - Self: Cast spell 20794 on Victim // Twilight Flamecaller - In Combat - Cast ''Flamestrike'''),
(39436, 0, 1, 0, 0, 0, 100, 0, 5000, 5000, 12000, 14000, 0, 11, 80182, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Twilight Proveditor - In Combat - Cast ''Uppercut'''),
(46925, 0, 5, 0, 0, 0, 100, 0, 5100, 5200, 11300, 12500, 0, 11, 80561, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ashbearer - In Combat - Cast ''Flame Edge'''),
(46925, 0, 6, 0, 0, 0, 100, 0, 9900, 9900, 23400, 26800, 0, 11, 80594, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Ashbearer - In Combat - Cast ''Flame Thrower'''),
(46925, 0, 7, 0, 0, 0, 100, 0, 3000, 4000, 17800, 19200, 0, 11, 18543, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Ashbearer - In Combat - Cast ''Flame Lash''');
