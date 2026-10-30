-- #988 wave 2 (merge): Elwynn Forest and Northshire creature abilities from DeltaTho PR 15 file 2026_10_24_06_world.sql, ADDED to creatures that already have smart_scripts.
-- 2 creatures, 3 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 1642:1 6866:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 1642 AND `id` >= 1)
  OR (`entryorguid` = 6866 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(1642, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Northshire Guard - In Combat - Cast ''Shoot'''),
(6866, 0, 3, 0, 9, 0, 100, 0, 0, 8, 15800, 18300, 0, 11, 66060, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bodyguard - Within 0-8 Range - Cast ''Sprint'''),
(6866, 0, 4, 0, 0, 0, 100, 0, 5000, 7800, 16800, 17900, 0, 11, 60842, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bodyguard - In Combat - Cast ''Expose Armor''');
