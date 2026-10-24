-- Blackrock Spire: creature abilities
-- Most of the dungeon's creatures (the Blackrock and Spirestone orcs and ogres, the Smolderthorn trolls, the dragonkin, scorpids and worgs) had no AI or ran classic rows. Cataclysm left these classic dungeons
-- largely as they were, so port their abilities from the classic (AzerothCore) data first, with its dungeon flags and spell-set
-- phases, falling back to the Cataclysm data; rows needing events, actions or action lists LoA lacks are left out. The creatures no reference scripts cast their sniffed spells (creature_template
-- spell1-8).

SET @ENTRIES := '9096,9267,9416,9596,9817,10376,10742';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(9096, 0, 0, 0, 0, 0, 100, 2, 4000, 13200, 6600, 14400, 11, 15580, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Rage Talon Dragonspawn - In Combat - Cast ''Strike'' (Phase 1) (No Repeat) (Normal Dungeon)'),
(9096, 0, 1, 0, 0, 0, 100, 2, 7700, 17100, 20300, 34200, 11, 12021, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Rage Talon Dragonspawn - In Combat - Cast ''Fixate'' (Phase 1) (No Repeat) (Normal Dungeon)'),
(9096, 0, 2, 0, 0, 0, 100, 2, 1400, 12300, 7100, 11700, 11, 15572, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Rage Talon Dragonspawn - In Combat - Cast ''Sunder Armor'' (Phase 1) (No Repeat) (Normal Dungeon)'),
(9096, 0, 3, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 8876, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rage Talon Dragonspawn - In Combat - Cast ''Thrash'' (Phase 1) (No Repeat) (Normal Dungeon)'),
(9096, 0, 4, 0, 2, 0, 100, 2, 0, 30, 120000, 120000, 11, 8269, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Rage Talon Dragonspawn - Between 0-30% Health - Cast ''Frenzy'' (Normal Dungeon)'),
(9267, 0, 0, 0, 0, 0, 100, 2, 0, 0, 2300, 3900, 11, 15795, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Smolderthorn Axe Thrower - In Combat CMC - Cast ''Throw'' (Normal Dungeon)'),
(9267, 0, 1, 0, 0, 0, 100, 2, 6000, 9000, 9000, 12000, 11, 12787, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Smolderthorn Axe Thrower - In Combat - Cast ''Thrash'' (Normal Dungeon)'),
(9267, 0, 2, 0, 0, 0, 100, 2, 9000, 14000, 11000, 15000, 11, 11428, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Smolderthorn Axe Thrower - In Combat - Cast ''Knockdown'' (Normal Dungeon)'),
(9267, 0, 3, 0, 2, 0, 100, 3, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Smolderthorn Axe Thrower - Between 0-15% Health - Flee For Assist (No Repeat) (Normal Dungeon)'),
(9416, 0, 0, 0, 2, 0, 100, 1, 40, 80, 0, 0, 11, 3604, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Scarshield Worg - Between 40-80% Health - Cast ''Tendon Rip'' (No Repeat)'),
(9596, 0, 0, 0, 0, 0, 100, 0, 2300, 3200, 9500, 11000, 11, 6466, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Every 9.5 - 11 seconds (2.3 - 3.2s initially)  - Self: Cast spell Axe Toss (6466) on Random hostile'),
(9817, 0, 1, 0, 4, 0, 100, 2, 0, 0, 0, 0, 39, 10, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dreadweaver - On Aggro - Call For Help (Normal Dungeon)'),
(9817, 0, 4, 0, 2, 0, 100, 3, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dreadweaver - Between 0-15% Health - Flee For Assist (Normal Dungeon)'),
(9817, 0, 5, 6, 1, 0, 100, 2, 0, 8000, 12000, 14000, 92, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dreadweaver - Out of Combat - Interrupt Spell (Normal Dungeon)'),
(9817, 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dreadweaver - Out of Combat - Set Event Phase 1 (Normal Dungeon)'),
(9817, 0, 7, 0, 1, 1, 100, 2, 0, 0, 0, 0, 5, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dreadweaver - Out of Combat - Play Emote 1 (Normal Dungeon)'),
(9817, 0, 8, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 12380, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dreadweaver - On Reset - Cast ''Shadow Channeling'' (Normal Dungeon)'),
(9817, 0, 9, 0, 0, 0, 100, 2, 0, 0, 3600, 6300, 11, 12739, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dreadweaver - In Combat - Cast ''Shadow Bolt'' (Normal Dungeon)'),
(9817, 0, 10, 0, 0, 0, 100, 2, 9400, 21400, 22000, 28800, 11, 7068, 3, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dreadweaver - In Combat - Cast ''Veil of Shadow'' (Normal Dungeon)'),
(9817, 0, 11, 12, 1, 1, 100, 2, 4000, 4000, 4000, 4000, 11, 12380, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dreadweaver - Out of Combat - Cast ''Shadow Channeling'' (Phase 1) (Normal Dungeon)'),
(9817, 0, 12, 0, 61, 1, 100, 0, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dreadweaver - Out of Combat - Set Event Phase 0 (Phase 1) (Normal Dungeon)'),
(9817, 0, 13, 0, 0, 0, 100, 0, 2000, 4000, 12000, 24000, 11, 6909, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dreadweaver - In Combat - Cast Curse of Thorns (timer not verified)'),
(10376, 0, 0, 0, 6, 0, 100, 2, 0, 0, 0, 0, 11, 16103, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Crystal Fang - On Death - Cast Spell Summon Spire Spiderling (16103)'),
(10742, 0, 0, 0, 0, 0, 100, 0, 10000, 20000, 15000, 30000, 11, 16637, 0, 0, 0, 0, 0, 26, 0, 0, 0, 0, 0, 0, 0, 'Blackhand Dragon Handler - In Combat - Cast ''Mend Dragon'' (timers not verified)');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '9218,9219,9701,9718,10177,10264';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(9218, 0, 0, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 10101, 0, 0, 2, 'Spirestone Battle Lord - In Combat - Cast ''Knock Away'''),
(9218, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 26141, 0, 0, 2, 'Spirestone Battle Lord - In Combat - Cast ''Hamstring'''),
(9218, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 16856, 0, 0, 2, 'Spirestone Battle Lord - In Combat - Cast ''Mortal Strike'''),
(9218, 0, 3, 0, 0, 0, 100, 0, 10000, 12000, 16000, 20000, 11, 15655, 0, 0, 2, 'Spirestone Battle Lord - In Combat - Cast ''Shield Slam'''),
(9219, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 16856, 0, 0, 2, 'Spirestone Butcher - In Combat - Cast ''Mortal Strike'''),
(9219, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 26141, 0, 0, 2, 'Spirestone Butcher - In Combat - Cast ''Hamstring'''),
(9701, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 25497, 64, 0, 2, 'Spire Scorpid - In Combat - Cast ''Venom Spit'''),
(9718, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 16856, 0, 0, 2, 'Ghok Bashguud - In Combat - Cast ''Mortal Strike'''),
(9718, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 24671, 0, 0, 2, 'Ghok Bashguud - In Combat - Cast ''Snap Kick'''),
(10177, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 25025, 64, 0, 2, 'Spire Scarab - In Combat - Cast ''Earth Shock'''),
(10264, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 16727, 0, 0, 1, 'Solakar Flamewreath - Within 0-8 Range - Cast ''War Stomp''');
