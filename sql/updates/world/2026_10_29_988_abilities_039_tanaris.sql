-- #988 wave 2 (merge): Tanaris creature abilities from DeltaTho PR 15 file 2026_10_24_48_world.sql, ADDED to creatures that already have smart_scripts.
-- 17 creatures, 27 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 5420:1 5427:1 5429:4 5450:2 5451:6 5454:1 5455:2 5458:2 5459:2 5460:2 5645:1 5646:2 5647:3 7855:1 7858:2 8204:2 39081:2

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 5420 AND `id` >= 1)
  OR (`entryorguid` = 5427 AND `id` >= 1)
  OR (`entryorguid` = 5429 AND `id` >= 4)
  OR (`entryorguid` = 5450 AND `id` >= 2)
  OR (`entryorguid` = 5451 AND `id` >= 6)
  OR (`entryorguid` = 5454 AND `id` >= 1)
  OR (`entryorguid` = 5455 AND `id` >= 2)
  OR (`entryorguid` = 5458 AND `id` >= 2)
  OR (`entryorguid` = 5459 AND `id` >= 2)
  OR (`entryorguid` = 5460 AND `id` >= 2)
  OR (`entryorguid` = 5645 AND `id` >= 1)
  OR (`entryorguid` = 5646 AND `id` >= 2)
  OR (`entryorguid` = 5647 AND `id` >= 3)
  OR (`entryorguid` = 7855 AND `id` >= 1)
  OR (`entryorguid` = 7858 AND `id` >= 2)
  OR (`entryorguid` = 8204 AND `id` >= 2)
  OR (`entryorguid` = 39081 AND `id` >= 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(5420, 0, 1, 0, 0, 0, 100, 0, 4000, 4500, 15000, 16000, 0, 11, 5106, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Glasshide Gazer - In Combat - Cast ''Crystal Flash'''),
(5420, 0, 2, 0, 2, 0, 100, 0, 0, 60, 12000, 13000, 0, 11, 83497, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Glasshide Gazer - Between 0-60% Health - Cast ''Glass Hide'''),
(5427, 0, 1, 0, 9, 0, 100, 0, 0, 9, 15000, 17800, 0, 11, 82797, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Rabid Blisterpaw - Within 0-9 Range - Cast ''Leaping Bite'''),
(5429, 0, 4, 0, 9, 0, 100, 0, 0, 8, 19000, 22000, 0, 11, 24423, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Fire Roc - Within 0-8 Range - Cast ''Demoralizing Screech'''),
(5450, 0, 2, 0, 0, 0, 100, 0, 2000, 3000, 9000, 9800, 0, 11, 3391, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hazzali Stinger - In Combat - Cast ''Thrash'''),
(5450, 0, 3, 0, 0, 0, 100, 0, 6000, 7000, 25000, 27000, 0, 11, 81224, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Hazzali Stinger - In Combat - Cast ''Silithid Toxin'''),
(5451, 0, 6, 0, 0, 0, 100, 0, 6000, 7000, 25000, 27000, 0, 11, 81224, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Hazzali Swarmer - In Combat - Cast ''Silithid Toxin'''),
(5454, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 9000, 9800, 0, 11, 3391, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hazzali Sandreaver - In Combat - Cast ''Thrash'''),
(5454, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 66060, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hazzali Sandreaver - On Aggro - Cast ''Sprint'' (No Repeat)'),
(5454, 0, 3, 0, 0, 0, 100, 0, 6000, 7000, 25000, 27000, 0, 11, 81224, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Hazzali Sandreaver - In Combat - Cast ''Silithid Toxin'''),
(5455, 0, 2, 0, 0, 0, 100, 0, 3500, 3500, 18500, 22000, 0, 11, 81109, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Centipaar Wasp - In Combat - Cast ''Poison Bolt'''),
(5455, 0, 3, 0, 0, 0, 100, 0, 4000, 4000, 60000, 65000, 0, 11, 81224, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Centipaar Wasp - In Combat - Cast ''Silithid Toxin'''),
(5458, 0, 2, 0, 0, 0, 100, 0, 3500, 3500, 18500, 22000, 0, 11, 83574, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Centipaar Worker - In Combat - Cast ''Acidic Adhesive'''),
(5458, 0, 3, 0, 0, 0, 100, 0, 4000, 4000, 60000, 65000, 0, 11, 81224, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Centipaar Worker - In Combat - Cast ''Silithid Toxin'''),
(5459, 0, 2, 0, 0, 0, 100, 0, 4000, 4000, 60000, 65000, 0, 11, 81224, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Centipaar Tunneler - In Combat - Cast ''Silithid Toxin'''),
(5460, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 66060, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Centipaar Sandreaver - On Aggro - Cast ''Sprint'' (No Repeat)'),
(5460, 0, 3, 0, 0, 0, 100, 0, 4000, 4000, 60000, 65000, 0, 11, 81224, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Centipaar Sandreaver - In Combat - Cast ''Silithid Toxin'''),
(5645, 0, 1, 0, 0, 0, 100, 0, 2000, 4500, 15000, 18000, 0, 11, 81705, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Hideskinner - In Combat - Cast ''Scalp Slash'''),
(5646, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 81173, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Axe Thrower - Between 0-30% Health - Cast ''Frenzy'' (No Repeat)'),
(5647, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 82641, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Firecaller - In Combat - Cast ''Lava Gout'''),
(5647, 0, 4, 0, 0, 0, 100, 0, 9000, 12000, 33000, 35000, 0, 11, 79886, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Firecaller - In Combat - Cast ''Lava Burst'''),
(7855, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 83622, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Southsea Pirate - On Aggro - Cast ''Leaping Cleave'' (No Repeat)'),
(7858, 0, 2, 0, 0, 0, 100, 0, 2000, 4500, 12000, 14000, 0, 11, 75361, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Southsea Swashbuckler - In Combat - Cast ''Swashbuckling Slice'''),
(7858, 0, 3, 0, 13, 0, 100, 0, 20000, 30000, 0, 0, 0, 11, 79732, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Southsea Swashbuckler - On Victim Casting ''Alexander''s Test Periodic Aura'' - Cast ''Shield Bash'''),
(8204, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Soriid the Devourer - On Aggro - Cast ''Rushing Charge'' (No Repeat)'),
(8204, 0, 3, 0, 0, 0, 100, 0, 12000, 13000, 60000, 65000, 0, 11, 81224, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Soriid the Devourer - In Combat - Cast ''Silithid Toxin'''),
(39081, 0, 2, 0, 6, 0, 100, 1, 0, 0, 0, 0, 0, 11, 73441, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hazzali Cocoon - On Just Died - Cast ''Summon Captured Bughunter'' (No Repeat)');
