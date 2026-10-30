-- #988 wave 2 (merge): Gilneas and Gilneas City creature abilities from DeltaTho PR 15 file 2026_10_24_10_world.sql, ADDED to creatures that already have smart_scripts.
-- 2 creatures, 3 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 35660:1 47091:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 35660 AND `id` >= 1)
  OR (`entryorguid` = 47091 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35660, 0, 1, 2, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'When health between 0% and 30% (check every 0 - 0 ms) - Self: Cast spell 8599 on Self // Rampaging Worgen - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(35660, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Talk 0 // Rampaging Worgen - Between 0-30% Health - Say Line 0 (No Repeat)'),
(47091, 0, 3, 0, 1, 0, 100, 0, 1, 1, 30000, 30000, 0, 11, 46577, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'When out of combat and timer at the begining between 1 and 1 ms (and later repeats every 30000 and 30000 ms) - Self: Cast spell 46577 on Self // ');
