-- #987 wave 1 (pure additions): Terokkar Forest creature abilities, imported from DeltaTho PR 15 file 2026_10_24_73_world.sql.
-- 27 creatures, 45 smart_scripts rows. Only creatures that have NO source_type 0 rows today are written here;
-- scripts that would replace existing rows are wave 2. Filters/fixes: see (not shipped)
-- Idempotent: DELETE of the listed entries then INSERT. AIName is set to SmartAI only where AIName and ScriptName are empty.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND FIND_IN_SET(entryorguid, <entries below>); and
--   UPDATE creature_template SET AIName='' WHERE FIND_IN_SET(entry, <entries below>); (entries that already had AIName SmartAI: see backup in (not shipped).

SET @ENTRIES := '15241,18449,18475,18483,18689,18973,19000,19002,19501,21244,21636,21638,21652,21724,21849,21912,21985,22038,22045,22407,22452,23161,23165,23205,23207,24920,51908';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `AIName` = '' AND `ScriptName` = '' AND FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(15241, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 38182, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gryphon Rider Guard - In Combat - Cast ''Stormhammer'''),
(18449, 0, 0, 0, 11, 0, 100, 1, 0, 0, 0, 0, 11, 29651, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Shienor Talonite - On Respawn - Cast ''Dual Wield'' (No Repeat)'),
(18475, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 31994, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Mug''gok - Within 8-25 Range - Cast ''Shoulder Charge'''),
(18483, 0, 0, 0, 0, 0, 100, 0, 3000, 5000, 10000, 14000, 11, 11977, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Empoor''s Bodyguard - In Combat - Cast ''Rend'''),
(18483, 0, 1, 0, 0, 0, 100, 0, 5000, 7000, 15000, 20000, 11, 13730, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Empoor''s Bodyguard - In Combat - Cast ''Demoralizing Shout'''),
(18689, 0, 0, 0, 0, 0, 100, 0, 3000, 5000, 11000, 15000, 11, 39214, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Crippler - In Combat - Cast ''Pierce Armor'''),
(18689, 0, 1, 0, 0, 0, 100, 0, 8000, 12000, 13000, 18000, 11, 38621, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Crippler - In Combat - Cast ''Debilitating Strike'''),
(18689, 0, 2, 0, 2, 0, 100, 1, 0, 50, 0, 0, 11, 38882, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Crippler - Between 0-50% Health - Cast ''Bone Armor'' (No Repeat)'),
(18973, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Stonebreaker Grunt - In Combat - Cast ''Cleave'''),
(18973, 0, 1, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 8599, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stonebreaker Grunt - Between 0-30% Health - Cast ''Enrage'' (No Repeat)'),
(19000, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 15547, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'High Elf Ranger - In Combat - Cast ''Shoot'''),
(19002, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 40505, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Gardok Ripjaw - In Combat - Cast ''Cleave'''),
(19501, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 15284, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Lower City Operative - In Combat - Cast ''Cleave'''),
(21244, 0, 0, 0, 9, 0, 100, 0, 8, 25, 15000, 20000, 11, 35570, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bonechewer Messenger - Within 8-25 Range - Cast ''Charge'''),
(21636, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 3391, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Vengeful Draenei - In Combat - Cast ''Thrash'''),
(21638, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 36553, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Vengeful Harbinger - In Combat - Cast ''PetWait'''),
(21652, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 11, 37589, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skettis Time-Shifter - On Aggro - Cast ''Shriveling Gaze'' (No Repeat)'),
(21652, 0, 1, 0, 0, 0, 100, 0, 2500, 5000, 8500, 10000, 11, 16006, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skettis Time-Shifter - In Combat - Cast ''Chain Lightning'''),
(21724, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 36590, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hawkbane - In Combat - Cast ''Rip'''),
(21849, 0, 0, 0, 0, 0, 100, 0, 1000, 6000, 8000, 11000, 11, 32738, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bone Crawler - In Combat - Cast Bore'),
(21849, 0, 1, 0, 9, 0, 100, 0, 4, 50, 2000, 3500, 11, 31747, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Bone Crawler - Within Range 5-50yd - Cast Poison'),
(21912, 0, 0, 0, 0, 0, 100, 0, 2500, 5000, 10000, 15000, 11, 37577, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skettis Sentinel - In Combat - Cast ''Debilitating Strike'''),
(21912, 0, 1, 0, 2, 0, 100, 1, 20, 80, 0, 0, 11, 30619, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skettis Sentinel - Between 20-80% Health - Cast ''Cleave'' (No Repeat)'),
(21912, 0, 2, 0, 2, 0, 100, 1, 5, 30, 0, 0, 11, 23537, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Skettis Sentinel - Between 5-30% Health - Cast ''Frenzy'' (No Repeat)'),
(21985, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 3000, 4000, 11, 10277, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skettis Eviscerator - In Combat - Cast ''Throw'''),
(22038, 0, 0, 0, 0, 0, 100, 0, 1000, 6000, 8000, 11000, 11, 32738, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hai''shulud - In Combat - Cast Bore'),
(22038, 0, 1, 0, 9, 0, 100, 0, 4, 50, 2000, 3500, 11, 31747, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Hai''shulud - Within Range 5-50yd - Cast Poison'),
(22045, 0, 0, 0, 0, 0, 100, 0, 5000, 10000, 15000, 20000, 11, 38621, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Vengeful Husk - In Combat - Cast ''Debilitating Strike'''),
(22407, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 32009, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Caravan Defender - In Combat - Cast ''Cutdown'''),
(22452, 0, 0, 0, 0, 0, 100, 1, 2000, 4000, 0, 0, 11, 8258, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Reanimated Exarch - In Combat - Cast ''Devotion Aura'' (No Repeat)'),
(23161, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Darkscreecher Akkarai - On Reset - Set Event Phase 1'),
(23161, 0, 1, 0, 0, 1, 100, 0, 8000, 12000, 8000, 12000, 11, 40429, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Darkscreecher Akkarai - In Combat - Cast ''Frostbolt'' (Phase 1)'),
(23161, 0, 2, 0, 0, 2, 100, 0, 2000, 6000, 2000, 6000, 11, 40430, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Darkscreecher Akkarai - In Combat - Cast ''Frostbolt'' (Phase 2)'),
(23161, 0, 3, 0, 0, 0, 100, 0, 10000, 10000, 15000, 20000, 11, 40428, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Darkscreecher Akkarai - In Combat - Cast ''Shadow Bolt Volley'''),
(23161, 0, 4, 0, 2, 0, 100, 0, 0, 30, 5000, 10000, 11, 15730, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Darkscreecher Akkarai - Between 0-30% Health - Cast ''Curse of Mending'''),
(23161, 0, 5, 0, 0, 0, 100, 0, 15000, 20000, 15000, 20000, 11, 13341, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Darkscreecher Akkarai - In Combat - Cast ''Fire Blast'''),
(23165, 0, 0, 0, 21, 0, 100, 0, 0, 0, 0, 0, 22, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Karrog - On Reached Home - Set Event Phase 1'),
(23165, 0, 1, 0, 0, 1, 100, 0, 8000, 16000, 8000, 40000, 11, 40488, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Karrog - In Combat - Cast ''Trample'' (Phase 1)'),
(23205, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 40488, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Karrog Shardling - Within 0-8 Range - Cast ''Trample'''),
(23207, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 10277, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Skettis Assassin - In Combat - Cast ''Throw'''),
(24920, 0, 0, 0, 0, 0, 100, 0, 2500, 5000, 15000, 20000, 11, 46202, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Razorthorn Flayer - In Combat - Cast ''Pierce Armor'''),
(24920, 0, 1, 0, 0, 0, 100, 0, 10000, 12500, 25000, 30000, 11, 13443, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Razorthorn Flayer - In Combat - Cast ''Rend'''),
(24920, 0, 2, 0, 2, 0, 100, 1, 20, 80, 0, 0, 11, 33896, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Razorthorn Flayer - Between 20-80% Health - Cast ''Desperate Defense'' (No Repeat)'),
(24920, 0, 3, 0, 2, 0, 100, 1, 10, 30, 0, 0, 11, 33898, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Razorthorn Flayer - Between 10-30% Health - Cast ''Desperate Rage'' (No Repeat)'),
(51908, 0, 0, 0, 0, 0, 100, 0, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Allerian Defender - In Combat - Cast ''Shoot''');
