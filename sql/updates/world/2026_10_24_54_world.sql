-- Swamp of Sorrows: creature abilities
-- Most of the zone's creatures (the Sawtooth and Marshtide wildlife, the naga, the Lost Ones and the Stonard and Marshtide forces) had no AI or ran classic rows. Port their abilities from the
-- Cataclysm data, and from the classic data where Cataclysm has none, keeping the quest scripts already in place; gossip and quest
-- reward scripts of the references are left out. The creatures no reference scripts cast their sniffed spells (creature_template
-- spell1-8).

SET @ENTRIES := '763,768,922,1088,46714,46770,46775';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(763, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Enrage at 30% HP'),
(768, 0, 0, 0, 0, 0, 100, 0, 2300, 2700, 9700, 12700, 11, 51772, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadow Panther - In Combat - Cast ''Claw'''),
(922, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 5424, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Claw Grasp'),
(922, 0, 1, 0, 25, 0, 50, 0, 0, 0, 0, 0, 11, 88038, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Stolen Silversnap Ice on Spawn'),
(1088, 0, 0, 0, 0, 0, 100, 0, 5000, 6000, 12000, 18500, 11, 13443, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Rend'),
(46714, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Shoot'),
(46714, 0, 1, 0, 9, 0, 100, 0, 0, 10, 19000, 25000, 11, 78578, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Immolation Trap on Close'),
(46714, 0, 2, 0, 9, 0, 100, 0, 5, 30, 18000, 22000, 11, 80009, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpent Sting on Close'),
(46714, 0, 3, 0, 9, 0, 100, 0, 5, 30, 10000, 20000, 11, 80012, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Chimera Shot on Close'),
(46770, 0, 0, 0, 1, 0, 100, 1, 3000, 5000, 0, 0, 11, 11939, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Summon Imp on Spawn'),
(46770, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Shadow Bolt'),
(46770, 0, 2, 0, 0, 0, 100, 0, 9000, 9000, 20000, 27000, 11, 11980, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Curse of Weakness'),
(46770, 0, 3, 0, 1, 0, 100, 1, 8000, 15000, 0, 0, 11, 87116, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Ritual of Summoning - Summon Portal'),
(46775, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 79858, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Frostbolt'),
(46775, 0, 1, 0, 0, 0, 100, 0, 8000, 8000, 19000, 25000, 11, 79860, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 'Cast Blizzard'),
(46775, 0, 2, 0, 1, 0, 100, 1, 8000, 15000, 0, 0, 11, 87116, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Ritual of Summoning - Summon Portal');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '14445,14881,46368,51789,51918';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(14445, 0, 0, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 12533, 0, 0, 2, 'Captain Wyrmak - Within 0-5 Range - Cast ''Acid Breath'''),
(14445, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 15496, 0, 0, 2, 'Captain Wyrmak - In Combat - Cast ''Cleave'''),
(14881, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 18000, 24000, 11, 745, 0, 0, 2, 'Spider - In Combat - Cast ''Web'''),
(14881, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 744, 0, 0, 2, 'Spider - In Combat - Cast ''Poison'''),
(14881, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 6751, 0, 0, 2, 'Spider - In Combat - Cast ''Weak Poison'''),
(46368, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 6951, 0, 0, 2, 'Enthralled Makrura - In Combat - Cast ''Decayed Strength'''),
(51789, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 80012, 0, 0, 2, 'Marshtide Sentry - In Combat - Cast ''Chimera Shot'''),
(51789, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 22910, 0, 0, 2, 'Marshtide Sentry - In Combat - Cast ''Immolation Trap'''),
(51789, 0, 2, 0, 0, 0, 100, 0, 10000, 13000, 15000, 19000, 11, 80009, 0, 0, 2, 'Marshtide Sentry - In Combat - Cast ''Serpent Sting'''),
(51789, 0, 3, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 2, 'Marshtide Sentry - In Combat - Cast ''Shoot'''),
(51918, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 12170, 0, 0, 2, 'Stonard Grunt - In Combat - Cast ''Revenge'''),
(51918, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 12169, 32, 0, 1, 'Stonard Grunt - In Combat - Cast ''Shield Block'''),
(51918, 0, 2, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 2, 'Stonard Grunt - In Combat - Cast ''Shoot''');
