-- The Temple of Atal'Hakkar: creature abilities
-- Most of the dungeon's creatures (the Atal'ai trolls, the dragonkin and the oozes) had no AI or ran classic rows. Cataclysm left these classic dungeons
-- largely as they were, so port their abilities from the classic (AzerothCore) data first, with its dungeon flags and spell-set
-- phases, falling back to the Cataclysm data; rows needing events, actions or action lists LoA lacks are left out. The creatures no reference scripts cast their sniffed spells (creature_template
-- spell1-8).

SET @ENTRIES := '5277,5280,5400,8319';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(5277, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 3637, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Nightmare Scalebane - On Reset - Cast Improved Blocking III'),
(5280, 0, 0, 0, 0, 0, 100, 0, 0, 1000, 3000, 4000, 11, 15653, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Nightmare Wyrmkin - In Combat - Cast Acid Spit'),
(5280, 0, 1, 0, 0, 0, 100, 0, 4000, 12000, 12000, 18000, 11, 12098, 0, 0, 0, 0, 0, 6, 30, 0, 0, 0, 0, 0, 0, 'Nightmare Wyrmkin - In Combat - Cast Sleep'),
(5400, 0, 0, 0, 0, 0, 100, 0, 0, 5000, 60000, 60000, 11, 8282, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Every 60 - 60 seconds (0 - 5s initially) - Self: Cast spell Curse of Blood (8282) on Victim'),
(5400, 0, 1, 0, 0, 0, 100, 0, 5000, 10000, 120000, 120000, 11, 7102, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Every 120 - 120 seconds (5 - 10s initially) - Self: Cast spell Contagion of Rot (7102) on Victim'),
(8319, 0, 0, 0, 0, 0, 100, 0, 2400, 4900, 11800, 17200, 11, 15653, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Nightmare Whelp - In Combat - Cast ''Acid Spit''');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '5399,14881,46068,46424,46623';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(5399, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 1, 'Veyzhak the Cannibal - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(14881, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 18000, 24000, 11, 745, 0, 0, 2, 'Spider - In Combat - Cast ''Web'''),
(14881, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 744, 0, 0, 2, 'Spider - In Combat - Cast ''Poison'''),
(14881, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 6751, 0, 0, 2, 'Spider - In Combat - Cast ''Weak Poison'''),
(46068, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 81109, 64, 0, 2, 'Corrupted Guardian - In Combat - Cast ''Poison Bolt'''),
(46068, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 11639, 0, 0, 2, 'Corrupted Guardian - In Combat - Cast ''Shadow Word: Pain'''),
(46424, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 18266, 0, 0, 2, 'Priestess Udum''bra - In Combat - Cast ''Curse of Agony'''),
(46424, 0, 1, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9613, 64, 0, 2, 'Priestess Udum''bra - In Combat - Cast ''Shadow Bolt'''),
(46623, 0, 0, 0, 0, 0, 100, 0, 2000, 5000, 20000, 25000, 11, 79893, 0, 0, 1, 'Gomora the Bloodletter - In Combat - Cast ''Bloodworm'''),
(46623, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 88844, 0, 0, 2, 'Gomora the Bloodletter - In Combat - Cast ''Cauterizing Strike''');
