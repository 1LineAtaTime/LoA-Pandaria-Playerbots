-- Eastern Plaguelands, Borelgore (The Corpulent One, 27477): he starts at the south end of his trench, where he was sniffed.
--
-- Reported in game: Borelgore could not be found. He was alive at the first point of his path (2975 -4300), the north end of
-- the trench, 275 yd from the player who was searching the south end: a creature only walks while a player is near enough
-- to load its grid, so he stood at his spawn point until somebody came to that end.
--
-- Sourced: both TDBs spawn him at 2928.84 -4617.39 77.08, the south end, the part of the trench between Eastwall Tower and
--   Ix'lar's Domain where players come from. He spawns there now.
-- Ours: the path (2026_11_14_36) is the same 24 points, renumbered to begin at the point next to that spawn (the old point
--   12), so he sets off along the trench from there.

UPDATE `creature` SET `position_x`=2928.84,`position_y`=-4617.39,`position_z`=77.0834,`orientation`=1.06602 WHERE `guid`=175254 AND `id`=11896;
DELETE FROM `waypoint_data` WHERE `id`=175254;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`) VALUES
(175254,1,2921.0,-4630.0,79.10),
(175254,2,2894.0,-4660.0,82.75),
(175254,3,2921.0,-4630.0,79.10),
(175254,4,2941.0,-4600.0,73.64),
(175254,5,2955.0,-4570.0,74.04),
(175254,6,2971.0,-4540.0,74.22),
(175254,7,2977.0,-4510.0,75.86),
(175254,8,2971.0,-4480.0,75.70),
(175254,9,2968.0,-4450.0,87.89),
(175254,10,2963.0,-4420.0,95.43),
(175254,11,2979.0,-4390.0,93.19),
(175254,12,2977.0,-4360.0,75.06),
(175254,13,2978.0,-4330.0,78.44),
(175254,14,2975.0,-4300.0,79.56),
(175254,15,2978.0,-4330.0,78.44),
(175254,16,2977.0,-4360.0,75.06),
(175254,17,2979.0,-4390.0,93.19),
(175254,18,2963.0,-4420.0,95.43),
(175254,19,2968.0,-4450.0,87.89),
(175254,20,2971.0,-4480.0,75.70),
(175254,21,2977.0,-4510.0,75.86),
(175254,22,2971.0,-4540.0,74.22),
(175254,23,2955.0,-4570.0,74.04),
(175254,24,2941.0,-4600.0,73.64);
