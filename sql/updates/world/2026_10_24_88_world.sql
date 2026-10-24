-- The Steamvault: creature abilities
-- Most of the dungeon's creatures (the Coilfang naga and engineers, the steam elementals, the Bog Overlords) had no AI or ran
-- classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events,
-- actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon
-- (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '17799,17800,21338,21694';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(17799, 0, 0, 0, 2, 0, 100, 1, 0, 20, 0, 0, 11, 8269, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Dreghood Slave - Between 0-20% Health - Cast ''Frenzy'' (No Repeat)'),
(17800, 0, 0, 0, 12, 0, 100, 7, 0, 20, 0, 0, 11, 7160, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Myrmidon - Target Between 0-20% Health - Cast ''Execute'' (No Repeat) (Dungeon)'),
(17800, 0, 1, 0, 0, 0, 100, 0, 3000, 5000, 5000, 7000, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Myrmidon - In Combat - Cast ''Cleave'''),
(17800, 0, 2, 0, 0, 0, 100, 0, 15000, 30000, 30000, 30000, 11, 18765, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Myrmidon - In Combat - Cast ''Sweeping Strikes'''),
(21338, 0, 0, 0, 1, 0, 100, 7, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Out of Combat - Disable Combat Movement (Phase 1)'),
(21338, 0, 1, 0, 0, 0, 100, 7, 0, 0, 0, 0, 30, 1, 3, 5, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - In Combat - Set Random Phase(1, 3, 5) (Phase 1)'),
(21338, 0, 7, 8, 3, 1, 100, 7, 0, 15, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Between 0-15% Mana - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(21338, 0, 8, 0, 61, 1, 100, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Between 0-15% Mana - Increment Phase By 1 (Phase 1) (No Repeat) (Dungeon)'),
(21338, 0, 9, 0, 9, 1, 100, 7, 35, 80, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Within 35-80 Range - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(21338, 0, 10, 0, 9, 1, 100, 7, 5, 15, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Within 5-15 Range - Disable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(21338, 0, 11, 0, 9, 1, 100, 7, 0, 5, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Within 0-5 Range - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(21338, 0, 14, 0, 3, 2, 100, 6, 30, 100, 100, 100, 23, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Between 30-100% Mana - Decrement Phase By 1 (Phase 1) (No Repeat) (Dungeon)'),
(21338, 0, 19, 20, 9, 4, 100, 7, 25, 80, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Within 25-80 Range - Enable Combat Movement (Phase 4) (No Repeat) (Dungeon)'),
(21338, 0, 20, 0, 61, 4, 100, 0, 0, 0, 0, 0, 20, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Within 25-80 Range - Start Attacking (Phase 4) (No Repeat) (Dungeon)'),
(21338, 0, 21, 22, 9, 4, 100, 7, 0, 5, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Within 0-5 Range - Enable Combat Movement (Phase 4) (No Repeat) (Dungeon)'),
(21338, 0, 22, 23, 61, 4, 100, 0, 0, 0, 0, 0, 40, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Within 0-5 Range - Set Sheath Melee (Phase 4) (No Repeat) (Dungeon)'),
(21338, 0, 23, 0, 61, 4, 100, 0, 0, 0, 0, 0, 20, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Within 0-5 Range - Start Attacking (Phase 4) (No Repeat) (Dungeon)'),
(21338, 0, 24, 25, 9, 4, 100, 7, 5, 15, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Within 5-15 Range - Disable Combat Movement (Phase 4) (No Repeat) (Dungeon)'),
(21338, 0, 25, 0, 61, 4, 100, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Within 5-15 Range - Stop Attacking (Phase 4) (No Repeat) (Dungeon)'),
(21338, 0, 28, 0, 0, 8, 100, 1, 1000, 1000, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - In Combat - Enable Combat Movement (Phase 4) (No Repeat) (Dungeon)'),
(21338, 0, 33, 34, 2, 0, 100, 7, 0, 15, 0, 0, 22, 6, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Between 0-15% Health - Set Event Phase 6 (No Repeat) (Dungeon)'),
(21338, 0, 34, 35, 61, 0, 100, 0, 0, 0, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Between 0-15% Health - Enable Combat Movement (No Repeat) (Dungeon)'),
(21338, 0, 35, 0, 61, 0, 100, 0, 0, 0, 0, 0, 25, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - Between 0-15% Health - Flee For Assist (No Repeat) (Dungeon)'),
(21338, 0, 36, 0, 7, 0, 100, 7, 0, 0, 0, 0, 40, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - On Evade - Set Sheath Melee (No Repeat) (Dungeon)'),
(21338, 0, 37, 0, 4, 0, 100, 0, 0, 0, 0, 0, 31, 1, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - On Aggro - Set Phase Random Between 1-3'),
(21338, 0, 38, 0, 74, 1, 100, 2, 0, 40, 10000, 11000, 11, 11642, 0, 0, 0, 0, 0, 9, 0, 0, 40, 0, 0, 0, 0, 'Coilfang Leper - Friendly Below 40% HP - Cast ''Heal'' (Phase 1) (Normal Dungeon)'),
(21338, 0, 39, 0, 74, 1, 100, 4, 0, 40, 10000, 11000, 11, 15586, 0, 0, 0, 0, 0, 9, 0, 0, 40, 0, 0, 0, 0, 'Coilfang Leper - Friendly Below 40% HP - Cast ''Heal'' (Phase 1) (Heroic Dungeon)'),
(21338, 0, 40, 0, 0, 2, 100, 0, 4000, 6000, 7000, 8000, 11, 13446, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - In Combat - Cast ''Strike'' (Phase 2)'),
(21338, 0, 41, 0, 0, 2, 100, 0, 3000, 7000, 10000, 15000, 11, 13444, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - In Combat - Cast ''Sunder Armor'' (Phase 2)'),
(21338, 0, 42, 0, 0, 2, 100, 0, 9000, 14000, 12000, 15000, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - In Combat - Cast ''Cleave'' (Phase 2)'),
(21338, 0, 43, 0, 0, 4, 100, 2, 4000, 6000, 9000, 10000, 11, 13339, 0, 0, 0, 0, 0, 5, 20, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - In Combat - Cast ''Fire Blast'' (Phase 3) (Normal Dungeon)'),
(21338, 0, 44, 0, 0, 4, 100, 4, 4000, 6000, 9000, 10000, 11, 14145, 0, 0, 0, 0, 0, 5, 20, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - In Combat - Cast ''Fire Blast'' (Phase 3) (Heroic Dungeon)'),
(21338, 0, 45, 0, 0, 4, 100, 2, 0, 0, 3000, 3000, 11, 9613, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - In Combat - Cast ''Shadow Bolt'' (Phase 3) (Normal Dungeon)'),
(21338, 0, 46, 0, 0, 4, 100, 4, 0, 0, 3000, 3000, 11, 12739, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Coilfang Leper - In Combat - Cast ''Shadow Bolt'' (Phase 3) (Heroic Dungeon)'),
(21694, 0, 0, 0, 1, 0, 100, 3, 1000, 1000, 0, 0, 11, 37266, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bog Overlord - Out of Combat - Cast ''Disease Cloud'' (No Repeat)'),
(21694, 0, 1, 0, 1, 0, 100, 5, 1000, 1000, 0, 0, 11, 37863, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bog Overlord - Out of Combat - Cast ''Disease Cloud'' (No Repeat)'),
(21694, 0, 2, 0, 1, 0, 100, 0, 2000, 5000, 12000, 19000, 11, 34158, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bog Overlord - Out of Combat - Cast ''Fungal Decay'''),
(21694, 0, 3, 0, 0, 0, 100, 0, 3000, 4000, 5000, 7000, 11, 37272, 0, 0, 0, 0, 0, 5, 30, 0, 0, 0, 0, 0, 0, 'Bog Overlord - In Combat - Cast ''Poison Bolt'''),
(21694, 0, 4, 0, 0, 0, 100, 0, 7000, 9500, 12000, 15000, 11, 40340, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bog Overlord - In Combat - Cast ''Trample'''),
(21694, 0, 5, 0, 2, 0, 100, 1, 0, 20, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bog Overlord - Between 0-20% Health - Cast ''Enrage'' (No Repeat)');
