-- The Slave Pens: creature abilities
-- Most of the dungeon's creatures (the Coilfang naga, the Wastewalker slaves, the bog lords and rays) had no AI or ran classic
-- rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions
-- or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon
-- (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '17959,17960,40446';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(17959, 0, 0, 0, 1, 0, 90, 6, 60000, 120000, 360000, 420000, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Slavehandler - Out of Combat - Say Line 0 (Phase 8) (No Repeat) (Dungeon)'),
(17959, 0, 1, 0, 1, 0, 90, 6, 180000, 240000, 540000, 680000, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Slavehandler - Out of Combat - Say Line 1 (Phase 8) (No Repeat) (Dungeon)'),
(17959, 0, 2, 0, 4, 0, 20, 7, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Slavehandler - On Aggro - Say Line 2 (Phase 8) (No Repeat) (Dungeon)'),
(17959, 0, 3, 0, 0, 0, 100, 0, 6150, 15800, 19400, 25500, 11, 9080, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Slavehandler - In Combat - Cast Hamstring'),
(17959, 0, 4, 0, 0, 0, 100, 0, 12150, 24250, 24250, 27900, 11, 16172, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Slavehandler - In Combat - Cast Head Crack'),
(17959, 0, 5, 0, 1, 0, 50, 0, 5000, 25000, 20000, 40000, 11, 6754, 0, 0, 0, 0, 0, 19, 17964, 5, 0, 0, 0, 0, 0, 'Coilfang Slavehandler - Out Of Combat - Cast Slap!'),
(17960, 0, 0, 0, 1, 0, 100, 7, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Soothsayer - Out of Combat - Disable Combat Movement (Phase 8) (No Repeat) (Dungeon)'),
(17960, 0, 4, 5, 3, 1, 100, 7, 0, 15, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Soothsayer - Between 0-15% Mana - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(17960, 0, 5, 0, 61, 1, 100, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Soothsayer - Between 0-15% Mana - Increment Phase By 1 (Phase 1) (No Repeat) (Dungeon)'),
(17960, 0, 6, 0, 9, 1, 100, 7, 25, 80, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Soothsayer - Within 25-80 Range - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(17960, 0, 7, 0, 9, 1, 100, 7, 5, 15, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Soothsayer - Within 5-15 Range - Disable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(17960, 0, 8, 0, 9, 1, 100, 7, 0, 5, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Soothsayer - Within 0-5 Range - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(17960, 0, 9, 0, 3, 0, 100, 6, 30, 100, 100, 100, 23, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Soothsayer - Between 30-100% Mana - Decrement Phase By 1 (Phase 1) (No Repeat) (Dungeon)'),
(17960, 0, 10, 0, 0, 0, 100, 0, 9950, 12150, 23100, 24300, 11, 15790, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Soothsayer - In Combat - Cast ''Arcane Missiles'''),
(17960, 0, 11, 0, 0, 0, 100, 0, 19400, 27600, 26500, 42300, 11, 31555, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Soothsayer - In Combat - Cast ''Decayed Intellect'''),
(17960, 0, 12, 0, 0, 0, 100, 0, 20650, 25500, 23100, 24300, 11, 30923, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Soothsayer - In Combat - Cast ''Domination'''),
(40446, 0, 0, 0, 1, 0, 100, 1, 0, 0, 0, 0, 11, 75427, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Skar''this the Summoner - OOC - Cast ''Frost Channelling'''),
(40446, 0, 1, 0, 0, 0, 100, 0, 5000, 5000, 15000, 15000, 11, 55909, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skar''this the Summoner - IC - Cast Crashing Wave'),
(40446, 0, 2, 0, 0, 0, 100, 0, 10000, 10000, 20000, 20000, 11, 11831, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skar''this the Summoner - IC - Cast Frost Nova'),
(40446, 0, 3, 0, 0, 0, 100, 0, 7000, 7000, 9000, 9000, 11, 15043, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skar''this the Summoner - IC - Cast Frostbolt');
