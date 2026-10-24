-- Scholomance: creature abilities
-- The Expired Test Subjects and Reanimated Corpses of MoP's Scholomance had no AI, and no reference scripts them. Cast their
-- sniffed spells (creature_template spell1-8).

SET @ENTRIES := '59100,59501';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(59100, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 130260, 32, 0, 1, 'Expired Test Subject - In Combat - Cast ''Shadow Blink'''),
(59501, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 114493, 64, 0, 2, 'Reanimated Corpse - In Combat - Cast ''Dark Plague''');
