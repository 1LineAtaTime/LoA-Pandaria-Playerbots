-- Throne of the Tides: creature abilities
-- Most of the dungeon's creatures (the naga, the Gilgoblins and Ozumat's faceless servants) had no AI or ran classic rows: port
-- their abilities from the Cataclysm data, falling back to the classic data where Cataclysm has none, keeping the quest scripts
-- already in place; gossip and quest reward scripts of the references are left out, keeping the dungeon (normal/heroic) flags. The
-- creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '40577,40925';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(40577, 0, 0, 0, 0, 0, 100, 0, 4000, 6000, 4000, 6000, 11, 77218, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 4000 and 6000 ms (and later repeats every 4000 and 6000 ms) - Self: Cast spell 77218 on Random hostile // '),
(40577, 0, 1, 0, 0, 0, 100, 0, 13000, 14000, 13000, 14000, 11, 76721, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 13000 and 14000 ms (and later repeats every 13000 and 14000 ms) - Self: Cast spell 76721 on Victim // '),
(40925, 0, 0, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 76625, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On death - Self: Cast spell Summon Unstable Corruptions (76625) on Self // '),
(40925, 0, 1, 0, 0, 0, 100, 0, 18000, 20000, 18000, 20000, 11, 76634, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 18000 min and 20000 max ms (repeat 18000 min 20000 max ms) - Self: Cast spell Swell (76634) on Self // ');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '40584,40943,41139';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(40584, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 76790, 32, 0, 1, 'Naz''jar Invader - In Combat - Cast ''Bubble Shield'''),
(40584, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 76807, 0, 0, 2, 'Naz''jar Invader - In Combat - Cast ''Lacerate'''),
(40943, 0, 0, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 76560, 0, 0, 2, 'Gilgoblin Aquamage - Within 0-5 Range - Cast ''Tsunami'''),
(40943, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 76535, 64, 0, 2, 'Gilgoblin Aquamage - In Combat - Cast ''Water Bolt'''),
(41139, 0, 0, 0, 2, 0, 100, 0, 0, 40, 15000, 20000, 11, 76813, 0, 0, 1, 'Naz''jar Spiritmender - Between 0-40% Health - Cast ''Healing Wave'''),
(41139, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 18000, 24000, 11, 76820, 0, 0, 2, 'Naz''jar Spiritmender - In Combat - Cast ''Hex''');
