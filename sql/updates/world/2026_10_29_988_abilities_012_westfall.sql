-- #988 wave 2 (merge): Westfall creature abilities from DeltaTho PR 15 file 2026_10_24_17_world.sql, ADDED to creatures that already have smart_scripts.
-- 17 creatures, 27 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 154:1 157:1 391:2 449:4 453:3 506:4 519:2 573:1 589:4 594:4 624:1 625:1 833:1 834:2 1065:4 1109:1 1424:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 154 AND `id` >= 1)
  OR (`entryorguid` = 157 AND `id` >= 1)
  OR (`entryorguid` = 391 AND `id` >= 2)
  OR (`entryorguid` = 449 AND `id` >= 4)
  OR (`entryorguid` = 453 AND `id` >= 3)
  OR (`entryorguid` = 506 AND `id` >= 4)
  OR (`entryorguid` = 519 AND `id` >= 2)
  OR (`entryorguid` = 573 AND `id` >= 1)
  OR (`entryorguid` = 589 AND `id` >= 4)
  OR (`entryorguid` = 594 AND `id` >= 4)
  OR (`entryorguid` = 624 AND `id` >= 1)
  OR (`entryorguid` = 625 AND `id` >= 1)
  OR (`entryorguid` = 833 AND `id` >= 1)
  OR (`entryorguid` = 834 AND `id` >= 2)
  OR (`entryorguid` = 1065 AND `id` >= 4)
  OR (`entryorguid` = 1109 AND `id` >= 1)
  OR (`entryorguid` = 1424 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(154, 0, 1, 0, 0, 0, 100, 0, 4500, 9500, 17500, 22300, 0, 11, 13443, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Greater Fleshripper - In Combat - Cast ''Rend'''),
(157, 0, 1, 0, 0, 0, 100, 0, 5500, 7800, 18500, 26300, 0, 11, 32019, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Goretusk - In Combat - Cast ''Gore'''),
(391, 0, 2, 0, 0, 0, 100, 0, 2000, 3000, 12000, 14000, 0, 11, 80555, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Old Murk-Eye - In Combat - Cast ''Sonic Spear'''),
(391, 0, 3, 0, 0, 0, 100, 0, 12000, 15000, 27000, 35000, 0, 11, 78542, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Old Murk-Eye - In Combat - Cast ''Splash'''),
(391, 0, 4, 5, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Old Murk-Eye - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(391, 0, 5, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Old Murk-Eye - Between 0-30% Health - Say Line 0 (No Repeat)'),
(449, 0, 4, 0, 0, 0, 100, 0, 2000, 4500, 12000, 20000, 0, 11, 75356, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Defias Knuckleduster - In Combat - Cast ''Fisticuffs'''),
(449, 0, 5, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 80597, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Defias Knuckleduster - Between 0-30% Health - Cast ''Bash and Smash'' (No Repeat)'),
(453, 0, 3, 0, 0, 0, 100, 0, 3500, 4500, 12500, 15500, 0, 11, 79913, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Riverpaw Mystic - In Combat - Cast ''Chain Lightning'''),
(453, 0, 4, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 79924, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Riverpaw Mystic - Between 0-30% Health - Cast ''Healing Wave'' (No Repeat)'),
(506, 0, 4, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 79853, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sergeant Brashclaw - Between 0-30% Health - Cast ''Evasion'' (No Repeat)'),
(519, 0, 2, 0, 0, 0, 100, 0, 5500, 7500, 12500, 18500, 0, 11, 79831, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Slark - In Combat - Cast ''Wave Crash'''),
(519, 0, 3, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79892, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Slark - Out of Combat - Cast ''Water Shield'''),
(573, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 15000, 20000, 0, 11, 7342, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Foe Reaper 4000 - In Combat - Cast ''Wide Slash'''),
(573, 0, 2, 0, 2, 0, 100, 0, 0, 40, 25000, 29000, 0, 11, 80572, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Foe Reaper 4000 - Between 0-40% Health - Cast ''Energize!'''),
(589, 0, 4, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 13878, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Defias Pillager - In Combat - Cast ''Scorch'''),
(589, 0, 5, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 79849, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Defias Pillager - Out of Combat - Cast ''Molten Armor'''),
(589, 0, 6, 0, 9, 0, 100, 0, 0, 8, 13600, 14500, 0, 11, 79857, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Defias Pillager - Within 0-8 Range - Cast ''Blast Wave'''),
(594, 0, 4, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 78894, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Defias Henchman - On Aggro - Cast ''Leaping Cleave'' (No Repeat)'),
(624, 0, 1, 0, 1, 0, 100, 1, 0, 0, 0, 0, 0, 11, 26047, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Undead Excavator - Out of Combat - Cast ''Birth'' (No Repeat)'),
(625, 0, 1, 0, 1, 0, 100, 1, 0, 0, 0, 0, 0, 11, 26047, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Undead Dynamiter - Out of Combat - Cast ''Birth'' (No Repeat)'),
(833, 0, 1, 0, 0, 0, 100, 0, 7000, 12000, 24000, 32000, 0, 11, 80362, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Coyote Packleader - In Combat - Cast ''Crushing Bite'''),
(834, 0, 2, 0, 0, 0, 100, 0, 7000, 12000, 24000, 32000, 0, 11, 80362, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Coyote - In Combat - Cast ''Crushing Bite'''),
(1065, 0, 4, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 12550, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Riverpaw Shaman - Out of Combat - Cast ''Lightning Shield'''),
(1109, 0, 1, 0, 0, 0, 100, 0, 4500, 9500, 17500, 22300, 0, 11, 13443, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Fleshripper - In Combat - Cast ''Rend'''),
(1424, 0, 3, 0, 0, 0, 100, 0, 6000, 12000, 27000, 33000, 0, 11, 80382, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Master Digger - In Combat - Cast ''Dirt Toss'''),
(1424, 0, 4, 0, 0, 0, 100, 0, 4000, 7000, 18000, 27000, 0, 11, 6016, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Master Digger - In Combat - Cast ''Pierce Armor''');
