-- Deadwind Pass: creature abilities
-- Most of the zone's creatures (the Deadwind ogres and widows) had no AI or ran classic rows. Port their abilities from the
-- Cataclysm data, and from the classic data where Cataclysm has none, keeping the quest scripts already in place; gossip and quest
-- reward scripts of the references are left out. The creatures no reference scripts cast their sniffed spells (creature_template
-- spell1-8).

SET @ENTRIES := '7379';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(7379, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 20824, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast bolt'),
(7379, 0, 1, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Deadwind Ogre Mage - Between 0-15% Health - Flee For Assist (No Repeat)'),
(7379, 0, 2, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 11, 12550, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Deadwind Ogre Mage - Ooc - Cast Lightning Shield'),
(7379, 0, 3, 0, 0, 0, 100, 0, 4700, 8800, 10500, 13800, 11, 15039, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Deadwind Mauler - In Combat - Cast ''Flame Shock'''),
(7379, 0, 4, 0, 2, 0, 100, 1, 0, 45, 0, 0, 11, 6742, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Deadwind Ogre Mage - Between 0-45% Health - Cast ''Bloodlust'' (No Repeat)'),
(7379, 0, 5, 0, 16, 0, 100, 0, 6742, 30, 20000, 40000, 11, 6742, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Deadwind Ogre Mage - At 30% Health Less - Cast ''Bloodlust'' on Friendlies (Repeat)');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '51987,57910';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(51987, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 6751, 0, 0, 2, 'Deadwind Widow - In Combat - Cast ''Weak Poison'''),
(57910, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 110038, 32, 0, 1, 'Nalice - In Combat - Cast ''Arcane Destruction''');
