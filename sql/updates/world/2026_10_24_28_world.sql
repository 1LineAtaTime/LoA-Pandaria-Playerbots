-- Blackfathom Deeps: creature abilities
-- The Wrathtail naga at the entrance, the Fallenroot Shadowstalkers, Blackfathom Sea Witches, Blindlight Murlocs, Aqua Guardians and
-- Lady Sarevess ran classic rows or lacked some of their abilities. Port them from the Cataclysm data, keeping its normal-dungeon
-- flags, and keep the scripts already in place.

SET @ENTRIES := '3713,3717,4798,4805,4818,4831,6047';

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE FIND_IN_SET(`entry`, @ENTRIES);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND FIND_IN_SET(`entryorguid`, @ENTRIES);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3713, 0, 0, 0, 0, 0, 100, 0, 4000, 5000, 18000, 24500, 11, 78732, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Wrathtail Wave Rider - In Combat - Cast ''Whirlpool'''),
(3713, 0, 1, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Wrathtail Wave Rider - Between 0-15% Health - Flee For Assist (No Repeat)'),
(3717, 0, 0, 0, 0, 0, 100, 0, 0, 0, 3400, 4700, 11, 9672, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Wrathtail Sorceress - In Combat - Cast ''Frostbolt'''),
(3717, 0, 1, 0, 2, 0, 100, 1, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Wrathtail Sorceress - Between 0-15% Health - Flee For Assist (No Repeat)'),
(3717, 0, 2, 0, 9, 0, 100, 0, 0, 8, 12000, 13000, 11, 11831, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Wrathtail Sorceress - Within 0-8 Range - Cast ''Frost Nova'''),
(3717, 0, 3, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 11, 34827, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Wrathtail Sorceress - Out of Combat - Cast ''Water Shield'''),
(4798, 0, 0, 0, 0, 0, 100, 2, 5000, 7000, 20000, 35000, 11, 11980, 32, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Fallenroot Shadowstalker - In Combat - Cast ''Curse of Weakness'' (Phase 1) (No Repeat)'),
(4805, 0, 0, 0, 1, 0, 100, 2, 1000, 1000, 1800000, 1800000, 11, 12544, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blackfathom Sea Witch - Out of Combat - Cast ''Frost Armor'' (No Repeat)'),
(4805, 0, 1, 0, 0, 0, 100, 2, 7000, 10000, 16000, 20000, 11, 122, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blackfathom Sea Witch - In Combat - Cast ''Frost Nova'' (No Repeat)'),
(4805, 0, 2, 0, 0, 0, 100, 3, 3000, 6000, 0, 0, 11, 15044, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blackfathom Sea Witch - In Combat - Cast ''Frost Ward'' (No Repeat)'),
(4805, 0, 3, 0, 2, 0, 100, 3, 0, 15, 0, 0, 25, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Blackfathom Sea Witch - Between 0-15% Health - Flee For Assist (No Repeat)'),
(4818, 0, 0, 0, 4, 0, 100, 3, 0, 0, 0, 0, 11, 7164, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Blindlight Murloc - On Aggro - Cast ''Defensive Stance'' (No Repeat) (Normal Dungeon)'),
(4818, 0, 1, 0, 0, 0, 75, 2, 3000, 5000, 8000, 11000, 11, 22691, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Blindlight Murloc - In Combat - Cast ''Sunder Armor'' (No Repeat) (Normal Dungeon)'),
(4831, 0, 0, 0, 4, 0, 100, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lady Sarevess - On Aggro - Say Line 0 (No Repeat) (Normal Dungeon)'),
(4831, 0, 1, 0, 0, 0, 100, 2, 0, 0, 2300, 3900, 11, 6660, 64, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Lady Sarevess - In Combat CMC - Cast ''Shoot'' (Normal Dungeon)'),
(4831, 0, 2, 0, 0, 0, 100, 2, 9000, 12000, 12000, 15000, 11, 8435, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Lady Sarevess - In Combat - Cast ''Forked Lightning'' (Normal Dungeon)'),
(4831, 0, 3, 0, 0, 0, 100, 2, 6000, 8500, 9000, 13000, 11, 11831, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lady Sarevess - In Combat - Cast ''Frost Nova'' (Normal Dungeon)'),
(4831, 0, 4, 0, 0, 0, 100, 2, 7000, 9000, 9000, 13000, 11, 246, 33, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 'Lady Sarevess - In Combat - Cast ''Slow'' (Normal Dungeon)'),
(4831, 0, 5, 0, 6, 0, 100, 3, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Lady Sarevess - On Just Died - Say Line 1 (Normal Dungeon)'),
(6047, 0, 0, 0, 0, 0, 100, 2, 5000, 9000, 14000, 18000, 11, 11831, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Aqua Guardian - In Combat - Cast ''Frost Nova''');
