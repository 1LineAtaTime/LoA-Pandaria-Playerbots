-- Yetimus the Yeti Lord 47484 (quest 28485): one boss on the sniffed patrol instead of two on stub paths.
-- Sourced: TDB 4.4.2 and TDB 1210 both have one spawn, guid 323060 at (-151.922, -171.311, 104.999), on the 38 nodes of path 25844800,
-- with visibility distance type 4 in the template addon. mop's two paths (5 and 22 nodes) contain sniffed nodes 2, 3, 19 and 20,
-- so they are fragments of that route. The Wowhead comments on npc 47484 and quest 28485 fit one patrol: "either of his path ends",
-- the "yeti cave that he paths by", waiting "for him to path by the Sludge Fields" (nodes 7-9), and the spots 41.6, 50.6 and north of
-- 47.36, 50.71 (nodes 1 and 20).
-- Terrain under nodes 1-20 is within 0.21 yd of each node's z (the other 18 repeat them).
-- Ours: respawn stays at mop's 300 s (the TDBs' 120 s looks like a default rather than a measurement).
DELETE FROM `creature_addon` WHERE `guid`=176908;
DELETE FROM `creature` WHERE `guid`=176908 AND `id`=47484;
DELETE FROM `waypoint_data` WHERE `id` IN (176908,176975);
UPDATE `creature` SET `position_x`=-151.922,`position_y`=-171.311,`position_z`=104.999,`orientation`=5.28631,`currentwaypoint`=0 WHERE `guid`=176975 AND `id`=47484;
UPDATE `creature_addon` SET `path_id`=176975,`visibilityDistanceType`=4 WHERE `guid`=176975;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`move_flag`,`action`,`action_chance`,`wpguid`,`entry`) VALUES
(176975,1,-151.922,-171.311,104.999,0,0,0,0,100,0,NULL),
(176975,2,-197.392,-164.851,92.9974,0,0,0,0,100,0,NULL),
(176975,3,-240.981,-167.431,80.6763,0,0,0,0,100,0,NULL),
(176975,4,-279.948,-174.962,74.1438,0,0,0,0,100,0,NULL),
(176975,5,-301.554,-187.203,68.3701,0,0,0,0,100,0,NULL),
(176975,6,-319.957,-198.203,64.633,0,0,0,0,100,0,NULL),
(176975,7,-348.99,-235.068,62.7893,0,0,0,0,100,0,NULL),
(176975,8,-368.51,-265.285,60.2035,0,0,0,0,100,0,NULL),
(176975,9,-381.052,-305.778,59.7142,0,0,0,0,100,0,NULL),
(176975,10,-383.299,-340.087,59.1627,0,0,0,0,100,0,NULL),
(176975,11,-370.479,-366.644,58.3729,0,0,0,0,100,0,NULL),
(176975,12,-349.299,-399.33,58.9684,0,0,0,0,100,0,NULL),
(176975,13,-324.72,-430.033,59.3361,0,0,0,0,100,0,NULL),
(176975,14,-297.215,-456.95,60.7625,0,0,0,0,100,0,NULL),
(176975,15,-268.215,-460.696,64.3599,0,0,0,0,100,0,NULL),
(176975,16,-247.248,-462.217,65.8604,0,0,0,0,100,0,NULL),
(176975,17,-222.099,-464.127,67.6233,0,0,0,0,100,0,NULL),
(176975,18,-195.962,-466.226,71.8647,0,0,0,0,100,0,NULL),
(176975,19,-160.519,-459.127,76.6852,0,0,0,0,100,0,NULL),
(176975,20,-141.003,-451.144,76.6852,0,0,0,0,100,0,NULL),
(176975,21,-160.519,-459.127,76.6852,0,0,0,0,100,0,NULL),
(176975,22,-195.962,-466.226,71.8647,0,0,0,0,100,0,NULL),
(176975,23,-222.099,-464.127,67.6233,0,0,0,0,100,0,NULL),
(176975,24,-247.248,-462.217,65.8604,0,0,0,0,100,0,NULL),
(176975,25,-268.215,-460.696,64.3599,0,0,0,0,100,0,NULL),
(176975,26,-297.215,-456.95,60.7625,0,0,0,0,100,0,NULL),
(176975,27,-324.72,-430.033,59.3361,0,0,0,0,100,0,NULL),
(176975,28,-349.299,-399.33,58.9684,0,0,0,0,100,0,NULL),
(176975,29,-370.479,-366.644,58.3729,0,0,0,0,100,0,NULL),
(176975,30,-383.299,-340.087,59.1627,0,0,0,0,100,0,NULL),
(176975,31,-381.052,-305.778,59.7142,0,0,0,0,100,0,NULL),
(176975,32,-368.51,-265.285,60.2035,0,0,0,0,100,0,NULL),
(176975,33,-348.99,-235.068,62.7893,0,0,0,0,100,0,NULL),
(176975,34,-319.957,-198.203,64.633,0,0,0,0,100,0,NULL),
(176975,35,-301.554,-187.203,68.3701,0,0,0,0,100,0,NULL),
(176975,36,-279.948,-174.962,74.1438,0,0,0,0,100,0,NULL),
(176975,37,-240.981,-167.431,80.6763,0,0,0,0,100,0,NULL),
(176975,38,-197.392,-164.851,92.9974,0,0,0,0,100,0,NULL);
