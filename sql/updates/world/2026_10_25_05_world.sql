-- Ahn'kahet: The Old Kingdom: creature abilities
-- Most of the dungeon's creatures (the Twilight cultists, the Ahn'kahar nerubians and the faceless ones) had no AI or ran classic
-- rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events, actions
-- or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon
-- (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '30111,30179,30319,30338';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(30111, 0, 0, 0, 0, 0, 100, 2, 0, 0, 3400, 4800, 11, 17290, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Twilight Worshipper - Combat CMC - Cast ''Fireball'' (Normal Dungeon)'),
(30111, 0, 1, 0, 0, 0, 100, 4, 0, 0, 5000, 8000, 11, 61567, 64, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Twilight Worshipper - Combat CMC - Cast ''Fireball'' (Heroic Dungeon)'),
(30111, 0, 2, 0, 0, 0, 100, 2, 9000, 12000, 14000, 17000, 11, 56858, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Twilight Worshipper - Combat - Cast ''Flamestrike'' (Normal Dungeon)'),
(30111, 0, 3, 0, 0, 0, 100, 4, 9000, 12000, 14000, 17000, 11, 61568, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Twilight Worshipper - Combat - Cast ''Flamestrike'' (Heroic Dungeon)'),
(30111, 0, 4, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 11, 56711, 32, 0, 0, 0, 0, 19, 30413, 5, 0, 0, 0, 0, 0, 'Twilight Worshipper - Out of Combat - Cast ''Image Channel'' (No Repeat)'),
(30111, 0, 5, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 11, 56713, 32, 0, 0, 0, 0, 19, 30413, 5, 0, 0, 0, 0, 0, 'Twilight Worshipper - Out of Combat - Cast ''Image Channel'' (No Repeat)'),
(30179, 0, 0, 0, 11, 0, 100, 2, 0, 0, 0, 0, 11, 12550, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Twilight Apostle - On Respawn - Cast ''Lightning Shield'' (No Repeat)'),
(30179, 0, 1, 0, 11, 0, 100, 4, 0, 0, 0, 0, 11, 61570, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Twilight Apostle - On Respawn - Cast ''Lightning Shield'' (No Repeat)'),
(30179, 0, 2, 0, 14, 0, 100, 6, 10000, 40, 14000, 17000, 11, 11986, 1, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Twilight Apostle - Friendly At 10000 Health - Cast ''Healing Wave'' (No Repeat)'),
(30179, 0, 3, 0, 2, 0, 100, 6, 0, 30, 12000, 15000, 11, 28902, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Twilight Apostle - Between 0-30% Health - Cast ''Bloodlust'' (No Repeat)'),
(30179, 0, 4, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 11, 56711, 32, 0, 0, 0, 0, 19, 30413, 5, 0, 0, 0, 0, 0, 'Twilight Apostle - Out of Combat - Cast ''Image Channel'' (No Repeat)'),
(30179, 0, 5, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 11, 56713, 32, 0, 0, 0, 0, 19, 30413, 5, 0, 0, 0, 0, 0, 'Twilight Apostle - Out of Combat - Cast ''Image Channel'' (No Repeat)'),
(30319, 0, 0, 0, 0, 0, 100, 2, 0, 0, 4000, 5000, 11, 12739, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Twilight Darkcaster - In Combat - Cast ''Shadow Bolt'''),
(30319, 0, 1, 0, 0, 0, 100, 4, 0, 0, 5000, 7000, 11, 61562, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Twilight Darkcaster - In Combat - Cast ''Shadow Bolt'''),
(30319, 0, 2, 0, 0, 0, 100, 2, 4000, 7000, 17000, 20000, 11, 56898, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Twilight Darkcaster - In Combat - Cast ''Corruption'''),
(30319, 0, 3, 0, 0, 0, 100, 4, 4000, 7000, 17000, 20000, 11, 61563, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Twilight Darkcaster - In Combat - Cast ''Corruption'''),
(30319, 0, 4, 0, 0, 0, 100, 6, 12000, 15000, 19000, 23000, 11, 13338, 1, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 'Twilight Darkcaster - In Combat - Cast ''Curse of Tongues'''),
(30319, 0, 5, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 11, 56711, 32, 0, 0, 0, 0, 19, 30413, 5, 0, 0, 0, 0, 0, 'Twilight Darkcaster - Out of Combat - Cast ''Image Channel'' (No Repeat)'),
(30319, 0, 6, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 11, 56713, 32, 0, 0, 0, 0, 19, 30413, 5, 0, 0, 0, 0, 0, 'Twilight Darkcaster - Out of Combat - Cast ''Image Channel'' (No Repeat)'),
(30338, 0, 0, 0, 0, 0, 100, 0, 1000, 1000, 35000, 45000, 11, 56354, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ahn''kahar Swarmer - In Combat - Cast ''Sprint'''),
(30338, 0, 1, 2, 11, 0, 100, 6, 0, 0, 0, 0, 11, 56158, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ahn''kahar Swarmer - In Combat - Cast ''Sprint'''),
(30338, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 11, 56281, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ahn''kahar Swarmer - On Spawn - Cast Swarm (Dungeon Only)');
