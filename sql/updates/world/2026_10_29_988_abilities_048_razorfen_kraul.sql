-- #988 wave 2 (merge): Razorfen Kraul creature abilities from DeltaTho PR 15 file 2026_10_24_60_world.sql, ADDED to creatures that already have smart_scripts.
-- 3 creatures, 3 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 4436:2 4442:2 4517:4

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 4436 AND `id` >= 2)
  OR (`entryorguid` = 4442 AND `id` >= 2)
  OR (`entryorguid` = 4517 AND `id` >= 4));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4436, 0, 2, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 11, 7165, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Razorfen Quilguard - On Aggro - Cast Battle Stance'),
(4442, 0, 2, 0, 0, 0, 100, 0, 0, 2000, 120000, 120000, 0, 11, 7164, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Razorfen Defender - In Combat - Cast Defensive Stance'),
(4517, 0, 4, 0, 1, 0, 100, 2, 1000, 1000, 1800000, 1800000, 0, 11, 21562, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Death''s Head Priest - Out of Combat - Cast ''Power Word: Fortitude''');
