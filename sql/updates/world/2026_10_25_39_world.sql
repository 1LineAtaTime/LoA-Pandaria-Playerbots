-- The Veiled Stair: creature abilities
-- No reference scripts the zone's creatures (the creatures of the Veiled Stair), and most had no AI: they cast their sniffed spells
-- (creature_template spell1-8).

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '62902,62926';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(62902, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 16636, 0, 0, 2, 'Darkhatched Throatripper - Within 8-25 Range - Cast ''Berserker Charge'''),
(62926, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 18000, 24000, 11, 12024, 0, 0, 2, 'Exchange Guard - In Combat - Cast ''Net'''),
(62926, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 23337, 64, 0, 2, 'Exchange Guard - In Combat - Cast ''Shoot''');
