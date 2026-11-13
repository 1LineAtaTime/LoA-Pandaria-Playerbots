-- Eastern Plaguelands, Ix'lar the Underlord (27487): he walks his loop through Ix'lar's Domain and yells on the pull.
-- Sourced: the loop (TDB 4.4.2 path 25596000 of the same spawn, 27 points, its start point and facing 1.4); the yell
--   (broadcast_text 50350; Wowhead comment 2011-11-21 [+10] quotes it on aggro; "yells" is the comment's word).
-- Ours: SheathState 1 on the addon (the core default).
-- Chain (wiki infoboxes: 27487 "previous = Argent Call: Northdale" 27466; 27488 "previous = Ix'lar the Underlord"; neither
--   TDB has a row): Ix'lar after Argent Call: Northdale, Impatience after Ix'lar, so Gidwin's prayer book in Northdale is
--   searched only once he has run off. (A Wowhead reply had both offered together at Eastwall; the wiki order is the story's.)
UPDATE `creature` SET `position_x`=2845.47,`position_y`=-4767.62,`position_z`=84.6052,`orientation`=1.4,`MovementType`=2,`wander_distance`=0 WHERE `guid`=175234 AND `id`=45744;
DELETE FROM `creature_addon` WHERE `guid`=175234;
INSERT INTO `creature_addon` (`guid`,`path_id`,`SheathState`) VALUES (175234,175234,1);
DELETE FROM `waypoint_data` WHERE `id`=175234;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`) VALUES
(175234,1,2845.47,-4767.62,84.6052),(175234,2,2854.41,-4778.39,91.9647),(175234,3,2862.23,-4789.15,95.0908),(175234,4,2856.74,-4803.69,94.6798),
(175234,5,2842.84,-4805.25,97.0197),(175234,6,2833.87,-4790.56,96.8413),(175234,7,2825.65,-4788.4,99.6292),(175234,8,2812.69,-4791.82,95.803),
(175234,9,2805.72,-4798.94,90.9042),(175234,10,2795.53,-4802.25,85.2129),(175234,11,2783.56,-4801.21,83.1404),(175234,12,2768.65,-4800.15,79.3376),
(175234,13,2755.91,-4794.9,76.572),(175234,14,2747,-4777.48,76.4895),(175234,15,2748.14,-4746.14,76.9713),(175234,16,2747.03,-4722.73,74.8358),
(175234,17,2750.93,-4698.12,78.9586),(175234,18,2760.24,-4670.02,80.0269),(175234,19,2770.4,-4660.86,86.9222),(175234,20,2783.42,-4664.73,92.6719),
(175234,21,2797.51,-4678.94,95.491),(175234,22,2809.22,-4682.13,95.9214),(175234,23,2826.65,-4680.09,90.4176),(175234,24,2838.29,-4692.48,93.1768),
(175234,25,2853.44,-4706.6,89.9358),(175234,26,2861.5,-4729.15,82.2275),(175234,27,2855.89,-4746.56,81.9898);

DELETE FROM `creature_text` WHERE `CreatureID`=45744;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(45744,0,0,'The Scourge will rise again, under a dark new master... Ix''lar!',14,0,100,0,0,0,0,50350,0,'Ix''lar the Underlord - aggro');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=45744;
DELETE FROM `smart_scripts` WHERE `entryorguid`=45744 AND `source_type`=0 AND `id`=3;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45744,0,3,0,4,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Ix''lar the Underlord - On Aggro - Say Line 0');

INSERT INTO `quest_template_addon` (`ID`,`PrevQuestID`) VALUES (27487,27466) ON DUPLICATE KEY UPDATE `PrevQuestID`=27466;
UPDATE `quest_template_addon` SET `PrevQuestID`=27487 WHERE `ID`=27488;
