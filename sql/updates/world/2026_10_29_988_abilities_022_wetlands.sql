-- #988 wave 2 (merge): Wetlands creature abilities from DeltaTho PR 15 file 2026_10_24_30_world.sql, ADDED to creatures that already have smart_scripts.
-- 2 creatures, 3 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 1036:6 1038:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 1036 AND `id` >= 6)
  OR (`entryorguid` = 1038 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(1036, 0, 6, 0, 0, 0, 90, 1, 3000, 5000, 4000, 5000, 0, 11, 78660, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dragonmaw Centurion - In Combat - Cast ''Devotion Aura'' (No Repeat)'),
(1036, 0, 7, 0, 0, 0, 70, 0, 1200, 3800, 11900, 13700, 0, 11, 76622, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dragonmaw Centurion - In Combat - Cast ''Shield Bash'' (No Repeat)'),
(1038, 0, 3, 0, 0, 0, 100, 0, 2200, 6200, 20200, 31000, 0, 11, 78648, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dragonmaw Shadowwarder - In Combat - Cast ''Corruption''');
