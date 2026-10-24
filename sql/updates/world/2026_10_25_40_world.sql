-- Shado-Pan Monastery: creature abilities
-- No reference scripts the dungeon's creatures (the Sha, the Shado-Pan's corrupted and the Hozen raiders), and most had no AI: they
-- cast their sniffed spells (creature_template spell1-8).

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '59741';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(59741, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 106421, 0, 0, 2, 'Shado-Pan Guardian - In Combat - Cast ''Palm Strike'''),
(59741, 0, 1, 0, 9, 0, 100, 0, 0, 5, 10000, 14000, 11, 128304, 0, 0, 2, 'Shado-Pan Guardian - Within 0-5 Range - Cast ''Roundhouse Kick''');
