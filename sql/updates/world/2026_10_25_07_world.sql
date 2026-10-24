-- Gundrak: creature abilities
-- Most of the dungeon's creatures (the Drakkari trolls and their beasts) had no AI or ran classic rows: port their abilities from
-- the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions or action lists LoA lacks are
-- left out, and AzerothCore's range and area events are converted, keeping the dungeon (normal/heroic) flags. The creatures no
-- reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '29768,29774';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(29768, 0, 0, 0, 0, 0, 100, 2, 4000, 9000, 9000, 12000, 11, 55602, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Unyielding Constrictor - In Combat - Cast ''Vicious Bite'''),
(29768, 0, 1, 0, 0, 0, 100, 4, 4000, 9000, 9000, 12000, 11, 59021, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Unyielding Constrictor - In Combat - Cast ''Vicious Bite'''),
(29768, 0, 2, 0, 0, 0, 100, 1, 1000, 1000, 0, 0, 11, 55603, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Unyielding Constrictor - In Combat - Cast ''Puncturing Strike'' (No Repeat)'),
(29774, 0, 0, 0, 0, 0, 100, 2, 5000, 10000, 17000, 20000, 11, 55703, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spitting Cobra - In Combat - Cast Cobra Strike'),
(29774, 0, 1, 0, 0, 0, 100, 4, 5000, 10000, 17000, 20000, 11, 59020, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spitting Cobra - In Combat - Cast Cobra Strike'),
(29774, 0, 2, 0, 0, 0, 100, 0, 1000, 5000, 7000, 11000, 11, 55700, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spitting Cobra - In Combat - Cast ''Venom Spit''');
