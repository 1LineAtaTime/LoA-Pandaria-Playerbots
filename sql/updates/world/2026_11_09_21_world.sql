-- The Hinterlands: each rare once, on its sniffed spot.
-- Sourced: the Cataclysm Classic TDB 4.4.2 (and retail 1210, same rows) spawns each of the ten Hinterlands rares once, with no pool. The
-- copy on (or nearest) the TDB spot stays; the old copies go with their pools. The Reak, Ironback and Jalinde Summerdrake shared master
-- pool 1045113 (max 1), so only one of the three was up at a time. Pool placeholders Vilebranch Aman'zasi Guard 12537 (no TDB spawn near)
-- and Silvermane Stalker 13352 (TDB spot 278516 already held by 192045) go with their pools. Mith'rethis stands on the TDB spot;
-- Grimungous and Retherokk walk the TDB paths 20002480 and 20002560 from the TDB spots, with the TDB's SheathState 1.
-- Ours: the path ids (the spawn guids); respawn times stay at mop's 28800 (the TDB's 120000 is a default, not a measured timer).
DELETE FROM `creature` WHERE `id`=2648 AND `guid` IN (12537);
DELETE FROM `creature` WHERE `id`=2926 AND `guid` IN (13352);
-- Razortalon
DELETE FROM `creature` WHERE `id`=8210 AND `guid` IN (13895,131405,131406);
-- Old Cliff Jumper
DELETE FROM `creature` WHERE `id`=8211 AND `guid` IN (53451);
-- The Reak
DELETE FROM `creature` WHERE `id`=8212 AND `guid` IN (14116,131407,131408);
-- Ironback
DELETE FROM `creature` WHERE `id`=8213 AND `guid` IN (12912,131442,131443);
-- Jalinde Summerdrake
DELETE FROM `creature` WHERE `id`=8214 AND `guid` IN (53452,131444,131445);
-- Grimungous
DELETE FROM `creature` WHERE `id`=8215 AND `guid` IN (13388);
-- Retherokk the Berserker
DELETE FROM `creature` WHERE `id`=8216 AND `guid` IN (12998);
-- Mith'rethis the Enchanter
DELETE FROM `creature` WHERE `id`=8217 AND `guid` IN (91547);
-- Witherheart the Stalker
DELETE FROM `creature` WHERE `id`=8218 AND `guid` IN (12937);
-- Zul'arek Hatefowler
DELETE FROM `creature` WHERE `id`=8219 AND `guid` IN (53453,131446);

-- Their addon, pool and path rows (only once the creature is gone), then the empty pools.
DELETE a FROM `creature_addon` a LEFT JOIN `creature` c ON c.`guid`=a.`guid` WHERE a.`guid` IN (12537,12912,12937,12998,13352,13388,13895,14116,53451,53452,53453,91547,131405,131406,131407,131408,131442,131443,131444,131445,131446) AND c.`guid` IS NULL;
DELETE p FROM `pool_creature` p LEFT JOIN `creature` c ON c.`guid`=p.`guid` WHERE p.`guid` IN (12537,12912,12937,12998,13352,13388,13895,14116,53451,53452,53453,91547,131405,131406,131407,131408,131442,131443,131444,131445,131446) AND c.`guid` IS NULL;
DELETE FROM `waypoint_data` WHERE `id` IN (12998,13388) AND NOT EXISTS (SELECT 1 FROM `creature_addon` a WHERE a.`path_id`=`waypoint_data`.`id`);
DELETE pp FROM `pool_pool` pp WHERE pp.`mother_pool`=1045113 AND pp.`pool_id` IN (1284,1285,1286) AND NOT EXISTS (SELECT 1 FROM `pool_creature` pc WHERE pc.`pool_entry`=pp.`pool_id`);
DELETE t FROM `pool_template` t WHERE t.`entry` IN (142,174,311,1284,1285,1286,1287,1045113)
  AND NOT EXISTS (SELECT 1 FROM `pool_creature` pc WHERE pc.`pool_entry`=t.`entry`)
  AND NOT EXISTS (SELECT 1 FROM `pool_gameobject` pg WHERE pg.`pool_entry`=t.`entry`)
  AND NOT EXISTS (SELECT 1 FROM `pool_pool` pp WHERE pp.`pool_id`=t.`entry` OR pp.`mother_pool`=t.`entry`);

-- Mith'rethis the Enchanter stands on his TDB spot (TDB guid 250033).
UPDATE `creature` SET `position_x`=-525.307,`position_y`=-4033.5,`position_z`=215.861,`orientation`=4.73139,`wander_distance`=0,`MovementType`=0 WHERE `guid`=312694 AND `id`=8217;

-- Grimungous walks TDB path 20002480 from its TDB spot (TDB guid 250031).
UPDATE `creature` SET `position_x`=-131.599,`position_y`=-4337.36,`position_z`=135.607,`orientation`=5.57514,`wander_distance`=0,`MovementType`=2 WHERE `guid`=191979 AND `id`=8215;
DELETE FROM `creature_addon` WHERE `guid`=191979;
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(191979,191979,0,0,0,0,0,1,0,0,0,0,0,0,NULL);
DELETE FROM `waypoint_data` WHERE `id`=191979;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`move_flag`,`action`,`action_chance`,`wpguid`) VALUES
(191979,1,-131.599,-4337.36,135.607,0,0,0,0,100,0),
(191979,2,-107.97,-4329.76,136.531,0,0,0,0,100,0),
(191979,3,-84.5573,-4338.76,128.724,0,0,0,0,100,0),
(191979,4,-58.8715,-4358.4,132.083,0,0,0,0,100,0),
(191979,5,-29.8125,-4387.25,145.153,0,0,0,0,100,0),
(191979,6,3.05382,-4376.49,138.288,0,0,0,0,100,0),
(191979,7,31.809,-4406.14,144.501,0,0,0,0,100,0),
(191979,8,52.0556,-4423.45,142.369,0,0,0,0,100,0),
(191979,9,45.2309,-4454.27,144.504,0,0,0,0,100,0),
(191979,10,54.7517,-4469.25,142.061,0,0,0,0,100,0),
(191979,11,72.7413,-4460.87,147.031,0,0,0,0,100,0),
(191979,12,93.0139,-4476.59,145.245,0,0,0,0,100,0),
(191979,13,87.2431,-4491.62,141.287,0,0,0,0,100,0),
(191979,14,61.4427,-4518.49,142.65,0,0,0,0,100,0),
(191979,15,35.6615,-4527.33,146.523,0,0,0,0,100,0),
(191979,16,51.1736,-4522.93,145.351,0,0,0,0,100,0),
(191979,17,76.0729,-4504.04,144.116,0,0,0,0,100,0),
(191979,18,97.6163,-4520.25,135.29,0,0,0,0,100,0),
(191979,19,126.234,-4516.11,127.969,0,0,0,0,100,0),
(191979,20,147.832,-4539.19,120.36,0,0,0,0,100,0),
(191979,21,146.368,-4586.85,119.907,0,0,0,0,100,0),
(191979,22,181.186,-4595.21,123.124,0,0,0,0,100,0),
(191979,23,146.368,-4586.85,119.907,0,0,0,0,100,0),
(191979,24,147.832,-4539.19,120.36,0,0,0,0,100,0),
(191979,25,126.234,-4516.11,127.969,0,0,0,0,100,0),
(191979,26,97.6163,-4520.25,135.29,0,0,0,0,100,0),
(191979,27,76.0729,-4504.04,144.116,0,0,0,0,100,0),
(191979,28,51.1736,-4522.93,145.351,0,0,0,0,100,0),
(191979,29,35.6615,-4527.33,146.523,0,0,0,0,100,0),
(191979,30,61.1856,-4518.58,142.615,0,0,0,0,100,0),
(191979,31,87.2431,-4491.62,141.287,0,0,0,0,100,0),
(191979,32,93.0139,-4476.59,145.245,0,0,0,0,100,0),
(191979,33,72.7413,-4460.87,147.031,0,0,0,0,100,0),
(191979,34,54.7517,-4469.25,142.061,0,0,0,0,100,0),
(191979,35,45.2309,-4454.27,144.504,0,0,0,0,100,0),
(191979,36,52.0556,-4423.45,142.369,0,0,0,0,100,0),
(191979,37,31.809,-4406.14,144.501,0,0,0,0,100,0),
(191979,38,3.30078,-4376.67,138.385,0,0,0,0,100,0),
(191979,39,-29.8125,-4387.25,145.153,0,0,0,0,100,0),
(191979,40,-58.8715,-4358.4,132.083,0,0,0,0,100,0),
(191979,41,-84.5573,-4338.76,128.724,0,0,0,0,100,0),
(191979,42,-107.97,-4329.76,136.531,0,0,0,0,100,0);

-- Retherokk the Berserker walks TDB path 20002560 from its TDB spot (TDB guid 250032).
UPDATE `creature` SET `position_x`=-239.615,`position_y`=-3406.91,`position_z`=152.684,`orientation`=3.83404,`wander_distance`=0,`MovementType`=2 WHERE `guid`=312695 AND `id`=8216;
DELETE FROM `creature_addon` WHERE `guid`=312695;
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(312695,312695,0,0,0,0,0,1,0,0,0,0,0,0,NULL);
DELETE FROM `waypoint_data` WHERE `id`=312695;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`move_flag`,`action`,`action_chance`,`wpguid`) VALUES
(312695,1,-239.615,-3406.91,152.684,0,0,0,0,100,0),
(312695,2,-247.024,-3413.87,156.985,0,0,0,0,100,0),
(312695,3,-252.918,-3418.76,164.162,0,0,0,0,100,0),
(312695,4,-269.934,-3435.76,187.134,0,0,0,0,100,0),
(312695,5,-275.066,-3436.22,187.13,0,0,0,0,100,0),
(312695,6,-291.474,-3451.38,190.471,0,0,0,0,100,0),
(312695,7,-291.946,-3456.25,191.754,0,0,0,0,100,0),
(312695,8,-291.474,-3451.38,190.471,0,0,0,0,100,0),
(312695,9,-275.066,-3436.22,187.13,0,0,0,0,100,0),
(312695,10,-269.934,-3435.76,187.134,0,0,0,0,100,0),
(312695,11,-252.918,-3418.76,164.162,0,0,0,0,100,0),
(312695,12,-247.024,-3413.87,156.985,0,0,0,0,100,0);
