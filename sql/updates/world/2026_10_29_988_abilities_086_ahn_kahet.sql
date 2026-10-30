-- #988 wave 2 (merge): Ahn'kahet creature abilities from DeltaTho PR 15 file 2026_10_25_05_world.sql, ADDED to creatures that already have smart_scripts.
-- 4 creatures, 5 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 30111:4 30179:4 30319:5 30338:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 30111 AND `id` >= 4)
  OR (`entryorguid` = 30179 AND `id` >= 4)
  OR (`entryorguid` = 30319 AND `id` >= 5)
  OR (`entryorguid` = 30338 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(30111, 0, 4, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 0, 11, 56711, 32, 0, 0, 0, 0, 19, 30413, 5, 0, 0, 0, 0, 0, 0, 'Twilight Worshipper - Out of Combat - Cast ''Image Channel'' (No Repeat)'),
(30179, 0, 4, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 0, 11, 56711, 32, 0, 0, 0, 0, 19, 30413, 5, 0, 0, 0, 0, 0, 0, 'Twilight Apostle - Out of Combat - Cast ''Image Channel'' (No Repeat)'),
(30319, 0, 5, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 0, 11, 56711, 32, 0, 0, 0, 0, 19, 30413, 5, 0, 0, 0, 0, 0, 0, 'Twilight Darkcaster - Out of Combat - Cast ''Image Channel'' (No Repeat)'),
(30338, 0, 1, 2, 11, 0, 100, 6, 0, 0, 0, 0, 0, 11, 56158, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ahn''kahar Swarmer - In Combat - Cast ''Sprint'''),
(30338, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 11, 56281, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Ahn''kahar Swarmer - On Spawn - Cast Swarm (Dungeon Only)');
