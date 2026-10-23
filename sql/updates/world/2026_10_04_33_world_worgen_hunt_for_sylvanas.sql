-- Refs #439, #475. Quest 24902 "The Hunt For Sylvanas" (Gilneas City, map 654, phase 2097152).
--
-- Intended (Wowhead tooltip API quest=24902, read 2026-10-04: "Follow Tobias Mistmantle in the Hunt for
-- Sylvanas ... Requirements: Hunt for Sylvanas"; Wowpedia "The Hunt For Sylvanas" notes): on accept Lorna
-- calls Tobias Mistmantle, who sneaks to the cathedral; the Forsaken General yells, Tobias hides in the water
-- and the player watches Sylvanas, General Warhowl and High Executor Crenshaw talk; the event completes the quest.
-- Fork today: Tobias 38507, Sylvanas 38530, Warhowl 38533 and Crenshaw 38537 have creature_text rows but no AI
-- (AIName=''), Lorna 38611 has no AI, there are no waypoints, and the quest has no objective, so it completes
-- on accept and the whole scene is missing.
--
-- Source: MoP 5.4.8 world data:
-- smart_scripts 38611/38507/38530/38533/38537 and `waypoints` 38507/385070/385071/385073/38530/38533/385330/38537/385370,
-- mapped to this fork's smart_scripts columns (event_param5 / target_param4 = 0). One change: MoP row
-- 38530 id 10 starts path 385300, which does not exist; it is dropped and id 9 links straight to id 11
-- (the event credit). Tobias summons Sylvanas as a 120 s timed summon, so she still leaves.
-- Completion: SpecialFlags |= 2 (QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT) so the quest completes from Sylvanas'
-- CALL_AREAEXPLOREDOREVENTHAPPENS(24902) at the end of the scene (players within 100 yd), as on retail.
--
-- Rollback:
--   UPDATE `creature_template` SET `AIName` = '' WHERE `entry` IN (38611, 38507, 38530, 38533, 38537);
--   DELETE FROM `smart_scripts` WHERE `entryorguid` IN (38611, 38507, 38530, 38533, 38537) AND `source_type` = 0;
--   DELETE FROM `waypoints` WHERE `entry` IN (38507, 385070, 385071, 385073, 38530, 38533, 385330, 38537, 385370);
--   UPDATE `quest_template_addon` SET `SpecialFlags` = `SpecialFlags` & ~2 WHERE `ID` = 24902;
-- Re-runnable: UPDATE + DELETE-then-INSERT.

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (38611, 38507, 38530, 38533, 38537) AND `ScriptName` = '';

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (38611, 38507, 38530, 38533, 38537) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38611, 0, 0, 0, 19, 0, 100, 0, 24902, 0, 0, 0, 0, 12, 38507, 3, 210000, 0, 0, 0, 8, 0, 0, 0, 0, -1654.18, 1604.09, 23.13, 3.9, 'Lorna Crowley - On q a 24902 summon 38507'),
(38507, 0, 0, 0, 60, 0, 100, 1, 4000, 4000, 4000, 4000, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - On update 4 sec talk text - 0'),
(38507, 0, 1, 0, 60, 0, 100, 1, 8000, 8000, 8000, 8000, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - after4 sec say text 1'),
(38507, 0, 2, 0, 60, 0, 100, 1, 12000, 12000, 12000, 12000, 0, 1, 2, 0, 0, 0, 0, 0, 21, 20, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - after 12 sec text 2'),
(38507, 0, 3, 0, 60, 0, 100, 1, 18000, 18000, 18000, 18000, 0, 97, 20, 20, 0, 0, 0, 0, 8, 0, 0, 0, 0, -1590.24, 1606.87, 21.59, 21.59, 'Tobias Mistmantle - After 18 sec jump 86859'),
(38507, 0, 4, 0, 60, 0, 100, 1, 26000, 26000, 26000, 26000, 0, 97, 20, 20, 0, 0, 0, 0, 8, 0, 0, 0, 0, -1576.35, 1627.63, 20.64, 5.98, 'Tobias Mistmantle - After 26 sec jump next trigger'),
(38507, 0, 5, 0, 60, 0, 100, 1, 32000, 32000, 32000, 32000, 0, 53, 1, 38507, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - After 32 sec Start WP 1'),
(38507, 0, 6, 0, 40, 0, 100, 1, 1, 38507, 0, 0, 0, 97, 20, 20, 0, 0, 0, 0, 8, 0, 0, 0, 0, -1538.74, 1618.07, 20.49, 5.94, 'Tobias Mistmantle - on WP reach'),
(38507, 0, 7, 0, 60, 0, 100, 1, 37000, 37000, 37000, 37000, 0, 53, 1, 385070, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - After 37 sec WP2 start'),
(38507, 0, 8, 0, 40, 0, 100, 1, 3, 385070, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 38617, 50, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - On wp reached 2 make talk General'),
(38507, 0, 9, 10, 60, 0, 100, 1, 60000, 60000, 60000, 60000, 0, 1, 3, 0, 0, 0, 0, 0, 21, 20, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - After 60 sec Talk text 3 and do WP3'),
(38507, 0, 10, 0, 61, 0, 100, 1, 0, 0, 0, 0, 0, 53, 1, 385071, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - Wp3 start on link 9'),
(38507, 0, 11, 14, 40, 0, 100, 1, 11, 385071, 0, 0, 0, 97, 5, 5, 0, 0, 0, 0, 8, 0, 0, 0, 0, -1613.6, 1538.48, 27.27, 5.51, 'Tobias Mistmantle - On wp3 Reach cast jump on trigger'),
(38507, 0, 14, 15, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - Text 4 on link 10'),
(38507, 0, 15, 16, 61, 0, 100, 0, 0, 0, 0, 0, 0, 12, 38530, 3, 120000, 0, 0, 0, 8, 0, 0, 0, 0, -1574.55, 1542.17, 29.21, 3.893, 'Tobias Mistmantle - On wp reach summon sylvanas'),
(38507, 0, 16, 17, 61, 0, 100, 0, 0, 0, 0, 0, 0, 12, 38533, 3, 120000, 0, 0, 0, 8, 0, 0, 0, 0, -1571.86, 1539.48, 29.21, 3.98, 'Tobias Mistmantle - On wp reach summon General'),
(38507, 0, 17, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 12, 38537, 3, 120000, 0, 0, 0, 8, 0, 0, 0, 0, -1569.67, 1537.31, 29.219, 3.92, 'Tobias Mistmantle - On wp reach summon Crenshaw'),
(38507, 0, 18, 0, 60, 0, 100, 1, 5000, 5000, 5000, 5000, 0, 53, 1, 385073, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - On 5 sec lets WP 1 start'),
(38530, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 0, 53, 0, 38530, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Sylvanas Windrunner - On summoned do WP'),
(38530, 0, 1, 0, 60, 0, 100, 1, 26000, 26000, 26000, 26000, 0, 1, 0, 0, 0, 0, 0, 0, 19, 38533, 100, 0, 0, 0, 0, 0, 0, 'Lady Sylvanas Windrunner - After 26sec Make talk General'),
(38530, 0, 2, 0, 58, 0, 100, 0, 3, 38530, 0, 0, 0, 69, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, -1601.31, 1521.2, 29.01, 0.62, 'Lady Sylvanas Windrunner - ON WP Reach'),
(38530, 0, 3, 0, 60, 0, 100, 1, 34000, 34000, 34000, 34000, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Sylvanas Windrunner - update 34sec'),
(38530, 0, 4, 0, 60, 0, 100, 1, 45000, 45000, 45000, 45000, 0, 1, 1, 0, 0, 0, 0, 0, 19, 38533, 100, 0, 0, 0, 0, 0, 0, 'Lady Sylvanas Windrunner - 45 sec talk 1 General'),
(38530, 0, 5, 0, 60, 0, 100, 1, 58000, 58000, 58000, 58000, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Sylvanas Windrunner - 58 sec Sylvanas text 1'),
(38530, 0, 6, 0, 60, 0, 100, 1, 72000, 72000, 72000, 72000, 0, 1, 2, 0, 0, 0, 0, 0, 19, 38533, 100, 0, 0, 0, 0, 0, 0, 'Lady Sylvanas Windrunner - 72sec Say text 2 general'),
(38530, 0, 7, 0, 60, 0, 100, 1, 79000, 79000, 79000, 79000, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Sylvanas Windrunner - 79 sec'),
(38530, 0, 8, 0, 60, 0, 100, 1, 85000, 85000, 85000, 85000, 0, 1, 0, 0, 0, 0, 0, 0, 19, 38537, 100, 0, 0, 0, 0, 0, 0, 'Lady Sylvanas Windrunner - 85sec talk Crenshaw'),
(38530, 0, 9, 11, 60, 0, 100, 1, 96000, 96000, 96000, 96000, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Lady Sylvanas Windrunner - 96 sec talk and do WP  and quest complete'),
(38530, 0, 11, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 15, 24902, 0, 0, 0, 0, 0, 18, 100, 0, 0, 0, 0, 0, 0, 0, 'Lady Sylvanas Windrunner - Event Complete'),
(38533, 0, 0, 0, 60, 0, 100, 1, 3000, 3000, 3000, 3000, 0, 53, 0, 38533, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'General Warhowl - On summoned do wp'),
(38533, 0, 1, 0, 60, 0, 100, 1, 75000, 75000, 75000, 75000, 0, 53, 0, 385330, 0, 0, 1000, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'General Warhowl - After 75 sec WP 2'),
(38537, 0, 0, 0, 60, 0, 100, 1, 3000, 3000, 3000, 3000, 0, 53, 0, 38537, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'High Executor Crenshaw - On summoned start wp'),
(38537, 0, 1, 2, 60, 0, 100, 1, 110000, 110000, 110000, 110000, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'High Executor Crenshaw - Update 110 sec talk 1'),
(38537, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 53, 0, 385370, 0, 0, 100, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'High Executor Crenshaw - wp start 2');

DELETE FROM `waypoints` WHERE `entry` IN (38507, 385070, 385071, 385073, 38530, 38533, 385330, 38537, 385370);
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `point_comment`) VALUES
(38507, 1, -1555.59, 1619.31, 20.8307, 0, 0, ''),
(38530, 1, -1564.62, 1552.24, 29.2244, 0, 0, ''),
(38530, 2, -1576.57, 1540.68, 29.2129, 0, 0, ''),
(38530, 3, -1603.74, 1520.56, 29.2485, 0, 0, ''),
(38533, 1, -1565.98, 1551.01, 29.2228, 0, 0, ''),
(38533, 2, -1576.66, 1540.44, 29.2131, 0, 0, ''),
(38533, 3, -1595.19, 1526.14, 29.235, 0, 0, ''),
(38537, 1, -1566.99, 1550.3, 29.2016, 0, 0, ''),
(38537, 2, -1573.24, 1543.88, 29.2059, 0, 0, ''),
(38537, 3, -1592.1, 1521.45, 29.2363, 0, 0, ''),
(385070, 1, -1514.53, 1603.12, 20.4856, 0, 0, ''),
(385070, 2, -1504.51, 1583.39, 20.4856, 0, 0, ''),
(385070, 3, -1491.99, 1576.42, 20.4856, 0, 0, ''),
(385071, 1, -1505.06, 1582.71, 20.4856, 0, 0, ''),
(385071, 2, -1520.19, 1582.69, 26.2706, 0, 0, ''),
(385071, 3, -1534.11, 1580.41, 26.9622, 0, 0, ''),
(385071, 4, -1550.85, 1567.72, 29.2209, 0, 0, ''),
(385071, 5, -1557.66, 1567.85, 29.1986, 0, 0, ''),
(385071, 6, -1566.24, 1559.66, 29.1908, 0, 0, ''),
(385071, 7, -1566.93, 1551.57, 29.2267, 0, 0, ''),
(385071, 8, -1588.49, 1529.09, 29.2281, 0, 0, ''),
(385071, 9, -1593.3, 1527.39, 29.2325, 0, 0, ''),
(385071, 10, -1599.47, 1528.52, 29.235, 0, 0, ''),
(385071, 11, -1610.49, 1535.68, 29.2288, 0, 0, ''),
(385073, 1, -1664.86, 1613.22, 20.4896, 0, 0, ''),
(385073, 2, -1648.61, 1620.66, 20.4893, 0, 0, ''),
(385073, 3, -1627.57, 1614.18, 21.5902, 0, 0, ''),
(385330, 1, -1577.42, 1540.39, 29.2166, 0, 0, ''),
(385330, 2, -1563.92, 1553.29, 29.2246, 0, 0, ''),
(385330, 3, -1564.31, 1556.02, 29.2228, 0, 0, ''),
(385370, 1, -1572.36, 1544.62, 29.2054, 0, 0, ''),
(385370, 2, -1564.2, 1552.82, 29.2247, 0, 0, ''),
(385370, 3, -1566.89, 1555.4, 29.2011, 0, 0, '');

UPDATE `quest_template_addon` SET `SpecialFlags` = `SpecialFlags` | 2 WHERE `ID` = 24902;
