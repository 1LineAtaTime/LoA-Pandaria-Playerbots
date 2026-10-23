-- Refs #439, #483 (item 3). Quest 24677 "Flank the Forsaken": the Stout Mountain Horse ride from Lord Hewell.
--
-- Intended (Wowpedia "Flank the Forsaken", MediaWiki API read 2026-10-04): "Speak to Lord Hewell to obtain a ride
-- to the Livery Outpost and report to Lorna Crowley"; without the horse it is "a very long run".
-- Fork today: King Genn 37876 SAI teleports the player to the Livery Outpost on accept; Hewell 38764 has gossip
-- menu 11079 but no AI, and its option has OptionNpcFlag 0, so Player::PrepareGossipMenu (Player.cpp:15484) never
-- shows "I need a horse.".
--
-- Source: MoP 5.4.8 world data: smart_scripts 38764
-- (gossip 11079/0 -> close gossip + invoker cast 72772 "Summon Stout Mountain Horse"), gossip_menu_option
-- 11079/0 npc_option_npcflag 1, and `waypoints` 38765 (76 points Hewell -> Livery Outpost).
-- DBC: 72772 summons 38765 with SummonProperties 161 (control 4 = vehicle: the summoner is seated).
-- Conflict: this fork already uses 38765 + `waypoints` 38765 (15 points) for 14465 "To Greymane Manor"
-- (alexkulya 2d4b1928, Gwen Armstead 36452 casts the same 72772 in Duskhaven). So the MoP path is stored as
-- path 387650, the boarding row is split by the horse's area (Hewell stands in area 4788; Duskhaven is 4786),
-- and the existing WP-reached rows get pathID 38765 (SmartScript.cpp:3327 checks it) so point 15 of the long
-- path does not eject the rider.
--
-- Rollback:
--   UPDATE `gossip_menu_option` SET `OptionNpcFlag` = 0 WHERE `MenuID` = 11079 AND `OptionID` = 0;
--   UPDATE `creature_template` SET `AIName` = '' WHERE `entry` = 38764;
--   DELETE FROM `smart_scripts` WHERE `entryorguid` = 38764 AND `source_type` = 0;
--   DELETE FROM `smart_scripts` WHERE `entryorguid` = 38765 AND `source_type` = 0 AND `id` IN (7, 8, 9);
--   UPDATE `smart_scripts` SET `event_param2` = 0 WHERE `entryorguid` = 38765 AND `source_type` = 0 AND `id` IN (1, 2, 3, 5, 6);
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 38765 AND `SourceGroup` IN (1, 8);
--   DELETE FROM `waypoints` WHERE `entry` = 387650;
--   INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
--   (37876, 0, 0, 0, 19, 0, 100, 0, 24677, 0, 0, 0, 0, 62, 654, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, -1369.87, 1201.49, 35.5576, 1.58346, 'King Genn Greymane - Teleport');
-- Re-runnable: UPDATE + DELETE-then-INSERT.

-- No more teleport on accept: the ride replaces it.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 37876 AND `source_type` = 0 AND `id` = 0 AND `event_type` = 19 AND `event_param1` = 24677;

UPDATE `gossip_menu_option` SET `OptionNpcFlag` = 1 WHERE `MenuID` = 11079 AND `OptionID` = 0;
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 38764 AND `ScriptName` = '';

-- Existing 14465 rows: only for path 38765.
UPDATE `smart_scripts` SET `event_param2` = 38765 WHERE `entryorguid` = 38765 AND `source_type` = 0 AND `id` IN (1, 2, 3, 5, 6) AND `event_type` = 40;

DELETE FROM `smart_scripts` WHERE (`entryorguid` = 38764 AND `source_type` = 0) OR (`entryorguid` = 38765 AND `source_type` = 0 AND `id` IN (7, 8, 9));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38764, 0, 0, 1, 62, 0, 100, 0, 11079, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Hewell - On Gossip Select - Close Gossip'),
(38764, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 85, 72772, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Hewell - On Gossip Select - Invoker Cast Summon Stout Mountain Horse'),
(38765, 0, 7, 0, 27, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 387650, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Stout Mountain Horse - On Passenger Boarded at Lord Hewell - Start WP 387650 (24677)'),
(38765, 0, 8, 9, 40, 0, 100, 0, 76, 387650, 0, 0, 0, 28, 46598, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Stout Mountain Horse - On WP 76 of path 387650 (Livery Outpost) - Remove Passenger'),
(38765, 0, 9, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Stout Mountain Horse - On WP 76 of path 387650 - Despawn');

-- Split the boarding by area: 4788 = Lord Hewell (24677), anything else = the 14465 Duskhaven ride.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 38765 AND `SourceGroup` IN (1, 8);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, 38765, 0, 0, 23, 1, 4788, 0, 0, 1, 0, 0, '', 'Stout Mountain Horse - 14465 path only outside Lord Hewell''s area'),
(22, 8, 38765, 0, 0, 23, 1, 4788, 0, 0, 0, 0, 0, '', 'Stout Mountain Horse - 24677 path only in Lord Hewell''s area');

DELETE FROM `waypoints` WHERE `entry` = 387650;
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `point_comment`) VALUES
(387650, 1, -2043.51, 971.751, 70.0641, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 2, -2051.27, 960.127, 70.0962, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 3, -2054.27, 951.06, 70.0172, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 4, -2049.76, 945.173, 69.9058, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 5, -2037.85, 942.373, 68.8608, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 6, -2024.34, 938.771, 65.8467, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 7, -2010.33, 929.75, 61.5077, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 8, -2002.81, 920.062, 58.8925, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 9, -1995.87, 902.164, 57.1566, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 10, -1991.55, 891.491, 57.8934, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 11, -1980.98, 871.16, 63.5979, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 12, -1972.82, 863.64, 64.4, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 13, -1946.91, 857.269, 65.3255, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 14, -1933.24, 857.888, 69.4688, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 15, -1912.38, 860.041, 75.408, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 16, -1895.56, 863.66, 80.0938, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 17, -1885.44, 872.751, 81.9168, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 18, -1878.03, 896.832, 81.101, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 19, -1876.78, 907.252, 80.0149, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 20, -1875.41, 938.711, 78.4323, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 21, -1876.2, 953.704, 77.6575, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 22, -1881.51, 969.001, 76.7696, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 23, -1887.66, 982.358, 75.8184, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 24, -1882.21, 998.779, 72.8388, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 25, -1880.43, 1015.48, 68.6976, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 26, -1875, 1034.73, 63.7603, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 27, -1866, 1049.49, 60.2971, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 28, -1860.66, 1058.53, 58.0102, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 29, -1853.13, 1077.48, 52.8661, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 30, -1850.45, 1098.29, 47.5895, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 31, -1845.46, 1119.29, 42.0725, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 32, -1846.62, 1140.24, 37.3987, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 33, -1849.3, 1161.06, 33.307, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 34, -1855.24, 1191.97, 26.7668, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 35, -1858.99, 1211.97, 23.4134, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 36, -1857.08, 1224.13, 22.2441, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 37, -1855.33, 1224.98, 22.1217, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 38, -1832.74, 1203.03, 19.7035, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 39, -1816.36, 1189.94, 19.7795, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 40, -1798.55, 1178.88, 20.755, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 41, -1783.45, 1171.8, 21.8187, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 42, -1763.66, 1164.79, 24.3709, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 43, -1745.48, 1158.22, 26.9495, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 44, -1731.23, 1150.99, 27.2255, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 45, -1711.28, 1144.5, 25.9203, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 46, -1698.47, 1140.45, 25.6273, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 47, -1680.45, 1129.74, 26.0414, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 48, -1666.59, 1122.15, 27.1156, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 49, -1657.23, 1117.41, 27.7283, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 50, -1639.72, 1112.05, 29.1036, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 51, -1629.23, 1112.32, 29.1893, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 52, -1608.44, 1114.89, 30.087, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 53, -1598.07, 1116.51, 30.225, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 54, -1577.25, 1119.26, 30.3166, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 55, -1564.35, 1120.15, 30.241, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 56, -1542.24, 1117.42, 30.2171, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 57, -1532.27, 1111.27, 30.0303, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 58, -1521.59, 1101.39, 29.4908, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 59, -1505.32, 1089.47, 29.0793, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 60, -1489.19, 1083.18, 29.6792, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 61, -1473.93, 1081.27, 29.7116, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 62, -1460.47, 1081.21, 29.2542, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 63, -1440.07, 1082.5, 29.0997, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 64, -1419.61, 1087.15, 28.8834, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 65, -1400.19, 1095.03, 29.7621, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 66, -1390.87, 1099.86, 29.9771, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 67, -1372.74, 1110.42, 30.4781, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 68, -1355.68, 1116.74, 31.9977, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 69, -1336.96, 1119.93, 34.6389, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 70, -1319.93, 1124.14, 36.7613, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 71, -1315.68, 1131.31, 37.1873, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 72, -1320.67, 1135.37, 36.9177, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 73, -1334.59, 1151.08, 36.0186, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 74, -1347.96, 1167.27, 34.7997, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 75, -1361.64, 1183.2, 34.8543, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost'),
(387650, 76, -1383.54, 1209.61, 35.5577, NULL, 0, 'Stout Mountain Horse - 24677 Hewell to Livery Outpost');
