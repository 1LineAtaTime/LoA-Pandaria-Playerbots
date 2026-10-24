-- Utgarde Pinnacle: creature abilities
-- Most of the dungeon's creatures (the Ymirjar vrykul, the Scourge and the proto-drakes) had no AI or ran classic rows: port their
-- abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions or action lists
-- LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon (normal/heroic) flags. The
-- creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '26555';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(26555, 0, 0, 0, 0, 0, 100, 0, 5000, 7000, 15000, 17000, 11, 48697, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Scourge Hulk - In Combat - Cast ''Mighty Blow'''),
(26555, 0, 1, 0, 0, 0, 100, 0, 10000, 10000, 20000, 20000, 11, 59228, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Scourge Hulk - In Combat - Cast ''Volatile Infection''');
