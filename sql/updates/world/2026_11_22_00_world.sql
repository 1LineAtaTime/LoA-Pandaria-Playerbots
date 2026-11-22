-- Brill (Tirisfal Glades): a player cannot reach Junior Apothecary Holland (10665), the quest giver in the apothecary
--   building. The walk to him stops about 24 yards short of him.
-- Cause: spawn 192607 stands at z 41.06 on an upper level that the ground path does not reach, and a five-point
--   waypoint path (id 192607, all points at z 41.06) moves him around up there. The sniff-based references have him
--   on the ground floor, standing still.
-- Change: move spawn 192607 to the reference position and orientation, set MovementType 0 (no movement), clear path_id
--   in creature_addon (other columns stay) and remove waypoint_data id 192607 (used by no other spawn, formation or
--   script).
-- Source: the Cataclysm Preservation world database (built from sniffs) and the SkyFire 5.4.8 world database both have
--   Holland at 2250.27 316.375 34.8149, orientation 5.73859, MovementType 0, no path. The wiki lists him at map
--   coordinates [60.2, 52.4], which is closer to the reference position (60.1, 52.7) than to the old one (59.9, 52.8).
-- Rollback:
--   UPDATE `creature` SET `position_x`=2248.34,`position_y`=325.164,`position_z`=41.0628,`orientation`=4.78587,`MovementType`=2,`wander_distance`=0 WHERE `guid`=192607 AND `id`=10665;
--   UPDATE `creature_addon` SET `path_id`=192607 WHERE `guid`=192607;
--   DELETE FROM `waypoint_data` WHERE `id`=192607;
--   INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`move_flag`,`action`,`action_chance`,`wpguid`,`entry`) VALUES
--   (192607,1,2248.27,326.162,41.0628,0,0,0,0,100,0,NULL),
--   (192607,2,2248.34,325.164,41.0628,0,0,0,0,100,0,NULL),
--   (192607,3,2248.5,323.08,41.0628,0,0,0,0,100,0,NULL),
--   (192607,4,2247.47,322.609,41.0628,0,0,0,0,100,0,NULL),
--   (192607,5,2247.47,322.609,41.0628,0,0,0,0,100,0,NULL);
UPDATE `creature` SET `position_x`=2250.27,`position_y`=316.375,`position_z`=34.8149,`orientation`=5.73859,`MovementType`=0,`wander_distance`=0 WHERE `guid`=192607 AND `id`=10665;
UPDATE `creature_addon` SET `path_id`=0 WHERE `guid`=192607;
DELETE FROM `waypoint_data` WHERE `id`=192607;
