-- Old Hillsbrad Foothills: creature abilities
-- Most of the dungeon's creatures (the Durnholde wardens and veterans, the Infinite Dragonflight, the Tarren Mill guards) had no AI
-- or ran classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing
-- events, actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the
-- dungeon (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '2354,2385,23175,23177,23179,28132';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(2354, 0, 0, 0, 0, 0, 100, 0, 2200, 2600, 7400, 8100, 11, 31279, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Vicious Gray Bear - In Combat - Cast ''Swipe'''),
(2385, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 89582, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Foothill Stalker - In Combat - Cast ''Devour'''),
(23175, 0, 0, 0, 0, 0, 100, 0, 0, 5000, 8500, 10500, 11, 16856, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Infinite Slayer - In Combat - Cast Mortal Strike'),
(23177, 0, 0, 0, 0, 0, 100, 2, 0, 1000, 3500, 3500, 11, 15232, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Infinite Defiler - In Combat - Cast Shadow Bolt'),
(23177, 0, 1, 0, 0, 0, 100, 4, 0, 1000, 3500, 3500, 11, 38386, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Infinite Defiler - In Combat - Cast Shadow Bolt'),
(23177, 0, 2, 0, 0, 0, 100, 0, 5000, 6000, 13000, 15000, 11, 21068, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Infinite Defiler - In Combat - Cast Corruption'),
(23177, 0, 3, 0, 0, 0, 100, 2, 0, 10000, 25000, 35000, 11, 31977, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Infinite Defiler - In Combat - Cast Curse of Infinity'),
(23177, 0, 4, 0, 0, 0, 100, 4, 0, 10000, 25000, 35000, 11, 38387, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Infinite Defiler - In Combat - Cast Bane of Infinity'),
(23179, 0, 0, 0, 0, 0, 100, 0, 0, 5000, 8500, 10500, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Infinite Saboteur - In Combat - Cast Cleave'),
(23179, 0, 1, 0, 0, 0, 100, 0, 6000, 10000, 20000, 30000, 11, 31976, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Infinite Saboteur - In Combat - Cast Shadow Shield'),
(23179, 0, 2, 0, 0, 0, 100, 0, 5000, 6000, 13000, 15000, 11, 17234, 0, 0, 0, 0, 0, 5, 20, 0, 0, 0, 0, 0, 0, 'Infinite Saboteur - In Combat - Cast Shadow Shock'),
(28132, 0, 0, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 11, 50736, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Don Carlos - Out of Combat - Cast Summon Guerrero'),
(28132, 0, 1, 0, 0, 0, 100, 2, 0, 0, 2500, 2500, 11, 16100, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Don Carlos - In Combat - Cast Shoot'),
(28132, 0, 2, 0, 0, 0, 100, 4, 0, 0, 2500, 2500, 11, 16496, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Don Carlos - In Combat - Cast Shoot'),
(28132, 0, 3, 0, 9, 0, 100, 2, 0, 20, 15000, 17000, 11, 12024, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Don Carlos - In Combat - Cast Net'),
(28132, 0, 4, 0, 9, 0, 100, 4, 0, 30, 15000, 17000, 11, 50762, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Don Carlos - In Combat - Cast Net'),
(28132, 0, 5, 0, 9, 0, 100, 2, 0, 10, 20000, 20000, 11, 50733, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Don Carlos - In Combat - Cast Scatter Shot'),
(28132, 0, 6, 0, 9, 0, 100, 4, 0, 10, 20000, 20000, 11, 46681, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Don Carlos - In Combat - Cast Scatter Shot');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '23176,23178';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(23176, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 15749, 0, 0, 2, 'Tarren Mill Guardsman - Within 8-25 Range - Cast ''Shield Charge'''),
(23178, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 17174, 64, 0, 2, 'Tarren Mill Lookout - In Combat - Cast ''Concussive Shot'''),
(23178, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 35511, 0, 0, 2, 'Tarren Mill Lookout - In Combat - Cast ''Serpent Sting'''),
(23178, 0, 2, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 15620, 0, 0, 2, 'Tarren Mill Lookout - In Combat - Cast ''Shoot''');
