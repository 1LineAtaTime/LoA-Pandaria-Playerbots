-- Hour of Twilight: creature abilities
-- No reference scripts these creatures of Hour of Twilight (the Twilight's Hammer and Deathwing's servants): they cast their sniffed spells (creature_template spell1-8).

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '54555,58235';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(54555, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 105432, 0, 0, 1, 'Frozen Servitor - Within 0-8 Range - Cast ''Icy Boulder'''),
(58235, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 108183, 0, 0, 1, 'Twilight Siege Captain - Within 0-8 Range - Cast ''Twilight Submission'''),
(58235, 0, 1, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 108172, 0, 0, 1, 'Twilight Siege Captain - Within 0-8 Range - Cast ''Twilight Volley''');
