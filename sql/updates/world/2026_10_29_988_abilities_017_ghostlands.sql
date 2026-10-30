-- #988 wave 2 (merge): Ghostlands creature abilities from DeltaTho PR 15 file 2026_10_24_24_world.sql, ADDED to creatures that already have smart_scripts.
-- 2 creatures, 2 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 16318:15 16345:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 16318 AND `id` >= 15)
  OR (`entryorguid` = 16345 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(16318, 0, 15, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 28729, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Deatholme Darkmage - On reset - Cast Ribbon of Souls'),
(16345, 0, 1, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 0, 11, 28904, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Shadowpine Catlord - Out of Combat - Cast ''Summon Ghostclaw Lynx'' (No Repeat)');
