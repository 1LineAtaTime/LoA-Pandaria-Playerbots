-- Mogu'shan Palace: creature abilities
-- No reference scripts the dungeon's creatures (the mogu, the Gurthan, Harthak and Kargesh clans and the Glintrok), and most had no
-- AI: they cast their sniffed spells (creature_template spell1-8).

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '61239';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(61239, 0, 0, 0, 2, 0, 100, 0, 0, 40, 15000, 20000, 11, 118940, 0, 0, 1, 'Glintrok Oracle - Between 0-40% Health - Cast ''Cleansing Flame''');
