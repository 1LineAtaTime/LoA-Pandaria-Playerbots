-- #988 wave 2 (merge): Mulgore and Camp Narache creature abilities from DeltaTho PR 15 file 2026_10_24_12_world.sql, ADDED to creatures that already have smart_scripts.
-- 3 creatures, 4 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 3232:1 36845:2 36931:21

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 3232 AND `id` >= 1)
  OR (`entryorguid` = 36845 AND `id` >= 2)
  OR (`entryorguid` = 36931 AND `id` >= 21));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3232, 0, 1, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 81653, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bristleback Interloper - On Respawn - Cast ''Bristleback'' (No Repeat)'),
(3232, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 22000, 24000, 0, 11, 79584, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bristleback Interloper - In Combat - Cast ''Fan of Knives'''),
(36845, 0, 2, 0, 0, 0, 100, 0, 4500, 5500, 18500, 21500, 0, 11, 81305, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Agitated Earth Spirit - In Combat - Cast ''Rock Barrage'''),
(36931, 0, 21, 0, 0, 0, 100, 0, 2000, 4500, 19000, 25000, 0, 11, 77558, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Orno Grimtotem - In Combat - Cast ''Bloody Strike''');
