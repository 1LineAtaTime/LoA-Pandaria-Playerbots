-- #988 wave 2 (merge): Zul'Gurub creature abilities from DeltaTho PR 15 file 2026_10_25_29_world.sql, ADDED to creatures that already have smart_scripts.
-- 2 creatures, 3 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 52086:5 52381:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 52086 AND `id` >= 5)
  OR (`entryorguid` = 52381 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(52086, 0, 5, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 45104, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On reset (e.g. after reaching home) - Self: Cast spell 45104 on Self // '),
(52086, 0, 6, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 96418, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On death - Self: Cast spell 96418 on Self // '),
(52381, 0, 1, 0, 0, 0, 100, 0, 7000, 7000, 14000, 14000, 0, 11, 96475, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 7000 and 7000 ms (and later repeats every 14000 and 14000 ms) - Self: Cast spell 96475 on Self // ');
