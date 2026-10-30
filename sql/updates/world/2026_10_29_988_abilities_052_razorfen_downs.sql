-- #988 wave 2 (merge): Razorfen Downs creature abilities from DeltaTho PR 15 file 2026_10_24_65_world.sql, ADDED to creatures that already have smart_scripts.
-- 5 creatures, 5 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 7332:4 7347:1 7348:1 7352:1 7353:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 7332 AND `id` >= 4)
  OR (`entryorguid` = 7347 AND `id` >= 1)
  OR (`entryorguid` = 7348 AND `id` >= 1)
  OR (`entryorguid` = 7352 AND `id` >= 1)
  OR (`entryorguid` = 7353 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(7332, 0, 4, 0, 0, 0, 100, 0, 4000, 9000, 12000, 20000, 0, 11, 11397, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 0, 'Withered Spearhide - In Combat - Cast Diseased Shot'),
(7347, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 12539, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Boneflayer Ghoul - On Reset - Cast Ghoul Rot'),
(7348, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 12539, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Thorn Eater Ghoul - On Reset - Cast Ghoul Rot'),
(7352, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 12529, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Frozen Soul - On Reset - Cast Chilling Touch'),
(7353, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 12529, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Freezing Spirit - On Reset - Cast Chilling Touch');
