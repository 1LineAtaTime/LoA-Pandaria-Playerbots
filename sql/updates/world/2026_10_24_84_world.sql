-- Auchenai Crypts: creature abilities
-- Most of the dungeon's creatures (the Auchenai monks and mystics, the Phantasmal Possessors, the Raging Skeletons and soul eaters)
-- had no AI or ran classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows
-- needing events, actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping
-- the dungeon (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '18493,18524,18702';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(18493, 0, 0, 0, 1, 0, 100, 7, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Soulpriest - Out of Combat - Disable Combat Movement'),
(18493, 0, 7, 8, 3, 1, 100, 7, 0, 15, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Soulpriest - Between 0-15% Mana - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18493, 0, 8, 0, 61, 1, 100, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Soulpriest - Between 0-15% Mana - Increment Phase By 1 (Phase 1) (No Repeat) (Dungeon)'),
(18493, 0, 9, 0, 9, 1, 100, 7, 35, 80, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Soulpriest - Within 35-80 Range - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18493, 0, 10, 0, 9, 1, 100, 7, 5, 15, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Soulpriest - Within 5-15 Range - Disable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18493, 0, 11, 0, 9, 1, 100, 7, 0, 5, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Soulpriest - Within 0-5 Range - Enable Combat Movement (Phase 1) (No Repeat) (Dungeon)'),
(18493, 0, 12, 0, 3, 0, 100, 6, 30, 100, 100, 100, 23, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Soulpriest - Between 30-100% Mana - Decrement Phase By 1 (Phase 1) (No Repeat) (Dungeon)'),
(18493, 0, 13, 0, 0, 0, 100, 0, 400, 4650, 2850, 4650, 11, 32860, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Soulpriest - In Combat - Cast ''Shadow Bolt'''),
(18493, 0, 14, 0, 0, 0, 100, 0, 13800, 31900, 13800, 31900, 11, 32859, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Soulpriest - In Combat - Cast ''Falter'''),
(18493, 0, 15, 0, 0, 0, 100, 0, 3600, 6500, 16900, 33700, 11, 32858, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Soulpriest - In Combat - Cast ''Touch of the Forgotten'''),
(18524, 0, 0, 0, 1, 0, 100, 1, 500, 500, 0, 0, 11, 32885, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Angered Skeleton - Out of Combat - Cast Infuriate'),
(18702, 0, 0, 0, 14, 0, 100, 2, 4000, 15, 6000, 8000, 11, 33325, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Necromancer - Friendly At 4000 Health - Cast ''Shadow Mend'' (Phase 1) (No Repeat) (Dungeon)'),
(18702, 0, 1, 0, 14, 0, 100, 4, 7000, 15, 6000, 8000, 11, 37367, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Necromancer - Friendly At 7000 Health - Cast ''Shadow Mend'' (Phase 1) (No Repeat) (Dungeon)'),
(18702, 0, 2, 0, 0, 0, 100, 0, 1000, 15200, 4800, 15700, 11, 35839, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Necromancer - In Combat - Cast ''Drain Soul'''),
(18702, 0, 3, 0, 0, 0, 100, 2, 12100, 29000, 22900, 35000, 11, 32863, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Necromancer - In Combat - Cast ''Seed of Corruption'' (Normal Dungeon)'),
(18702, 0, 4, 0, 0, 0, 100, 4, 12100, 17500, 4800, 9600, 11, 38252, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Auchenai Necromancer - In Combat - Cast ''Seed of Corruption'' (Heroic Dungeon)'),
(18702, 0, 5, 0, 74, 0, 100, 2, 0, 70, 15700, 26500, 11, 33325, 0, 0, 0, 0, 0, 9, 0, 0, 15, 0, 0, 0, 0, 'Auchenai Necromancer - On Friendly Between 0-70% Health - Cast ''Shadow Mend'' (Normal Dungeon)'),
(18702, 0, 6, 0, 74, 0, 100, 4, 0, 70, 6100, 8400, 11, 37367, 0, 0, 0, 0, 0, 9, 0, 0, 15, 0, 0, 0, 0, 'Auchenai Necromancer - On Friendly Between 0-70% Health - Cast ''Shadow Mend'' (Heroic Dungeon)');
