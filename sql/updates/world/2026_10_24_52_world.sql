-- Un'Goro Crater: creature abilities
-- Most of the zone's creatures (the devilsaurs and stegodons, the Gorishi silithids, the oozes and elementals) had no AI or ran classic rows. Port their abilities from the
-- Cataclysm data, and from the classic data where Cataclysm has none, keeping the quest scripts already in place; gossip and quest
-- reward scripts of the references are left out. The creatures no reference scripts cast their sniffed spells (creature_template
-- spell1-8).

SET @ENTRIES := '6506,6514,6521,6556,6557,6560,9163,38254';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(6506, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 6268, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Ravasaur Runner - On Aggro - Cast ''Rushing Charge'''),
(6514, 0, 0, 0, 2, 0, 100, 1, 30, 60, 0, 0, 11, 12555, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Un''Goro Gorilla - Between 30-60% Health - Cast ''Pummel'' (No Repeat)'),
(6521, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 11, 11350, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Living Blaze - On Aggro - Cast ''Fire Shield'''),
(6556, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 11, 14133, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Muculent Ooze - On Reset - Cast ''Muculent Fever Proc'''),
(6557, 0, 0, 0, 8, 0, 100, 0, 16031, 0, 0, 0, 22, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Primal Ooze - On Spellhit ''Releasing Corrupt Ooze'' - Set Event Phase 2'),
(6557, 0, 1, 2, 60, 2, 100, 1, 1500, 1500, 0, 0, 45, 0, 1, 0, 0, 0, 0, 9, 10290, 0, 35, 0, 0, 0, 0, 'Primal Ooze - On Update - Set Data 0 1 (Phase 2) (No Repeat)'),
(6557, 0, 2, 0, 61, 2, 100, 0, 0, 0, 0, 0, 29, 0, 0, 10290, 1, 1, 0, 9, 10290, 0, 35, 0, 0, 0, 0, 'Primal Ooze - On Update - Start Follow Closest Creature ''Captured Felwood Ooze'' (Phase 2) (No Repeat)'),
(6557, 0, 3, 4, 65, 2, 100, 0, 0, 0, 0, 0, 47, 0, 0, 0, 0, 0, 0, 7, 0, 1, 0, 0, 0, 0, 0, 'Primal Ooze - On Follow Complete - Set Visibility Off (Phase 2)'),
(6557, 0, 4, 5, 61, 2, 100, 0, 0, 0, 0, 0, 12, 9621, 6, 20000, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Primal Ooze - On Follow Complete - Summon Creature ''Gargantuan Ooze'' (Phase 2)'),
(6557, 0, 5, 6, 61, 2, 100, 0, 0, 0, 0, 0, 11, 16032, 0, 0, 0, 0, 0, 9, 9621, 0, 5, 0, 0, 0, 0, 'Primal Ooze - On Follow Complete - Cast ''Merging Oozes'' (Phase 2)'),
(6557, 0, 6, 0, 61, 2, 100, 0, 0, 0, 0, 0, 41, 50, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Primal Ooze - On Follow Complete - Despawn In 50 ms (Phase 2)'),
(6557, 0, 7, 0, 2, 0, 100, 1, 0, 30, 0, 0, 11, 14146, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Primal Ooze - Between 0-30% Health - Cast Clone (No Repeat)'),
(6560, 0, 0, 0, 2, 0, 100, 1, 30, 60, 0, 0, 11, 5810, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Stone Guardian - Between 30-60% Health - Cast ''Stone Skin'' (No Repeat)'),
(9163, 0, 1, 2, 8, 0, 100, 513, 71775, 0, 0, 0, 33, 38355, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Get Credit'),
(9163, 0, 2, 0, 61, 0, 100, 512, 0, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Disapear'),
(9163, 0, 3, 0, 0, 0, 100, 0, 8000, 8000, 15000, 15000, 11, 13692, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Diemetradon - In Combat - Cast ''Dire Growl'' (Normal Dungeon)'),
(38254, 0, 0, 0, 0, 0, 100, 0, 5000, 9000, 19000, 27000, 11, 83383, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 'Steaming Fury - In Combat - Cast ''<Spell not found!>''');

-- Creatures without reference data: their sniffed creature_template spell1-8
SET @ENTRIES := '38305,38307,38329,38708,51525';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `target_type`, `comment`) VALUES
(38305, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 71791, 0, 0, 1, 'Gorishi Fledgling Colossus - Within 0-8 Range - Cast ''Poison Explosion'''),
(38305, 0, 1, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 71792, 0, 0, 2, 'Gorishi Fledgling Colossus - In Combat - Cast ''Poison Spit'''),
(38307, 0, 0, 0, 9, 0, 100, 0, 0, 8, 12000, 18000, 11, 5568, 0, 0, 1, 'Gormashh - Within 0-8 Range - Cast ''Trample'''),
(38329, 0, 0, 0, 0, 0, 100, 0, 5000, 8000, 12000, 16000, 11, 83386, 0, 0, 2, 'Durrin Direshovel - In Combat - Cast ''Heave'''),
(38329, 0, 1, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 11971, 0, 0, 2, 'Durrin Direshovel - In Combat - Cast ''Sunder Armor'''),
(38708, 0, 0, 0, 0, 0, 100, 0, 3000, 6000, 15000, 20000, 11, 44531, 32, 0, 1, 'Devilsaur Queen - In Combat - Cast ''Dash'''),
(51525, 0, 0, 0, 0, 0, 100, 0, 2000, 4000, 8000, 12000, 11, 40505, 0, 0, 2, 'Marshal Expeditions Mercenary - In Combat - Cast ''Cleave'''),
(51525, 0, 1, 0, 0, 0, 100, 0, 4000, 7000, 12000, 16000, 11, 95826, 0, 0, 2, 'Marshal Expeditions Mercenary - In Combat - Cast ''Shoot''');
