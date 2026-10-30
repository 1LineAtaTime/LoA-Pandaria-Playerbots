-- #988 wave 2 (merge): Searing Gorge creature abilities from DeltaTho PR 15 file 2026_10_24_50_world.sql, ADDED to creatures that already have smart_scripts.
-- 8 creatures, 12 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 5839:2 5840:9 5846:2 5856:7 8391:5 8400:21 8566:2 47311:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 5839 AND `id` >= 2)
  OR (`entryorguid` = 5840 AND `id` >= 9)
  OR (`entryorguid` = 5846 AND `id` >= 2)
  OR (`entryorguid` = 5856 AND `id` >= 7)
  OR (`entryorguid` = 8391 AND `id` >= 5)
  OR (`entryorguid` = 8400 AND `id` >= 21)
  OR (`entryorguid` = 8566 AND `id` >= 2)
  OR (`entryorguid` = 47311 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(5839, 0, 2, 0, 9, 0, 100, 0, 5, 30, 13000, 14500, 0, 11, 8858, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Iron Geologist - Within 5-30 Range - Cast ''Bomb'''),
(5840, 0, 9, 0, 0, 0, 100, 0, 5000, 7000, 15000, 20000, 0, 11, 11983, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Iron Steamsmith - In Combat - Cast ''Steam Jet'''),
(5846, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 22120, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Iron Taskmaster - On Aggro - Cast ''Charge'' (No Repeat)'),
(5846, 0, 3, 0, 0, 0, 100, 0, 5000, 7000, 120000, 125000, 0, 11, 32064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Iron Taskmaster - In Combat - Cast ''Battle Shout'''),
(5856, 0, 7, 0, 0, 0, 100, 0, 4000, 4500, 16000, 20000, 0, 11, 744, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Glassweb Spider - In Combat - Cast ''Poison'''),
(5856, 0, 8, 0, 9, 0, 100, 0, 0, 20, 8000, 9000, 0, 11, 745, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Glassweb Spider - Within 0-20 Range - Cast ''Web'''),
(8391, 0, 5, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lathoric the Black - In Combat - Cast ''Shadow Bolt'''),
(8391, 0, 6, 0, 0, 0, 100, 0, 6000, 9000, 15000, 17000, 0, 11, 32063, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lathoric the Black - In Combat - Cast ''Corruption'''),
(8400, 0, 21, 0, 0, 0, 100, 0, 5000, 8000, 18000, 20000, 0, 11, 84165, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Obsidion - In Combat - Cast ''Flame Blast'''),
(8566, 0, 2, 0, 9, 0, 100, 0, 0, 5, 15000, 17000, 0, 11, 32915, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Iron Lookout - Within 0-5 Range - Cast ''Raptor Strike'''),
(8566, 0, 3, 0, 0, 0, 100, 0, 8000, 11000, 17000, 25000, 0, 11, 14443, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Iron Lookout - In Combat - Cast ''Multi-Shot'''),
(47311, 0, 3, 0, 0, 0, 100, 0, 3000, 12000, 25000, 29000, 0, 11, 78906, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Minister Finister - In Combat - Cast ''Goblin Orderly''');
