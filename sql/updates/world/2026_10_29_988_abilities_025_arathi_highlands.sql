-- #988 wave 2 (merge): Arathi Highlands creature abilities from DeltaTho PR 15 file 2026_10_24_33_world.sql, ADDED to creatures that already have smart_scripts.
-- 19 creatures, 24 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 2555:3 2560:1 2566:3 2572:1 2585:1 2591:3 2592:1 2595:2 2599:4 2602:3 2605:2 2606:3 2619:3 2733:1 2761:1 2783:2 2793:3 10696:2 15126:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 2555 AND `id` >= 3)
  OR (`entryorguid` = 2560 AND `id` >= 1)
  OR (`entryorguid` = 2566 AND `id` >= 3)
  OR (`entryorguid` = 2572 AND `id` >= 1)
  OR (`entryorguid` = 2585 AND `id` >= 1)
  OR (`entryorguid` = 2591 AND `id` >= 3)
  OR (`entryorguid` = 2592 AND `id` >= 1)
  OR (`entryorguid` = 2595 AND `id` >= 2)
  OR (`entryorguid` = 2599 AND `id` >= 4)
  OR (`entryorguid` = 2602 AND `id` >= 3)
  OR (`entryorguid` = 2605 AND `id` >= 2)
  OR (`entryorguid` = 2606 AND `id` >= 3)
  OR (`entryorguid` = 2619 AND `id` >= 3)
  OR (`entryorguid` = 2733 AND `id` >= 1)
  OR (`entryorguid` = 2761 AND `id` >= 1)
  OR (`entryorguid` = 2783 AND `id` >= 2)
  OR (`entryorguid` = 2793 AND `id` >= 3)
  OR (`entryorguid` = 10696 AND `id` >= 2)
  OR (`entryorguid` = 15126 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(2555, 0, 3, 0, 0, 0, 100, 0, 5000, 8000, 25000, 37000, 0, 11, 39591, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Searing Totem'),
(2560, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 15000, 15000, 0, 11, 83366, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Howling Screech'),
(2566, 0, 3, 0, 9, 0, 100, 0, 0, 8, 15000, 25000, 0, 11, 84709, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Stomp on Close'),
(2572, 0, 1, 0, 0, 0, 100, 0, 5000, 6000, 12000, 13000, 0, 11, 11976, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Strike'),
(2585, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 7164, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Defensive Stance on Aggro'),
(2591, 0, 3, 0, 0, 0, 100, 0, 8000, 8000, 18000, 25000, 0, 11, 8364, 1, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Blizzard'),
(2592, 0, 1, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 82839, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Hardened on Spawn'),
(2592, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 82841, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Rock Bash'),
(2595, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 76892, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpentine Lunge on Aggro'),
(2599, 0, 4, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 80182, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Uppercut'),
(2602, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 20822, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Frostbolt'),
(2605, 0, 2, 0, 0, 0, 100, 0, 5000, 5000, 20000, 25000, 0, 11, 4974, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Wither Touch'),
(2605, 0, 3, 0, 2, 0, 100, 1, 0, 40, 0, 0, 0, 11, 8599, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Enrage at 40% HP'),
(2605, 0, 4, 0, 0, 0, 100, 0, 3000, 5000, 12000, 13000, 0, 11, 9081, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Shadow Bolt Volley'),
(2606, 0, 3, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 32064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Battle Shout on Aggro'),
(2619, 0, 3, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 15572, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Sunder Armor'),
(2619, 0, 4, 0, 2, 0, 100, 1, 0, 40, 0, 0, 0, 11, 82836, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Blood Fury at 40% HP'),
(2733, 0, 1, 0, 2, 0, 100, 1, 0, 40, 0, 0, 0, 11, 54633, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Drink Healing Potion at 40% HP'),
(2733, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 15000, 15000, 0, 11, 84713, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Toxic Potion'),
(2761, 0, 1, 0, 0, 0, 100, 0, 1400, 7300, 25600, 32300, 0, 11, 11831, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cresting Exile - In Combat - Cast Frost Nova'),
(2783, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Shadow Bolt'),
(2793, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9672, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Frostbolt'),
(10696, 0, 2, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Shoot'),
(15126, 0, 3, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 32064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Battle Shout on Aggro');
