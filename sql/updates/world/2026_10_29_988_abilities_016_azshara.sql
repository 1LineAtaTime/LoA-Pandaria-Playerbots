-- #988 wave 2 (merge): Azshara creature abilities from DeltaTho PR 15 file 2026_10_24_23_world.sql, ADDED to creatures that already have smart_scripts.
-- 8 creatures, 18 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 8761:2 35111:10 35759:2 35880:4 35881:4 36131:2 36638:2 36873:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 8761 AND `id` >= 2)
  OR (`entryorguid` = 35111 AND `id` >= 10)
  OR (`entryorguid` = 35759 AND `id` >= 2)
  OR (`entryorguid` = 35880 AND `id` >= 4)
  OR (`entryorguid` = 35881 AND `id` >= 4)
  OR (`entryorguid` = 36131 AND `id` >= 2)
  OR (`entryorguid` = 36638 AND `id` >= 2)
  OR (`entryorguid` = 36873 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(8761, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 22120, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Mosshoof Courser - On Aggro - Cast ''Charge'' (No Repeat)'),
(8761, 0, 3, 0, 0, 0, 100, 0, 4000, 7000, 15000, 18000, 0, 11, 31274, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mosshoof Courser - In Combat - Cast ''Knockdown'''),
(35111, 0, 10, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 32735, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Runaway Shredder - On Aggro - Cast ''Saw Blade'' (No Repeat)'),
(35759, 0, 2, 0, 0, 0, 100, 0, 4000, 6000, 15000, 18000, 0, 11, 80835, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Balboa - In Combat - Cast ''Thundering Stomp'''),
(35880, 0, 4, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Silisthra - In Combat - Cast ''Shoot'''),
(35880, 0, 5, 0, 9, 0, 100, 0, 5, 30, 12000, 18000, 0, 11, 15495, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Silisthra - Within 5-30 Range - Cast ''Explosive Shot'''),
(35880, 0, 6, 0, 0, 0, 100, 0, 4000, 7000, 15000, 22000, 0, 11, 20299, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Silisthra - In Combat - Cast ''Forked Lightning'''),
(35880, 0, 7, 0, 9, 0, 100, 0, 5, 30, 9000, 26000, 0, 11, 12551, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Silisthra - Within 5-30 Range - Cast ''Frost Shot'''),
(35880, 0, 8, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 15507, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Silisthra - Out of Combat - Cast ''Lightning Shield'''),
(35881, 0, 4, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9672, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Vesthra - In Combat - Cast ''Frostbolt'''),
(35881, 0, 5, 0, 9, 0, 100, 0, 0, 8, 13600, 14500, 0, 11, 11831, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Vesthra - Within 0-8 Range - Cast ''Frost Nova'''),
(35881, 0, 6, 0, 9, 0, 100, 0, 0, 25, 15000, 18000, 0, 11, 21067, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Vesthra - Within 0-25 Range - Cast ''Poison Bolt'''),
(35881, 0, 7, 0, 0, 0, 100, 0, 4000, 5000, 14500, 15800, 0, 11, 38663, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Vesthra - In Combat - Cast ''Slow'''),
(35881, 0, 8, 0, 9, 0, 100, 0, 0, 15, 25000, 28000, 0, 11, 81039, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Vesthra - Within 0-15 Range - Cast ''Toxic Nova'''),
(36131, 0, 2, 0, 0, 0, 100, 0, 2000, 3000, 9000, 12000, 0, 11, 32011, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Vile Splash - In Combat - Cast ''Water Bolt'''),
(36638, 0, 2, 0, 2, 0, 100, 0, 0, 60, 28000, 32000, 0, 11, 69135, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Twilight Lord Katrana - Between 0-60% Health - Cast ''Twilight Barrier'''),
(36873, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9734, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'High Priestess Silthera - In Combat - Cast ''Holy Smite'''),
(36873, 0, 2, 0, 0, 0, 100, 0, 5000, 7000, 25000, 28000, 0, 11, 11639, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'High Priestess Silthera - In Combat - Cast ''Shadow Word: Pain''');
