-- #988 wave 2 (merge): Old Hillsbrad Foothills creature abilities from DeltaTho PR 15 file 2026_10_24_85_world.sql, ADDED to creatures that already have smart_scripts.
-- 1 creatures, 1 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 28132:5

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 28132 AND `id` >= 5));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(28132, 0, 5, 0, 9, 0, 100, 2, 0, 10, 20000, 20000, 0, 11, 50733, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Don Carlos - In Combat - Cast Scatter Shot');
