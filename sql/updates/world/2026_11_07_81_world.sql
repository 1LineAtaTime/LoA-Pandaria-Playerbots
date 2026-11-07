-- Tarren Mill: walk the four Dreadguard (47214) pairs up and down their road, side by side.
-- Sourced that they move:
--  * the user's report in game ("supposed to be walking up and down that piece of road", 3120013/3120014);
--  * TCPP's sniff import (sql/old/4.3.4/TDB00_to_TDB01_updates/world/110_hillsbrad_foothills.sql) tags exactly these four pairs
--    "(possible waypoints or random movement)", and the other Tarren Mill guards not; each pair is 3 yd apart, side by side;
--  * Wowhead MoP Classic map pins with no static guard under them: (55.2,49.2) and (59.2,46.2) on routes, (54.4,46.2) on the
--    same road 8 yd past the north end of the 3120013 route.
-- No source has a route (neither TDB has a path or formation for them): THE ROUTES ARE OURS, built from:
--  * each pair's sniffed spot and heading (the direction it was walking when first seen; the first leg keeps it);
--  * the pins above;
--  * ground textures in the 5.4.8 client (world.MPQ, Azeroth_33_31/32_tex0.adt): the bricks and dirt tracks of the roads;
--  * for 3120013: both ends are nodes of both TDBs' sniffed path 1233920, which passes along this road on its way south, and the
--    pair's heading points exactly at the first. That route is the best supported; the other three run beside a track or wall and
--    are guesses in shape and length. Heights are the terrain's; at 1 yd sampling no model stands on the leader's lane or 3 yd to
--    either side of it.
-- Moving: the lower guid leads on a path (MovementType 2), the other follows it through creature_formations (MovementType 0).
-- In this core CreatureGroup::LeaderStartedMoving builds a new FormationMovementGenerator for the member at every leader node, with
-- the row's angle; it mirrors that angle for one leg only, when the leg leaves 0-based node point_1 or point_2. So only two legs can
-- be mirrored, and every route has exactly two legs in the mirrored direction: point_1 (a turn) and point_2 (the next node) start
-- them. All other legs use the row's angle, which puts the member on the same world side of the leader as in the sniff, so it does
-- not cross him on the road or at a turn. That two-leg limit is why the 3120013 route ends at (-17.2,-802.6) and not at the pin.
-- Stops: 4 s at both ends. Waypoint orientation stays 0, since the member's slot during a pause is taken from the leader's facing.
-- The spawn addons drop the template's state emote 333 while walking, as patrols 187199/188250 do, and keep its aura.

UPDATE `creature` SET `MovementType`=2,`wander_distance`=0 WHERE `id`=47214 AND `guid` IN (3120000,3120006,3120010,3120013);
UPDATE `creature` SET `MovementType`=0,`wander_distance`=0 WHERE `id`=47214 AND `guid` IN (3120001,3120007,3120011,3120014);

DELETE FROM `creature_addon` WHERE `guid` IN (3120000,3120001,3120006,3120007,3120010,3120011,3120013,3120014);
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(3120000,31200000,0,0,0,0,0,1,0,0,0,0,0,0,'5301'),
(3120001,0,0,0,0,0,0,1,0,0,0,0,0,0,'5301'),
(3120006,31200060,0,0,0,0,0,1,0,0,0,0,0,0,'5301'),
(3120007,0,0,0,0,0,0,1,0,0,0,0,0,0,'5301'),
(3120010,31200100,0,0,0,0,0,1,0,0,0,0,0,0,'5301'),
(3120011,0,0,0,0,0,0,1,0,0,0,0,0,0,'5301'),
(3120013,31200130,0,0,0,0,0,1,0,0,0,0,0,0,'5301'),
(3120014,0,0,0,0,0,0,1,0,0,0,0,0,0,'5301');

DELETE FROM `waypoint_data` WHERE `id` IN (31200000,31200060,31200100,31200130);
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`move_flag`) VALUES
(31200000,1,14,-1026.75,56.02,0,0,0),
(31200000,2,24,-1026,56.54,0,4000,0),
(31200000,3,2.17,-1027.32,56.12,0,0,0),
(31200000,4,-16,-1031,54.47,0,4000,0),
(31200000,5,2.17,-1027.32,56.12,0,0,0),
(31200060,1,-113.7,-832.4,56.88,0,0,0),
(31200060,2,-120,-833.5,56.47,0,4000,0),
(31200060,3,-105.86,-831.22,57.37,0,0,0),
(31200060,4,-85,-833,57.14,0,4000,0),
(31200060,5,-105.86,-831.22,57.37,0,0,0),
(31200100,1,15,-835.6,57.85,0,0,0),
(31200100,2,22,-835,57.63,0,4000,0),
(31200100,3,10.7,-835.9,58.43,0,0,0),
(31200100,4,-10,-837.5,60.07,0,4000,0),
(31200100,5,10.7,-835.9,58.43,0,0,0),
(31200130,1,-49.4,-832.7,56.73,0,0,0),
(31200130,2,-51.5,-836.7,56.52,0,4000,0),
(31200130,3,-42,-822.8,57.19,0,0,0),
(31200130,4,-17.2,-802.6,58.75,0,4000,0),
(31200130,5,-42,-822.8,57.19,0,0,0),
(31200130,6,-47.31,-828.73,56.88,0,0,0);

DELETE FROM `creature_formations` WHERE `memberGUID` IN (3120000,3120001,3120006,3120007,3120010,3120011,3120013,3120014) OR `leaderGUID` IN (3120000,3120001,3120006,3120007,3120010,3120011,3120013,3120014);
INSERT INTO `creature_formations` (`leaderGUID`,`memberGUID`,`dist`,`angle`,`groupAI`,`point_1`,`point_2`) VALUES
(3120000,3120000,0,0,515,0,0),
(3120000,3120001,3,270,515,1,2),
(3120006,3120006,0,0,515,0,0),
(3120006,3120007,3,90,515,1,2),
(3120010,3120010,0,0,515,0,0),
(3120010,3120011,3,270,515,1,2),
(3120013,3120013,0,0,515,0,0),
(3120013,3120014,3,90,515,1,2);
