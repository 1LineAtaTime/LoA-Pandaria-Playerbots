-- ported from PR 15 2026_10_25_55_world.sql (#986)
-- Silverpine Forest wagons. The sniffed spawns are snapshots of two wagon lines on the main road, and every wagon stood still (or ran a
-- few sniffed yards back and forth) while its Ettin, driver and troopers stood or wandered beside it. Per the sniffs, the Ettin
-- (seat 1, 6.7 yards ahead of the wagon), the Horde Engineer driver (seat 0), the troopers and the coffins all ride the wagon with
-- Ride Vehicle Hardcoded, so they now ride it as accessories. As on retail (Wowpedia):
-- * The Horde Hauler (44731) waits for passengers at the Tirisfal border, carries Forsaken Troopers south, stops at Forsaken High
--   Command and at the crossroads in front of the Sepulcher, puts its riders off at the Forsaken Front and despawns a little further
--   down the road.
-- * The Horde Coffin Hauler (44764) carries coffins from the Forsaken Front north into Tirisfal without stopping and despawns there,
--   putting its riders off.
-- An invisible bunny at the start of each line sends a wagon off every minute. The route is pieced together from all snapshots,
-- and the wagons keep their sniffed run speed. Creatures board with Ride Vehicle Hardcoded and players with Ride Horde Hauler, so
-- removing the players' aura at the Forsaken Front leaves the Ettin, driver and troopers aboard.
-- The once-only "Out of Combat" events are flagged not repeatable: without the flag this core fires an event with no repeat timer on
-- every update, and the wagon restarted its path from the first point several times a second. They are also flagged not to reset:
-- the threat of a mob that attacks a rider goes to the wagon, and when that fight ended the wagon's evade reset the event, which
-- started the path again from the Tirisfal border. The wagons are passive and do not evade, so they drive on through a fight
-- instead of chasing the mob and then returning to their last stop.
-- A despawning SmartAI creature turns invisible and leaves 5 seconds later, and a vehicle's accessories stay behind for 2 more
-- seconds when it goes, so the Ettin, driver, troopers and coffins stood on the road without their wagon. At its last point each
-- wagon tells its accessories to despawn and despawns with them, so they all vanish together.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (44731, 44734, 44764);
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (44732, 44733, 44737, 44766, 46559, 46560);
UPDATE `creature_template` SET `speed_run` = 2 WHERE `entry` = 44764;
-- Players click the Horde Hauler to ride it, so it must be selectable (its template had UNIT_FLAG_NOT_SELECTABLE)
UPDATE `creature_template` SET `unit_flags` = 768 WHERE `entry` = 44731;

-- The snapshots go: wagons, Ettins, drivers, troopers and coffin stacks
DELETE FROM `creature_addon` WHERE `guid` IN (SELECT `guid` FROM `creature` WHERE `id` IN (44731, 44732, 44733, 44734, 44737, 44764,
44766, 46559, 46560));
DELETE FROM `creature` WHERE `id` IN (44731, 44732, 44733, 44734, 44737, 44764, 44766, 46559, 46560);
DELETE FROM `waypoint_data` WHERE `id` IN (187571, 187581, 187590, 187599, 187636, 187643, 188102);
DELETE FROM `vehicle_accessory` WHERE `guid` IN (188034, 188199);

-- Wagon spawners (General Purpose Bunny JMF): the Horde Hauler's at the Tirisfal border, the Coffin Hauler's at the Forsaken Front
DELETE FROM `creature_addon` WHERE `guid` IN (188034, 188199);
DELETE FROM `creature` WHERE `guid` IN (188034, 188199);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `MovementType`) VALUES
(188034, 45979, 0, 85, 85, 1, 1, 11686, 1542.8, 583.1, 45.0, 2.42, 120, 0, 0),
(188199, 45979, 0, 130, 5387, 1, 1, 11686, -151.7, 1236.8, 52.4, 6.0, 120, 0, 0);

DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (44731, 44764);
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(44731, 44734, 0, 1, 'Horde Hauler - Horde Engineer', 8, 0),
(44731, 44737, 1, 1, 'Horde Hauler - Subdued Forest Ettin', 8, 0),
(44731, 44733, 3, 1, 'Horde Hauler - Forsaken Trooper', 8, 0),
(44731, 44733, 4, 1, 'Horde Hauler - Forsaken Trooper', 8, 0),
(44731, 44733, 5, 1, 'Horde Hauler - Forsaken Trooper', 8, 0),
(44731, 44732, 6, 1, 'Horde Hauler - Forsaken Trooper', 8, 0),
(44731, 44732, 7, 1, 'Horde Hauler - Forsaken Trooper', 8, 0),
(44764, 46559, 0, 1, 'Horde Coffin Hauler - Horde Engineer', 8, 0),
(44764, 46560, 1, 1, 'Horde Coffin Hauler - Subdued Forest Ettin', 8, 0),
(44764, 44766, 3, 1, 'Horde Coffin Hauler - Coffins', 8, 0),
(44764, 44766, 4, 1, 'Horde Coffin Hauler - Coffins', 8, 0),
(44764, 44766, 5, 1, 'Horde Coffin Hauler - Coffins', 8, 0);

-- Players ride with Ride Horde Hauler, the accessories with Ride Vehicle Hardcoded
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (44731, 44764);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(44731, 86807, 1, 0),
(44731, 46598, 1, 0),
(44764, 86807, 1, 0),
(44764, 46598, 1, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` IN (44731, 44764);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 44731 AND `SourceId` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 44731, 86807, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Horde Hauler - Ride Horde Hauler only for players'),
(18, 44731, 46598, 0, 0, 31, 0, 3, 0, 0, 0, 0, 0, '', 'Horde Hauler - Ride Vehicle Hardcoded only for creatures'),
(18, 44764, 86807, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Horde Coffin Hauler - Ride Horde Hauler only for players'),
(18, 44764, 46598, 0, 0, 31, 0, 3, 0, 0, 0, 0, 0, '', 'Horde Coffin Hauler - Ride Vehicle Hardcoded only for creatures'),
(22, 11, 44731, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Horde Hauler - Passenger Boarded - Only for players');

DELETE FROM `creature_text` WHERE `CreatureID` = 44734;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(44734, 0, 0, 'ALL ABOARD! Next stop: The Sepulcher.', 14, 1, 100, 0, 0, 0, 0, 44796, 0, 'Horde Engineer - Forsaken High Command'),
(44734, 1, 0, 'ALL ABOARD! Last stop: The Forsaken Front!', 14, 1, 100, 0, 0, 0, 0, 44797, 0, 'Horde Engineer - The Sepulcher'),
(44734, 2, 0, 'Final stop, the Forsaken Front! EVERYBODY OUT!', 12, 1, 100, 0, 0, 0, 0, 44788, 0, 'Horde Engineer - The Forsaken Front'),
(44734, 3, 0, 'This is a one way trip, kid, and you definitely don''t want to be comin'' back on the other wagon.', 12, 1, 100, 0, 0, 0, 0, 44790, 0, 'Horde Engineer - Player boards');

DELETE FROM `waypoints` WHERE `entry` IN (4473100, 4476400, 4476401);
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `point_comment`) VALUES
(4473100, 1, 1500.1, 620.6, 47.4, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 2, 1459.9, 647.9, 47.4, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 3, 1415.5, 710.0, 47.2, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 4, 1384.8, 784.0, 47.1, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 5, 1356.3, 860.5, 52.1, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 6, 1341.7, 936.3, 54.7, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 7, 1337.6, 959.3, 54.6, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 8, 1332.6, 998.1, 54.7, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 9, 1237.0, 1115.0, 52.3, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 10, 1215.2, 1141.0, 50.5, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 11, 1193.2, 1156.8, 49.3, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 12, 1179.0, 1166.9, 49.0, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 13, 1162.9, 1176.9, 48.5, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 14, 1143.1, 1189.0, 48.0, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 15, 1098.9, 1207.3, 47.9, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 16, 1066.3, 1241.4, 46.3, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 17, 1053.2, 1254.0, 46.2, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 18, 1042.0, 1274.3, 46.6, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 19, 1001.4, 1292.8, 47.5, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 20, 959.2, 1319.8, 47.7, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 21, 914.7, 1350.7, 48.2, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 22, 880.6, 1358.5, 52.1, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 23, 861.4, 1359.9, 55.1, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 24, 840.6, 1360.7, 55.7, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 25, 793.0, 1361.0, 62.9, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 26, 722.8, 1353.6, 75.1, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 27, 677.7, 1319.8, 80.4, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 28, 668.6, 1314.2, 81.5, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 29, 657.5, 1307.4, 83.3, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 30, 646.8, 1303.2, 84.9, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 31, 633.5, 1298.2, 86.2, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 32, 446.5, 1226.6, 87.9, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 33, 406.4, 1233.8, 83.7, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 34, 384.7, 1235.9, 82.4, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 35, 362.5, 1240.6, 81.7, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 36, 354.0, 1242.6, 81.5, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 37, 293.0, 1259.0, 79.6, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 38, 241.5, 1270.5, 77.2, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 39, 203.1, 1259.1, 73.3, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 40, 188.0, 1259.0, 72.8, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 41, 177.6, 1256.2, 71.8, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 42, 165.1, 1252.8, 71.0, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 43, 149.7, 1248.0, 70.9, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 44, 129.4, 1241.8, 70.8, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 45, 42.9, 1212.0, 65.8, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 46, 1.9, 1199.6, 64.7, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 47, -113.5, 1214.2, 58.5, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4473100, 48, -187.4, 1210.9, 56.1, NULL, 0, 'Horde Hauler - Tirisfal to the Forsaken Front'),
(4476400, 1, -101.4, 1217.5, 58.3, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 2, 1.9, 1199.6, 64.7, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 3, 42.9, 1212.0, 65.8, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 4, 129.4, 1241.8, 70.8, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 5, 149.7, 1248.0, 70.9, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 6, 165.1, 1252.8, 71.0, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 7, 177.6, 1256.2, 71.8, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 8, 188.0, 1259.0, 72.8, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 9, 203.1, 1259.1, 73.3, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 10, 241.5, 1270.5, 77.2, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 11, 293.0, 1259.0, 79.6, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 12, 354.0, 1242.6, 81.5, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 13, 362.5, 1240.6, 81.7, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 14, 384.7, 1235.9, 82.4, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 15, 406.4, 1233.8, 83.7, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 16, 446.5, 1226.6, 87.9, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 17, 633.5, 1298.2, 86.2, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 18, 646.8, 1303.2, 84.9, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 19, 657.5, 1307.4, 83.3, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 20, 668.6, 1314.2, 81.5, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 21, 677.7, 1319.8, 80.4, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 22, 722.8, 1353.6, 75.1, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 23, 793.0, 1361.0, 62.9, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 24, 840.6, 1360.7, 55.7, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 25, 861.4, 1359.9, 55.1, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 26, 880.6, 1358.5, 52.1, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 27, 914.7, 1350.7, 48.2, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 28, 959.2, 1319.8, 47.7, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 29, 1001.4, 1292.8, 47.5, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 30, 1042.0, 1274.3, 46.6, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 31, 1053.2, 1254.0, 46.2, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 32, 1066.3, 1241.4, 46.3, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 33, 1098.9, 1207.3, 47.9, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 34, 1143.1, 1189.0, 48.0, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 35, 1162.9, 1176.9, 48.5, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 36, 1179.0, 1166.9, 49.0, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 37, 1193.2, 1156.8, 49.3, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 38, 1215.2, 1141.0, 50.5, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 39, 1237.0, 1115.0, 52.3, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 40, 1332.6, 998.1, 54.7, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 41, 1337.6, 959.3, 54.6, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 42, 1341.7, 936.3, 54.7, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 43, 1356.3, 860.5, 52.1, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 44, 1384.8, 784.0, 47.1, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 45, 1415.5, 710.0, 47.2, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 46, 1459.9, 647.9, 47.4, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 47, 1500.1, 620.6, 47.4, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal'),
(4476400, 48, 1542.8, 583.1, 45.0, NULL, 0, 'Horde Coffin Hauler - Forsaken Front to Tirisfal');

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (44731, 44732, 44733, 44734, 44737, 44764, 44766, 46559, 46560, -188034, -187562, -188199, -188200) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(-188034, 0, 0, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 0, 48, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'General Purpose Bunny JMF - Out of Combat - Set Active'),
(-188034, 0, 1, 0, 1, 0, 100, 0, 2000, 2000, 60000, 60000, 0, 12, 44731, 8, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'General Purpose Bunny JMF - Out of Combat - Summon Horde Hauler'),
(-188199, 0, 0, 0, 1, 0, 100, 1, 1000, 1000, 0, 0, 0, 48, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'General Purpose Bunny JMF - Out of Combat - Set Active'),
(-188199, 0, 1, 0, 1, 0, 100, 0, 2000, 2000, 60000, 60000, 0, 12, 44764, 8, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'General Purpose Bunny JMF - Out of Combat - Summon Horde Coffin Hauler'),
(44731, 0, 0, 1, 1, 0, 100, 65537, 1000, 1000, 0, 0, 0, 48, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Hauler - Out of Combat - Set Active'),
(44731, 0, 1, 12, 61, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 4473100, 0, 0, 1000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Start Waypoint'),
(44731, 0, 2, 3, 40, 0, 100, 0, 7, 4473100, 0, 0, 0, 45, 1, 0, 0, 0, 0, 0, 19, 44734, 5, 0, 0, 0, 0, 0, 0, 'Horde Hauler - On Waypoint 7 Reached - Set Data 1 0 (Horde Engineer) (Forsaken High Command)'),
(44731, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 54, 20000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Pause Waypoint'),
(44731, 0, 4, 5, 40, 0, 100, 0, 31, 4473100, 0, 0, 0, 45, 1, 1, 0, 0, 0, 0, 19, 44734, 5, 0, 0, 0, 0, 0, 0, 'Horde Hauler - On Waypoint 31 Reached - Set Data 1 1 (Horde Engineer) (The Sepulcher)'),
(44731, 0, 5, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 54, 20000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Pause Waypoint'),
(44731, 0, 6, 7, 40, 0, 100, 0, 47, 4473100, 0, 0, 0, 45, 1, 2, 0, 0, 0, 0, 19, 44734, 5, 0, 0, 0, 0, 0, 0, 'Horde Hauler - On Waypoint 47 Reached - Set Data 1 2 (Horde Engineer) (The Forsaken Front)'),
(44731, 0, 7, 8, 61, 0, 100, 0, 0, 0, 0, 0, 0, 28, 86807, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Remove Aura ''Ride Horde Hauler'''),
(44731, 0, 8, 9, 61, 0, 100, 0, 0, 0, 0, 0, 0, 83, 16777216, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Remove Npc Flag Spellclick'),
(44731, 0, 9, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 54, 5000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Pause Waypoint'),
(44731, 0, 10, 0, 27, 0, 100, 0, 0, 0, 0, 0, 0, 45, 1, 3, 0, 0, 0, 0, 19, 44734, 5, 0, 0, 0, 0, 0, 0, 'Horde Hauler - On Passenger Boarded - Set Data 1 3 (Horde Engineer)'),
(44731, 0, 11, 0, 40, 0, 100, 0, 1, 4473100, 0, 0, 0, 54, 20000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Hauler - On Waypoint 1 Reached - Pause Waypoint (Tirisfal border)'),
(44731, 0, 12, 13, 61, 0, 100, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Set Reactstate Passive'),
(44731, 0, 13, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 117, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Disable Evade'),
(44731, 0, 14, 15, 40, 0, 100, 0, 48, 4473100, 0, 0, 0, 45, 2, 1, 0, 0, 0, 0, 9, 44734, 0, 15, 0, 0, 0, 0, 0, 'Horde Hauler - On Waypoint 48 Reached - Set Data 2 1 (Horde Engineer)'),
(44731, 0, 15, 16, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 2, 1, 0, 0, 0, 0, 9, 44737, 0, 15, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Set Data 2 1 (Subdued Forest Ettin)'),
(44731, 0, 16, 17, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 2, 1, 0, 0, 0, 0, 9, 44733, 0, 15, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Set Data 2 1 (Forsaken Trooper)'),
(44731, 0, 17, 18, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 2, 1, 0, 0, 0, 0, 9, 44732, 0, 15, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Set Data 2 1 (Forsaken Trooper)'),
(44731, 0, 18, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Hauler - Linked - Despawn'),
(44734, 0, 0, 0, 38, 0, 100, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Engineer - On Data Set 1 0 - Say Line 0'),
(44734, 0, 1, 0, 38, 0, 100, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Engineer - On Data Set 1 1 - Say Line 1'),
(44734, 0, 2, 0, 38, 0, 100, 0, 1, 2, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Engineer - On Data Set 1 2 - Say Line 2'),
(44734, 0, 3, 0, 38, 0, 100, 0, 1, 3, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Engineer - On Data Set 1 3 - Say Line 3'),
(44734, 0, 4, 0, 38, 0, 100, 0, 2, 1, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Engineer - On Data Set 2 1 - Despawn'),
(44732, 0, 0, 0, 38, 0, 100, 0, 2, 1, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Forsaken Trooper - On Data Set 2 1 - Despawn'),
(44733, 0, 0, 0, 38, 0, 100, 0, 2, 1, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Forsaken Trooper - On Data Set 2 1 - Despawn'),
(44737, 0, 0, 0, 38, 0, 100, 0, 2, 1, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Subdued Forest Ettin - On Data Set 2 1 - Despawn'),
(44766, 0, 0, 0, 38, 0, 100, 0, 2, 1, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Coffins - On Data Set 2 1 - Despawn'),
(46559, 0, 0, 0, 38, 0, 100, 0, 2, 1, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Engineer - On Data Set 2 1 - Despawn'),
(46560, 0, 0, 0, 38, 0, 100, 0, 2, 1, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Subdued Forest Ettin - On Data Set 2 1 - Despawn'),
(44764, 0, 0, 1, 1, 0, 100, 65537, 1000, 1000, 0, 0, 0, 48, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Coffin Hauler - Out of Combat - Set Active'),
(44764, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 4476400, 0, 0, 1000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Coffin Hauler - Linked - Start Waypoint'),
(44764, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Coffin Hauler - Linked - Set Reactstate Passive'),
(44764, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 117, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Coffin Hauler - Linked - Disable Evade'),
(44764, 0, 4, 5, 40, 0, 100, 0, 48, 4476400, 0, 0, 0, 45, 2, 1, 0, 0, 0, 0, 9, 46559, 0, 15, 0, 0, 0, 0, 0, 'Horde Coffin Hauler - On Waypoint 48 Reached - Set Data 2 1 (Horde Engineer)'),
(44764, 0, 5, 6, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 2, 1, 0, 0, 0, 0, 9, 46560, 0, 15, 0, 0, 0, 0, 0, 'Horde Coffin Hauler - Linked - Set Data 2 1 (Subdued Forest Ettin)'),
(44764, 0, 6, 7, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 2, 1, 0, 0, 0, 0, 9, 44766, 0, 15, 0, 0, 0, 0, 0, 'Horde Coffin Hauler - Linked - Set Data 2 1 (Coffins)'),
(44764, 0, 7, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Horde Coffin Hauler - Linked - Despawn');
