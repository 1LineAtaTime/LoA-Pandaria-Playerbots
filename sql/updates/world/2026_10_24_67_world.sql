-- Zul'Farrak: creature abilities
-- Most of the dungeon's creatures (the Sandfury trolls, the scarabs, basilisks and lashers) had no AI or ran classic rows. Cataclysm left these classic dungeons
-- largely as they were, so port their abilities from the classic (AzerothCore) data first, with its dungeon flags and spell-set
-- phases, falling back to the Cataclysm data; rows needing events, actions or action lists LoA lacks are left out. The creatures no reference scripts cast their sniffed spells (creature_template
-- spell1-8).

SET @ENTRIES := '5648,7246,7267,7268,7608,7795,7796,7797,8138,10080,10082';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(5648, 0, 0, 0, 2, 0, 100, 3, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Shadowcaster - Between 0-15% Health - Flee For Assist (No Repeat) (Normal Dungeon)'),
(5648, 0, 1, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 18396, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Shadowcaster - On Aggro - Cast Dismounting Blast'),
(5648, 0, 2, 0, 1, 0, 100, 0, 1000, 1000, 1200000, 1200000, 11, 20798, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Shadowcaster - Out of Combat - Cast Demon Skin'),
(5648, 0, 3, 0, 0, 0, 100, 0, 0, 1000, 3000, 4000, 11, 12471, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Shadowcaster - In Combat - Cast Shadow Bolt'),
(5648, 0, 4, 0, 0, 0, 100, 0, 4000, 10000, 13000, 24000, 11, 14032, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Sandfury Shadowcaster - In Combat - Cast Shadow Word: Pain'),
(7246, 0, 0, 0, 2, 0, 100, 3, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Shadowhunter - Between 0-15% Health - Flee For Assist (No Repeat) (Normal Dungeon)'),
(7246, 0, 1, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 18396, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Shadowhunter - On Aggro - Cast Dismounting Blast'),
(7246, 0, 2, 0, 0, 0, 100, 0, 0, 1000, 2000, 2000, 11, 15547, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Shadowhunter - In Combat - Cast Shoot'),
(7246, 0, 3, 0, 0, 0, 100, 0, 4000, 10000, 13000, 24000, 11, 11641, 0, 0, 0, 0, 0, 6, 20, 0, 0, 0, 0, 0, 0, 'Sandfury Shadowhunter - In Combat - Cast Hex'),
(7267, 0, 0, 0, 5, 0, 100, 2, 5000, 5000, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Say on Player Kill'),
(7267, 0, 1, 0, 2, 0, 100, 3, 0, 30, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Say Text at 30% HP'),
(7267, 0, 2, 0, 2, 0, 100, 3, 0, 60, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Say Text at 60% HP'),
(7267, 0, 3, 0, 4, 0, 100, 3, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Say on Aggro'),
(7267, 0, 4, 0, 0, 0, 100, 0, 3000, 8000, 7000, 11000, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Chief Ukorz Sandscalp - In Combat - Cast Cleave'),
(7267, 0, 5, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8269, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Chief Ukorz Sandscalp - Between Health 0-30% - Cast Frenzy'),
(7268, 0, 0, 0, 0, 0, 85, 2, 2000, 2000, 6000, 6000, 11, 12787, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sandfury Guardian - In Combat - Cast ''Thrash'' (Phase 2) (No Repeat) (Normal Dungeon)'),
(7608, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 3000, 6000, 11, 9734, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Murta Grimgut - In Combat - Cast Holy Smite'),
(7795, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 78801, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hydromancer Velratha - In Combat - Cast ''Water Bolt'''),
(7795, 0, 1, 0, 2, 0, 100, 1, 0, 50, 0, 0, 11, 12491, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hydromancer Velratha - Between 0-50% Health - Cast ''Healing Wave'' (No Repeat)'),
(7795, 0, 2, 0, 9, 0, 100, 0, 0, 20, 14000, 16000, 11, 78802, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hydromancer Velratha - Within 0-20 Range - Cast ''Crashing Wave'''),
(7796, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 19471, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Nekrum Gutchewer - On Aggro - Cast Berserker Charge'),
(7796, 0, 1, 0, 0, 0, 100, 0, 0, 20000, 30000, 60000, 11, 8600, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Nekrum Gutchewer - In Combat - Cast Fevered Plague'),
(7796, 0, 2, 0, 0, 0, 100, 0, 0, 16000, 12000, 20000, 11, 26141, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Nekrum Gutchewer - In Combat - Cast Hamstring'),
(7797, 0, 0, 0, 12, 0, 100, 2, 20, 20, 30000, 30000, 11, 77720, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Execute'),
(7797, 0, 1, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 7366, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ruuzlu - On Aggro - Cast Berserker Stance'),
(7797, 0, 2, 0, 0, 0, 100, 0, 3000, 8000, 7000, 11000, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ruuzlu - In Combat - Cast Cleave'),
(8138, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 8876, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sul''lithuz Broodling - In Combat - Cast ''Thrash'''),
(10080, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 13730, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sandarr Dunereaver - On Aggro - Cast ''Demoralizing Shout'''),
(10080, 0, 1, 0, 0, 0, 100, 0, 2700, 3100, 9600, 12300, 11, 14516, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sandarr Dunereaver - In Combat - Cast ''Strike'''),
(10080, 0, 2, 0, 13, 0, 100, 0, 19600, 19600, 0, 0, 11, 15615, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sandarr Dunereaver - In Combat - Cast ''Pummel'''),
(10082, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 6533, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Zerillis - On Aggro - Cast ''Net'''),
(10082, 0, 1, 0, 0, 0, 100, 0, 2100, 2500, 8600, 9400, 11, 15547, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Zerillis - In Combat - Cast ''Shoot'''),
(10082, 0, 2, 0, 2, 0, 100, 1, 20, 40, 0, 0, 11, 12551, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Zerillis - Between 20-40% Health - Cast ''Frost Shot'' (No Repeat)');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '7786';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(7786, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 11998, 0, 0, 2, 'Skeleton of Zum''rah - In Combat - Cast ''Strike'''),
(7786, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 18663, 0, 0, 2, 'Skeleton of Zum''rah - In Combat - Cast ''Shadow Shock''');
