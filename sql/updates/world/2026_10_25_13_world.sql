-- Pit of Saron: creature abilities
-- Most of the dungeon's creatures (the Scourge, the Ymirjar and the overseers of the enslaved prisoners) had no AI or ran classic
-- rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions
-- or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon
-- (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '36840,36842,36892,36896,37712,37728';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36840, 0, 0, 0, 60, 0, 100, 65537, 0, 0, 0, 0, 11, 50142, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On Update - Cast Emerge'),
(36840, 0, 1, 0, 0, 0, 100, 0, 2000, 7000, 8000, 15000, 11, 69603, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Blight'),
(36842, 0, 0, 0, 0, 0, 100, 0, 500, 2000, 3500, 3500, 11, 69573, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frostbolt'),
(36842, 0, 1, 0, 0, 0, 100, 0, 7000, 9000, 13000, 15000, 11, 69574, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Freezing Circle'),
(36892, 0, 0, 0, 0, 0, 100, 0, 1000, 3000, 3000, 4000, 11, 69528, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Empowered Shadow Bolt'),
(36896, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 4000, 5000, 11, 69520, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gargoyle Strike'),
(36896, 0, 1, 0, 2, 0, 100, 1, 1, 15, 100000, 100000, 11, 69575, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stoneform'),
(37712, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 4000, 4000, 11, 70386, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadow Bolt'),
(37728, 0, 0, 0, 0, 0, 100, 0, 1000, 3000, 5000, 6000, 11, 75330, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadow Bolt');
