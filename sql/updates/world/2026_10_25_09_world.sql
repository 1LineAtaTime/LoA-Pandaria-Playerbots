-- Halls of Lightning: creature abilities
-- Most of the dungeon's creatures (the iron dwarves, the stormforged and the titan constructs) had no AI or ran classic rows: port
-- their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions or action
-- lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon (normal/heroic) flags.
-- The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '28580';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(28580, 0, 0, 0, 0, 0, 100, 2, 0, 0, 2300, 5000, 11, 16100, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hardened Steel Skycaller - Combat CMC - Cast ''Shoot'' (Normal Dungeon)'),
(28580, 0, 1, 0, 0, 0, 100, 4, 0, 0, 2300, 5000, 11, 61515, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hardened Steel Skycaller - Combat CMC - Cast ''Shoot'' (Heroic Dungeon)'),
(28580, 0, 2, 0, 0, 0, 100, 2, 9000, 12000, 15000, 20000, 11, 52754, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Hardened Steel Skycaller - Combat - Cast ''Impact Shot'' (Normal Dungeon)'),
(28580, 0, 3, 0, 0, 0, 100, 2, 7000, 12000, 12000, 18000, 11, 52775, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Hardened Steel Skycaller - Combat - Cast ''Impact Multi-Shot'' (Normal Dungeon)'),
(28580, 0, 4, 0, 0, 0, 100, 4, 9000, 12000, 15000, 20000, 11, 59148, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Hardened Steel Skycaller - Combat - Cast ''Impact Shot'' (Heroic Dungeon)'),
(28580, 0, 5, 0, 0, 0, 100, 4, 7000, 12000, 12000, 18000, 11, 59147, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Hardened Steel Skycaller - Combat - Cast ''Impact Multi-Shot'' (Heroic Dungeon)'),
(28580, 0, 6, 0, 9, 0, 100, 6, 0, 5, 6000, 9000, 11, 61507, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hardened Steel Skycaller - 0-5 Range - Cast ''Disengage'' (Dungeon)'),
(28580, 0, 7, 0, 0, 0, 100, 2, 7000, 12000, 12000, 18000, 11, 52755, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Hardened Steel Skycaller - In Combat - Cast ''Impact Multi-Shot'' (Normal Dungeon)');
