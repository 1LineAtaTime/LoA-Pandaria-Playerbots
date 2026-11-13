-- ported from PR 15 2026_10_28_41_world.sql (#986)
-- Hillsbrad Foothills, Azurelode Mine and the Sludge Fields: make the quests completable.

-- Hillsbrad Foothills, Azurelode Mine: restore the Hillsbrad Miners, Infested Bears and Domesticated Mine Creepers that
-- mop_world lacks (it has 2 of 38, 16 of 35 and 1 of 10 of the Cataclysm Classic 4.4.2 sniff). Only 4.4.2 spawns with no spawn of
-- the same entry within 15 yd are ported. Respawn/wander follow the existing mop spawns of each entry. Guids 3100000-3100063.
DELETE FROM `creature` WHERE `guid` BETWEEN 3100000 AND 3100063;
DELETE FROM `creature_addon` WHERE `guid` BETWEEN 3100000 AND 3100063;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3100000,2269,0,267,288,1,1,0,0,0,0,-869.394,61.9253,2.83463,0.506145,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323441
(3100001,2269,0,267,288,1,1,0,0,0,0,-860.563,79.809,3.95971,5.11381,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323444
(3100002,2269,0,267,288,1,1,0,0,0,0,-848.665,55.4618,3.89097,1.29154,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323445
(3100003,2269,0,267,288,1,1,0,0,0,0,-838.149,68.467,1.84621,3.54302,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323449
(3100004,2269,0,267,267,1,1,0,0,0,0,-784.233,35.8698,14.3469,0.418879,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323460
(3100005,2269,0,267,267,1,1,0,0,0,0,-710.427,29.842,12.8298,1.78024,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323646
(3100006,2269,0,267,267,1,1,0,0,0,0,-765.575,42.9045,14.079,2.93215,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323651
(3100007,2269,0,267,288,1,1,0,0,0,0,-840.58,79.342,3.71694,3.35103,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323657
(3100008,2269,0,267,288,1,1,0,0,0,0,-846.042,100.189,4.02171,0.0349066,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323694
(3100009,2269,0,267,288,1,1,0,0,0,0,-825.512,142.002,17.6065,5.79449,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323726
(3100010,2269,0,267,288,1,1,0,0,0,0,-806.137,116.134,16.9954,0.994838,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323729
(3100011,2269,0,267,288,1,1,0,0,0,0,-804.7,102.198,6.67607,4.06662,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323740
(3100012,2269,0,267,288,1,1,0,0,0,0,-820.924,156.135,17.5591,4.29351,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323742
(3100013,2269,0,267,288,1,1,0,0,0,0,-822.863,140.314,2.78693,3.01942,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323743
(3100014,2269,0,267,288,1,1,0,0,0,0,-786.471,92.4722,8.23385,3.01942,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323751
(3100015,2269,0,267,288,1,1,0,0,0,0,-775.061,81.9653,11.1969,4.64258,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323756
(3100016,2269,0,267,288,1,1,0,0,0,0,-773.184,117.194,16.2855,4.57276,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323758
(3100017,2269,0,267,267,1,1,0,0,0,0,-753.759,63.1649,12.3501,1.76278,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323761
(3100018,2269,0,267,267,1,1,0,0,0,0,-734.363,42.2899,11.6095,0.191986,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323763
(3100019,2269,0,267,288,1,1,0,0,0,0,-750.53,92.1615,12.4376,5.84685,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323765
(3100020,2269,0,267,267,1,1,0,0,0,0,-732.8,67.2344,10.0565,3.78736,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323766
(3100021,2269,0,267,288,1,1,0,0,0,0,-751.104,106.233,14.5482,1.309,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323770
(3100022,2269,0,267,267,1,1,0,0,0,0,-717.696,58.967,9.69042,5.044,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323773
(3100023,2269,0,267,288,1,1,0,0,0,0,-733.271,95.0087,13.9253,2.28638,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323774
(3100024,2269,0,267,267,1,1,0,0,0,0,-704.375,50.0903,12.3267,3.735,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323775
(3100025,2269,0,267,288,1,1,0,0,0,0,-793.781,147.762,2.4743,3.03687,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323930
(3100026,2269,0,267,288,1,1,0,0,0,0,-818.564,164.776,2.56275,2.98451,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 323932
(3100027,2269,0,267,288,1,1,0,0,0,0,-830.035,186.309,20.8517,1.78024,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 324003
(3100028,2269,0,267,288,1,1,0,0,0,0,-831.809,180.524,3.66788,0.645772,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 324014
(3100029,2269,0,267,288,1,1,0,0,0,0,-808.512,183.63,20.5318,3.49066,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 324017
(3100030,2269,0,267,288,1,1,0,0,0,0,-801.495,164.293,20.2229,1.0472,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 324018
(3100031,2269,0,267,288,1,1,0,0,0,0,-769.063,160.931,21.3047,2.04204,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 324041
(3100032,2269,0,267,288,1,1,0,0,0,0,-754.967,177.304,22.9019,5.23599,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 324044
(3100033,2269,0,267,288,1,1,0,0,0,0,-740.656,132.526,20.648,5.93412,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 324057
(3100034,2269,0,267,288,1,1,0,0,0,0,-708.115,156.299,19.1837,2.23402,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 324071
(3100035,2269,0,267,288,1,1,0,0,0,0,-691.346,166.191,16.4011,3.97935,60,0,3,0,471,0,1,0,0,0,0,0,'',0,0), -- Hillsbrad Miner (Human Infestation), 4.4.2 guid 324074
(3100036,47204,0,267,288,1,1,0,0,0,0,-1042.74,172.78,65.4724,3.11621,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323390
(3100037,47204,0,267,288,1,1,0,0,0,0,-1015.09,113.433,53.7973,4.6276,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323401
(3100038,47204,0,267,288,1,1,0,0,0,0,-981.972,79.7985,52.3574,2.01855,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323412
(3100039,47204,0,267,288,1,1,0,0,0,0,-880.462,79.4454,36.0741,5.72738,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323422
(3100040,47204,0,267,288,1,1,0,0,0,0,-844.042,72.5851,37.7693,4.11898,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323431
(3100041,47204,0,267,288,1,1,0,0,0,0,-896.134,106.207,39.3018,0.959931,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323450
(3100042,47204,0,267,288,1,1,0,0,0,0,-960.328,157.501,46.7915,3.748,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323667
(3100043,47204,0,267,288,1,1,0,0,0,0,-841.781,131.703,38.7089,4.60767,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323684
(3100044,47204,0,267,288,1,1,0,0,0,0,-821.727,117.328,40.7864,3.07443,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323696
(3100045,47204,0,267,288,1,1,0,0,0,0,-805.911,142.473,43.208,3.29469,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323705
(3100046,47204,0,267,267,1,1,0,0,0,0,-782.183,84.276,38.0517,5.26259,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323715
(3100047,47204,0,267,288,1,1,0,0,0,0,-776.984,124.41,42.9572,5.56366,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 323730
(3100048,47204,0,267,288,1,1,0,0,0,0,-754.682,202.569,54.6108,0,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 324023
(3100049,47204,0,267,288,1,1,0,0,0,0,-745.619,187.417,50.3454,1.20693,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 324032
(3100050,47204,0,267,288,1,1,0,0,0,0,-730.002,117.028,42.2995,3.64774,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 324048
(3100051,47204,0,267,288,1,1,0,0,0,0,-707.676,161.436,46.6054,3.7931,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 324062
(3100052,47204,0,267,288,1,1,0,0,0,0,-714.244,220.679,51.0158,0.204747,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 324220
(3100053,47204,0,267,267,1,1,0,0,0,0,-703.069,245.036,50.6704,3.63029,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 324231
(3100054,47204,0,267,288,1,1,0,0,0,0,-723.903,270.716,49.3726,1.79772,60,0,3,0,653,100,1,0,0,0,0,0,'',0,0), -- Infested Bear (Glorious Harvest, Past Their Prime), 4.4.2 guid 324251
(3100055,47205,0,267,288,1,1,0,0,0,0,-848.821,76.349,3.70763,1.15192,300,0,3,0,458,0,1,0,0,0,0,0,'',0,0), -- Domesticated Mine Creeper, 4.4.2 guid 323443
(3100056,47205,0,267,288,1,1,0,0,0,0,-882.373,174.776,18.279,4.72984,300,0,3,0,458,0,1,0,0,0,0,0,'',0,0), -- Domesticated Mine Creeper, 4.4.2 guid 323681
(3100057,47205,0,267,288,1,1,0,0,0,0,-811.632,97.0764,5.06261,2.52757,300,0,3,0,458,0,1,0,0,0,0,0,'',0,0), -- Domesticated Mine Creeper, 4.4.2 guid 323714
(3100058,47205,0,267,288,1,1,0,0,0,0,-796.582,113.8,16.3133,2.55095,300,0,3,0,458,0,1,0,0,0,0,0,'',0,0), -- Domesticated Mine Creeper, 4.4.2 guid 323725
(3100059,47205,0,267,288,1,1,0,0,0,0,-765.932,63.2934,12.1862,1.16666,300,0,3,0,458,0,1,0,0,0,0,0,'',0,0), -- Domesticated Mine Creeper, 4.4.2 guid 323755
(3100060,47205,0,267,288,1,1,0,0,0,0,-846.891,212.536,8.74423,5.25789,300,0,3,0,458,0,1,0,0,0,0,0,'',0,0), -- Domesticated Mine Creeper, 4.4.2 guid 323989
(3100061,47205,0,267,288,1,1,0,0,0,0,-837.448,189.606,18.958,3.56927,300,0,3,0,458,0,1,0,0,0,0,0,'',0,0), -- Domesticated Mine Creeper, 4.4.2 guid 323990
(3100062,47205,0,267,288,1,1,0,0,0,0,-761.677,167.615,21.904,0.397445,300,0,3,0,458,0,1,0,0,0,0,0,'',0,0), -- Domesticated Mine Creeper, 4.4.2 guid 324022
(3100063,47205,0,267,288,1,1,0,0,0,0,-724.186,125.832,20.9982,1.05596,300,0,3,0,458,0,1,0,0,0,0,0,'',0,0); -- Domesticated Mine Creeper, 4.4.2 guid 324058

-- Hillsbrad Foothills: Welcome to the Machine (28096), polish only (the quest already works).
-- The Skeletal Steed 47445 at Southpoint Gate summons a rideable steed 47488 for whoever talks to it, and the steed's script
-- summons Dumass 47444, Kingslayer Orkus 47443 and Johnny Awesome 47442 as manual-despawn creatures; they only went away when
-- someone picked their gossip option. Anyone could start it, and every abandoned run left three quest NPCs standing at the gate
-- for good. Now the steed only answers a player with the quest in progress, and the three despawn after 2 minutes (in 5.4.8
-- they are personal summons, SummonProperties 3051, and leave when the player dismounts).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceGroup`=1 AND `SourceEntry`=47445 AND `ConditionTypeOrReference`=47;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,1,47445,0,0,47,0,28096,8,0,0,0,0,'','Skeletal Steed - only for a player with Welcome to the Machine in progress');

UPDATE `smart_scripts` SET `action_param2`=3, `action_param3`=120000 WHERE `entryorguid`=4748800 AND `source_type`=9 AND `id` IN (1,2,3) AND `action_type`=12;

-- Hillsbrad Foothills: Coastal Delicacies! (28146). The Hungry Mine Creeper 47662 (summoned by spell_area 88762 while the quest
-- is in the log) only gave credit for murlocs it landed the killing blow on itself, and only for Muckdwellers and Coastrunners.
-- It is a level 1 summon next to level 18-20 murlocs, so most kills were the player's and counted for nothing. The quest says
-- "Whatever you kill, it'll eat!" (feed it 12 murlocs from the Western Strand), and all four Torn Fin murlocs live on the
-- Western Strand. A murloc now also gives the credit when the player on the quest kills it with their spider within 30 yd.
-- The spider's own kill credit stays. The 30 yd range and the four entries are ours.
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (2374,2375,2376,2377) AND `source_type`=0 AND `id`=10;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(2374,0,10,0,6,0,100,0,0,0,0,0,0,33,47752,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Torn Fin Muckdweller - On Death - Quest Credit ''Coastal Delicacies!'' to Killer'),
(2375,0,10,0,6,0,100,0,0,0,0,0,0,33,47752,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Torn Fin Coastrunner - On Death - Quest Credit ''Coastal Delicacies!'' to Killer'),
(2376,0,10,0,6,0,100,0,0,0,0,0,0,33,47752,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Torn Fin Oracle - On Death - Quest Credit ''Coastal Delicacies!'' to Killer'),
(2377,0,10,0,6,0,100,0,0,0,0,0,0,33,47752,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Torn Fin Tidehunter - On Death - Quest Credit ''Coastal Delicacies!'' to Killer');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceGroup`=11 AND `SourceEntry` IN (2374,2375,2376,2377);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,11,2374,0,0,9,0,28146,0,0,0,0,0,'','Torn Fin Muckdweller - killer has Coastal Delicacies! in progress'),
(22,11,2374,0,0,29,0,47662,30,0,0,0,0,'','Torn Fin Muckdweller - killer has a Hungry Mine Creeper within 30 yd'),
(22,11,2375,0,0,9,0,28146,0,0,0,0,0,'','Torn Fin Coastrunner - killer has Coastal Delicacies! in progress'),
(22,11,2375,0,0,29,0,47662,30,0,0,0,0,'','Torn Fin Coastrunner - killer has a Hungry Mine Creeper within 30 yd'),
(22,11,2376,0,0,9,0,28146,0,0,0,0,0,'','Torn Fin Oracle - killer has Coastal Delicacies! in progress'),
(22,11,2376,0,0,29,0,47662,30,0,0,0,0,'','Torn Fin Oracle - killer has a Hungry Mine Creeper within 30 yd'),
(22,11,2377,0,0,9,0,28146,0,0,0,0,0,'','Torn Fin Tidehunter - killer has Coastal Delicacies! in progress'),
(22,11,2377,0,0,29,0,47662,30,0,0,0,0,'','Torn Fin Tidehunter - killer has a Hungry Mine Creeper within 30 yd');

-- Hillsbrad Foothills: Do the Right Thing (28189) could not be completed by freeing the Human Seedlings 47872. Their spellclick
-- cast 89552 "Seedling Grows to Sapling" (it summons a Sapling Protector 48042, a Plants vs Ghouls ally) and their SmartAI had
-- no On Spellclick event, so freeing one gave no credit; the seedlings' thank-you lines (actionlist 4787200) were never used.
-- Wiki: right-click a seedling to free it from the dirt (that "saves" it); smashing it with the Shovel is the wrong choice.
-- Spellclick now casts 89095 "Do the Right Thing" (dummy on the clicker), and the seedling gives the credit, thanks the player
-- and despawns (respawn 60 s).
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=47872;
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`cast_flags`,`user_type`) VALUES
(47872,89095,1,0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=47872;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(18,47872,89095,0,0,9,0,28189,0,0,0,0,0,'','Human Seedling - spellclick only while Do the Right Thing is in progress');

DELETE FROM `smart_scripts` WHERE `entryorguid`=47872 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=4787200 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(47872,0,0,1,73,0,100,0,0,0,0,0,0,33,47872,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - On Spellclick - Quest Credit ''Do the Right Thing'''),
(47872,0,1,2,61,0,100,0,0,0,0,0,0,83,16777216,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Human Seedling - Linked - Remove Npc Flag Spellclick'),
(47872,0,2,3,61,0,100,0,0,0,0,0,0,64,1,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Human Seedling - Linked - Store Clicker'),
(47872,0,3,0,61,0,100,0,0,0,0,0,0,80,4787200,0,2,0,0,0,1,0,0,0,0,0,0,0,0,'Human Seedling - Linked - Run Script'),
(4787200,9,0,0,0,0,100,0,2000,2000,0,0,0,1,0,0,0,0,0,0,12,1,0,0,0,0,0,0,0,'Human Seedling - On Script - Say Line Random'),
(4787200,9,1,0,0,0,100,0,3000,3000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Human Seedling - On Script - Despawn');

-- Hillsbrad Foothills, the Sludge Fields: the freed Master Apothecary Lydon 48020 stood 12 yd from his caged self 47900 from
-- the start, and the second Johnny Awesome 48021 was out before anyone had freed Lydon. In 5.4.8 both are summoned after
-- Freedom for Lydon (89295 "Summon Lydon", 89296 "Summon Johnny", controller aura 89292) and follow the player; neither sniff
-- has them. mop keeps them as static quest givers (they start/end Protocol, Do it for Twinkles, Burnside Must Fall and start
-- A Blight Upon the Land), so show them only once Freedom for Lydon is rewarded. Phase 188 "Quest Zone-Specific 13" is an
-- unused generic phase; the assignment is ours.
UPDATE `creature` SET `phaseId`=188 WHERE `guid` IN (372480,372482) AND `id` IN (48020,48021);

DELETE FROM `phase_area` WHERE `AreaId`=267 AND `PhaseId`=188;
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(267,188,'Hillsbrad Foothills - freed Lydon 48020 and Johnny Awesome 48021 after Freedom for Lydon');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup`=188 AND `SourceEntry`=267;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,188,267,0,0,47,0,28209,64,0,0,0,0,'','Hillsbrad Foothills - phase 188 once Freedom for Lydon is rewarded');

-- Hillsbrad Foothills, the Sludge Fields: the camp showed three Warden Stillwaters and two Flesh-Shaper Arnaulds at once, and
-- A Blight Upon the Land (28237) could not be completed: its target, Warden Stillwater 48080, is faction 68 (Undercity) and can't
-- be attacked, and nothing turns him hostile.
--
-- 5.4.8 has phases 332 "A Blight Upon the Land" and 333 "The Sludge Fields Reclaimed" (Phase.dbc, phase group 540). Neither
-- the 4.3.4 (TCPP) nor the 4.4.2 sniff has spawns of 47834, 48080, 48081 or 48083, so mop's static copies are the finale and
-- cutscene layers. The phase assignment below is inferred:
--  * phase 332 (while 28237 is in the log): Stillwater 48080, Arnauld 48081 and his eight Sludge Guards 48083;
--  * phase 189 "Quest Zone-Specific 14" (unused generic phase, ours): the camp's normal Stillwater 47781 and Arnauld 47899, seen
--    until 28237 is taken (and while The Battle for Hillsbrad, Discretion is Key or For Science!, which they end, is still in
--    the log);
--  * phase 326 "The Sludge Fields" (given by the camera ride 89149 of No One Here Gets Out Alive): Stillwater 47834, the
--    cutscene actor (unselectable faction 35 in the 4.4.2 template).
-- Stillwater 48080 himself (his speech, the hostile turn and his new form) is scripted in C++ (2026_10_28_44); his followers
-- join him when he sets data 1 1 on them and return to faction 68 when they evade or respawn.
UPDATE `creature` SET `phaseId`=332 WHERE `guid` IN (312678,312687,312679,312680,312681,312682,312683,312684,312685,312686) AND `id` IN (48080,48081,48083);
UPDATE `creature` SET `phaseId`=189 WHERE `guid` IN (176112,176107) AND `id` IN (47781,47899);
UPDATE `creature` SET `phaseId`=326 WHERE `guid`=312262 AND `id`=47834;

DELETE FROM `phase_area` WHERE `AreaId`=267 AND `PhaseId` IN (189,332);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(267,189,'Hillsbrad Foothills - Sludge Fields camp (Stillwater 47781, Arnauld 47899) before A Blight Upon the Land'),
(267,332,'Hillsbrad Foothills - A Blight Upon the Land: Stillwater 48080 and his followers');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=267 AND `SourceGroup` IN (189,332);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,189,267,0,0,47,0,28237,33,0,0,0,0,'','Hillsbrad Foothills - phase 189 while A Blight Upon the Land is not taken'),
(26,189,267,0,1,47,0,28196,10,0,0,0,0,'','Hillsbrad Foothills - phase 189 while The Battle for Hillsbrad is in the log'),
(26,189,267,0,2,47,0,28199,10,0,0,0,0,'','Hillsbrad Foothills - phase 189 while For Science! is in the log'),
(26,189,267,0,3,47,0,28197,10,0,0,0,0,'','Hillsbrad Foothills - phase 189 while Discretion is Key is in the log'),
(26,332,267,0,0,47,0,28237,10,0,0,0,0,'','Hillsbrad Foothills - phase 332 while A Blight Upon the Land is in the log');

-- Stillwater 48080's followers.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (48081,48083);

DELETE FROM `smart_scripts` WHERE `entryorguid`=48081 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=48083 AND `source_type`=0 AND `id` BETWEEN 1 AND 5;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(48081,0,0,1,38,0,100,0,1,1,0,0,0,2,14,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Flesh-Shaper Arnauld - On Data Set 1 1 - Set Faction Hostile'),
(48081,0,1,0,61,0,100,0,0,0,0,0,0,49,0,0,0,0,0,0,21,40,0,0,0,0,0,0,0,'Flesh-Shaper Arnauld - Linked - Attack Closest Player'),
(48081,0,2,0,7,0,100,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Flesh-Shaper Arnauld - On Evade - Restore Faction'),
(48081,0,3,0,11,0,100,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Flesh-Shaper Arnauld - On Respawn - Restore Faction (a kill does not evade)'),
(48083,0,1,2,38,0,100,0,1,1,0,0,0,2,14,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Sludge Guard - On Data Set 1 1 - Set Faction Hostile'),
(48083,0,2,0,61,0,100,0,0,0,0,0,0,49,0,0,0,0,0,0,21,40,0,0,0,0,0,0,0,'Sludge Guard - Linked - Attack Closest Player'),
(48083,0,3,0,7,0,100,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Sludge Guard - On Evade - Restore Faction'),
(48083,0,5,0,11,0,100,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Sludge Guard - On Respawn - Restore Faction (a kill does not evade)');
