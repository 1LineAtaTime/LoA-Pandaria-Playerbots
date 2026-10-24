-- Zul'Gurub: creature abilities
-- Most of the dungeon's creatures (the Gurubashi trolls, their priests and beasts) had no AI or ran classic rows: port their
-- abilities from the Cataclysm data, falling back to the classic data where Cataclysm has none, keeping the quest scripts already
-- in place; gossip and quest reward scripts of the references are left out, keeping the dungeon (normal/heroic) flags. The
-- creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '52076,52082,52085,52086,52088,52322,52327,52339,52345,52381';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(52076, 0, 0, 0, 14, 0, 100, 4, 120000, 40, 15000, 18000, 11, 97386, 1, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Gurubashi Cauldron-Mixer - On Friendlies - Cast Crush Herbs'),
(52076, 0, 1, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 96488, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro - Self: Cast spell 96488 on Self // '),
(52076, 0, 2, 0, 0, 0, 100, 1, 5000, 5000, 0, 0, 11, 96449, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 5000 and 5000 ms (and later repeats every 0 and 0 ms) - Self: Cast spell 96449 on Random hostile // '),
(52076, 0, 3, 0, 0, 0, 100, 0, 21000, 21000, 20000, 20500, 11, 96413, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 21000 and 21000 ms (and later repeats every 20000 and 20500 ms) - Self: Cast spell 96413 on Random hostile // '),
(52082, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 96487, 64, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro - Self: Cast spell 96487 on Self // '),
(52082, 0, 1, 0, 0, 0, 100, 0, 5000, 6000, 2500, 2500, 11, 96804, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 5000 and 6000 ms (and later repeats every 2500 and 2500 ms) - Self: Cast spell 96804 on Victim // '),
(52082, 0, 2, 0, 0, 0, 100, 0, 21000, 21000, 20000, 20500, 11, 96413, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 21000 and 21000 ms (and later repeats every 20000 and 20500 ms) - Self: Cast spell 96413 on Random hostile // '),
(52085, 0, 0, 0, 0, 0, 100, 1, 9000, 15000, 0, 0, 11, 3391, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 9000 and 15000 ms (and later repeats every 0 and 0 ms) - Self: Cast spell 3391 on Victim // '),
(52086, 0, 0, 0, 1, 0, 100, 5, 1000, 1000, 1000, 1000, 11, 74919, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hakkari Witch Doctor - On Spawn - Cast Dark Aura'),
(52086, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 45104, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On reset (e.g. after reaching home) - Self: Cast spell 45104 on Self // '),
(52086, 0, 2, 0, 0, 0, 100, 0, 400, 400, 2500, 2500, 11, 96416, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 400 and 400 ms (and later repeats every 2500 and 2500 ms) - Self: Cast spell 96416 on Victim // '),
(52086, 0, 3, 4, 0, 0, 100, 0, 18000, 19000, 30000, 31000, 11, 97398, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 18000 and 19000 ms (and later repeats every 30000 and 31000 ms) - Self: Cast spell 97398 on Self // '),
(52086, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 67, 0, 4000, 4000, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ' Linked - Trigger timed event #0 in 4000 - 4000 ms // '),
(52086, 0, 5, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 96418, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On death - Self: Cast spell 96418 on Self // '),
(52086, 0, 6, 0, 59, 0, 100, 1, 0, 0, 0, 0, 11, 24065, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On timed event 0 triggered - Self: Cast spell 24065 on Self // '),
(52086, 0, 7, 0, 0, 0, 100, 0, 28000, 30000, 30000, 32000, 11, 97397, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 28000 and 30000 ms (and later repeats every 30000 and 32000 ms) - Self: Cast spell 97397 on Random hostile // '),
(52088, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 96488, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro - Self: Cast spell 96488 on Self // '),
(52088, 0, 1, 0, 0, 0, 100, 1, 5000, 5000, 0, 0, 11, 96449, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 5000 and 5000 ms (and later repeats every 0 and 0 ms) - Self: Cast spell 96449 on Random hostile // '),
(52088, 0, 2, 0, 0, 0, 100, 0, 21000, 21000, 20000, 20500, 11, 96413, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 21000 and 21000 ms (and later repeats every 20000 and 20500 ms) - Self: Cast spell 96413 on Random hostile // '),
(52322, 0, 0, 1, 4, 0, 100, 0, 0, 0, 0, 0, 11, 97016, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro - Self: Cast spell 97016 on Self // '),
(52322, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Talk 0 // '),
(52327, 0, 0, 0, 0, 0, 100, 0, 400, 400, 2500, 2500, 11, 97239, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 400 and 400 ms (and later repeats every 2500 and 2500 ms) - Self: Cast spell 97239 on Victim // '),
(52339, 0, 0, 0, 0, 0, 100, 0, 10000, 12000, 32000, 33000, 11, 96790, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 10000 and 12000 ms (and later repeats every 32000 and 33000 ms) - Self: Cast spell 96790 on Self // '),
(52339, 0, 1, 0, 0, 0, 100, 0, 9000, 10000, 30000, 31000, 11, 96849, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 9000 and 10000 ms (and later repeats every 30000 and 31000 ms) - Self: Cast spell 96849 on Self // '),
(52345, 0, 0, 0, 0, 0, 100, 0, 8000, 10000, 8000, 10000, 11, 97355, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 8000 and 10000 ms (and later repeats every 8000 and 10000 ms) - Self: Cast spell 97355 on Random hostile // '),
(52381, 0, 0, 1, 25, 0, 100, 0, 0, 0, 0, 0, 86, 96937, 0, 10, 295004, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On reset (e.g. after reaching home) - SMART_TARGET_CREATURE_GUID: Cast spell 96937 at Self // '),
(52381, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 86, 96936, 0, 10, 295002, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ' Linked - SMART_TARGET_CREATURE_GUID: Cast spell 96936 at Self // '),
(52381, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 18, 33554432, 0, 0, 0, 0, 0, 10, 295003, 0, 0, 0, 0, 0, 0, ' Linked - Creature Zanzil''s Cauldron of Toxic Torment 52529 (295003): Set UNIT_FLAGS to UNIT_FLAG_NOT_SELECTABLE // '),
(52381, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 86, 97122, 0, 10, 295003, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ' Linked - SMART_TARGET_CREATURE_GUID: Cast spell 97122 at Self // '),
(52381, 0, 4, 5, 6, 0, 100, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On death - Self: Talk 1 // '),
(52381, 0, 5, 6, 61, 0, 100, 0, 0, 0, 0, 0, 19, 33554432, 0, 0, 0, 0, 0, 10, 295003, 0, 0, 0, 0, 0, 0, ' Linked - Creature Zanzil''s Cauldron of Toxic Torment 52529 (295003): Remove UNIT_FLAGS to UNIT_FLAG_NOT_SELECTABLE // '),
(52381, 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 28, 97122, 0, 0, 0, 0, 0, 10, 295003, 0, 0, 0, 0, 0, 0, ' Linked - Creature Zanzil''s Cauldron of Toxic Torment 52529 (295003): Remove aura due to spell 97122 // '),
(52381, 0, 7, 8, 4, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro - Self: Talk 0 // '),
(52381, 0, 8, 9, 61, 0, 100, 0, 0, 0, 0, 0, 28, 96936, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Remove aura due to spell 96936 // '),
(52381, 0, 9, 0, 61, 0, 100, 0, 0, 0, 0, 0, 28, 96937, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Remove aura due to spell 96937 // '),
(52381, 0, 10, 0, 0, 0, 100, 0, 4000, 4000, 6000, 6000, 11, 96918, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 4000 and 4000 ms (and later repeats every 6000 and 6000 ms) - Self: Cast spell 96918 on Victim // '),
(52381, 0, 11, 0, 0, 0, 100, 0, 7000, 7000, 14000, 14000, 11, 96475, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 7000 and 7000 ms (and later repeats every 14000 and 14000 ms) - Self: Cast spell 96475 on Self // ');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '52081,52089,52417';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(52081, 0, 0, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 96413, 0, 0, 2, 'Gurubashi Cauldron-Mixer - In Combat - Cast ''Gout of Flame'''),
(52089, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 96332, 32, 0, 1, 'Gurubashi Worker - Out of Combat - Cast ''Hypothermia'''),
(52417, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 12097, 0, 0, 2, 'Shredtooth Frenzy - In Combat - Cast ''Pierce Armor''');
