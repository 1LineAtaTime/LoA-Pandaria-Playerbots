-- #988 wave 2 (merge): The Arcatraz creature abilities from DeltaTho PR 15 file 2026_10_24_91_world.sql, ADDED to creatures that already have smart_scripts.
-- 5 creatures, 5 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 20866:4 20879:2 20880:1 20882:1 20902:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 20866 AND `id` >= 4)
  OR (`entryorguid` = 20879 AND `id` >= 2)
  OR (`entryorguid` = 20880 AND `id` >= 1)
  OR (`entryorguid` = 20882 AND `id` >= 1)
  OR (`entryorguid` = 20902 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(20866, 0, 4, 0, 2, 0, 100, 0, 0, 30, 14000, 21000, 0, 11, 33958, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Soul Devourer - Between 0-30% Health - Cast ''Enrage'''),
(20879, 0, 2, 0, 0, 0, 100, 0, 15000, 17000, 14000, 21000, 0, 11, 36778, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Eredar Soul-Eater - In Combat - Cast ''Soul Steal'''),
(20880, 0, 1, 0, 0, 0, 100, 0, 6000, 12000, 8000, 14000, 0, 11, 36787, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Eredar Deathbringer - In Combat - Cast ''Forceful Cleave'''),
(20882, 0, 1, 0, 0, 0, 100, 0, 4000, 10000, 10000, 20000, 0, 11, 36862, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Skulking Witch - In Combat - Cast ''Gouge'''),
(20902, 0, 3, 0, 0, 0, 100, 0, 7000, 10000, 13000, 20000, 0, 11, 36829, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Hellcaller - In Combat - Cast ''Hell Rain''');
