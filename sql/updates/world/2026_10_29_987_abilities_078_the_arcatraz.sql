-- #987 wave 1 (pure additions): The Arcatraz creature abilities, imported from DeltaTho PR 15 file 2026_10_24_91_world.sql.
-- 8 creatures, 24 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '20859,20864,20869,20873,20875,20883,20898,20901';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
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
(20869, 0, 0, 0, 2, 0, 100, 1, 0, 10, 0, 0, 11, 36719, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Arcatraz Sentinel - Between 0-10% Health - Cast ''Explode'' (No Repeat)'),
(20873, 0, 0, 0, 0, 0, 100, 0, 15000, 22000, 15000, 22000, 11, 36813, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Negaton Warp-Master - In Combat - Cast ''Summon Negaton Field'''),
(20875, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Negaton Screamer - On Reset - Set Event Phase 0'),
(20875, 0, 1, 0, 4, 0, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Negaton Screamer - On Aggro - Set Event Phase 1'),
(20875, 0, 2, 0, 0, 0, 100, 0, 17000, 25000, 17000, 25000, 11, 13704, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Negaton Screamer - In Combat - Cast ''Psychic Scream'''),
(20883, 0, 0, 0, 0, 0, 100, 0, 1000, 3000, 12000, 16000, 11, 36886, 0, 0, 0, 0, 0, 6, 50, 0, 0, 0, 0, 0, 0, 'Spiteful Temptress - In Combat - Cast ''Spiteful Fury'''),
(20883, 0, 1, 0, 0, 0, 100, 0, 14000, 15000, 25000, 25000, 11, 36866, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 'Spiteful Temptress - In Combat - Cast ''Domination'''),
(20883, 0, 2, 0, 9, 0, 100, 0, 0, 40, 7000, 12000, 11, 36868, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Spiteful Temptress - Within 0-40 Range - Cast ''Shadow Bolt'''),
(20898, 0, 0, 0, 0, 0, 100, 0, 4000, 6000, 10000, 12000, 11, 38855, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gargantuan Abyssal - In Combat - Cast ''Fire Shield'''),
(20898, 0, 1, 0, 0, 0, 100, 0, 7000, 9000, 12000, 17000, 11, 36837, 0, 0, 0, 0, 0, 5, 0, 1, 0, 0, 0, 0, 0, 'Gargantuan Abyssal - In Combat - Cast ''Meteor'''),
(20901, 0, 0, 0, 0, 0, 100, 2, 0, 0, 2400, 4800, 11, 22907, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Archer - In Combat CMC - Cast ''Shoot'' (Normal Dungeon)'),
(20901, 0, 1, 0, 0, 0, 100, 4, 0, 0, 2400, 4800, 11, 38940, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Archer - In Combat CMC - Cast ''Shoot'' (Heroic Dungeon)'),
(20901, 0, 2, 0, 0, 0, 100, 0, 12000, 18000, 15000, 20000, 11, 36827, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Archer - In Combat - Cast ''Hooked Net'''),
(20901, 0, 3, 0, 0, 0, 100, 0, 13000, 16000, 18000, 22000, 11, 23601, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Archer - In Combat - Cast ''Scatter Shot'''),
(20901, 0, 4, 0, 2, 0, 100, 0, 0, 30, 23000, 30000, 11, 36828, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sargeron Archer - Between 0-30% Health - Cast ''Rapid Fire''');
