-- The Arcatraz: creature abilities
-- Most of the dungeon's creatures (the prisoners of the Arcatraz, the Protean horrors and spawns, the Sightless Eyes, the Ethereum)
-- had no AI or ran classic rows: port their abilities from the classic (AzerothCore) data, falling back to the Cataclysm data; rows
-- needing events, actions or action lists LoA lacks are left out, and AzerothCore's range and area events are converted, keeping
-- the dungeon (normal/heroic) flags. The creatures no reference scripts cast their sniffed spells (creature_template spell1-8).

SET @ENTRIES := '20859,20864,20866,20869,20873,20875,20879,20880,20881,20882,20883,20898,20901,20902';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(20859, 0, 0, 0, 1, 0, 100, 0, 10000, 18000, 5000, 5000, 11, 36327, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Arcatraz Warder - Out of Combat - Cast Spell Shoot Arcane Explosion Arrow'),
(20859, 0, 1, 0, 0, 0, 100, 2, 2000, 2000, 3000, 3000, 11, 35012, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Arcatraz Warder - In Combat - Cast Spell Shoot'),
(20859, 0, 2, 0, 0, 0, 100, 4, 2000, 2000, 3000, 3000, 11, 22907, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Arcatraz Warder - In Combat - Cast Spell Shoot'),
(20859, 0, 3, 0, 0, 0, 100, 2, 5000, 8000, 6000, 6000, 11, 36609, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Arcatraz Warder - In Combat - Cast Spell Arcane Shoot'),
(20859, 0, 4, 0, 0, 0, 100, 4, 5000, 8000, 6000, 6000, 11, 38807, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Arcatraz Warder - In Combat - Cast Spell Arcane Shoot'),
(20859, 0, 5, 0, 9, 0, 100, 0, 0, 5, 10000, 10000, 11, 35963, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Arcatraz Warder - Within Range 0-5yd - Cast Spell Improved Wing Clip'),
(20864, 0, 0, 0, 0, 0, 100, 0, 16900, 17600, 19300, 26500, 11, 36617, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Protean Nightmare - In Combat - Cast ''Gaping Maw'''),
(20864, 0, 1, 0, 0, 0, 100, 0, 22900, 26300, 10800, 22900, 11, 36619, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Protean Nightmare - In Combat - Cast ''Infectious Poison'''),
(20864, 0, 2, 0, 0, 0, 100, 0, 15200, 21700, 10900, 22900, 11, 36622, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Protean Nightmare - In Combat - Cast ''Incubation'''),
(20866, 0, 0, 0, 2, 0, 100, 0, 0, 30, 14000, 21000, 11, 33958, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Soul Devourer - Between 0-30% Health - Cast ''Enrage'''),
(20866, 0, 1, 0, 0, 0, 100, 0, 7000, 11000, 15000, 23000, 11, 36654, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Soul Devourer - In Combat - Cast ''Fel Breath'''),
(20866, 0, 2, 0, 0, 0, 100, 0, 15000, 17000, 120000, 180000, 11, 36644, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Soul Devourer - In Combat - Cast ''Sightless Eye'''),
(20869, 0, 0, 0, 2, 0, 100, 1, 0, 10, 0, 0, 11, 36719, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Arcatraz Sentinel - Between 0-10% Health - Cast ''Explode'' (No Repeat)'),
(20873, 0, 0, 0, 0, 0, 100, 0, 15000, 22000, 15000, 22000, 11, 36813, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Negaton Warp-Master - In Combat - Cast ''Summon Negaton Field'''),
(20875, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Negaton Screamer - On Reset - Set Event Phase 0'),
(20875, 0, 1, 0, 4, 0, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Negaton Screamer - On Aggro - Set Event Phase 1'),
(20875, 0, 2, 0, 0, 0, 100, 0, 17000, 25000, 17000, 25000, 11, 13704, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Negaton Screamer - In Combat - Cast ''Psychic Scream'''),
(20879, 0, 0, 0, 1, 0, 100, 0, 0, 0, 0, 0, 11, 36784, 0, 0, 0, 0, 0, 17, 0, 100, 0, 0, 0, 0, 0, 'Eredar Soul-Eater - At Range - Cast Entropic Aura'),
(20879, 0, 1, 0, 0, 0, 100, 0, 6000, 15000, 15000, 22000, 11, 36786, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Eredar Soul-Eater - In Combat - Cast ''Soul Chill'''),
(20879, 0, 2, 0, 0, 0, 100, 0, 15000, 17000, 14000, 21000, 11, 36778, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Eredar Soul-Eater - In Combat - Cast ''Soul Steal'''),
(20879, 0, 3, 0, 31, 0, 100, 0, 36778, 0, 0, 0, 11, 36782, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Eredar Soul-Eater - On Target Spellhit ''Soul Steal'' - Cast ''Soul Steal'''),
(20880, 0, 0, 0, 11, 0, 100, 1, 0, 0, 0, 0, 11, 27987, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Eredar Deathbringer - On Respawn - Cast ''Unholy Aura'' (Phase 1) (No Repeat)'),
(20880, 0, 1, 0, 0, 0, 100, 0, 6000, 12000, 8000, 14000, 11, 36787, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Eredar Deathbringer - In Combat - Cast ''Forceful Cleave'''),
(20881, 0, 0, 0, 0, 0, 100, 0, 6000, 10000, 13000, 22000, 11, 36887, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Unbound Devastator - In Combat - Cast ''Deafening Roar'''),
(20881, 0, 1, 0, 9, 0, 100, 0, 0, 8, 14000, 21000, 11, 36891, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Unbound Devastator - Within 0-8 Range - Cast ''Devastate'''),
(20882, 0, 0, 0, 11, 0, 100, 1, 0, 0, 0, 0, 11, 16380, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Skulking Witch - On Respawn - Cast ''Greater Invisibility'' (Phase 1) (No Repeat)'),
(20882, 0, 1, 0, 0, 0, 100, 0, 4000, 10000, 10000, 20000, 11, 36862, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skulking Witch - In Combat - Cast ''Gouge'''),
(20883, 0, 0, 0, 0, 0, 100, 0, 1000, 3000, 12000, 16000, 11, 36886, 0, 0, 0, 0, 0, 6, 50, 0, 0, 0, 0, 0, 0, 'Spiteful Temptress - In Combat - Cast ''Spiteful Fury'''),
(20883, 0, 1, 0, 0, 0, 100, 0, 14000, 15000, 25000, 25000, 11, 36866, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 'Spiteful Temptress - In Combat - Cast ''Domination'''),
(20883, 0, 2, 0, 9, 0, 100, 0, 0, 40, 7000, 12000, 11, 36868, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spiteful Temptress - Within 0-40 Range - Cast ''Shadow Bolt'''),
(20898, 0, 0, 0, 0, 0, 100, 0, 4000, 6000, 10000, 12000, 11, 38855, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gargantuan Abyssal - In Combat - Cast ''Fire Shield'''),
(20898, 0, 1, 0, 0, 0, 100, 0, 7000, 9000, 12000, 17000, 11, 36837, 0, 0, 0, 0, 0, 5, 0, 1, 0, 0, 0, 0, 0, 'Gargantuan Abyssal - In Combat - Cast ''Meteor'''),
(20901, 0, 0, 0, 0, 0, 100, 2, 0, 0, 2400, 4800, 11, 22907, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Archer - In Combat CMC - Cast ''Shoot'' (Normal Dungeon)'),
(20901, 0, 1, 0, 0, 0, 100, 4, 0, 0, 2400, 4800, 11, 38940, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Archer - In Combat CMC - Cast ''Shoot'' (Heroic Dungeon)'),
(20901, 0, 2, 0, 0, 0, 100, 0, 12000, 18000, 15000, 20000, 11, 36827, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Archer - In Combat - Cast ''Hooked Net'''),
(20901, 0, 3, 0, 0, 0, 100, 0, 13000, 16000, 18000, 22000, 11, 23601, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Archer - In Combat - Cast ''Scatter Shot'''),
(20901, 0, 4, 0, 2, 0, 100, 0, 0, 30, 23000, 30000, 11, 36828, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Archer - Between 0-30% Health - Cast ''Rapid Fire'''),
(20902, 0, 0, 0, 0, 0, 100, 0, 7000, 10000, 13000, 20000, 11, 36829, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Hellcaller - In Combat - Cast ''Hell Rain'''),
(20902, 0, 1, 0, 0, 0, 100, 0, 8000, 13000, 11000, 15000, 11, 36832, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Hellcaller - In Combat - Cast ''Incinerate'''),
(20902, 0, 2, 0, 0, 0, 100, 0, 7000, 10000, 16000, 21000, 11, 36831, 32, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Hellcaller - In Combat - Cast ''Curse of the Elements''');
