-- Blackrock Caverns: creature abilities
-- Most of the dungeon's creatures (the Twilight's Hammer cultists, their elementals and the Blackrock's beasts) had no AI or ran
-- classic rows: port their abilities from the Cataclysm data, falling back to the classic data where Cataclysm has none, keeping
-- the quest scripts already in place; gossip and quest reward scripts of the references are left out, keeping the dungeon
-- (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '40017';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(40017, 0, 0, 0, 0, 0, 100, 6, 4000, 5000, 12000, 13000, 11, 76680, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 'Twilight Element Warden - In Combat - Cast Chain Lightning'),
(40017, 0, 1, 0, 9, 0, 100, 6, 0, 20, 9000, 10000, 11, 76677, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Twilight Element Warden - On Close - Cast Fire Blast'),
(40017, 0, 2, 0, 0, 0, 100, 6, 3000, 4000, 10000, 15000, 11, 58025, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Twilight Element Warden - In Combat - Cast Frost Buffet'),
(40017, 0, 3, 0, 9, 0, 100, 6, 0, 40, 15000, 18000, 11, 76682, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Twilight Element Warden - On Close - Cast Frostbomb'),
(40017, 0, 4, 0, 1, 0, 100, 0, 200, 300, 4000, 5000, 11, 75205, 0, 0, 0, 0, 0, 19, 40343, 200, 0, 0, 0, 0, 0, 'Twilight Element Warden - Out of Combat - Cast ''Bore''');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '40004,40015,40023';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(40004, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 76621, 0, 0, 2, 'Quicksilver - In Combat - Cast ''Flame Buffet'''),
(40015, 0, 0, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 76628, 0, 0, 2, 'Runty - In Combat - Cast ''Lava Drool'''),
(40015, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 76665, 64, 0, 2, 'Runty - In Combat - Cast ''Little Big Flame Breath'''),
(40023, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 76716, 0, 0, 2, 'Defiled Earth Rager - In Combat - Cast ''Bludgeoning Strike'''),
(40023, 0, 1, 0, 0, 0, 100, 0, 6000, 9000, 15000, 20000, 11, 76717, 0, 0, 2, 'Defiled Earth Rager - In Combat - Cast ''Meteor''');
