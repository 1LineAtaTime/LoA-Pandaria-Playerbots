-- Eastern Plaguelands, Fiona's Caravan: the third ride, Eastwall Tower to Northpass Tower (Nobody to Blame but Myself 27489).
--
--  * Before: the quest could not be completed and the main chain stopped there (27522, 27524-27527 hang on it): the wagon at
--    Eastwall had no ride option, the harness 45782 and wagon 45783 had no vehicle, script or accessories, the riders
--    46199-46201 no lines, and there was no road for the ride.
--  * Now, built as the first two rides (2026_11_14_10): option 2 of menu 12035 while 27489 is in the log; the player casts
--    85393, whose harness drives him from Eastwall to Northpass with Fiona driving, Argus beside her and Tarenar and the player
--    behind, while they speak the 22 lines; on arrival he is credited 45400 and set down by the caravan parked at Northpass.
--    While he rides, the Eastwall stop (49416: wagon, Fiona, Argus, and Rimblat through his chained row) and Tarenar (60922)
--    are hidden from him; Vex'tul, who does not ride, stays.
--  * Sourced: menu 12035 option 2 "Ride Fiona's Caravan to its next destination." (broadcast 45675, both TDBs); 85393 and its
--    spell_target_position (in mop); harness 45782 and wagon 45783 as the first two rides' 45436 / 45542 and 45423 / 45437
--    (same models and fields in mop); the seating (wiki 27489: "Fiona and Argus Highbeacon sitting at the front and Tarenar
--    Sunstrike and the hero sitting at the back"); the 22 lines, their speakers and say / yell (wiki 27489; broadcast_text
--    46296-46317); the credit 45400 (objective of 27489); the end of the ride by the Northpass parking spot of both TDBs
--    (3163.43,-4327.63; it stops 29 yd short of it, as the first two rides stop 24-26 yd short of theirs).
--  * The road, ours: no sniff of the caravan's path exists. From the Eastwall spot down the west slope of the tower's hill
--    (the gentlest grade on the map's terrain) to the road at 2505.8,-4656.6; then north along the road as a sniffed road
--    patrol of TDB 12.1.0 walks it (path 7383040, nodes 23-21, 19-1 and 262-265: 26 points with their heights) to the
--    crossroads at 2849,-4406; from there east to Northpass the way the server's own mmaps lead (Detour) and up the tower's
--    hill on its gentlest side, every 8 yd at the terrain height + 0.1. 80 points, 884 yd. speed_run 1 (7 yd/s, ours) makes
--    the ride about 128 s, so the lines (one every 6 s from 4 s) end as it arrives.

UPDATE `creature_template` SET `VehicleId`=958, `AIName`='', `ScriptName`='npc_epl_caravan_harness', `speed_run`=1 WHERE `entry`=45782;
UPDATE `creature_template` SET `VehicleId`=959, `AIName`='', `ScriptName`='' WHERE `entry`=45783;

DELETE FROM `vehicle_template_accessory` WHERE `entry` IN (45782,45783);
INSERT INTO `vehicle_template_accessory` (`entry`,`accessory_entry`,`seat_id`,`minion`,`description`,`summontype`,`summontimer`) VALUES
(45782,45433,0,1,'Fiona''s Caravan Harness - Preston',8,0),
(45782,45434,1,1,'Fiona''s Caravan Harness - Argyle',8,0),
(45782,45783,2,1,'Fiona''s Caravan Harness - Fiona''s Caravan',8,0),
(45783,46201,0,1,'Fiona''s Caravan - Fiona',8,0),
(45783,46200,1,1,'Fiona''s Caravan - Argus Highbeacon',8,0),
(45783,46199,2,1,'Fiona''s Caravan - Tarenar Sunstrike',8,0);

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (45782,45783);
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`cast_flags`,`user_type`) VALUES
(45782,84896,1,0),
(45783,46598,1,0);

DELETE FROM `waypoints` WHERE `entry`=45782;
INSERT INTO `waypoints` (`entry`,`pointid`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`point_comment`) VALUES
(45782,1,2525.09,-4765.95,102.68,NULL,0,''),
(45782,2,2516.0,-4764.0,101.63,NULL,0,''),
(45782,3,2511.0,-4756.0,99.32,NULL,0,''),
(45782,4,2506.0,-4748.0,95.88,NULL,0,''),
(45782,5,2505.9,-4740.4,93.85,NULL,0,''),
(45782,6,2505.8,-4732.8,92.8,NULL,0,''),
(45782,7,2505.7,-4725.2,91.64,NULL,0,''),
(45782,8,2505.6,-4717.6,89.84,NULL,0,''),
(45782,9,2505.5,-4710.0,87.38,NULL,0,''),
(45782,10,2505.4,-4702.4,84.49,NULL,0,''),
(45782,11,2505.3,-4694.8,82.24,NULL,0,''),
(45782,12,2505.2,-4687.2,80.56,NULL,0,''),
(45782,13,2505.1,-4679.6,78.21,NULL,0,''),
(45782,14,2505.0,-4672.0,76.43,NULL,0,''),
(45782,15,2505.79,-4656.55,75.7666,NULL,0,''),
(45782,16,2528.11,-4655.13,76.8887,NULL,0,''),
(45782,17,2541.99,-4653.26,77.6674,NULL,0,''),
(45782,18,2555.86,-4651.34,78.5097,NULL,0,''),
(45782,19,2569.34,-4643.39,79.6035,NULL,0,''),
(45782,20,2581.34,-4636.17,80.6435,NULL,0,''),
(45782,21,2605.27,-4621.64,83.0451,NULL,0,''),
(45782,22,2617.23,-4614.37,84.1513,NULL,0,''),
(45782,23,2629.2,-4607.1,84.5638,NULL,0,''),
(45782,24,2641.25,-4599.98,84.6561,NULL,0,''),
(45782,25,2653.44,-4593.09,84.9497,NULL,0,''),
(45782,26,2665.6,-4586.15,85.6021,NULL,0,''),
(45782,27,2677.69,-4579.1,86.5389,NULL,0,''),
(45782,28,2701.68,-4564.66,87.4361,NULL,0,''),
(45782,29,2725.94,-4550.69,88.0814,NULL,0,''),
(45782,30,2737.98,-4543.55,88.4874,NULL,0,''),
(45782,31,2750.62,-4533.67,88.8859,NULL,0,''),
(45782,32,2770.79,-4514.28,89.4751,NULL,0,''),
(45782,33,2780.41,-4504.1,89.6763,NULL,0,''),
(45782,34,2789.95,-4493.86,89.8303,NULL,0,''),
(45782,35,2801.03,-4481.72,89.9453,NULL,0,''),
(45782,36,2806.6,-4475.69,89.9669,NULL,0,''),
(45782,37,2828.65,-4451.18,89.8812,NULL,0,''),
(45782,38,2836.19,-4439.39,89.7144,NULL,0,''),
(45782,39,2843.55,-4427.48,89.5383,NULL,0,''),
(45782,40,2851.53,-4414.44,89.354,NULL,0,''),
(45782,41,2859.61,-4413.54,89.68,NULL,0,''),
(45782,42,2867.69,-4412.64,89.97,NULL,0,''),
(45782,43,2875.78,-4411.74,90.36,NULL,0,''),
(45782,44,2883.86,-4410.85,90.86,NULL,0,''),
(45782,45,2891.94,-4409.95,91.18,NULL,0,''),
(45782,46,2900.02,-4409.05,91.65,NULL,0,''),
(45782,47,2908.1,-4408.15,91.99,NULL,0,''),
(45782,48,2916.18,-4407.25,92.58,NULL,0,''),
(45782,49,2924.27,-4406.35,93.14,NULL,0,''),
(45782,50,2932.35,-4405.45,93.55,NULL,0,''),
(45782,51,2940.43,-4404.56,94.08,NULL,0,''),
(45782,52,2948.51,-4403.66,94.61,NULL,0,''),
(45782,53,2956.59,-4402.76,95.55,NULL,0,''),
(45782,54,2964.68,-4401.86,96.83,NULL,0,''),
(45782,55,2972.76,-4400.96,97.2,NULL,0,''),
(45782,56,2980.84,-4400.06,96.83,NULL,0,''),
(45782,57,2988.92,-4399.16,96.76,NULL,0,''),
(45782,58,2997.0,-4398.26,97.41,NULL,0,''),
(45782,59,3005.08,-4397.37,97.9,NULL,0,''),
(45782,60,3013.17,-4396.47,98.48,NULL,0,''),
(45782,61,3021.25,-4395.57,98.69,NULL,0,''),
(45782,62,3029.33,-4394.67,99.02,NULL,0,''),
(45782,63,3032.89,-4387.11,99.45,NULL,0,''),
(45782,64,3036.44,-4379.56,99.73,NULL,0,''),
(45782,65,3040.0,-4372.0,100.03,NULL,0,''),
(45782,66,3047.73,-4371.82,102.02,NULL,0,''),
(45782,67,3055.45,-4371.64,104.07,NULL,0,''),
(45782,68,3063.18,-4371.45,106.52,NULL,0,''),
(45782,69,3070.91,-4371.27,108.36,NULL,0,''),
(45782,70,3078.64,-4371.09,110.17,NULL,0,''),
(45782,71,3086.36,-4370.91,114.38,NULL,0,''),
(45782,72,3094.09,-4370.73,121.31,NULL,0,''),
(45782,73,3101.82,-4370.55,122.76,NULL,0,''),
(45782,74,3109.55,-4370.36,127.15,NULL,0,''),
(45782,75,3117.27,-4370.18,132.15,NULL,0,''),
(45782,76,3125.0,-4370.0,133.6,NULL,0,''),
(45782,77,3129.12,-4363.38,133.6,NULL,0,''),
(45782,78,3133.25,-4356.75,133.6,NULL,0,''),
(45782,79,3137.38,-4350.12,133.37,NULL,0,''),
(45782,80,3141.5,-4343.5,133.29,NULL,0,'');

DELETE FROM `creature_text` WHERE `CreatureID` IN (46199,46200,46201);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(46199,0,0,'I can''t believe I let Gidwin run off like that!',14,0,100,0,0,0,0,46296,0,'Tarenar Sunstrike - Nobody to Blame but Myself ride, line 1'),
(46201,0,0,'It''s not your fault, Tarenar.  He ran off on his own will.  None of us could stop him.',12,0,100,0,0,0,0,46297,0,'Fiona - Nobody to Blame but Myself ride, line 2'),
(46200,0,0,'We can''t be certain that he was killed.  Maybe he just ran off somewhere.',12,0,100,0,0,0,0,46298,0,'Argus Highbeacon - Nobody to Blame but Myself ride, line 3'),
(46199,1,0,'No, he''d never just leave his prayer book behind!  He was always nose-deep in that book, studying.',14,0,100,0,0,0,0,46299,0,'Tarenar Sunstrike - Nobody to Blame but Myself ride, line 4'),
(46199,2,0,'He was always studying, always trying to get better.',12,0,100,0,0,0,0,46300,0,'Tarenar Sunstrike - Nobody to Blame but Myself ride, line 5'),
(46200,1,0,'Even more reason to believe that he''s still alive.',12,0,100,0,0,0,0,46301,0,'Argus Highbeacon - Nobody to Blame but Myself ride, line 6'),
(46200,2,0,'A paladin like Gidwin makes a fine death knight.  Any member of the Scourge powerful enough to defeat him would know that.',12,0,100,0,0,0,0,46302,0,'Argus Highbeacon - Nobody to Blame but Myself ride, line 7'),
(46200,3,0,'It''s likely that he''s being held somewhere, awaiting a death ritual.',12,0,100,0,0,0,0,46303,0,'Argus Highbeacon - Nobody to Blame but Myself ride, line 8'),
(46201,1,0,'Really?  That''s great news!',14,0,100,0,0,0,0,46304,0,'Fiona - Nobody to Blame but Myself ride, line 9'),
(46201,2,0,'Well, actually, that''s pretty bad news, but considering the alternatives... ',12,0,100,0,0,0,0,46305,0,'Fiona - Nobody to Blame but Myself ride, line 10'),
(46200,4,0,'If we''re to find him, we''re headed in the right direction.  It''s likely that he was taken to Stratholme, or potentially to the Plaguewood.',12,0,100,0,0,0,0,46306,0,'Argus Highbeacon - Nobody to Blame but Myself ride, line 11'),
(46200,5,0,'Both are places of great evil.  This is the leg of my journey that I shun the most.',12,0,100,0,0,0,0,46307,0,'Argus Highbeacon - Nobody to Blame but Myself ride, line 12'),
(46199,3,0,'I''m not sure I''m ready to go in there alone.',12,0,100,0,0,0,0,46308,0,'Tarenar Sunstrike - Nobody to Blame but Myself ride, line 13'),
(46201,3,0,'You''re not alone, Tarenar.  You''ve made allies along the way.  And besides, you''re a skilled paladin in your own right.',12,0,100,0,0,0,0,46309,0,'Fiona - Nobody to Blame but Myself ride, line 14'),
(46199,4,0,'Me, skilled?  No, no, Gidwin was the good paladin.  He''s the one who stayed up for hours every night practicing... ',12,0,100,0,0,0,0,46310,0,'Tarenar Sunstrike - Nobody to Blame but Myself ride, line 15'),
(46201,4,0,'...yet it was you that continually kept up with him.  Without practice.',12,0,100,0,0,0,0,46311,0,'Fiona - Nobody to Blame but Myself ride, line 16'),
(46201,5,0,'I''ve watched both of you fight, Tarenar.  You''ve got natural talent - more than Gidwin ever will.',12,0,100,0,0,0,0,46312,0,'Fiona - Nobody to Blame but Myself ride, line 17'),
(46200,6,0,'Perhaps that''s why he left without you.',12,0,100,0,0,0,0,46313,0,'Argus Highbeacon - Nobody to Blame but Myself ride, line 18'),
(46199,5,0,'What do you mean?',12,0,100,0,0,0,0,46314,0,'Tarenar Sunstrike - Nobody to Blame but Myself ride, line 19'),
(46200,7,0,'Perhaps he felt he needed to prove that he was as talented as you.',12,0,100,0,0,0,0,46315,0,'Argus Highbeacon - Nobody to Blame but Myself ride, line 20'),
(46199,6,0,'This is all my fault.  I''ve got to go find him, to set things right.',12,0,100,0,0,0,0,46316,0,'Tarenar Sunstrike - Nobody to Blame but Myself ride, line 21'),
(46200,8,0,'I wouldn''t say that it''s your fault, Tarenar.  But don''t let that stop you from saving Gidwin.',12,0,100,0,0,0,0,46317,0,'Argus Highbeacon - Nobody to Blame but Myself ride, line 22');

DELETE FROM `gossip_menu_option` WHERE `MenuID`=12035 AND `OptionID`=2;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(12035,2,0,'Ride Fiona''s Caravan to its next destination.',45675,1,1,0,0,0,0,NULL,0,0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=12035 AND `SourceEntry`=2;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,12035,2,0,0,9,0,27489,0,0,0,0,0,'','Fiona''s Caravan: ride to Northpass while Nobody to Blame but Myself is in the log');
