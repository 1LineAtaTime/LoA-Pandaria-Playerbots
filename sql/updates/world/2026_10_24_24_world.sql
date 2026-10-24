-- Ghostlands: creature abilities
-- Most of the zone is scripted, but the Ghostclaw cats, Mistbats, Deatholme Darkmages and Shadowpine Catlords lacked some or all of
-- their abilities. Port them from the Cataclysm data, and from the classic data where Cataclysm has none (Starving Ghostclaw,
-- Mistbat). The wild spiders cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '16318,16345,16347,16348,16353';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(16318, 0, 0, 0, 0, 0, 100, 0, 1000, 1000, 1700, 2000, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Deatholme Darkmage - IC - Cast ''Shadow Bolt'''),
(16318, 0, 1, 0, 0, 0, 100, 0, 7000, 11000, 32000, 36000, 11, 18267, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Deatholme Darkmage - IC - Cast ''Curse of Weakness'''),
(16318, 0, 2, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Deatholme Darkmage - Between 0-15% Health - Flee For Assist (No Repeat)'),
(16318, 0, 3, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 28729, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Deatholme Darkmage - On reset - Cast Ribbon of Souls'),
(16345, 0, 0, 0, 16, 0, 100, 0, 28902, 30, 20000, 40000, 11, 28902, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Shadowpine Catlord - On Friendly Unit Missing Buff ''Bloodlust'' - Cast ''Bloodlust'''),
(16345, 0, 1, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 11, 28904, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowpine Catlord - Out of Combat - Cast ''Summon Ghostclaw Lynx'' (No Repeat)'),
(16347, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 32918, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Starving Ghostclaw - On Aggro - Cast ''Chilling Howl'''),
(16348, 0, 0, 0, 0, 0, 100, 0, 3000, 7000, 8000, 12000, 11, 16827, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ghostclaw Lynx - IC - Cast Claw'),
(16348, 0, 1, 0, 0, 0, 100, 0, 5000, 10000, 10000, 150000, 11, 59881, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ghostclaw Lynx - IC - Cast Rake'),
(16353, 0, 0, 0, 2, 0, 100, 1, 20, 80, 0, 0, 11, 8281, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mistbat - Between 20-80% Health - Cast ''Sonic Burst'' (No Repeat)');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '14881';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(14881, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 18000, 24000, 11, 745, 0, 0, 2, 'Spider - In Combat - Cast ''Web'''),
(14881, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 744, 0, 0, 2, 'Spider - In Combat - Cast ''Poison'''),
(14881, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 6751, 0, 0, 2, 'Spider - In Combat - Cast ''Weak Poison''');
