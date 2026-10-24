-- Uldaman: creature abilities
-- Most of the dungeon's creatures (the Shadowforge dwarves, the Stonevault troggs, the Earthen guardians and the Venture Co.) had no AI or ran classic rows. Cataclysm left these classic dungeons
-- largely as they were, so port their abilities from the classic (AzerothCore) data first, with its dungeon flags and spell-set
-- phases, falling back to the Cataclysm data; rows needing events, actions or action lists LoA lacks are left out. The creatures no reference scripts cast their sniffed spells (creature_template
-- spell1-8).

SET @ENTRIES := '4847,4850,4852,4855,7023,7175,7291,7320,7321';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4847, 0, 0, 0, 0, 0, 100, 2, 8000, 11000, 12000, 16000, 11, 6726, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Shadowforge Relic Hunter - In Combat - Cast ''Silence'''),
(4850, 0, 0, 0, 1, 0, 100, 3, 1000, 1000, 0, 0, 11, 22766, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Cave Lurker - Out of Combat - Cast ''Sneak'' (No Repeat) (Normal Dungeon)'),
(4852, 0, 0, 0, 2, 0, 100, 2, 0, 50, 20000, 20000, 11, 11986, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Oracle - Between 0-50% Health - Cast ''Healing Wave'' (Normal Dungeon)'),
(4852, 0, 1, 0, 0, 0, 100, 2, 8000, 11000, 48000, 55000, 11, 5605, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Oracle - In Combat - Cast ''Healing Ward'' (Normal Dungeon)'),
(4852, 0, 2, 0, 0, 0, 100, 2, 4000, 4000, 30000, 35000, 11, 8264, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Oracle - In Combat - Cast ''Lava Spout Totem'' (Normal Dungeon)'),
(4852, 0, 3, 0, 2, 0, 100, 3, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Oracle - Between 0-15% Health - Flee For Assist (Normal Dungeon)'),
(4855, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 3417, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Brawler - On Reset - Cast Thrash'),
(4855, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Brawler - Between 0-30% Health - Cast Frenzy'),
(7023, 0, 0, 0, 0, 0, 100, 0, 1000, 11000, 16000, 24000, 11, 9906, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Obsidian Sentinel - In Combat - Cast Reflection'),
(7023, 0, 1, 0, 0, 0, 100, 0, 20000, 20000, 20000, 20000, 11, 10072, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Obsidian Sentinel - In Combat - Cast Splintered Obsidian'),
(7023, 0, 2, 0, 0, 0, 100, 0, 10000, 20000, 20000, 30000, 11, 10061, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Obsidian Sentinel - In Combat - Cast Summon Obsidian Shard'),
(7175, 0, 0, 0, 2, 0, 100, 2, 0, 30, 120000, 120000, 11, 8599, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Ambusher - Between 0-30% Health - Cast ''Enrage'' (Normal Dungeon)'),
(7175, 0, 1, 0, 2, 0, 100, 3, 0, 30, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Ambusher - Between 0-30% Health - Say Line 0 (Normal Dungeon)'),
(7175, 0, 2, 0, 2, 0, 100, 3, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Ambusher - Between 0-15% Health - Flee For Assist (Normal Dungeon)'),
(7291, 0, 0, 0, 4, 0, 100, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Galgann Firehammer - On Aggro - Say Line 0 (No Repeat) (Normal Dungeon)'),
(7291, 0, 1, 0, 0, 0, 100, 2, 9000, 11000, 13000, 16000, 11, 11969, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Galgann Firehammer - In Combat - Cast ''Fire Nova'' (No Repeat) (Normal Dungeon)'),
(7291, 0, 2, 0, 0, 0, 100, 2, 2000, 5000, 13000, 18000, 11, 3356, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Galgann Firehammer - In Combat - Cast ''Flame Lash'' (No Repeat) (Normal Dungeon)'),
(7291, 0, 3, 0, 0, 0, 100, 2, 5000, 6000, 14000, 19000, 11, 9482, 32, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Galgann Firehammer - In Combat - Cast ''Amplify Flames'' (No Repeat) (Normal Dungeon)'),
(7320, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 3417, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Mauler - On Reset - Cast Thrash'),
(7320, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Mauler - Between 0-30% Health - Cast Frenzy'),
(7321, 0, 0, 0, 0, 0, 100, 2, 9000, 12000, 18000, 21000, 11, 2602, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Flameweaver - In Combat - Cast ''Fire Shield IV'' (Normal Dungeon)'),
(7321, 0, 1, 0, 0, 0, 100, 2, 15000, 18000, 15000, 21000, 11, 7739, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Flameweaver - In Combat - Cast ''Inferno Shell'' (Normal Dungeon)'),
(7321, 0, 2, 0, 2, 0, 100, 3, 0, 15, 0, 0, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Stonevault Flameweaver - Between 0-15% Health - Flee For Assist (Normal Dungeon)');
