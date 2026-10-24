-- The Shattered Halls: creature abilities
-- Most of the dungeon's creatures (the Shattered Hand orcs and their beasts, the Shattered Hand Legionnaires, the heathens and
-- sharpshooters) had no AI or ran classic rows: port their abilities from the classic (AzerothCore) data, falling back to the
-- Cataclysm data; rows needing events, actions or action lists LoA lacks are left out, and AzerothCore's range and area events are
-- converted, keeping the dungeon (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells
-- (creature_template spell1-8).

SET @ENTRIES := '16593,16700,16704,17357,17465,17622,17695';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(16593, 0, 0, 1, 0, 0, 100, 0, 1500, 1500, 7000, 10000, 11, 36020, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Brawler - In Combat - Cast ''Curse of the Shattered Hand'''),
(16593, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 13, 100, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Brawler - In Combat - Set Single Threat 100-0'),
(16593, 0, 2, 0, 0, 0, 100, 6, 500, 500, 9500, 9500, 11, 36033, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Brawler - In Combat - Cast ''Kick'''),
(16593, 0, 3, 0, 0, 0, 100, 6, 4000, 4000, 5500, 12500, 11, 12787, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Brawler - In Combat - Cast ''Thrash'''),
(16700, 0, 0, 0, 0, 0, 100, 0, 1500, 5000, 240000, 240000, 11, 30472, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Legionnaire - In Combat - Cast ''Aura of Discipline'''),
(16700, 0, 1, 0, 13, 0, 100, 0, 10000, 15000, 0, 0, 11, 15615, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Legionnaire - On Victim Casting - Cast ''Pummel'''),
(16700, 0, 2, 0, 38, 0, 100, 0, 1, 1, 60000, 60000, 11, 30485, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Legionnaire - On Data Set 1 1 (Friendly Dies Nearby) - Cast ''Enrage'''),
(16704, 0, 0, 1, 1, 0, 100, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Out of Combat - Disable Combat Movement'),
(16704, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Out of Combat - Stop Attacking'),
(16704, 0, 10, 11, 9, 0, 100, 0, 25, 80, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Within 25-80 Range - Enable Combat Movement'),
(16704, 0, 11, 0, 61, 0, 100, 0, 0, 0, 0, 0, 20, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Within 25-80 Range - Start Attacking'),
(16704, 0, 12, 13, 9, 0, 100, 0, 0, 5, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Within 0-5 Range - Enable Combat Movement'),
(16704, 0, 13, 14, 61, 0, 100, 0, 0, 0, 0, 0, 40, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Within 0-5 Range - Set Sheath Melee'),
(16704, 0, 14, 0, 61, 0, 100, 0, 0, 0, 0, 0, 20, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Within 0-5 Range - Start Attacking'),
(16704, 0, 15, 16, 9, 0, 100, 0, 5, 15, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Within 5-15 Range - Disable Combat Movement'),
(16704, 0, 16, 0, 61, 0, 100, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Within 5-15 Range - Stop Attacking'),
(16704, 0, 23, 0, 7, 0, 100, 0, 0, 0, 0, 0, 40, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - On Evade - Set Sheath Melee'),
(16704, 0, 24, 0, 9, 0, 100, 2, 5, 30, 2300, 5000, 11, 15620, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Within 5-30 Range - Cast ''Shoot'' (Normal Dungeon)'),
(16704, 0, 25, 0, 9, 0, 100, 4, 5, 30, 2300, 5000, 11, 22907, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Within 5-30 Range - Cast ''Shoot'' (Heroic Dungeon)'),
(16704, 0, 26, 0, 9, 0, 100, 0, 5, 30, 6000, 9000, 11, 30481, 64, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Within 5-30 Range - Cast ''Incendiary Shot'''),
(16704, 0, 27, 0, 9, 0, 100, 4, 5, 30, 10000, 14000, 11, 37551, 64, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - Within 5-30 Range - Cast ''Viper Sting'' (Heroic Dungeon)'),
(16704, 0, 28, 0, 0, 0, 100, 0, 13350, 21000, 20700, 39250, 11, 23601, 64, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Sharpshooter - In Combat - Cast ''Scatter Shot'''),
(17357, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 3800, 6500, 11, 30494, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Creeping Oozeling - In Combat - Cast ''Sticky Ooze'''),
(17465, 0, 0, 0, 0, 0, 100, 2, 1500, 2500, 4000, 6000, 11, 15572, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Centurion - In Combat - Cast ''Sunder Armor'' (Normal Dungeon)'),
(17465, 0, 1, 0, 0, 0, 100, 4, 1500, 2500, 4000, 6000, 11, 16145, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Centurion - In Combat - Cast ''Sunder Armor'' (Heroic Dungeon)'),
(17465, 0, 2, 0, 0, 0, 100, 2, 3000, 7000, 17000, 21000, 11, 30931, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Centurion - In Combat - Cast ''Battle Shout'' (Normal Dungeon)'),
(17465, 0, 3, 0, 0, 0, 100, 4, 3000, 7000, 17000, 21000, 11, 31403, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Centurion - In Combat - Cast ''Battle Shout'' (Heroic Dungeon)'),
(17465, 0, 4, 0, 25, 0, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Centurion - On Reset - Set Event Phase 1 (Allow Sparring Match'),
(17465, 0, 5, 0, 4, 0, 100, 0, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Centurion - On Aggro - Set Event Phase 0'),
(17622, 0, 0, 1, 1, 0, 100, 7, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - Out of Combat - Disable Combat Movement (No Repeat) (Dungeon)'),
(17622, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - Out of Combat - Stop Attacking (No Repeat) (Dungeon)'),
(17622, 0, 10, 11, 9, 0, 100, 7, 25, 80, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - Within 25-80 Range - Enable Combat Movement (No Repeat) (Dungeon)'),
(17622, 0, 11, 0, 61, 0, 100, 0, 0, 0, 0, 0, 20, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - Within 25-80 Range - Start Attacking (No Repeat) (Dungeon)'),
(17622, 0, 12, 13, 9, 0, 100, 7, 0, 5, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - Within 0-5 Range - Enable Combat Movement (No Repeat) (Dungeon)'),
(17622, 0, 13, 14, 61, 0, 100, 0, 0, 0, 0, 0, 40, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - Within 0-5 Range - Set Sheath Melee (No Repeat) (Dungeon)'),
(17622, 0, 14, 0, 61, 0, 100, 0, 0, 0, 0, 0, 20, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - Within 0-5 Range - Start Attacking (No Repeat) (Dungeon)'),
(17622, 0, 15, 16, 9, 0, 100, 7, 5, 15, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - Within 5-15 Range - Disable Combat Movement (No Repeat) (Dungeon)'),
(17622, 0, 16, 0, 61, 0, 100, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - Within 5-15 Range - Stop Attacking (No Repeat) (Dungeon)'),
(17622, 0, 23, 0, 7, 0, 100, 7, 0, 0, 0, 0, 40, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - On Evade - Set Sheath Melee (Dungeon)'),
(17622, 0, 24, 0, 0, 0, 100, 2, 1200, 6100, 3100, 5600, 11, 15620, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - In Combat - Cast ''Shoot'' (Normal Dungeon)'),
(17622, 0, 25, 0, 0, 0, 100, 4, 1200, 6100, 3100, 5600, 11, 22907, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - In Combat - Cast ''Shoot'' (Heroic Dungeon)'),
(17622, 0, 26, 0, 0, 0, 100, 0, 12150, 30400, 12150, 30400, 11, 30481, 64, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - In Combat - Cast ''Incendiary Shot'''),
(17622, 0, 27, 0, 0, 0, 100, 0, 13350, 21000, 20700, 39250, 11, 23601, 64, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Sharpshooter Guard - In Combat - Cast ''Scatter Shot'''),
(17695, 0, 0, 0, 1, 0, 100, 7, 1000, 1000, 0, 0, 11, 30991, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Assassin - Out of Combat - Cast ''Stealth'' (No Repeat)'),
(17695, 0, 1, 0, 67, 0, 100, 0, 4500, 6500, 0, 0, 11, 30992, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Assassin - On Behind Target - Cast ''Backstab'''),
(17695, 0, 2, 0, 0, 0, 100, 0, 8000, 11000, 22000, 25000, 11, 36974, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Assassin - In Combat - Cast ''Wound Poison'''),
(17695, 0, 3, 0, 0, 0, 100, 0, 2000, 4500, 12000, 20000, 11, 30981, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Assassin - In Combat - Cast ''Crippling Poison'''),
(17695, 0, 4, 0, 10, 0, 100, 0, 0, 15, 12000, 15000, 11, 30980, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Assassin - Within 0-15 Range Out of Combat LoS - Cast ''Sap'''),
(17695, 0, 5, 0, 10, 0, 100, 0, 0, 8, 4000, 8000, 11, 30986, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Assassin - Within 0-8 Range Out of Combat LoS - Cast ''Cheap Shot'''),
(17695, 0, 6, 0, 9, 0, 100, 0, 0, 8, 4000, 8000, 11, 30986, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Shattered Hand Assassin - Within 0-8 Range - Cast ''Cheap Shot''');
