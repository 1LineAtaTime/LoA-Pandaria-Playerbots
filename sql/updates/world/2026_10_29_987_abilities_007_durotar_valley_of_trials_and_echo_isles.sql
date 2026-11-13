-- #987 wave 1 (pure additions): Durotar, Valley of Trials and Echo Isles creature abilities, imported from DeltaTho PR 15 file 2026_10_24_11_world.sql.
-- 29 creatures, 53 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '3098,3102,3126,3195,5952,37956,37961,38046,38217,39004,39215,39245,39249,39260,39261,39267,39268,39269,39272,39337,39351,39352,39353,39385,39452,39595,39596,39603,42946';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3098, 0, 0, 0, 0, 0, 100, 0, 2000, 4500, 12000, 15000, 11, 79581, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mottled Boar - In Combat - Cast ''Angry Snort'''),
(3102, 0, 0, 0, 2, 0, 100, 0, 0, 40, 18000, 24000, 11, 79616, 0, 0, 0, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 'Felstalker - Between 0-40% Health - Cast ''Drain Life'''),
(3126, 0, 0, 0, 2, 0, 100, 0, 0, 50, 24000, 26000, 11, 79840, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Armored Scorpid - Between 0-50% Health - Cast ''Harden'''),
(3195, 0, 0, 0, 0, 0, 100, 0, 5000, 5000, 15000, 18000, 11, 80146, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Thug - In Combat - Cast ''Bonk'''),
(3195, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 60000, 65000, 11, 80174, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Thug - In Combat - Cast ''Felblood'''),
(3195, 0, 2, 0, 0, 0, 100, 0, 2000, 4000, 25000, 26000, 11, 80182, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Burning Blade Thug - In Combat - Cast ''Uppercut'''),
(5952, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Den Grunt - In Combat - Cast ''Shoot'''),
(5952, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Den Grunt - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(5952, 0, 2, 0, 2, 0, 100, 1, 0, 30, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Den Grunt - Between 0-30% Health - Say Line 0 (No Repeat)'),
(37956, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bloodtalon Raptor - On Aggro - Cast ''Rushing Charge'' (No Repeat)'),
(37961, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Bloodtalon - On Aggro - Cast ''Rushing Charge'' (No Repeat)'),
(37961, 0, 1, 0, 0, 0, 100, 0, 2000, 4500, 25000, 26000, 11, 79774, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Bloodtalon - In Combat - Cast ''Corrupted'''),
(38046, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 75002, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Wildmane Cat - On Aggro - Cast ''Leaping Rush'' (No Repeat)'),
(38046, 0, 1, 0, 0, 0, 100, 0, 4000, 4500, 18000, 22000, 11, 31279, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Wildmane Cat - In Combat - Cast ''Swipe'''),
(38046, 0, 2, 0, 1, 0, 30, 0, 45000, 55000, 120000, 300000, 11, 71232, 0, 0, 0, 0, 0, 9, 38141, 8, 25, 0, 0, 0, 0, 'Wildmane Cat - Out of Combat - Cast ''Wild Pounce'''),
(38217, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Darkspear Watcher - In Combat - Cast ''Shoot'''),
(39004, 0, 0, 0, 0, 0, 100, 0, 3000, 5000, 15000, 19000, 11, 79765, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Pygmy Surf Crawler - In Combat - Cast ''Bubble Blast'''),
(39215, 0, 0, 0, 0, 0, 100, 0, 2000, 4500, 12000, 20000, 11, 75356, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Captured Northwatch Scout - In Combat - Cast ''Fisticuffs'''),
(39245, 0, 0, 0, 0, 0, 100, 0, 5000, 5000, 14000, 17000, 11, 75356, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Lug - In Combat - Cast ''Fisticuffs'''),
(39249, 0, 0, 0, 0, 0, 100, 0, 5000, 5000, 14000, 17000, 11, 75356, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Lug - In Combat - Cast ''Fisticuffs'''),
(39260, 0, 0, 0, 2, 0, 100, 0, 0, 40, 14000, 15000, 11, 78826, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Infantryman - Between 0-40% Health - Cast ''Evasion'''),
(39260, 0, 1, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 78894, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Infantryman - On Aggro - Cast ''Leaping Cleave'' (No Repeat)'),
(39261, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Ranger - In Combat - Cast ''Shoot'''),
(39261, 0, 1, 0, 9, 0, 100, 0, 5, 30, 15000, 18000, 11, 78754, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Ranger - Within 5-30 Range - Cast ''Arcane Shot'''),
(39261, 0, 2, 0, 9, 0, 100, 0, 0, 10, 60000, 65000, 11, 78578, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Ranger - Within 0-10 Range - Cast ''Immolation Trap'''),
(39267, 0, 0, 0, 0, 0, 100, 0, 2000, 3000, 18000, 19000, 11, 9080, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Marine - In Combat - Cast ''Hamstring'''),
(39267, 0, 1, 0, 0, 0, 100, 0, 7000, 9000, 22000, 27000, 11, 79726, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Marine - In Combat - Cast ''Ticking Time Bomb'''),
(39268, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Sharpshooter - In Combat - Cast ''Shoot'''),
(39268, 0, 1, 0, 9, 0, 100, 0, 5, 30, 18000, 21000, 11, 79721, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Sharpshooter - Within 5-30 Range - Cast ''Explosive Shot'''),
(39268, 0, 2, 0, 9, 0, 100, 0, 5, 30, 35000, 45000, 11, 79719, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Northwatch Sharpshooter - Within 5-30 Range - Cast ''Pinpoint'''),
(39269, 0, 0, 0, 1, 0, 100, 0, 2000, 2000, 16000, 18000, 5, 23, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Palliter - Out of Combat - Play Emote 23'),
(39269, 0, 1, 2, 4, 0, 100, 0, 0, 0, 0, 0, 11, 79733, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Palliter - On Aggro - Cast ''Commanding Shout'''),
(39269, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Palliter - On Aggro - Say Line 0'),
(39269, 0, 3, 0, 0, 0, 100, 0, 100, 100, 10000, 10000, 11, 78894, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Palliter - In Combat - Cast ''Leaping Cleave'''),
(39269, 0, 4, 0, 0, 0, 100, 0, 4000, 5000, 12000, 14000, 11, 79732, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Lieutenant Palliter - In Combat - Cast ''Shield Bash'''),
(39272, 0, 0, 0, 0, 0, 100, 0, 2000, 4500, 12000, 15000, 11, 32011, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Foaming Sea Elemental - In Combat - Cast ''Water Bolt'''),
(39272, 0, 1, 0, 0, 0, 100, 0, 7000, 9000, 24000, 27000, 11, 79831, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Foaming Sea Elemental - In Combat - Cast ''Wave Crash'''),
(39337, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Wayward Plainstrider - On Aggro - Cast ''Rushing Charge'' (No Repeat)'),
(39337, 0, 1, 0, 1, 0, 100, 1, 2000, 3000, 0, 0, 11, 73757, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Wayward Plainstrider - Out of Combat - Cast ''Water Walk'' (No Repeat)'),
(39351, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 76429, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ghislania - In Combat - Cast ''Static Burst'''),
(39351, 0, 1, 0, 0, 0, 100, 0, 5000, 5000, 19000, 21000, 11, 79913, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ghislania - In Combat - Cast ''Chain Lightning'''),
(39352, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 76423, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gaur Icehorn - Between 0-30% Health - Cast ''Taunka Rage'' (No Repeat)'),
(39352, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Gaur Icehorn - Between 0-30% Health - Say Line 0 (No Repeat)'),
(39353, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Griswold Hanniston - On Aggro - Say Line 0 (No Repeat)'),
(39353, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 22000, 26000, 11, 76490, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Griswold Hanniston - In Combat - Cast ''Flurry of Claws'''),
(39385, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Screamslash - On Aggro - Cast ''Rushing Charge'' (No Repeat)'),
(39385, 0, 1, 0, 0, 0, 100, 0, 3000, 5000, 24000, 26000, 11, 13443, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Screamslash - In Combat - Cast ''Rend'''),
(39452, 0, 0, 0, 9, 0, 100, 0, 0, 5, 12000, 16000, 11, 12166, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Dreadmaw Toothgnasher - Within 0-5 Range - Cast ''Muscle Tear'''),
(39595, 0, 0, 0, 0, 0, 100, 0, 5000, 6000, 16000, 18000, 11, 80117, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Furious Earthguard - In Combat - Cast ''Earth Spike'''),
(39596, 0, 0, 0, 0, 0, 100, 0, 4000, 6000, 24000, 26000, 11, 78542, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Teeming Waterguard - In Combat - Cast ''Splash'''),
(39603, 0, 0, 0, 0, 0, 100, 0, 4000, 8000, 19000, 25000, 11, 80132, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Swollen Voidwalker - In Combat - Cast ''Unbound Darkness'''),
(42946, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 77808, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Kor''kron Deathbringer - On Aggro - Cast ''Battle Shout'' (No Repeat)'),
(42946, 0, 1, 0, 0, 0, 100, 0, 4500, 6500, 15400, 19200, 11, 15496, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Kor''kron Deathbringer - In Combat - Cast ''Cleave''');
