-- Tol Barad Peninsula: creature abilities
-- Most of the zone's creatures (the Baradin Hold escapees, the restless dead and the peninsula's wildlife) had no AI or ran classic
-- rows: port their abilities from the Cataclysm data, falling back to the classic data where Cataclysm has none, keeping the quest
-- scripts already in place; gossip and quest reward scripts of the references are left out. The creatures no reference scripts cast
-- their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '46508,46569,46582,46586,46605,46608,46630,46648,46989,47130,47181,47182,47183,47627,47642,47657,47659';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(46508, 0, 0, 0, 0, 0, 100, 1, 4000, 5000, 0, 0, 11, 12023, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 4000 and 5000 ms (and later repeats every 0 and 0 ms) - Self: Cast spell 12023 on Victim // '),
(46569, 0, 0, 0, 0, 0, 100, 0, 6000, 6000, 30000, 30000, 11, 7102, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 6000 and 6000 ms (and later repeats every 30000 and 30000 ms) - Self: Cast spell Contagion of Rot (7102) on Victim // '),
(46569, 0, 1, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 85236, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On death - Self: Cast spell Corpse Rot (85236) on Self // '),
(46582, 0, 0, 0, 0, 0, 100, 0, 5000, 6000, 10000, 11000, 11, 85234, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 5000 and 6000 ms (and later repeats every 10000 and 11000 ms) - Self: Cast spell Fetid Breath (85234) on Victim // '),
(46582, 0, 1, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 85236, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On death - Self: Cast spell Corpse Rot (85236) on Self // '),
(46586, 0, 0, 0, 0, 0, 100, 0, 1, 1, 12000, 18000, 11, 49861, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 1 and 1 ms (and later repeats every 12000 and 18000 ms) - Self: Cast spell Infected Bite (49861) on Victim // '),
(46605, 0, 0, 0, 0, 0, 100, 0, 7000, 8000, 9000, 11000, 11, 75388, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 7000 and 8000 ms (and later repeats every 9000 and 11000 ms) - Self: Cast spell Rusty Cut (75388) on Victim // '),
(46608, 0, 0, 0, 0, 0, 100, 0, 6000, 6000, 30000, 30000, 11, 81173, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 6000 and 6000 ms (and later repeats every 30000 and 30000 ms) - Self: Cast spell Frenzy (81173) on Self // '),
(46608, 0, 1, 0, 0, 0, 100, 0, 12000, 12000, 16000, 16000, 11, 69203, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 12000 and 12000 ms (and later repeats every 16000 and 16000 ms) - Self: Cast spell Vicious Bite (69203) on Victim // '),
(46630, 0, 0, 0, 0, 0, 100, 0, 5000, 5000, 16000, 16000, 11, 79902, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 5000 and 5000 ms (and later repeats every 16000 and 16000 ms) - Self: Cast spell Plague Strike (79902) on Victim // '),
(46630, 0, 1, 0, 0, 0, 100, 0, 10000, 10000, 16000, 16000, 11, 83562, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 10000 and 10000 ms (and later repeats every 16000 and 16000 ms) - Self: Cast spell Curse of Life (83562) on Victim // '),
(46648, 0, 0, 0, 1, 0, 100, 0, 30000, 60000, 30000, 60000, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When out of combat and timer at the begining between 30000 and 60000 ms (and later repeats every 30000 and 60000 ms) - Self: Talk 0 // '),
(46648, 0, 1, 0, 0, 0, 100, 0, 4500, 5000, 8000, 11000, 11, 80146, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 4500 and 5000 ms (and later repeats every 8000 and 11000 ms) - Self: Cast spell Bonk (80146) on Victim // '),
(46989, 0, 0, 0, 0, 0, 100, 0, 1, 1, 28000, 29000, 11, 81219, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 1 and 1 ms (and later repeats every 28000 and 29000 ms) - Self: Cast spell Battle Shout (81219) on Self // '),
(46989, 0, 1, 0, 0, 0, 100, 0, 2500, 3000, 11000, 11000, 11, 79881, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 2500 and 3000 ms (and later repeats every 11000 and 11000 ms) - Self: Cast spell Slam (79881) on Victim // '),
(46989, 0, 2, 0, 0, 0, 100, 0, 10000, 11000, 18000, 20000, 11, 11977, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 10000 and 11000 ms (and later repeats every 18000 and 20000 ms) - Self: Cast spell Rend (11977) on Victim // '),
(47130, 0, 0, 0, 0, 0, 100, 0, 3000, 3000, 2000, 2000, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 3000 and 3000 ms (and later repeats every 2000 and 2000 ms) - Self: Cast spell Shoot (6660) on Victim // '),
(47181, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 3400, 11, 77721, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 0 and 0 ms (and later repeats every 3400 and 3400 ms) - Self: Cast spell 77721 on Victim // '),
(47181, 0, 1, 0, 0, 0, 100, 0, 3000, 3000, 7000, 7000, 11, 31976, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 3000 and 3000 ms (and later repeats every 7000 and 7000 ms) - Self: Cast spell 31976 on Self // '),
(47182, 0, 0, 0, 0, 0, 100, 0, 4500, 5000, 16000, 21000, 11, 79113, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 4500 and 5000 ms (and later repeats every 16000 and 21000 ms) - Self: Cast spell Ancient Darkness (79113) on Victim // '),
(47182, 0, 1, 0, 0, 0, 100, 0, 10500, 11000, 19000, 21000, 11, 88883, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 10500 and 11000 ms (and later repeats every 19000 and 21000 ms) - Self: Cast spell Pact of Darkness (88883) on Self // '),
(47183, 0, 0, 0, 0, 0, 100, 0, 5000, 6000, 8000, 9000, 11, 86085, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 5000 and 6000 ms (and later repeats every 8000 and 9000 ms) - Self: Cast spell 86085 on Victim // '),
(47627, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 66060, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro - Self: Cast spell Sprint (66060) on Self // '),
(47642, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 71, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On reset (e.g. after reaching home) - Self: Eqipt items from creature 2 // '),
(47642, 0, 1, 0, 4, 0, 100, 0, 0, 0, 0, 0, 71, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro - Self: Eqipt items from creature 1 // '),
(47642, 0, 2, 0, 0, 0, 100, 0, 4500, 5000, 9000, 11000, 11, 32011, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 4500 and 5000 ms (and later repeats every 9000 and 11000 ms) - Self: Cast spell Water Bolt (32011) on Victim // '),
(47642, 0, 3, 0, 0, 0, 100, 0, 8500, 9000, 33000, 34000, 11, 78799, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 8500 and 9000 ms (and later repeats every 33000 and 34000 ms) - Self: Cast spell Wave Crash (78799) on Victim // '),
(47642, 0, 4, 0, 1, 0, 100, 0, 14000, 19000, 14000, 19000, 5, 380, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When out of combat and timer at the begining between 14000 and 19000 ms (and later repeats every 14000 and 19000 ms) - Self: Play emote ONESHOT_FISHING (380) // '),
(47657, 0, 0, 0, 0, 0, 100, 0, 5000, 5000, 20000, 26000, 11, 79878, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 5000 and 5000 ms (and later repeats every 20000 and 26000 ms) - Self: Cast spell Bloodthirst (79878) on Victim // '),
(47657, 0, 1, 0, 0, 0, 100, 0, 8000, 9000, 9000, 11000, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 8000 and 9000 ms (and later repeats every 9000 and 11000 ms) - Self: Cast spell Cleave (15496) on Victim // '),
(47659, 0, 0, 0, 0, 0, 100, 1, 100, 100, 0, 0, 11, 78894, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'When in combat and timer at the begining between 100 and 100 ms (and later repeats every 0 and 0 ms) - Self: Cast spell 78894 on Victim // ');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '46570,46571,46606,46641,46643,46823,46825,46960,47287,47304,47373,47428,47447,47676,48253,48355';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(46570, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 3427, 0, 0, 2, 'Putrid Worg - In Combat - Cast ''Infected Wound'''),
(46570, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 85835, 0, 0, 2, 'Putrid Worg - In Combat - Cast ''Maul'''),
(46571, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 91289, 32, 0, 1, 'First Lieutenant Connor - Out of Combat - Cast ''Bone Shield'''),
(46571, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 43951, 0, 0, 2, 'First Lieutenant Connor - In Combat - Cast ''Bonegrinder'''),
(46571, 0, 2, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 86699, 0, 0, 2, 'First Lieutenant Connor - Within 0-5 Range - Cast ''Shockwave'''),
(46606, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 78744, 0, 0, 2, 'Spiny Tidecrawler - In Combat - Cast ''Bubblebeam'''),
(46606, 0, 1, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 61685, 0, 0, 2, 'Spiny Tidecrawler - Within 8-25 Range - Cast ''Charge'''),
(46606, 0, 2, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 16827, 0, 0, 2, 'Spiny Tidecrawler - In Combat - Cast ''Claw'''),
(46641, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 83643, 64, 0, 2, 'Ghastly Dockhand - In Combat - Cast ''Slippery Suds'''),
(46643, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 18266, 64, 0, 2, 'Accursed Shipbuilder - In Combat - Cast ''Curse of Agony'''),
(46643, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 90607, 0, 0, 2, 'Accursed Shipbuilder - In Combat - Cast ''Curse of Impotence'''),
(46643, 0, 2, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 11980, 0, 0, 2, 'Accursed Shipbuilder - In Combat - Cast ''Curse of Weakness'''),
(46823, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 13737, 0, 0, 2, 'Restless Infantry - In Combat - Cast ''Mortal Strike'''),
(46823, 0, 1, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 81140, 0, 0, 1, 'Restless Infantry - Within 0-8 Range - Cast ''Thunderclap'''),
(46823, 0, 2, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 15576, 0, 0, 1, 'Restless Infantry - Within 0-8 Range - Cast ''Whirlwind'''),
(46825, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 13737, 0, 0, 2, 'Restless Soldier - In Combat - Cast ''Mortal Strike'''),
(46825, 0, 1, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 81140, 0, 0, 1, 'Restless Soldier - Within 0-8 Range - Cast ''Thunderclap'''),
(46825, 0, 2, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 15576, 0, 0, 1, 'Restless Soldier - Within 0-8 Range - Cast ''Whirlwind'''),
(46960, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 38557, 0, 0, 2, 'Ghastly Scavenger - In Combat - Cast ''Throw'''),
(47287, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 9080, 0, 0, 2, 'Captain P. Harris - In Combat - Cast ''Hamstring'''),
(47287, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 57846, 0, 0, 2, 'Captain P. Harris - In Combat - Cast ''Heroic Strike'''),
(47287, 0, 2, 0, 0, 0, 100, 0, 10000, 12000, 16000, 20000, 11, 32736, 0, 0, 2, 'Captain P. Harris - In Combat - Cast ''Mortal Strike'''),
(47287, 0, 3, 0, 0, 0, 100, 0, 3000, 6000, 20000, 25000, 11, 79871, 32, 0, 1, 'Captain P. Harris - In Combat - Cast ''Recklessness'''),
(47304, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 78828, 32, 0, 1, 'Commander Largo - In Combat - Cast ''Bladestorm'''),
(47304, 0, 1, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 80983, 0, 0, 1, 'Commander Largo - Within 0-8 Range - Cast ''Commanding Shout'''),
(47304, 0, 2, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 32736, 0, 0, 2, 'Commander Largo - In Combat - Cast ''Mortal Strike'''),
(47373, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 13730, 0, 0, 1, 'Warden''s Sentry - Within 0-8 Range - Cast ''Demoralizing Shout'''),
(47428, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 63227, 0, 0, 1, 'Hellscream''s Sentry - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(47428, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 79881, 0, 0, 2, 'Hellscream''s Sentry - In Combat - Cast ''Slam'''),
(47447, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 13281, 64, 0, 2, 'Keep Lord Farson - In Combat - Cast ''Earth Shock'''),
(47447, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 7020, 32, 0, 1, 'Keep Lord Farson - In Combat - Cast ''Stoneform'''),
(47676, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 17253, 0, 0, 2, 'Baradin Fox - In Combat - Cast ''Bite'''),
(47676, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 21000, 26000, 11, 61684, 32, 0, 1, 'Baradin Fox - In Combat - Cast ''Dash'''),
(47676, 0, 2, 0, 2, 0, 100, 0, 0, 40, 30000, 35000, 11, 79853, 0, 0, 1, 'Baradin Fox - Between 0-40% Health - Cast ''Evasion'''),
(48253, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 22120, 0, 0, 2, 'Baradin Guard - Within 8-25 Range - Cast ''Charge'''),
(48253, 0, 1, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 80983, 0, 0, 1, 'Baradin Guard - Within 0-8 Range - Cast ''Commanding Shout'''),
(48253, 0, 2, 0, 0, 0, 100, 0, 3000, 6000, 18000, 24000, 11, 18395, 0, 0, 2, 'Baradin Guard - In Combat - Cast ''Dismounting Shot'''),
(48253, 0, 3, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 18328, 0, 0, 1, 'Baradin Guard - Within 0-8 Range - Cast ''Incapacitating Shout'''),
(48355, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 22120, 0, 0, 2, 'Hellscream Guard - Within 8-25 Range - Cast ''Charge'''),
(48355, 0, 1, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 80983, 0, 0, 1, 'Hellscream Guard - Within 0-8 Range - Cast ''Commanding Shout'''),
(48355, 0, 2, 0, 0, 0, 100, 0, 3000, 6000, 18000, 24000, 11, 18395, 0, 0, 2, 'Hellscream Guard - In Combat - Cast ''Dismounting Shot'''),
(48355, 0, 3, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 18328, 0, 0, 1, 'Hellscream Guard - Within 0-8 Range - Cast ''Incapacitating Shout''');
