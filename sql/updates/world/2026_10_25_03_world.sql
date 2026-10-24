-- Wintergrasp: creature abilities
-- Most of the zone's creatures (the elementals and creatures of the Wintergrasp lake) had no AI or ran classic rows: port their
-- abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions or action lists
-- LoA lacks are left out, and AzerothCore's range and area events are converted. The creatures no reference scripts cast their
-- sniffed spells (creature_template spell1-8).

SET @ENTRIES := '30842,30872';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(30842, 0, 0, 0, 0, 0, 100, 0, 4000, 8000, 12000, 24000, 11, 38240, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Wandering Shadow - In Combat - Cast ''Chilling Touch'''),
(30842, 0, 1, 0, 0, 0, 50, 0, 25000, 30000, 25000, 30000, 11, 18267, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Wandering Shadow - In Combat - Cast ''Curse of Weakness'''),
(30872, 0, 0, 0, 0, 0, 100, 0, 4000, 8000, 12000, 20000, 11, 51131, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadow Revenant - In Combat - Cast ''Strangulate''');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '30739,30740,30849,30875';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(30739, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 42648, 32, 0, 1, 'Warsong Champion - Out of Combat - Cast ''Sleeping Sleep'''),
(30739, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 11976, 0, 0, 2, 'Warsong Champion - In Combat - Cast ''Strike'''),
(30740, 0, 0, 0, 1, 0, 100, 0, 1000, 2000, 30000, 30000, 11, 42648, 32, 0, 1, 'Valiance Expedition Champion - Out of Combat - Cast ''Sleeping Sleep'''),
(30740, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 11976, 0, 0, 2, 'Valiance Expedition Champion - In Combat - Cast ''Strike'''),
(30849, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 43543, 0, 0, 2, 'Chilled Earth Elemental - In Combat - Cast ''Throw Ice'''),
(30875, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 56620, 32, 0, 1, 'Tempest Revenant - In Combat - Cast ''Seething Flames''');
