-- #988 wave 2 (merge): Hellfire Ramparts creature abilities from DeltaTho PR 15 file 2026_10_24_79_world.sql, ADDED to creatures that already have smart_scripts.
-- 3 creatures, 6 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 17264:1 17269:16 17478:2

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 17264 AND `id` >= 1)
  OR (`entryorguid` = 17269 AND `id` >= 16)
  OR (`entryorguid` = 17478 AND `id` >= 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(17264, 0, 1, 0, 0, 0, 100, 0, 6100, 17200, 20650, 26950, 0, 11, 30621, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Ravener - In Combat - Cast Kidney Shot'),
(17269, 0, 16, 0, 0, 0, 100, 2, 0, 2050, 1750, 2050, 0, 11, 15241, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - In Combat - Cast ''Scorch'' (Normal Dungeon)'),
(17269, 0, 17, 0, 0, 0, 100, 2, 15650, 23000, 20650, 31600, 0, 11, 20754, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - In Combat - Cast ''Rain of Fire'' (Normal Dungeon)'),
(17478, 0, 2, 0, 0, 0, 100, 2, 800, 1700, 3600, 4700, 0, 11, 12471, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Scryer - In Combat - Cast ''Shadow Bolt'' (Normal Dungeon)'),
(17478, 0, 3, 0, 0, 0, 100, 0, 4850, 23500, 15450, 23800, 0, 11, 30615, 1, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Scryer - In Combat - Cast ''Fear'''),
(17478, 0, 4, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 30659, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Scryer - On Death - Cast Fel Infusion');
