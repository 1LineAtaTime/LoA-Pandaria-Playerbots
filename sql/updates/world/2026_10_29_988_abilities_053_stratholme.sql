-- #988 wave 2 (merge): Stratholme creature abilities from DeltaTho PR 15 file 2026_10_24_66_world.sql, ADDED to creatures that already have smart_scripts.
-- 5 creatures, 5 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 10393:16 10398:12 10413:1 10425:3 11032:6

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 10393 AND `id` >= 16)
  OR (`entryorguid` = 10398 AND `id` >= 12)
  OR (`entryorguid` = 10413 AND `id` >= 1)
  OR (`entryorguid` = 10425 AND `id` >= 3)
  OR (`entryorguid` = 11032 AND `id` >= 6));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(10393, 0, 16, 0, 0, 0, 100, 0, 10000, 20000, 20000, 40000, 0, 11, 8364, 0, 0, 0, 0, 0, 5, 20, 0, 0, 0, 0, 0, 0, 0, 'Skul - In Combat - Cast Blizzard'),
(10398, 0, 12, 0, 1, 0, 100, 0, 6000, 6000, 0, 0, 0, 11, 12380, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Thuzadin Shadowcaster - Out of Combat - Cast Shadow Channeling'),
(10413, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 16428, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Crypt Beast - On Reset - Cast Virulent Poison Proc'),
(10425, 0, 3, 0, 0, 0, 100, 0, 2000, 7000, 11000, 15000, 0, 11, 15732, 32, 0, 0, 0, 0, 5, 20, 0, 0, 0, 0, 0, 0, 0, 'Crimson Battle Mage - In Combat - Cast Immolate'),
(11032, 0, 6, 0, 2, 0, 100, 0, 0, 70, 10000, 10000, 0, 11, 15493, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Malor the Zealous - Between Health 0-50% - Cast Holy Light');
