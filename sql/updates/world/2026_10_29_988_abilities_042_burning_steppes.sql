-- #988 wave 2 (merge): Burning Steppes creature abilities from DeltaTho PR 15 file 2026_10_24_51_world.sql, ADDED to creatures that already have smart_scripts.
-- 15 creatures, 23 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 615:4 7025:2 7026:2 7027:2 7034:3 7035:3 7038:3 9604:3 9697:3 9776:3 10078:4 48111:7 48119:3 48121:3 48201:3

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 615 AND `id` >= 4)
  OR (`entryorguid` = 7025 AND `id` >= 2)
  OR (`entryorguid` = 7026 AND `id` >= 2)
  OR (`entryorguid` = 7027 AND `id` >= 2)
  OR (`entryorguid` = 7034 AND `id` >= 3)
  OR (`entryorguid` = 7035 AND `id` >= 3)
  OR (`entryorguid` = 7038 AND `id` >= 3)
  OR (`entryorguid` = 9604 AND `id` >= 3)
  OR (`entryorguid` = 9697 AND `id` >= 3)
  OR (`entryorguid` = 9776 AND `id` >= 3)
  OR (`entryorguid` = 10078 AND `id` >= 4)
  OR (`entryorguid` = 48111 AND `id` >= 7)
  OR (`entryorguid` = 48119 AND `id` >= 3)
  OR (`entryorguid` = 48121 AND `id` >= 3)
  OR (`entryorguid` = 48201 AND `id` >= 3));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(615, 0, 4, 0, 2, 0, 100, 1, 0, 55, 0, 0, 0, 11, 82836, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Blood Fury at 55% HP'),
(615, 0, 5, 0, 0, 0, 100, 0, 5000, 6000, 14000, 15000, 0, 11, 11971, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Sunder Armor'),
(615, 0, 6, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 80676, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Spyglass on Spawn'),
(7025, 0, 2, 0, 2, 0, 100, 0, 0, 55, 22000, 25000, 0, 11, 48153, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Improved Blocking at 55% HP'),
(7026, 0, 2, 0, 0, 0, 100, 0, 5000, 6000, 15000, 18000, 0, 11, 20826, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Immolate'),
(7026, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 20825, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Shadow Bolt'),
(7026, 0, 4, 0, 0, 0, 100, 0, 7000, 9000, 18000, 24000, 0, 11, 13338, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Curse of Tongues'),
(7027, 0, 2, 0, 0, 0, 100, 0, 5000, 6000, 12000, 13000, 0, 11, 87081, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Strike'),
(7034, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9053, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Fireball'),
(7035, 0, 3, 0, 0, 0, 100, 0, 5000, 5500, 12000, 13000, 0, 11, 87081, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Strike'),
(7038, 0, 3, 0, 0, 0, 100, 0, 4000, 5000, 12000, 13000, 0, 11, 14873, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Sinister Strike'),
(7038, 0, 4, 0, 2, 0, 100, 0, 0, 40, 15000, 15000, 0, 11, 80576, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Shadowstep at 40% HP'),
(9604, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9053, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Fireball'),
(9697, 0, 3, 0, 2, 0, 100, 0, 0, 55, 22000, 25000, 0, 11, 3149, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Furious Howl at 55% HP'),
(9776, 0, 3, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 15664, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Venom Spit'),
(10078, 0, 4, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 11962, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Immolate'),
(48111, 0, 7, 0, 9, 0, 100, 0, 0, 8, 15000, 25000, 0, 11, 6266, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Kodo Stomp on Close'),
(48119, 0, 3, 0, 9, 0, 100, 0, 0, 100, 15000, 25000, 0, 11, 89596, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Unmask on Close'),
(48121, 0, 3, 0, 0, 0, 100, 0, 7000, 8000, 17000, 22000, 0, 11, 79846, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Flamestrike'),
(48121, 0, 4, 0, 9, 0, 100, 0, 0, 100, 15000, 25000, 0, 11, 89596, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Unmask on Close'),
(48121, 0, 5, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 9053, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Fireball'),
(48201, 0, 3, 0, 2, 0, 100, 0, 0, 55, 22000, 25000, 0, 11, 3248, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Improved Blocking at 55% HP'),
(48201, 0, 4, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Revenge');
