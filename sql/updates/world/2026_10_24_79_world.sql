-- Hellfire Ramparts: creature abilities
-- Most of the dungeon's creatures (the Bonechewer and Bleeding Hollow orcs, the Shattered Hand, the hellhounds) had no AI or ran
-- classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows needing events,
-- actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping the dungeon
-- (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '17264,17269,17455,17478';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(17264, 0, 0, 0, 11, 0, 100, 1, 0, 0, 0, 0, 11, 18950, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Ravener - On Respawn - Cast ''Invisibility and Stealth Detection'' (Phase 2)'),
(17264, 0, 1, 0, 0, 0, 100, 0, 6100, 17200, 20650, 26950, 11, 30621, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Ravener - In Combat - Cast Kidney Shot'),
(17269, 0, 0, 1, 1, 0, 100, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - Out of Combat - Disable Combat Movement'),
(17269, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - Out of Combat - Set Event Phase 0'),
(17269, 0, 8, 9, 3, 0, 100, 0, 0, 15, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - Between 0-15% Mana - Enable Combat Movement'),
(17269, 0, 9, 0, 61, 0, 100, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - Between 0-15% Mana - Increment Phase By 1'),
(17269, 0, 10, 0, 9, 0, 100, 7, 25, 80, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - Within 25-80 Range - Enable Combat Movement'),
(17269, 0, 11, 0, 9, 0, 100, 7, 5, 15, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - Within 5-15 Range - Disable Combat Movement'),
(17269, 0, 12, 0, 9, 0, 100, 7, 0, 5, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - Within 0-5 Range - Enable Combat Movement'),
(17269, 0, 13, 0, 3, 0, 100, 6, 30, 100, 100, 100, 23, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - Between 30-100% Mana - Decrement Phase By 1'),
(17269, 0, 14, 0, 0, 0, 100, 2, 0, 2050, 1750, 2050, 11, 15241, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - In Combat - Cast ''Scorch'' (Normal Dungeon)'),
(17269, 0, 15, 0, 0, 0, 100, 4, 0, 2050, 1750, 2050, 11, 36807, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - In Combat - Cast ''Scorch'' (Heroic Dungeon)'),
(17269, 0, 16, 0, 0, 0, 100, 2, 15650, 23000, 20650, 31600, 11, 20754, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - In Combat - Cast ''Rain of Fire'' (Normal Dungeon)'),
(17269, 0, 17, 0, 0, 0, 100, 4, 15650, 23000, 20650, 31600, 11, 36808, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Darkcaster - In Combat - Cast ''Rain of Fire'' (Heroic Dungeon)'),
(17455, 0, 0, 0, 4, 0, 100, 7, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Beastmaster - On Aggro - Say Line 0 (No Repeat)'),
(17455, 0, 1, 0, 0, 0, 100, 6, 4200, 12400, 30000, 30000, 11, 9128, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Beastmaster - In Combat - Cast ''Battle Shout'' (No Repeat)'),
(17455, 0, 2, 0, 0, 0, 100, 6, 1200, 7100, 10800, 12800, 11, 10966, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Beastmaster - In Combat - Cast ''Uppercut'' (No Repeat)'),
(17455, 0, 3, 4, 0, 0, 100, 7, 17000, 17000, 0, 0, 12, 17280, 1, 300000, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Beastmaster - In Combat - Summon Creature ''Shattered Hand Warhound'' (No Repeat) (Dungeon)'),
(17455, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Beastmaster - In Combat - Say Line 1 (No Repeat) (Dungeon)'),
(17455, 0, 5, 0, 0, 0, 100, 7, 17020, 17020, 0, 0, 12, 17280, 1, 300000, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Beastmaster - In Combat - Summon Creature ''Shattered Hand Warhound'' (No Repeat) (Dungeon)'),
(17455, 0, 6, 0, 0, 0, 50, 7, 17020, 17020, 0, 0, 12, 17280, 1, 300000, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Beastmaster - In Combat - Summon Creature ''Shattered Hand Warhound'' (No Repeat) (Dungeon)'),
(17455, 0, 7, 0, 0, 0, 20, 7, 17030, 17030, 0, 0, 12, 17280, 1, 300000, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Beastmaster - In Combat - Summon Creature ''Shattered Hand Warhound'' (No Repeat) (Dungeon)'),
(17478, 0, 0, 0, 11, 0, 100, 7, 0, 0, 0, 0, 11, 31059, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Scryer - On Respawn - Cast Hellfire Channeling'),
(17478, 0, 1, 0, 21, 0, 100, 7, 0, 0, 0, 0, 11, 31059, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Scryer - On Reached Home - Cast Hellfire Channeling'),
(17478, 0, 2, 0, 0, 0, 100, 2, 800, 1700, 3600, 4700, 11, 12471, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Scryer - In Combat - Cast ''Shadow Bolt'' (Normal Dungeon)'),
(17478, 0, 3, 0, 0, 0, 100, 4, 800, 1700, 3600, 4700, 11, 15232, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Scryer - In Combat - Cast ''Shadow Bolt'' (Heroic Dungeon)'),
(17478, 0, 4, 0, 0, 0, 100, 0, 4850, 23500, 15450, 23800, 11, 30615, 1, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Scryer - In Combat - Cast ''Fear'''),
(17478, 0, 5, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 30659, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bleeding Hollow Scryer - On Death - Cast Fel Infusion');
