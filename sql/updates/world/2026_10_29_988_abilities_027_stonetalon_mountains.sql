-- #988 wave 2 (merge): Stonetalon Mountains creature abilities from DeltaTho PR 15 file 2026_10_24_35_world.sql, ADDED to creatures that already have smart_scripts.
-- 3 creatures, 4 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 11917:1 11918:1 41437:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 11917 AND `id` >= 1)
  OR (`entryorguid` = 11918 AND `id` >= 1)
  OR (`entryorguid` = 41437 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(11917, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 20793, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Boulderslide Geomancer - In Combat - Cast ''Fireball'''),
(11917, 0, 2, 0, 0, 0, 100, 0, 5000, 9000, 25000, 27000, 0, 11, 81309, 2, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 'Boulderslide Geomancer - In Combat - Cast ''Cave-in'''),
(11918, 0, 1, 0, 9, 0, 100, 0, 0, 20, 12000, 19000, 0, 11, 81361, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Boulderslide Stonepounder - Within 0-20 Range - Cast ''Ground Spike'''),
(41437, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 81505, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Pump Controller - On Aggro - Cast ''Rocket Launched Support Shredder'' (No Repeat)');
