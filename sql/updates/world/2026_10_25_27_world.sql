-- Halls of Origination: creature abilities
-- Most of the dungeon's creatures (the titan constructs, the Tol'vir's foes and the elementals) had no AI or ran classic rows: port
-- their abilities from the Cataclysm data, falling back to the classic data where Cataclysm has none, keeping the quest scripts
-- already in place; gossip and quest reward scripts of the references are left out, keeping the dungeon (normal/heroic) flags. The
-- creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '39373,39440,40310,40311,42556,48139,48140,48143';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(39373, 0, 0, 0, 1, 0, 100, 0, 3000, 3000, 12000, 12000, 11, 74632, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When out of combat and timer at the begining between 3000 and 3000 ms (and later repeats every 12000 and 12000 ms) - Self: Cast spell 74632 on Self // '),
(39373, 0, 1, 0, 0, 0, 100, 1, 4000, 5000, 0, 0, 11, 73861, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 4000 and 5000 ms (and later repeats every 0 and 0 ms) - Self: Cast spell 73861 on Random hostile // '),
(39440, 0, 0, 0, 0, 0, 100, 0, 2400, 2400, 15000, 16000, 11, 75158, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 2400 min and 2400 max ms (repeat 15000 min 16000 max ms) - Self: Cast spell Surge (75158) on Random hostile // '),
(39440, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 17000, 19000, 11, 74121, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 4000 min and 5000 max ms (repeat 17000 min 19000 max ms) - Self: Cast spell Debilitating Venom (74121) on Victim // '),
(39440, 0, 2, 0, 0, 0, 100, 0, 9000, 10000, 30000, 31000, 11, 73963, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 9000 min and 10000 max ms (repeat 30000 min 31000 max ms) - Self: Cast spell Blinding Toxin (73963) on Victim // '),
(40310, 0, 0, 0, 0, 0, 100, 0, 4000, 4000, 3500, 4000, 11, 74122, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 4000 min and 4000 max ms (repeat 3500 min 4000 max ms) - Self: Cast spell Corrosive Spray (74122) on Self // '),
(40310, 0, 1, 0, 0, 0, 100, 0, 7000, 8000, 10000, 11000, 11, 74542, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 7000 min and 8000 max ms (repeat 10000 min 11000 max ms) - Self: Cast spell Serrated Slash (74542) on Victim // '),
(40311, 0, 0, 0, 0, 0, 100, 0, 6000, 6000, 43000, 45000, 11, 77357, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 6000 min and 6000 max ms (repeat 43000 min 45000 max ms) - Self: Cast spell Curse of Exhaustion (77357) on Random hostile // '),
(40311, 0, 1, 0, 0, 0, 100, 0, 7000, 7000, 2400, 2400, 11, 77570, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 7000 min and 7000 max ms (repeat 2400 min 2400 max ms) - Self: Cast spell Shadow Bolt (77570) on Victim // '),
(42556, 0, 0, 0, 0, 0, 100, 0, 2400, 2400, 13000, 16000, 11, 83233, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 2400 min and 2400 max ms (repeat 13000 min 16000 max ms) - Self: Cast spell Surge (83233) on Random hostile // '),
(48139, 0, 0, 0, 0, 0, 100, 0, 1000, 1000, 2400, 2400, 11, 83877, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 1000 and 1000 ms (and later repeats every 2400 and 2400 ms) - Self: Cast spell Shoot (83877) on Victim // '),
(48139, 0, 1, 2, 0, 0, 100, 0, 6000, 6000, 26000, 26000, 64, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 6000 and 6000 ms (and later repeats every 26000 and 26000 ms) - Self: Target_0 = Random hostile // '),
(48139, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 11, 89574, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Cast spell Charged Shot (89574) on storedTarget[0] // '),
(48139, 0, 3, 4, 61, 0, 100, 0, 0, 0, 0, 0, 11, 89571, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, ' Linked - Self: Cast spell Charged Shot (89571) on storedTarget[0] // '),
(48139, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, ' Linked - storedTarget[0]: Talk 0 // '),
(48140, 0, 0, 0, 9, 0, 100, 6, 0, 5, 9000, 11000, 11, 89554, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Temple Runecaster - On Close - Cast Runic Cleave'),
(48140, 0, 1, 0, 0, 0, 100, 6, 5000, 7500, 14500, 18000, 11, 89551, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Temple Runecaster - In Combat - Cast Curse of the Runecaster'),
(48140, 0, 2, 0, 0, 0, 100, 0, 13000, 13000, 33000, 35000, 11, 89549, 0, 0, 0, 0, 0, 26, 30, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 13000 and 13000 ms (and later repeats every 33000 and 35000 ms) - Self: Cast spell Rune of Healing (89549) on Closest friendly creature in 30 yards // '),
(48143, 0, 0, 0, 2, 0, 100, 7, 0, 35, 0, 0, 11, 86542, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Temple Fireshaper - At 35% HP - Cast Molten Barrier'),
(48143, 0, 1, 0, 0, 0, 100, 6, 8000, 8000, 25000, 25000, 11, 89547, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Temple Fireshaper - In Combat - Cast Blazing Eruption'),
(48143, 0, 2, 0, 0, 0, 100, 6, 4000, 6000, 17500, 18500, 11, 84032, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 'Temple Fireshaper - In Combat - Cast Meteor'),
(48143, 0, 3, 0, 0, 0, 100, 6, 0, 0, 3400, 4700, 11, 89538, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Temple Fireshaper - In Combat - Cast Fireball'),
(48143, 0, 4, 0, 0, 0, 100, 0, 9000, 9000, 30000, 31000, 11, 89542, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 9000 and 9000 ms (and later repeats every 30000 and 31000 ms) - Self: Cast spell Molten Barrier (89542) on Self // ');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '40251,40252,41148';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(40251, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 77373, 0, 0, 2, 'Stone Trogg Brute - In Combat - Cast ''Clobber'''),
(40252, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 77389, 64, 0, 2, 'Stone Trogg Rock Flinger - In Combat - Cast ''Stone Throw'''),
(41148, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 76903, 64, 0, 2, 'Void Seeker - In Combat - Cast ''Anti-Magic Prison''');
