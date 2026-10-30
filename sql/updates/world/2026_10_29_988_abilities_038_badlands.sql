-- #988 wave 2 (merge): Badlands creature abilities from DeltaTho PR 15 file 2026_10_24_47_world.sql, ADDED to creatures that already have smart_scripts.
-- 5 creatures, 5 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 2717:2 2729:2 2739:3 2740:3 2743:2

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 2717 AND `id` >= 2)
  OR (`entryorguid` = 2729 AND `id` >= 2)
  OR (`entryorguid` = 2739 AND `id` >= 3)
  OR (`entryorguid` = 2740 AND `id` >= 3)
  OR (`entryorguid` = 2743 AND `id` >= 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(2717, 0, 2, 0, 9, 0, 100, 0, 0, 5, 17000, 27000, 0, 11, 90208, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Mauler Mash on Close'),
(2729, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 32919, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Snarl'),
(2739, 0, 3, 0, 0, 0, 100, 0, 5000, 5000, 12000, 13000, 0, 11, 76622, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Sunder Armor'),
(2740, 0, 3, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79934, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Demon Armor on Spawn'),
(2743, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 32064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Battle Shout on Aggro');
