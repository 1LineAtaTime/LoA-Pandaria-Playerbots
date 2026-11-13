-- #987 wave 1 (pure additions): Duskwood creature abilities, imported from DeltaTho PR 15 file 2026_10_24_29_world.sql.
-- 18 creatures, 42 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '212,503,898,10038,37984,43731,44016,44020,44028,44029,44087,44089,45614,45619,45739,45771,45801,45811';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(212, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On reset (e.g. after reaching home) - Self: Set event phase to 1 // '),
(212, 0, 1, 0, 4, 0, 20, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro (20% chance) - Self: Talk 0 // '),
(212, 0, 2, 0, 4, 0, 100, 0, 0, 0, 0, 0, 22, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On aggro - Self: Set event phase to 2 // '),
(212, 0, 3, 0, 0, 0, 100, 0, 11000, 12000, 31000, 31000, 11, 78828, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 11000 min and 12000 max ms (repeat 31000 min 31000 max ms) - Self: Cast spell Bladestorm (78828) on Self // '),
(212, 0, 4, 0, 8, 0, 100, 0, 78828, 0, 0, 0, 67, 0, 6000, 6000, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On spell Bladestorm (78828) hit  - Trigger timed event #0 in 6000 - 6000 ms // '),
(212, 0, 5, 0, 59, 2, 100, 0, 0, 0, 0, 0, 11, 35856, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On timed event 0 triggered - Self: Cast spell Stun (35856) on Self // '),
(212, 0, 6, 0, 8, 0, 100, 0, 35856, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'On spell Stun (35856) hit  - Self: Talk 1 // '),
(503, 0, 0, 0, 0, 0, 100, 0, 5000, 11000, 22000, 33000, 11, 3537, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lord Malathrom - In Combat - Cast ''Minions of Malathrom'' (No Repeat)'),
(898, 0, 0, 0, 0, 0, 100, 0, 6000, 7000, 30000, 32000, 11, 84308, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 6000 min and 7000 max ms (repeat 30000 min 32000 max ms) - Self: Cast spell Furious Howl (84308) on Self // '),
(10038, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 95826, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Night Watch Guard - In Combat - Cast ''Shoot'''),
(37984, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 70074, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Crown Duster - In Combat - Cast ''Spray Chemical'''),
(43731, 0, 0, 0, 1, 0, 100, 0, 500, 1000, 1800000, 1800000, 11, 48168, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sister Elsington - Out of Combat - Cast ''Inner Fire'''),
(44016, 0, 0, 0, 2, 0, 100, 0, 0, 45, 31000, 31000, 11, 84868, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'When health between 0% and 45% (check every 31000 - 31000 ms) - Self: Cast spell Hibernate (84868) on Self // '),
(44016, 0, 1, 0, 0, 0, 100, 0, 4000, 5000, 6000, 6000, 11, 31279, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 4000 min and 5000 max ms (repeat 6000 min 6000 max ms) - Self: Cast spell Swipe (31279) on Victim // '),
(44020, 0, 0, 0, 0, 0, 100, 0, 18000, 19000, 20000, 21000, 11, 49865, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 18000 min and 19000 max ms (repeat 20000 min 21000 max ms) - Self: Cast spell Eye Peck (49865) on Victim // '),
(44020, 0, 1, 0, 0, 0, 100, 0, 25000, 26000, 20000, 21000, 11, 81680, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 25000 min and 26000 max ms (repeat 20000 min 21000 max ms) - Self: Cast spell Howling Screech (81680) on Self // '),
(44028, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 14000, 11, 82717, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Silent Shade - In Combat - Cast ''Vanish'''),
(44028, 0, 1, 0, 2, 0, 100, 0, 0, 40, 17000, 21000, 11, 78826, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Silent Shade - Between 0-40% Health - Cast ''Evasion'''),
(44029, 0, 0, 0, 1, 0, 100, 0, 500, 1000, 1800000, 1800000, 11, 79865, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Clattering Coldwraith - Out of Combat - Cast ''Frost Armor'''),
(44029, 0, 1, 0, 0, 0, 100, 0, 6000, 8000, 24000, 26000, 11, 79860, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Clattering Coldwraith - In Combat - Cast ''Blizzard'''),
(44029, 0, 2, 0, 0, 0, 100, 0, 3000, 5000, 12000, 15000, 11, 79859, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Clattering Coldwraith - In Combat - Cast ''Ice Lance'''),
(44087, 0, 0, 0, 0, 0, 100, 0, 15000, 16000, 11000, 12000, 11, 80576, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, ''),
(44089, 0, 0, 0, 0, 0, 100, 1, 100, 100, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 100 min and 100 max ms (repeat 0 min 0 max ms) - Self: Cast spell Rushing Charge (6268) on Victim // '),
(44089, 0, 1, 0, 0, 0, 100, 0, 12000, 12000, 34000, 34000, 11, 84873, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 12000 min and 12000 max ms (repeat 34000 min 34000 max ms) - Self: Cast spell Wallow (84873) on Self // '),
(45614, 0, 0, 0, 0, 0, 100, 0, 4000, 5000, 6000, 10000, 11, 80132, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Update (In Combat) 4000 min and 5000 max ms (repeat 6000 min 10000 max ms) - Self: Cast spell Unbound Darkness (80132) on Victim // '),
(45619, 0, 0, 0, 9, 0, 100, 0, 0, 5, 12000, 14500, 11, 16856, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Morbidius - Within 0-5 Range - Cast ''Mortal Strike'''),
(45739, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'The Unknown Soldier - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(45739, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'The Unknown Soldier - Between 0-30% Health - Say Line 0 (No Repeat)'),
(45739, 0, 2, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 80983, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'The Unknown Soldier - On Aggro - Cast ''Commanding Shout'' (No Repeat)'),
(45739, 0, 3, 0, 9, 0, 100, 0, 0, 5, 14000, 15000, 11, 85240, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'The Unknown Soldier - Within 0-5 Range - Cast ''Sundering Cleave'''),
(45771, 0, 0, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 81173, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Marus - Between 0-30% Health - Cast ''Frenzy'' (No Repeat)'),
(45771, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Marus - Between 0-30% Health - Say Line 0 (No Repeat)'),
(45771, 0, 2, 0, 2, 0, 100, 0, 0, 40, 16000, 21000, 11, 79853, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Marus - Between 0-40% Health - Cast ''Evasion'''),
(45771, 0, 3, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 80576, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marus - Between 0-30% Health - Cast ''Shadowstep'' (No Repeat)'),
(45801, 0, 0, 1, 4, 0, 100, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Eliza - On Aggro - Say Line 0 (No Repeat)'),
(45801, 0, 1, 0, 61, 0, 100, 1, 0, 0, 0, 0, 11, 3107, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Eliza - On Aggro - Cast ''Summon Eliza''s Guard'' (No Repeat)'),
(45801, 0, 2, 0, 2, 0, 100, 1, 0, 50, 0, 0, 11, 3107, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Eliza - Between 0-50% Health - Cast ''Summon Eliza''s Guard'' (No Repeat)'),
(45801, 0, 3, 0, 0, 0, 100, 0, 2500, 4500, 20500, 22000, 11, 81767, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Eliza - In Combat - Cast ''Lantern Blaze'''),
(45801, 0, 4, 0, 0, 0, 100, 0, 8000, 12000, 15000, 18000, 11, 81764, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Eliza - In Combat - Cast ''Throw Lantern'''),
(45811, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Marina DeSirrus - On Aggro - Say Line 0 (No Repeat)'),
(45811, 0, 1, 0, 0, 0, 100, 0, 2000, 3000, 9000, 12000, 11, 6595, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marina DeSirrus - In Combat - Cast ''Exploit Weakness'''),
(45811, 0, 2, 0, 0, 0, 100, 0, 5000, 7000, 16000, 22000, 11, 78509, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Marina DeSirrus - In Combat - Cast ''Torch Toss''');
