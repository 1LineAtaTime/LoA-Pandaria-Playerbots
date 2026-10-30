-- #987 wave 1 (pure additions): Blackrock Depths creature abilities, imported from DeltaTho PR 15 file 2026_10_24_68_world.sql.
-- 17 creatures, 48 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '8889,8898,8899,8902,8903,8906,8907,8909,8911,8912,8920,8922,8923,9499,9545,9547,9554';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(8889, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 13589, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Anvilrage Overseer - On Reset - Cast Haste Aura'),
(8898, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 8258, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Anvilrage Marshal - On Aggro - Cast Pummel'),
(8898, 0, 1, 0, 0, 0, 100, 0, 5000, 7000, 7000, 9000, 11, 13953, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Anvilrage Marshal - In Combat - Cast Holy Strike'),
(8899, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2000, 4000, 11, 15620, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Doomforge Dragoon - In Combat - Cast Shoot'),
(8899, 0, 1, 0, 0, 0, 100, 0, 4000, 6000, 8000, 12000, 11, 15495, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Doomforge Dragoon - In Combat - Cast Explosive Shot'),
(8899, 0, 2, 0, 0, 0, 100, 0, 5000, 7000, 8000, 12000, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Doomforge Dragoon - In Combat - Cast Cleave'),
(8902, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 31, 1, 5, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowforge Citizen - On Aggro - Set Phase Random Between 1-5'),
(8902, 0, 1, 0, 0, 1, 100, 0, 7000, 9000, 8000, 12000, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadowforge Citizen - In Combat - Cast Cleave (Phase 1)'),
(8902, 0, 2, 0, 0, 1, 100, 0, 5000, 7000, 7000, 10000, 11, 13446, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadowforge Citizen - In Combat - Cast Strike (Phase 1)'),
(8902, 0, 3, 0, 9, 2, 100, 0, 0, 10, 15000, 18000, 11, 11831, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shadowforge Citizen - In Combat - Cast Frost Nova (Phase 2)'),
(8902, 0, 4, 0, 0, 4, 100, 0, 0, 0, 3000, 4000, 11, 9613, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Shadowforge Citizen - In Combat - Cast Shadow Bolt (Phase 3)'),
(8902, 0, 5, 0, 0, 8, 100, 0, 3000, 6000, 6000, 12000, 11, 13339, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Shadowforge Citizen - In Combat - Cast Fire Blast (Phase 4)'),
(8903, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 11972, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Anvilrage Captain - In Combat - Cast ''Shield Bash'''),
(8903, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 12169, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Anvilrage Captain - In Combat - Cast ''Shield Block'''),
(8903, 0, 2, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 13730, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Anvilrage Captain - Within 0-8 Range - Cast ''Demoralizing Shout'''),
(8903, 0, 3, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 13737, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Anvilrage Captain - In Combat - Cast ''Mortal Strike'''),
(8906, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 15088, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ragereaver Golem - On Reset - Cast Flurry'),
(8906, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 12795, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Ragereaver Golem - Between 0-30% Health - Cast Frenzy (No Repeat)'),
(8907, 0, 0, 0, 0, 0, 100, 0, 1000, 1000, 6000, 9000, 11, 15575, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Every 6 - 9 seconds (1 - 1s initially) - Self: Cast spell Flame Cannon (15575) on Victim'),
(8907, 0, 1, 0, 0, 0, 100, 0, 5000, 15000, 5000, 15000, 11, 10966, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Every 5 - 15 seconds - Self: Cast spell Uppercut (10966) on Victim'),
(8909, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3000, 5000, 11, 15242, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Fireguard - In Combat - Cast Fireball'),
(8911, 0, 0, 0, 9, 0, 100, 2, 0, 30, 9000, 11000, 11, 15243, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Fireguard Destroyer - Within 0-30 Range - Cast ''Fireball Volley'' (Normal Dungeon)'),
(8911, 0, 1, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 3417, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Fireguard Destroyer - On Reset - Cast Thrash'),
(8912, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 13616, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Twilights Hammer Torturer - On Reset - Cast Wracking Pains Proc'),
(8912, 0, 1, 0, 0, 0, 100, 0, 3000, 6000, 14000, 17000, 11, 14032, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Twilights Hammer Torturer - In Combat - Cast Shadow Word: Pain'),
(8920, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 31, 1, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Weapon Technician - On Aggro - Set Phase Random Between 1-3'),
(8920, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2000, 4000, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Weapon Technician - In Combat - Cast Shoot'),
(8920, 0, 2, 0, 0, 1, 100, 0, 3000, 6000, 10000, 13000, 11, 7896, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Weapon Technician - In Combat - Cast Exploding Shot (Phase 1)'),
(8920, 0, 3, 0, 0, 2, 100, 0, 3000, 6000, 10000, 13000, 11, 12551, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Weapon Technician - In Combat - Cast Frost Shot (Phase 2)'),
(8920, 0, 4, 0, 0, 4, 100, 0, 5000, 7000, 13000, 15000, 11, 14443, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Weapon Technician - In Combat - Cast Multi-Shot (Phase 3)'),
(8922, 0, 0, 0, 0, 0, 100, 0, 5000, 7000, 11000, 13000, 11, 7140, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bloodhound Mastiff - In Combat - Cast Expose Weakness'),
(8922, 0, 1, 0, 0, 0, 100, 0, 9000, 11000, 20000, 22000, 11, 15608, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bloodhound Mastiff - In Combat - Cast Ravenous Claw'),
(8922, 0, 2, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 8279, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Bloodhound Mastiff - On Reset - Cast Stealth Detection'),
(8923, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 25710, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Panzor the Invincible - In Combat - Cast ''Heroic Strike'''),
(8923, 0, 1, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 22886, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Panzor the Invincible - Within 8-25 Range - Cast ''Berserker Charge'''),
(8923, 0, 2, 0, 0, 0, 100, 0, 6000, 8000, 12000, 16000, 11, 26211, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Panzor the Invincible - In Combat - Cast ''Hamstring'''),
(8923, 0, 3, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 25190, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Panzor the Invincible - Within 8-25 Range - Cast ''Stinger Charge'''),
(9499, 0, 0, 0, 0, 0, 100, 0, 3000, 5000, 15000, 18000, 11, 13338, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Plugger Spazzring - In Combat - Cast ''Curse of Tongues'''),
(9499, 0, 1, 0, 0, 0, 100, 0, 5000, 7000, 12000, 15000, 11, 12742, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Plugger Spazzring - In Combat - Cast ''Immolate'''),
(9499, 0, 2, 0, 0, 0, 100, 0, 1000, 2000, 3000, 4000, 11, 12739, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Plugger Spazzring - In Combat - Cast ''Shadow Bolt'''),
(9499, 0, 3, 0, 23, 0, 100, 0, 13787, 0, 10000, 10000, 11, 13787, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Plugger Spazzring - On aura missing - Cast ''Demon Armor'''),
(9545, 0, 0, 0, 13, 0, 25, 0, 5000, 8000, 0, 0, 11, 15610, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Grim Patron - In Combat - Cast ''Kick'''),
(9545, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 14822, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Grim Patron - Between 0-30% Health - Cast ''Drunken Rage'' (No Repeat)'),
(9547, 0, 0, 0, 0, 0, 100, 0, 1000, 2000, 15000, 15000, 11, 14868, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Guzzling Patron - In Combat - Cast ''Curse of Agony'''),
(9547, 0, 1, 0, 0, 0, 100, 0, 3000, 4000, 3000, 4000, 11, 20825, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Guzzling Patron - In Combat - Cast ''Shadow Bolt'''),
(9547, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 14822, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Guzzling Patron - Between 0-30% Health - Cast ''Drunken Rage'' (No Repeat)'),
(9554, 0, 0, 0, 0, 0, 100, 0, 3000, 5000, 5000, 8000, 11, 6253, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hammered Patron - In Combat - Cast ''Backhand'''),
(9554, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 14822, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Hammered Patron - Between 0-30% Health - Cast ''Drunken Rage'' (No Repeat)');
