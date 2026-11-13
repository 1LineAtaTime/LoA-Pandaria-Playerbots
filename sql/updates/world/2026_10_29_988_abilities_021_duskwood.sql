-- #988 wave 2 (merge): Duskwood creature abilities from DeltaTho PR 15 file 2026_10_24_29_world.sql, ADDED to creatures that already have smart_scripts.
-- 25 creatures, 37 new smart_scripts rows. Existing rows are untouched; new rows take ids after each creature's current max id.
-- Only casts whose spell (or a same-named spell) the creature does not already cast; link-chain companions kept unless a duplicate text group / covered cast;
-- phase-gated (state machine) and non-ability rows are not imported. Details: (not shipped)
-- Idempotent: DELETE of the new id range per creature, then INSERT. Undo: the same DELETEs (listed below as entry:first_new_id).
-- ranges: 3:1 48:2 202:1 205:1 206:1 210:1 217:1 218:1 507:2 511:2 522:1 533:1 534:2 574:2 604:2 628:1 889:1 920:2 930:2 948:2 1251:2 1270:2 3946:4 43761:7 43923:2

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND (
  (`entryorguid` = 3 AND `id` >= 1)
  OR (`entryorguid` = 48 AND `id` >= 2)
  OR (`entryorguid` = 202 AND `id` >= 1)
  OR (`entryorguid` = 205 AND `id` >= 1)
  OR (`entryorguid` = 206 AND `id` >= 1)
  OR (`entryorguid` = 210 AND `id` >= 1)
  OR (`entryorguid` = 217 AND `id` >= 1)
  OR (`entryorguid` = 218 AND `id` >= 1)
  OR (`entryorguid` = 507 AND `id` >= 2)
  OR (`entryorguid` = 511 AND `id` >= 2)
  OR (`entryorguid` = 522 AND `id` >= 1)
  OR (`entryorguid` = 533 AND `id` >= 1)
  OR (`entryorguid` = 534 AND `id` >= 2)
  OR (`entryorguid` = 574 AND `id` >= 2)
  OR (`entryorguid` = 604 AND `id` >= 2)
  OR (`entryorguid` = 628 AND `id` >= 1)
  OR (`entryorguid` = 889 AND `id` >= 1)
  OR (`entryorguid` = 920 AND `id` >= 2)
  OR (`entryorguid` = 930 AND `id` >= 2)
  OR (`entryorguid` = 948 AND `id` >= 2)
  OR (`entryorguid` = 1251 AND `id` >= 2)
  OR (`entryorguid` = 1270 AND `id` >= 2)
  OR (`entryorguid` = 3946 AND `id` >= 4)
  OR (`entryorguid` = 43761 AND `id` >= 7)
  OR (`entryorguid` = 43923 AND `id` >= 2));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3, 0, 1, 0, 2, 0, 100, 0, 0, 50, 31000, 31000, 0, 11, 83884, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'When health between 0% and 50% (check every 31000 - 31000 ms) - Self: Cast spell Cannibalize (83884) on Self // '),
(48, 0, 2, 0, 0, 0, 100, 0, 8000, 9000, 7000, 11000, 0, 11, 85240, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 8000 and 9000 ms (and later repeats every 7000 and 11000 ms) - Self: Cast spell Sundering Cleave (85240) on Victim // '),
(202, 0, 1, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 85236, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On death - Self: Cast spell Corpse Rot (85236) on Self // '),
(205, 0, 1, 0, 0, 0, 100, 0, 1000, 2000, 31000, 32000, 0, 11, 7137, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 1000 min and 2000 max ms (repeat 31000 min 32000 max ms) - Self: Cast spell Shadow Charge (7137) on Victim // '),
(206, 0, 1, 0, 0, 0, 100, 0, 7000, 8000, 20000, 21000, 0, 11, 84308, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 7000 min and 8000 max ms (repeat 20000 min 21000 max ms) - Self: Cast spell Furious Howl (84308) on Self // '),
(210, 0, 1, 0, 0, 0, 100, 0, 10000, 10000, 12000, 17000, 0, 11, 82625, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 10000 min and 10000 max ms (repeat 12000 min 17000 max ms) - Self: Cast spell Bone Toss (82625) on Victim // '),
(217, 0, 1, 0, 0, 0, 100, 0, 9600, 23200, 30200, 63200, 0, 11, 745, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Venom Web Spider - In Combat - Cast ''Web'''),
(217, 0, 2, 0, 0, 0, 100, 0, 9700, 25800, 18000, 28100, 0, 11, 744, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Venom Web Spider - In Combat - Cast ''Poison'''),
(218, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 9000, 12000, 0, 11, 6595, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Grave Robber - In Combat - Cast ''Exploit Weakness'''),
(507, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 11, 7137, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Fenros - On Aggro - Cast ''Shadow Charge'' (No Repeat)'),
(507, 0, 3, 0, 0, 0, 100, 0, 2000, 3000, 9000, 12000, 0, 11, 6595, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Fenros - In Combat - Cast ''Exploit Weakness'''),
(507, 0, 4, 0, 0, 0, 100, 0, 6000, 9000, 19000, 22000, 0, 11, 84308, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Fenros - In Combat - Cast ''Furious Howl'''),
(511, 0, 2, 0, 0, 0, 100, 0, 8000, 9000, 30000, 30000, 0, 11, 83586, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 8000 and 9000 ms (and later repeats every 30000 and 30000 ms) - Self: Cast spell Insane Rambling (83586) on Self // '),
(511, 0, 3, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 85236, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On death - Self: Cast spell Corpse Rot (85236) on Self // '),
(522, 0, 1, 0, 0, 0, 100, 0, 3500, 6000, 12000, 15000, 0, 11, 79961, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mor''Ladim - In Combat - Cast ''Holy Shock'''),
(533, 0, 1, 0, 0, 0, 100, 0, 6000, 6000, 11000, 11000, 0, 11, 77721, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 6000 and 6000 ms (and later repeats every 11000 and 11000 ms) - Self: Cast spell Shadow Weave (77721) on Victim // '),
(534, 0, 2, 0, 0, 0, 100, 0, 6000, 11000, 25000, 28000, 0, 11, 3427, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Nefaru - In Combat - Cast ''Infected Wound'''),
(574, 0, 2, 0, 0, 0, 100, 0, 3000, 4500, 15000, 17000, 0, 11, 13884, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Naraxis - In Combat - Cast ''Withering Poison'''),
(574, 0, 3, 0, 2, 0, 100, 0, 0, 40, 15000, 21000, 0, 11, 87080, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Naraxis - Between 0-40% Health - Cast ''Vanish'''),
(574, 0, 4, 0, 9, 0, 100, 0, 0, 30, 8000, 9000, 0, 11, 12023, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Naraxis - Within 0-30 Range - Cast ''Web'''),
(604, 0, 2, 0, 0, 0, 100, 0, 14000, 15000, 32000, 33000, 0, 11, 3436, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 14000 min and 15000 max ms (repeat 32000 min 33000 max ms) - Self: Cast spell Wandering Plague (3436) on Victim // '),
(628, 0, 1, 0, 0, 0, 100, 0, 7000, 8000, 31000, 31000, 0, 11, 80365, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 7000 min and 8000 max ms (repeat 31000 min 31000 max ms) - Self: Cast spell Flurry of Claws (80365) on Self // '),
(889, 0, 1, 0, 0, 0, 100, 0, 7000, 9000, 31000, 31000, 0, 11, 80182, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 7000 min and 9000 max ms (repeat 31000 min 31000 max ms) - Self: Cast spell Uppercut (80182) on Victim // '),
(920, 0, 2, 3, 0, 0, 100, 0, 10000, 10000, 61000, 61000, 0, 11, 85244, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 10000 min and 10000 max ms (repeat 61000 min 61000 max ms) - Self: Cast spell Worgen Taint (85244) on Victim // '),
(920, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 11, 98531, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Cast spell Worgen Taint (98531) on Victim // '),
(930, 0, 2, 0, 0, 0, 100, 0, 25000, 25000, 30000, 30000, 0, 11, 12023, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 25000 and 25000 ms (and later repeats every 30000 and 30000 ms) - Self: Cast spell Web (12023) on Victim // '),
(948, 0, 2, 0, 0, 0, 100, 0, 17000, 17000, 32000, 33000, 0, 11, 85234, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 17000 min and 17000 max ms (repeat 32000 min 33000 max ms) - Self: Cast spell Fetid Breath (85234) on Victim // '),
(1251, 0, 2, 0, 0, 0, 100, 0, 11000, 12000, 23000, 24000, 0, 11, 78509, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 11000 min and 12000 max ms (repeat 23000 min 24000 max ms) - Self: Cast spell Torch Toss (78509) on Victim // '),
(1251, 0, 3, 0, 0, 0, 100, 0, 3000, 4000, 8000, 12000, 0, 11, 13878, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 3000 min and 4000 max ms (repeat 8000 min 12000 max ms) - Self: Cast spell Scorch (13878) on Victim // '),
(1270, 0, 2, 0, 0, 0, 100, 0, 4000, 7000, 16000, 18000, 0, 11, 85234, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Fetid Corpse - In Combat - Cast ''Fetid Breath'''),
(3946, 0, 4, 0, 0, 0, 100, 0, 6000, 9000, 25000, 32000, 0, 11, 13860, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Velinde Starsong - In Combat - Cast ''Mind Blast'''),
(3946, 0, 5, 0, 0, 0, 100, 0, 5000, 12000, 15000, 27000, 0, 11, 16568, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Velinde Starsong - In Combat - Cast ''Mind Flay'''),
(3946, 0, 6, 0, 2, 0, 100, 0, 0, 50, 17000, 21000, 0, 11, 11640, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Velinde Starsong - Between 0-50% Health - Cast ''Renew'''),
(43761, 0, 7, 0, 2, 0, 100, 0, 0, 55, 22000, 26000, 0, 11, 3108, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Morbent Fel - Between 0-55% Health - Cast ''Touch of Death'''),
(43761, 0, 8, 0, 0, 0, 100, 0, 6000, 9000, 18000, 20000, 0, 11, 3109, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 'Morbent Fel - In Combat - Cast ''Presence of Death'''),
(43761, 0, 9, 0, 11, 0, 100, 1, 0, 0, 0, 0, 0, 11, 8909, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Morbent Fel - On Respawn - Cast ''Unholy Shield'' (No Repeat)'),
(43923, 0, 2, 0, 0, 0, 100, 0, 8000, 9000, 12000, 15000, 0, 11, 81193, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 8000 min and 9000 max ms (repeat 12000 min 15000 max ms) - Self: Cast spell Unrelenting Anguish (81193) on Victim // ');
