-- Reviewed Gilneas progression batch; back up all affected tables.

-- Requires the ride-snapshot and catapult safety native build.

-- Provenance: contrib/upstream-tests/batches/gilneas-20261105.json.

-- Match the loader optional-orientation contract without rewriting old values.

ALTER TABLE `waypoints` MODIFY COLUMN `orientation` FLOAT NULL DEFAULT NULL;

-- Source sql/updates/world/2026_10_04_30_world_worgen_two_by_sea_catapult.sql; SHA256(LF) 4c63747fdc3d3a4f30db45ebdae315252ab3da8fc3719c34f4185ba3bd158d31

UPDATE `creature_template` SET `faction` = 35, `unit_flags` = 16384 WHERE `entry` = 36283;

DELETE FROM `vehicle_template_accessory` WHERE `entry` = 36283;

INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(36283, 36292, 0, 1, 'Forsaken Catapult - Forsaken Machinist', 6, 30000);

DELETE FROM `spell_script_names` WHERE `spell_id` = 68659 AND `ScriptName` = 'spell_ioc_launch';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(68659, 'spell_ioc_launch');

DELETE `a` FROM `creature_addon` `a` JOIN `creature` `c` ON `c`.`guid` = `a`.`guid`
WHERE `c`.`id` = 36292 AND `a`.`guid` IN (221045, 221189, 221197, 221290, 221706, 221723, 221779, 221788);

DELETE FROM `creature` WHERE `id` = 36292 AND `guid` IN (221045, 221189, 221197, 221290, 221706, 221723, 221779, 221788);

-- Source sql/updates/world/2026_10_04_31_world_worgen_duskhaven_sparring.sql; SHA256(LF) 1f1833316592839aa47c085ea3f3a1eeff8154c08183f9a9532fd7c01667d687

UPDATE `creature_template` SET `ScriptName` = 'npc_forsaken_invader' WHERE `entry` = 34511;

UPDATE `creature_template` SET `ScriptName` = 'npc_duskhaven_watchman' WHERE `entry` = 36211;

-- Source sql/updates/world/2026_10_04_32_world_worgen_sacrifices_crowley_horse.sql; SHA256(LF) 8a4f58fdbf2ed5dee8218ee1b54b52b02a9f1ec3a6e654047795f996f0ad2e03

DELETE FROM `smart_scripts` WHERE `entryorguid` = 35231 AND `source_type` = 0 AND `id` = 8;

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35231, 0, 8, 0, 27, 0, 100, 1, 0, 0, 0, 0, 0, 80, 3523100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Crowley''s Horse - On Player Boarded (static spawn) - Start Timed Action Script 3523100');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 9 AND `SourceEntry` = 35231;

INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 9, 35231, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Crowley''s Horse - start the run only when a player boards');

-- Source sql/updates/world/2026_10_04_33_world_worgen_hunt_for_sylvanas.sql; SHA256(LF) 198d91a3a78ec17e15c86b563ba5a7366a731e07df3a80038c52017e051390e5

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

-- Source sql/updates/world/2026_10_04_34_world_worgen_koroth_banner.sql; SHA256(LF) 58c5ad4d40a9973af483905324819abc64d6b6e52d32608a2d48c8b0c87f73b1

UPDATE `creature_template` SET `ScriptName` = 'npc_koroth_the_hillbreaker' WHERE `entry` = 36294;

UPDATE `gameobject_template` SET `ScriptName` = 'go_koroth_banner' WHERE `entry` = 201594;

-- Source sql/updates/world/2026_10_04_35_world_worgen_losing_your_tail.sql; SHA256(LF) bd05ce152a42b5eaf8e2e4fa2e7304df58b39bf93e5670588344b793a495461f

UPDATE `creature` SET `phaseMask` = 196607 WHERE `guid` = 219500 AND `id` = 35374;

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (35374, 37953) AND `ScriptName` = '';

DELETE FROM `creature` WHERE `guid` = 222093 AND `id` = 37953;

DELETE FROM `creature_addon` WHERE `guid` = 222093;

DELETE FROM `smart_scripts` WHERE (`entryorguid` = -219500 OR `entryorguid` = 37953) AND `source_type` = 0;

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(-219500, 0, 0, 0, 10, 0, 100, 0, 1, 25, 1000, 1000, 1, 85, 70794, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Losing Your Tail trap trigger - OOC LOS player (quest 24616 taken, no scout near) - Invoker Cast Freezing Trap Effect'),
(37953, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 0, 59, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Just Summoned - Set Run'),
(37953, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Just Summoned - Say Line 1 (Such easy prey)'),
(37953, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Just Summoned - Boss Emote Line 2 (use the talisman)'),
(37953, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 18, 898, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Just Summoned - Set Unit Flags (non attackable, immune)'),
(37953, 0, 4, 0, 60, 0, 100, 1, 3500, 3500, 0, 0, 0, 11, 70796, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - 3.5 s after summon - Cast Aimed Shot on summoner'),
(37953, 0, 5, 6, 8, 0, 100, 1, 70797, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Spellhit Belysra''s Talisman - Say Line 0 (How did you--?!)'),
(37953, 0, 6, 7, 61, 0, 100, 0, 0, 0, 0, 0, 0, 92, 0, 70796, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Spellhit Belysra''s Talisman - Interrupt Aimed Shot'),
(37953, 0, 7, 8, 61, 0, 100, 0, 0, 0, 0, 0, 0, 19, 898, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Spellhit Belysra''s Talisman - Remove Unit Flags'),
(37953, 0, 8, 9, 61, 0, 100, 0, 0, 0, 0, 0, 0, 28, 70794, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Spellhit Belysra''s Talisman - Free summoner from Freezing Trap'),
(37953, 0, 9, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 49, 0, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Spellhit Belysra''s Talisman - Attack summoner'),
(37953, 0, 10, 0, 7, 0, 100, 0, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Scout - On Evade - Despawn');

DELETE FROM `conditions` WHERE (`SourceTypeOrReferenceId` = 22 AND `SourceEntry` = -219500) OR (`SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 70797);

INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, -219500, 0, 0, 9, 0, 24616, 0, 0, 0, 0, 0, '', 'Losing Your Tail trap - invoker has quest 24616 taken'),
(22, 1, -219500, 0, 0, 29, 0, 37953, 30, 0, 1, 0, 0, '', 'Losing Your Tail trap - no Dark Scout within 30 yd of the invoker'),
(13, 2, 70797, 0, 0, 31, 0, 3, 37953, 0, 0, 0, 0, '', 'Belysra''s Talisman effect 1 - targets Dark Scout');

DELETE FROM `creature_text` WHERE (`CreatureID`=37953 OR `entry`=37953) AND `GroupID` IN (1, 2);

INSERT INTO `creature_text` (`entry`,`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(37953,37953,1,0,'Such easy prey.  Sylvanas will be most pleased!',12,0,100,0,0,0,0,37672,0,'Dark Scout - ambush'),
(37953,37953,2,0,'Use Belrysa''s Talisman to escape before the Dark Scout shoots you!',41,0,100,0,0,0,0,37674,0,'Dark Scout - talisman hint');

-- Source sql/updates/world/2026_10_04_36_world_worgen_watchmen_survivors.sql; SHA256(LF) 119ff4efe39c98f52f2e0f31253e163be0222fc1494335cb9449ab6fa60a6839

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_drowning_watchman' WHERE `entry` = 36440;

DELETE FROM `smart_scripts` WHERE (`entryorguid` = 36440 AND `source_type` = 0) OR (`entryorguid` = 3644000 AND `source_type` = 9);

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_crash_survivor' WHERE `entry` = 37067;

DELETE FROM `creature` WHERE `id` = 37078 AND `guid` IN (221486, 221848, 221852, 221854, 221868, 221870, 221882, 221919, 221920, 221923, 221924);

DELETE FROM `creature_addon` WHERE `guid` IN (221486, 221848, 221852, 221854, 221868, 221870, 221882, 221919, 221920, 221923, 221924);

DELETE FROM `creature_text` WHERE (`CreatureID`=37067 OR `entry`=37067);

INSERT INTO `creature_text` (`entry`,`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(37067,37067,0,0,'You got here just in time.  Thanks!',12,0,100,0,0,0,0,36909,0,'Crash Survivor - saved'),
(37067,37067,0,1,'I wasn''t ready to die in a marsh.  Thanks again!',12,0,100,0,0,0,0,36910,0,'Crash Survivor - saved'),
(37067,37067,0,2,'Time to run!',12,0,100,0,0,0,0,36911,0,'Crash Survivor - saved'),
(37067,37067,0,3,'Let''s get out of this marsh!',12,0,100,0,0,0,0,36912,0,'Crash Survivor - saved');

-- Source sql/updates/world/2026_10_04_37_world_worgen_hewell_horse.sql; SHA256(LF) 6a58af61de28ed45409a3c0ed78505fa9ae85dad6d4a817149f15fd079cefa53

DELETE FROM `smart_scripts` WHERE `entryorguid` = 37876 AND `source_type` = 0 AND `id` = 0 AND `event_type` = 19 AND `event_param1` = 24677;

UPDATE `gossip_menu_option` SET `OptionNpcFlag` = 1 WHERE `MenuID` = 11079 AND `OptionID` = 0;

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 38764 AND `ScriptName` = '';

UPDATE `smart_scripts` SET `event_param2` = 38765 WHERE `entryorguid` = 38765 AND `source_type` = 0 AND `id` IN (1, 2, 3, 5, 6) AND `event_type` = 40;

DELETE FROM `smart_scripts` WHERE (`entryorguid` = 38764 AND `source_type` = 0) OR (`entryorguid` = 38765 AND `source_type` = 0 AND `id` IN (7, 8, 9));

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38764, 0, 0, 1, 62, 0, 100, 0, 11079, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Hewell - On Gossip Select - Close Gossip'),
(38764, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 85, 72772, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Lord Hewell - On Gossip Select - Invoker Cast Summon Stout Mountain Horse'),
(38765, 0, 7, 0, 27, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 387650, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Stout Mountain Horse - On Passenger Boarded at Lord Hewell - Start WP 387650 (24677)'),
(38765, 0, 8, 9, 40, 0, 100, 0, 76, 387650, 0, 0, 0, 28, 46598, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Stout Mountain Horse - On WP 76 of path 387650 (Livery Outpost) - Remove Passenger'),
(38765, 0, 9, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Stout Mountain Horse - On WP 76 of path 387650 - Despawn');

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

-- Source sql/updates/world/2026_10_04_38_world_worgen_battle_for_gilneas_city.sql; SHA256(LF) abdf0cd1eb93c0ff8dca9326cf2671ad9629ba7f440af0e9f166412724649ac2

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_krennan_aranas' WHERE `entry` = 38553;

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_prince_liam_greymane_gilneas' WHERE `entry` = 38218;

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_lady_sylvanas_gilneas' WHERE `entry` = 38469;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 37783 AND `source_type` = 0;

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(37783, 0, 0, 0, 19, 0, 100, 0, 24904, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 38553, 60, 0, 0, 0, 0, 0, 0, 'Lorna Crowley - On Quest 24904 Accept - Krennan Aranas yells "It''s time to join the fray"');

DELETE FROM `creature_text` WHERE ((`CreatureID`=38218 OR `entry`=38218) AND `GroupID` IN (12, 13, 14)) OR ((`CreatureID`=38469 OR `entry`=38469) AND `GroupID` = 2);

INSERT INTO `creature_text` (`entry`,`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(38218,38218,12,0,'FATHER!!!',14,0,100,0,0,19612,0,38369,0,'Prince Liam Greymane - battle finale'),
(38218,38218,13,0,'We did it, father...',12,0,100,0,0,0,0,38371,0,'Prince Liam Greymane - battle finale'),
(38218,38218,14,0,'We took back our city... we took back...',12,0,100,0,0,0,0,38372,0,'Prince Liam Greymane - battle finale'),
(38469,38469,2,0,'Such a waste.  That arrow''s poison was not meant to be wasted on your whelp.  We''ll meet again, Genn!',12,0,100,0,0,0,0,38370,0,'Lady Sylvanas Windrunner - battle finale');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 72116;

INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 72116, 0, 0, 31, 0, 3, 38218, 0, 0, 0, 0, '', 'Shoot Liam - targets Prince Liam Greymane (battle)');

-- Source sql/updates/world/2026_10_04_39_world_worgen_endgame_hippogryph.sql; SHA256(LF) 2d34c6103f0f5717899b04dc1e70d9cb1cbf1c0ca46e99e1957a187115aa0d40

UPDATE `creature_template` SET `npcflag` = `npcflag` | 1, `AIName` = '', `ScriptName` = 'npc_hippogryph_endgame' WHERE `entry` = 43747;

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_hippogryph' WHERE `entry` = 43751;

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_tobias_mistmantle_endgame' WHERE `entry` = 43749;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 43727 AND `source_type` = 0;

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(43727, 0, 0, 0, 19, 0, 100, 0, 26706, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 43749, 60, 0, 0, 0, 0, 0, 0, 'Lorna Crowley - On Quest 26706 Accept - Tobias Mistmantle says a waiting line');

DELETE FROM `creature_text` WHERE (`CreatureID`=43749 OR `entry`=43749);

INSERT INTO `creature_text` (`entry`,`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(43749,43749,0,0,'We must wait for the right time to strike!',12,0,100,0,0,0,0,43815,0,'Tobias Mistmantle - Endgame waiting'),
(43749,43749,0,1,'We must bide our time.  They won''t know what hit them.',12,0,100,0,0,0,0,43816,0,'Tobias Mistmantle - Endgame waiting'),
(43749,43749,0,2,'We''ll set flight once everyone is ready.',12,0,100,0,0,0,0,43837,0,'Tobias Mistmantle - Endgame waiting'),
(43749,43749,0,3,'Patience, friends.  We will strike soon.',12,0,100,0,0,0,0,43838,0,'Tobias Mistmantle - Endgame waiting'),
(43749,43749,0,4,'Hold your horses, friend.  We''ll hit them when we''re all ready.',12,0,100,0,0,0,0,43839,0,'Tobias Mistmantle - Endgame waiting'),
(43749,43749,0,5,'Let''s give everyone time to gather.  We''ll need a handful of people to come with us.',12,0,100,0,0,0,0,43840,0,'Tobias Mistmantle - Endgame waiting'),
(43749,43749,1,0,'Hop on a hippogryph, now''s our chance!',12,0,100,0,0,0,0,43814,0,'Tobias Mistmantle - Endgame signal'),
(43749,43749,2,0,'Time to go!',12,0,100,0,0,0,0,43813,0,'Tobias Mistmantle - Endgame signal');

-- Source sql/updates/world/2026_10_04_40_world_worgen_losing_your_tail_once.sql; SHA256(LF) b0312aa81506720daa0a9ed706016fcba649162f97ababa6c60787a3f2754567

UPDATE `smart_scripts` SET `event_param3` = 5000, `event_param4` = 5000 WHERE `entryorguid` = -219500 AND `source_type` = 0 AND `id` = 0;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = -219500 AND `SourceGroup` = 1 AND `ConditionTypeOrReference` = 1;

INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, -219500, 0, 0, 1, 0, 70794, 0, 0, 1, 0, 0, '', 'Losing Your Tail trap - invoker not already trapped (Freezing Trap Effect)');

-- Source sql/updates/world/2026_10_04_41_world_worgen_hunt_for_sylvanas_tobias_passive.sql; SHA256(LF) 50957a424f5ed7e4f159d0d2bae4ca4b6f2c8aaf3138b416e9932e2c5362209e

DELETE FROM `smart_scripts` WHERE `entryorguid` = 38507 AND `source_type` = 0 AND `id` IN (19, 20);

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38507, 0, 19, 20, 54, 0, 100, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - On Just Summoned - Set Passive'),
(38507, 0, 20, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 18, 768, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - On Just Summoned - Immune to PC and NPC (he sneaks, nothing may engage him)');

-- Source sql/updates/world/2026_10_04_42_world_worgen_stout_mountain_horse_ai.sql; SHA256(LF) b7d8f35f534f750334df2bcd38b90f36103b2fe6f27479403ce139692b5fae12

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_stout_mountain_horse' WHERE `entry` = 38765;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 38765 AND `source_type` = 0;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 38765;

-- Source sql/updates/world/2026_10_04_43_world_worgen_liam_phase2.sql; SHA256(LF) d522885e77f570f8c2a8a9c1d44dcd1680a23dea47d4d9f7a42d8f6745c6a673

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_prince_liam_greymane_phase2' WHERE `entry` = 34913;

-- Source sql/updates/world/2026_10_09_10_world_gilneas_lorna_post_immune_npc.sql; SHA256(LF) bf307ebfec139ff0e38880944970c0f87ac10029703224d787293c23aec1a0d7

UPDATE `creature` SET `unit_flags` = 32768 | 512 WHERE `guid` = 220832 AND `id` = 38611;

UPDATE `creature` SET `unit_flags` = 32768 | 512 WHERE `guid` IN (220885, 221855) AND `id` = 38613;

-- Source sql/updates/world/2026_10_09_11_world_gilneas_tobias_immune_npc.sql; SHA256(LF) 7b0a975a718f7e838b0fcb6f2a80a22eff13cd64da02e474c615f51a29a67a9f

UPDATE `creature_template` SET `unit_flags` = 32768 | 256 | 512 WHERE `entry` = 38507;

-- Source sql/updates/world/2026_10_09_13_world_gilneas_invasion_camera.sql; SHA256(LF) ed0aeebcf921e71496bd8f20113e8fc8e76855ffd9a21e917c69849991ffa849

UPDATE `gameobject` SET `phaseMask` = 1 | 4096 WHERE `guid` = 166866 AND `id` = 197337;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceGroup` = 2 AND `SourceEntry` = 69296;

INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 2, 69296, 0, 0, 31, 0, 5, 197337, 0, 0, 0, 0, '', 'Invasion Camera effect 1 - targets Gilneas Invasion Camera 197337');

-- Source sql/updates/world/2026_10_09_14_world_gilneas_endgame_gunship.sql; SHA256(LF) 300b6d2d87d919bd660a2bd7e2706c97c9736ae8af8c0fc8c96b5a0886e44dbf

DELETE FROM `gameobject_template` WHERE `entry` = 800100;

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `data0`, `data1`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(800100, 5, 8253, 'Orc Gunship', '', '', '', 1, 0, 0, '', '', 0);

DELETE FROM `gameobject` WHERE `guid` BETWEEN 4100001 AND 4100004;

INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`) VALUES
(4100001, 800100, 654, 4714, 4714, 1, 4194304, -1465.76, 3251.48, 93.84, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1),


(4100002, 204458, 654, 4714, 4714, 1, 4194304, -1494.8164, 3296.5706, 104.2867, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1),
(4100003, 204458, 654, 4714, 4714, 1, 4194304, -1493.0065, 3297.7837, 104.2948, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1),
(4100004, 204458, 654, 4714, 4714, 1, 4194304, -1496.3229, 3294.9650, 104.2769, 2.14335, 0, 0, 0.878003, 0.478655, 7200, 255, 1);

UPDATE `gameobject` SET `phaseMask` = 4194304 WHERE `guid` IN (166908, 166909) AND `id` = 204428;

UPDATE `creature` SET `phaseMask` = 4194304, `MovementType` = 0, `wander_distance` = 0 WHERE `map` = 654 AND `guid` IN (
  223311, 223312, 223313, 223314, 223315, 223316, 223317, 223318, 223319, 223320, 223321,
  223322, 223323, 223324, 223325, 223326, 223327, 223328, 223329, 223330, 223331, 223332,

  223334,

  223333, 223337, 223338, 223339, 223340,

  223307, 223308, 223335, 223336,

  223309, 223310,

  220393, 220394, 220395,

  220396, 220397, 220398, 220399, 220400, 220401, 220402, 220418, 220604, 222952,
  223170, 223303, 223304, 223305, 223306);

UPDATE `creature` SET `spawntimesecs` = 120 WHERE `map` = 654 AND `id` = 42141 AND `guid` BETWEEN 223311 AND 223332;

UPDATE `creature` SET `phaseMask` = 4194304, `position_x` = -1436.3203, `position_y` = 3229.6905, `position_z` = 105.4304,
  `orientation` = 3.18799, `MovementType` = 0, `wander_distance` = 0, `spawntimesecs` = 120 WHERE `guid` = 223342 AND `id` = 43567;

UPDATE `creature_template` SET `ScriptName` = 'npc_lorna_crowley_endgame' WHERE `entry` = 43566;

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_wyvern_endgame' WHERE `entry` = 43713;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 43713 AND `source_type` = 0;

DELETE FROM `creature_text` WHERE ((`CreatureID`=43566 OR `entry`=43566) AND `GroupID` = 7) OR ((`CreatureID`=43767 OR `entry`=43767) AND `GroupID` = 0);

INSERT INTO `creature_text` (`entry`,`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(43566,43566,7,0,'Attack!',14,0,100,0,0,0,0,0,0,'Lorna Crowley - gunship boarding'),
(43767,43767,0,0,'Dabu?',12,1,100,0,0,0,0,0,0,'Navigator - Hands up, greenskins');

-- Source sql/updates/world/2026_10_10_00_world_gilneas_tobias_phase.sql; SHA256(LF) e48753ce95e0e62a2b55391d2ab923f05e910d5e4cc00aba6143a2a05f739c66

UPDATE `smart_scripts` SET `link` = 21 WHERE `entryorguid` = 38507 AND `source_type` = 0 AND `id` = 20;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 38507 AND `source_type` = 0 AND `id` = 21;

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38507, 0, 21, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 44, 2097152, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - On Just Summoned - Phase 2097152 only (out of the Greymane Court 0x40000 spawns)');
