-- #988 wave 2 (merge): The Hinterlands creature abilities from DeltaTho PR 15 file 2026_10_24_41_world.sql, ADDED to creatures that already have smart_scripts.
-- 4 creatures, 6 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 2574:3 2657:2 42536:3 42555:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 2574 AND `id` >= 3)
  OR (`entryorguid` = 2657 AND `id` >= 2)
  OR (`entryorguid` = 42536 AND `id` >= 3)
  OR (`entryorguid` = 42555 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(2574, 0, 3, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 82617, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Heave'),
(2657, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 14000, 15000, 0, 11, 3147, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Trained Razorbeak - In Combat - Cast ''Rend Flesh'''),
(42536, 0, 3, 0, 0, 0, 100, 0, 5000, 7000, 16000, 18000, 0, 11, 80182, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Revantusk Stalker - In Combat - Cast ''Uppercut'''),
(42555, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Revantusk Hexxer - In Combat - Cast ''Shadow Bolt'''),
(42555, 0, 4, 0, 0, 0, 100, 0, 7000, 8000, 30000, 33000, 0, 11, 18267, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Revantusk Hexxer - In Combat - Cast ''Curse of Weakness'''),
(42555, 0, 5, 0, 0, 0, 100, 0, 11000, 12000, 18000, 20000, 0, 11, 16568, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Revantusk Hexxer - In Combat - Cast ''Mind Flay''');
