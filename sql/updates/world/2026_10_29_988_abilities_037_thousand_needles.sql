-- #988 wave 2 (merge): Thousand Needles creature abilities from DeltaTho PR 15 file 2026_10_24_46_world.sql, ADDED to creatures that already have smart_scripts.
-- 8 creatures, 22 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 40959:4 41131:2 41196:12 41199:12 47487:1 47602:6 47619:6 47620:6

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 40959 AND `id` >= 4)
  OR (`entryorguid` = 41131 AND `id` >= 2)
  OR (`entryorguid` = 41196 AND `id` >= 12)
  OR (`entryorguid` = 41199 AND `id` >= 12)
  OR (`entryorguid` = 47487 AND `id` >= 1)
  OR (`entryorguid` = 47602 AND `id` >= 6)
  OR (`entryorguid` = 47619 AND `id` >= 6)
  OR (`entryorguid` = 47620 AND `id` >= 6));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(40959, 0, 4, 0, 9, 0, 100, 0, 0, 5, 15000, 17000, 0, 11, 80182, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Den Whomper - Within 0-5 Range - Cast ''Uppercut'''),
(40959, 0, 5, 0, 2, 0, 100, 0, 0, 60, 14000, 16000, 0, 11, 4955, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Den Whomper - Between 0-60% Health - Cast ''Fist of Stone'''),
(40959, 0, 6, 0, 2, 0, 100, 0, 0, 30, 32000, 35000, 0, 11, 21049, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Den Whomper - Between 0-30% Health - Cast ''Bloodlust'''),
(41131, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 13901, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mad Magus Tirth - In Combat - Cast ''Arcane Bolt'''),
(41131, 0, 3, 0, 9, 0, 100, 0, 0, 8, 13600, 14500, 0, 11, 11975, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mad Magus Tirth - Within 0-8 Range - Cast ''Arcane Explosion'''),
(41131, 0, 4, 0, 2, 0, 100, 0, 0, 40, 33000, 35000, 0, 11, 6742, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mad Magus Tirth - Between 0-40% Health - Cast ''Bloodlust'''),
(41131, 0, 5, 0, 0, 0, 100, 0, 5000, 7000, 18000, 20000, 0, 11, 13747, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mad Magus Tirth - In Combat - Cast ''Slow'''),
(41196, 0, 12, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 38557, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Kravel Koalbeard - In Combat - Cast ''Throw'''),
(41199, 0, 12, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Trackmaster Zherin - In Combat - Cast ''Shoot'''),
(41199, 0, 13, 0, 9, 0, 100, 0, 0, 5, 13000, 14000, 0, 11, 57846, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Trackmaster Zherin - Within 0-5 Range - Cast ''Heroic Strike'''),
(47487, 0, 1, 0, 0, 0, 100, 0, 2000, 4500, 12000, 13000, 0, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Twilight Subduer - In Combat - Cast ''Cleave'''),
(47602, 0, 6, 0, 9, 0, 100, 0, 5, 30, 5000, 6000, 0, 11, 79866, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Codemaster Deethuk - Within 5-30 Range - Cast ''Deadly Throw'''),
(47602, 0, 7, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 79864, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Codemaster Deethuk - Between 0-30% Health - Cast ''Shadowstep'' (No Repeat)'),
(47602, 0, 8, 0, 0, 0, 100, 0, 8000, 11000, 23000, 26500, 0, 11, 79863, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Codemaster Deethuk - In Combat - Cast ''Hemorrhage'''),
(47619, 0, 6, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 79564, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Elementalist Starion - In Combat - Cast ''Twilight Fireball'''),
(47619, 0, 7, 0, 0, 0, 100, 0, 5000, 7000, 12000, 14000, 0, 11, 80117, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Elementalist Starion - In Combat - Cast ''Earth Spike'''),
(47619, 0, 8, 0, 0, 0, 100, 0, 9000, 12000, 24000, 27000, 0, 11, 79886, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Elementalist Starion - In Combat - Cast ''Lava Burst'''),
(47619, 0, 9, 0, 0, 0, 100, 0, 4000, 17000, 25000, 36000, 0, 11, 88771, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Elementalist Starion - In Combat - Cast ''Twilight Immolate'''),
(47620, 0, 6, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 32064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Commander Fastfuse - On Aggro - Cast ''Battle Shout'' (No Repeat)'),
(47620, 0, 7, 0, 2, 0, 100, 0, 0, 40, 20000, 21000, 0, 11, 79878, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Commander Fastfuse - Between 0-40% Health - Cast ''Bloodthirst'''),
(47620, 0, 8, 0, 0, 0, 100, 0, 2000, 4500, 16000, 17000, 0, 11, 11977, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Commander Fastfuse - In Combat - Cast ''Rend'''),
(47620, 0, 9, 0, 0, 0, 100, 0, 8000, 9000, 22000, 28000, 0, 11, 79881, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Commander Fastfuse - In Combat - Cast ''Slam''');
