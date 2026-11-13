-- #987 wave 1 (pure additions): Dragonblight creature abilities, imported from DeltaTho PR 15 file 2026_10_24_97_world.sql.
-- 12 creatures, 23 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '26162,26772,26943,27163,27330,27665,27695,28006,28018,32352,51797,51798';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(26162, 0, 0, 0, 1, 0, 100, 0, 0, 0, 1000, 1000, 11, 46677, 0, 0, 0, 0, 0, 19, 26167, 0, 0, 0, 0, 0, 0, 'Transborea Generator 001 - OOC - Cast Evacuee Reaches Dragonblight'),
(26772, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 11, 29266, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Icemist Warrior - On Respawn - Cast ''Permanent Feign Death'''),
(26943, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 50420, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Battered Drakkari Berserker - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(26943, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 37592, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Battered Drakkari Berserker - In Combat - Cast ''Knockdown'''),
(27163, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 18000, 24000, 11, 49215, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, '7th Legion Siege Engineer - In Combat - Cast ''Self-Destruct'''),
(27330, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 2000, 4000, 11, 50713, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Onslaught Infantry - In Combat - Cast ''Unrelenting Onslaught'''),
(27665, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 20000, 25000, 11, 20572, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Kor''kron Pillager - In Combat - Cast ''Blood Fury'''),
(27665, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Kor''kron Pillager - In Combat - Cast ''Cleave'''),
(27695, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 49365, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Fordragon Battle Mage - In Combat - Cast ''Arcane Tempest'''),
(27695, 0, 1, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 38064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Fordragon Battle Mage - Within 0-8 Range - Cast ''Blast Wave'''),
(27695, 0, 2, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 17274, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Fordragon Battle Mage - In Combat - Cast ''Pyroblast'''),
(28006, 0, 0, 0, 0, 0, 100, 0, 7000, 7000, 18000, 18000, 11, 32863, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Grand Necrolord Antiok - In Combat - Cast Seed of Corruption'),
(28006, 0, 1, 0, 0, 0, 100, 0, 1100, 1100, 2000, 3000, 11, 50455, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Grand Necrolord Antiok - In Combat - Cast Shadow Bolt'),
(28006, 0, 2, 0, 2, 0, 100, 1, 0, 25, 0, 0, 11, 50497, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Grand Necrolord Antiok - Between 0-25% Health - Cast Scream of Chaos'),
(28006, 0, 3, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 50472, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Grand Necrolord Antiok - On Just Died - Cast Drop Scythe of Antiok'),
(28006, 0, 4, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 50494, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Grand Necrolord Antiok - On Reset - Cast Shroud of Lightning'),
(28018, 0, 0, 0, 0, 0, 100, 0, 5000, 5000, 12000, 14000, 11, 50456, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Thiassi the Lightning Bringer - In Combat - Cast Thiassi''s Stormbolt'),
(28018, 0, 1, 0, 0, 0, 100, 0, 9000, 9000, 15000, 19000, 11, 15593, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Thiassi the Lightning Bringer - In Combat - Cast War Stomp'),
(32352, 0, 0, 0, 0, 0, 100, 0, 5000, 6000, 12000, 17000, 11, 51020, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Infinite Timerender - IC - Cast Time Lapse'),
(32352, 0, 1, 0, 6, 0, 100, 0, 0, 0, 0, 0, 45, 2, 1, 0, 0, 0, 0, 19, 32327, 20, 0, 0, 0, 0, 0, 'Infinite Timerender - On Death - Set Data on HoE'),
(51797, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 47059, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stars'' Rest Sentinel - In Combat - Cast ''Frost Arrow'''),
(51798, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 16244, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, '7th Legion Infantryman - Within 0-8 Range - Cast ''Demoralizing Shout'''),
(51798, 0, 1, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, '7th Legion Infantryman - In Combat - Cast ''Shoot''');
