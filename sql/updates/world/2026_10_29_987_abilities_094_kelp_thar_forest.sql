-- #987 wave 1 (pure additions): Kelp'thar Forest creature abilities, imported from DeltaTho PR 15 file 2026_10_25_16_world.sql.
-- 36 creatures, 44 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '39313,39918,39948,39964,40219,40276,40282,40685,40753,40759,40770,40782,40811,40855,40987,41002,41016,41017,41018,41037,41042,41183,41477,41481,41530,41549,41746,41764,41781,41805,41808,41996,41997,41998,46474,49913';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(39313, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 76892, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpentine Lunge on Aggro'),
(39918, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 19000, 21000, 11, 33661, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Crush Armor'),
(39948, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 14000, 15000, 11, 79400, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Brinescale Venom'),
(39964, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 14000, 15000, 11, 79400, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Brinescale Venom'),
(40219, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 69203, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Vicious Bite'),
(40276, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 49978, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Claw Grasp'),
(40282, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Rushing Charge on Aggro'),
(40282, 0, 1, 0, 0, 0, 100, 0, 7500, 9500, 13500, 15500, 11, 33661, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Crush Armor'),
(40685, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 49978, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Claw Grasp'),
(40753, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 76892, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Zin''jatar Raider - Within 8-25 Range - Cast ''Serpentine Lunge'''),
(40759, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 76892, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpentine Lunge on Aggro'),
(40770, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 76892, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpentine Lunge on Aggro'),
(40782, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 76892, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpentine Lunge on Aggro'),
(40811, 0, 0, 0, 0, 0, 100, 0, 7000, 8000, 17000, 19000, 11, 79379, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Cannonball Bash'),
(40855, 0, 0, 0, 0, 0, 100, 0, 5000, 6000, 21000, 22000, 11, 79356, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Slithering Slime'),
(40987, 0, 0, 0, 0, 0, 100, 0, 8000, 8000, 17000, 25000, 11, 81026, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Sea Splash'),
(41002, 0, 0, 0, 0, 0, 100, 0, 5000, 6000, 21000, 22000, 11, 79356, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Slithering Slime'),
(41016, 0, 0, 0, 0, 0, 100, 0, 7000, 8000, 17000, 19000, 11, 79379, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Cannonball Bash'),
(41017, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3500, 4100, 11, 79413, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Secret Shiny'),
(41018, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 32064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Battle Shout on Aggro'),
(41037, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 18000, 21000, 11, 79414, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Brittle Touch'),
(41042, 0, 0, 0, 0, 0, 100, 0, 5000, 6000, 17000, 19000, 11, 35201, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Paralytic Poison'),
(41183, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3500, 4100, 11, 79413, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Secret Shiny'),
(41477, 0, 0, 0, 9, 0, 100, 0, 20, 30, 15000, 19000, 11, 79411, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Foul Waters on Close'),
(41477, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 79409, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Pearlbinding'),
(41481, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 76892, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpentine Lunge on Aggro'),
(41481, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 14000, 15000, 11, 13445, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Rend'),
(41530, 0, 0, 0, 9, 0, 100, 0, 0, 10, 15000, 16000, 11, 86405, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Shell Shock on Close'),
(41530, 0, 1, 0, 9, 0, 100, 0, 0, 15, 20000, 24000, 11, 86407, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Opalescent Prison on Close'),
(41549, 0, 0, 0, 9, 0, 100, 0, 10, 40, 8000, 12000, 11, 79583, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Impaling Pull on Close'),
(41746, 0, 0, 0, 0, 0, 100, 0, 7000, 8000, 17000, 19000, 11, 79379, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Cannonball Bash'),
(41764, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 76892, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Zin''jatar Raider - Within 8-25 Range - Cast ''Serpentine Lunge'''),
(41781, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 76892, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpentine Lunge on Aggro'),
(41805, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 76892, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpentine Lunge on Aggro'),
(41808, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 76892, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpentine Lunge on Aggro'),
(41996, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 76892, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Cast Serpentine Lunge on Aggro'),
(41996, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 14000, 15000, 11, 13445, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Rend'),
(41997, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 69203, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Vicious Bite'),
(41998, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 15000, 11, 69203, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Vicious Bite'),
(46474, 0, 0, 0, 2, 0, 100, 0, 0, 70, 15000, 17000, 11, 80522, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Demoralizing Mmmrrrggglll at 70% HP'),
(46474, 0, 1, 0, 9, 0, 100, 0, 0, 20, 15000, 25000, 11, 6533, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Net on Close'),
(49913, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 19000, 21000, 11, 92895, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Cast Briny Romance'),
(49913, 0, 1, 0, 2, 0, 100, 0, 0, 40, 22000, 25000, 11, 92888, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Siren''s Song at 40% HP'),
(49913, 0, 2, 0, 1, 0, 100, 0, 500, 1000, 60000, 60000, 11, 92888, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Cast Siren''s Song on Spawn');
