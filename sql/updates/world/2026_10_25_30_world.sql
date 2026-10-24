-- End Time: creature abilities
-- No reference scripts these creatures of End Time (the Infinite Dragonflight and the echoes' servants): they cast their sniffed spells (creature_template spell1-8).

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '54550';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(54550, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 102057, 64, 0, 2, 'Undying Flame - In Combat - Cast ''Scorched'''),
(54550, 0, 1, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 105238, 0, 0, 1, 'Undying Flame - Within 0-8 Range - Cast ''Undying Flame''');
