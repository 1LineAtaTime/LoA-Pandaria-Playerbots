-- Arathi Highlands: the Forsaken Courier (2714) patrols alone south of Go'Shek Farm
-- The Cataclysm Classic TDB 4.4.2 and the retail TDB 1210 have one Forsaken Courier (their 285848, mop's 161449) on a
-- 10-node walking path (22867840) on the trail south of Go'Shek Farm, and no Forsaken Bodyguards (2721) anywhere.
-- Wowhead comments on Just Like Old Times (26110) agree: "Pats back and forth on the trail south of the farm",
-- "no adds anymore" (the bodyguards went in patch 2.3), spawn at 53.51 64.85 = mop's 161449.
-- mop also had the pre-Cataclysm courier 11304 standing with four Bodyguards at the farmhouse; nothing references them.
-- Remove those five, give 161449 the TDB path and its 120 s respawn. Ours: the path id 1614490.
DELETE FROM `creature` WHERE `guid`=11304 AND `id`=2714;
DELETE FROM `creature` WHERE `guid` IN (11431,12959,12967,60905) AND `id`=2721;
DELETE FROM `creature_addon` WHERE `guid` IN (11304,11431,12959,12967,60905);
DELETE FROM `game_event_creature` WHERE `guid` IN (11304,11431,12959,12967,60905);
DELETE FROM `waypoint_data` WHERE `id`=1614490;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`move_flag`,`action`,`action_chance`,`wpguid`) VALUES
(1614490,1,-1644.77,-2970.93,26.889,0,0,0,0,100,0),
(1614490,2,-1643.69,-2961.7,27.8141,0,0,0,0,100,0),
(1614490,3,-1640.91,-2951.57,28.433,0,0,0,0,100,0),
(1614490,4,-1639.89,-2948.18,28.7621,0,0,0,0,100,0),
(1614490,5,-1640.86,-2917.59,27.2608,0,0,0,0,100,0),
(1614490,6,-1641.81,-2914.43,26.8585,0,0,0,0,100,0),
(1614490,7,-1641.82,-2914.76,26.7874,0,0,0,0,100,0),
(1614490,8,-1639.27,-2923.27,27.669,0,0,0,0,100,0),
(1614490,9,-1637.15,-2931.19,28.2743,0,0,0,0,100,0),
(1614490,10,-1642.04,-2956,28.0418,0,0,0,0,100,0);
DELETE FROM `creature_addon` WHERE `guid`=161449;
INSERT INTO `creature_addon` (`guid`,`path_id`) VALUES (161449,1614490);
UPDATE `creature` SET `position_x`=-1644.77, `position_y`=-2970.93, `position_z`=26.889, `orientation`=4.98914, `MovementType`=2, `wander_distance`=0, `currentwaypoint`=0, `spawntimesecs`=120 WHERE `guid`=161449 AND `id`=2714;
