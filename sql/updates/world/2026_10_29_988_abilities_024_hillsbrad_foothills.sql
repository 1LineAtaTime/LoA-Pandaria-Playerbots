-- #988 wave 2 (merge): Hillsbrad Foothills creature abilities from DeltaTho PR 15 file 2026_10_24_32_world.sql, ADDED to creatures that already have smart_scripts.
-- 12 creatures, 20 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 2270:8 2319:2 2344:3 2358:5 2369:2 2386:1 2416:2 2628:1 14221:1 48269:3 48741:1 48742:1

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 2270 AND `id` >= 8)
  OR (`entryorguid` = 2319 AND `id` >= 2)
  OR (`entryorguid` = 2344 AND `id` >= 3)
  OR (`entryorguid` = 2358 AND `id` >= 5)
  OR (`entryorguid` = 2369 AND `id` >= 2)
  OR (`entryorguid` = 2386 AND `id` >= 1)
  OR (`entryorguid` = 2416 AND `id` >= 2)
  OR (`entryorguid` = 2628 AND `id` >= 1)
  OR (`entryorguid` = 14221 AND `id` >= 1)
  OR (`entryorguid` = 48269 AND `id` >= 3)
  OR (`entryorguid` = 48741 AND `id` >= 1)
  OR (`entryorguid` = 48742 AND `id` >= 1));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(2270, 0, 8, 0, 0, 0, 100, 0, 3000, 5000, 120000, 125000, 0, 11, 32064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Battle Shout'),
(2270, 0, 9, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 22120, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Charge on Aggro'),
(2319, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 20000, 22000, 0, 11, 13323, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Polymorph'),
(2344, 0, 3, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 8258, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dun Garok Mountaineer - On Reset - Cast Devotion Aura'),
(2358, 0, 5, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 0, 11, 79858, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Frostbolt'),
(2358, 0, 6, 0, 2, 0, 100, 1, 0, 40, 0, 0, 0, 11, 87179, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Summon Water Elemental at 40% HP'),
(2369, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 12555, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Pummel'),
(2386, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 12170, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Revenge'),
(2386, 0, 2, 0, 2, 0, 100, 0, 0, 70, 15000, 15000, 0, 11, 12169, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Shield Block at 70% HP'),
(2416, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3500, 4100, 0, 11, 9143, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Bomb'),
(2628, 0, 1, 0, 0, 0, 100, 0, 5000, 6000, 15000, 19000, 0, 11, 33661, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Crush Armor'),
(14221, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 0, 11, 18501, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Enrage at 30% HP'),
(48269, 0, 3, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 0, 11, 74613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Shoot'),
(48269, 0, 4, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 89702, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Camouflage on Spawn'),
(48741, 0, 1, 0, 9, 0, 100, 0, 0, 10, 15000, 25000, 0, 11, 79851, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Fan of Knives on Close'),
(48741, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 0, 11, 60842, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Expose Armor'),
(48741, 0, 3, 0, 2, 0, 100, 0, 0, 55, 22000, 25000, 0, 11, 79853, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Evasion at 55% HP'),
(48742, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 66060, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Sprint on Aggro'),
(48742, 0, 2, 0, 0, 0, 100, 0, 5000, 8000, 17000, 20000, 0, 11, 3583, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Deadly Poison'),
(48742, 0, 3, 0, 0, 0, 100, 0, 10000, 10000, 12000, 14000, 0, 11, 60850, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Mutilate');
