-- Timeless Isle: creature abilities
-- No reference scripts the creatures of Timeless Isle (the isle's elementals, spirits, Ordos's followers and the wildlife), and most had no AI
-- or sniffed spells: they cast the abilities Wowhead (MoP Classic) lists for them, with any sniffed spells (creature_template spell1-8).

-- Creatures without reference data: their Wowhead abilities and sniffed spell1-8
SET @ENTRIES := '71908';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(71908, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 147345, 0, 0, 2, 'Skunky Brew Alemental - In Combat - Cast ''Brew Bolt'''),
(71908, 0, 1, 0, 0, 0, 100, 0, 2000, 5000, 20000, 25000, 11, 148584, 0, 0, 1, 'Skunky Brew Alemental - In Combat - Cast ''Skunky Suds''');
