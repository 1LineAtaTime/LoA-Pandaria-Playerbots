-- #988 wave 2 (merge): Blackfathom Deeps creature abilities from DeltaTho PR 15 file 2026_10_24_28_world.sql, ADDED to creatures that already have smart_scripts.
-- 3 creatures, 3 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 4805:4 4818:1 4831:6

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 4805 AND `id` >= 4)
  OR (`entryorguid` = 4818 AND `id` >= 1)
  OR (`entryorguid` = 4831 AND `id` >= 6));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4805, 0, 4, 0, 0, 0, 100, 3, 3000, 6000, 0, 0, 0, 11, 15044, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Blackfathom Sea Witch - In Combat - Cast ''Frost Ward'' (No Repeat)'),
(4818, 0, 1, 0, 0, 0, 75, 2, 3000, 5000, 8000, 11000, 0, 11, 22691, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Blindlight Murloc - In Combat - Cast ''Sunder Armor'' (No Repeat) (Normal Dungeon)'),
(4831, 0, 6, 0, 0, 0, 100, 2, 6000, 8500, 9000, 13000, 0, 11, 11831, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Sarevess - In Combat - Cast ''Frost Nova'' (Normal Dungeon)');
