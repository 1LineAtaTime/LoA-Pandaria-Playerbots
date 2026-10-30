-- #988 wave 2 (merge): Northern Barrens creature abilities from DeltaTho PR 15 file 2026_10_24_22_world.sql, ADDED to creatures that already have smart_scripts.
-- 10 creatures, 13 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 3265:2 3268:1 3270:3 3271:2 3396:2 3452:1 3672:3 5835:1 34846:2 44164:4

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 3265 AND `id` >= 2)
  OR (`entryorguid` = 3268 AND `id` >= 1)
  OR (`entryorguid` = 3270 AND `id` >= 3)
  OR (`entryorguid` = 3271 AND `id` >= 2)
  OR (`entryorguid` = 3396 AND `id` >= 2)
  OR (`entryorguid` = 3452 AND `id` >= 1)
  OR (`entryorguid` = 3672 AND `id` >= 3)
  OR (`entryorguid` = 5835 AND `id` >= 1)
  OR (`entryorguid` = 34846 AND `id` >= 2)
  OR (`entryorguid` = 44164 AND `id` >= 4));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3265, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 6479, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Razormane Hunter- On Reset - Cast to summon Razormane Wolf'),
(3268, 0, 1, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 21335, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Razormane Thornweaver - Out of Combat - Cast ''Thorns'''),
(3270, 0, 3, 0, 0, 0, 100, 0, 4000, 5000, 18000, 22000, 0, 11, 75068, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Elder Mystic Razorsnout - In Combat - Cast ''Lava Burst'''),
(3270, 0, 4, 0, 0, 0, 100, 0, 3000, 6000, 15000, 27000, 0, 11, 39591, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Elder Mystic Razorsnout - In Combat - Cast ''Searing Totem'''),
(3271, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Razormane Mystic - In Combat - Cast ''Shadow Bolt'''),
(3396, 0, 2, 0, 0, 0, 100, 0, 8000, 9000, 21000, 22000, 0, 11, 6958, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Hezrul Bloodmark - In Combat - Cast ''Blood Leech'''),
(3452, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 11000, 22000, 0, 11, 39068, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Serena Bloodfeather - In Combat - Cast ''Feather Burst'''),
(3672, 0, 3, 0, 2, 0, 100, 0, 0, 40, 30000, 40000, 0, 11, 20790, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Boahn - Between 0-40% Health - Cast ''Healing Touch'''),
(5835, 0, 1, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 7165, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Foreman Grills - On Respawn - Cast ''Battle Stance'' (No Repeat)'),
(5835, 0, 2, 0, 0, 0, 100, 0, 3000, 5000, 17000, 25000, 0, 11, 6531, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Foreman Grills - In Combat - Cast ''Overseer''s Poison'''),
(34846, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9532, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Wyneth - In Combat - Cast ''Lightning Bolt'''),
(34846, 0, 3, 0, 0, 0, 100, 0, 5000, 7000, 18000, 26000, 0, 11, 6535, 1, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 'Wyneth - In Combat - Cast ''Lightning Cloud'''),
(44164, 0, 4, 0, 0, 0, 100, 0, 4000, 7000, 18000, 24000, 0, 11, 82608, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunscale Ravager - In Combat - Cast ''Howling Screech''');
