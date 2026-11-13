-- #988 wave 2 (merge): Southern Barrens creature abilities from DeltaTho PR 15 file 2026_10_24_39_world.sql, ADDED to creatures that already have smart_scripts.
-- 13 creatures, 17 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 3255:4 3256:4 3261:2 3275:3 3374:2 3375:2 3461:1 5829:1 5859:3 37090:2 37091:2 37171:3 37659:6

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 3255 AND `id` >= 4)
  OR (`entryorguid` = 3256 AND `id` >= 4)
  OR (`entryorguid` = 3261 AND `id` >= 2)
  OR (`entryorguid` = 3275 AND `id` >= 3)
  OR (`entryorguid` = 3374 AND `id` >= 2)
  OR (`entryorguid` = 3375 AND `id` >= 2)
  OR (`entryorguid` = 3461 AND `id` >= 1)
  OR (`entryorguid` = 5829 AND `id` >= 1)
  OR (`entryorguid` = 5859 AND `id` >= 3)
  OR (`entryorguid` = 37090 AND `id` >= 2)
  OR (`entryorguid` = 37091 AND `id` >= 2)
  OR (`entryorguid` = 37171 AND `id` >= 3)
  OR (`entryorguid` = 37659 AND `id` >= 6));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3255, 0, 4, 0, 0, 0, 100, 0, 4000, 7000, 18000, 24000, 0, 11, 82608, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunscale Screecher - In Combat - Cast ''Howling Screech'''),
(3255, 0, 5, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 44531, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunscale Screecher - On Aggro - Cast ''Dash'' (No Repeat)'),
(3256, 0, 4, 0, 0, 0, 60, 0, 15000, 15000, 20000, 20000, 0, 11, 12787, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sunscale Scytheclaw - In Combat - Cast ''Thrash'' (No Repeat)'),
(3261, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9739, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bristleback Thornweaver - In Combat - Cast ''Wrath'''),
(3275, 0, 3, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Kolkar Marauder - On Aggro - Cast ''Rushing Charge'' (No Repeat)'),
(3275, 0, 4, 0, 0, 0, 100, 0, 5000, 7000, 25000, 29000, 0, 11, 8014, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Kolkar Marauder - In Combat - Cast ''Tetanus'''),
(3374, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 12000, 14000, 0, 11, 57846, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Bael''dun Excavator - In Combat - Cast ''Heroic Strike'''),
(3375, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 32064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bael''dun Foreman - On Aggro - Cast ''Battle Shout'' (No Repeat)'),
(3461, 0, 1, 0, 0, 0, 100, 0, 2000, 7000, 18000, 26000, 0, 11, 80604, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Oasis Snapjaw - In Combat - Cast ''Crushing Bite'''),
(5829, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 12000, 15000, 0, 11, 3604, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Snort the Heckler - In Combat - Cast ''Tendon Rip'''),
(5859, 0, 3, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 7165, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hagg Taurenbane - On Aggro - Cast ''Battle Stance'' (No Repeat)'),
(37090, 0, 2, 0, 0, 0, 100, 0, 4000, 5000, 15000, 21000, 0, 11, 80608, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Deviate Terrortooth - In Combat - Cast ''Summon Pebble'''),
(37091, 0, 2, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 80807, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Deviate Plainstrider - On Respawn - Cast ''Sand Blast'' (No Repeat)'),
(37171, 0, 3, 4, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Escaped Stonetalon Prisoner - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(37171, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Escaped Stonetalon Prisoner - Between 0-30% Health - Say Line 0 (No Repeat)'),
(37659, 0, 6, 0, 9, 0, 100, 0, 0, 5, 12000, 13000, 0, 11, 70316, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Field Defender - Within 0-5 Range - Cast ''Cleave'''),
(37659, 0, 7, 0, 0, 0, 100, 0, 4000, 6000, 16000, 21000, 0, 11, 29426, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Field Defender - In Combat - Cast ''Heroic Strike''');
