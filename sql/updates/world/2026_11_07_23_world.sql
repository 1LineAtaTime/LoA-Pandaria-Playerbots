-- Durnholde: Infernus patrols. Sourced: TDB 4.4.2 and 1210 guid 322294, MovementType 2, path 25783520, 7 nodes, walking; his spot is its first node.
-- The 60 s respawn stays: "Respawn rate is about a minute" (Wowhead npc_49266 comment, 2014); the TDBs' 120 is their default.
UPDATE `creature` SET `position_x`=-573.038,`position_y`=-1449.43,`position_z`=53.9236,`orientation`=0.405148,`MovementType`=2,`wander_distance`=0 WHERE `guid`=177116 AND `id`=49266;
DELETE FROM `creature_addon` WHERE `guid`=177116;
INSERT INTO `creature_addon` (`guid`,`path_id`,`SheathState`) VALUES (177116,177116,1);
DELETE FROM `waypoint_data` WHERE `id`=177116;
INSERT INTO `waypoint_data` (`id`,`point`,`position_x`,`position_y`,`position_z`) VALUES
(177116,1,-573.038,-1449.43,53.9236),
(177116,2,-561.08,-1444.3,52.5659),
(177116,3,-554.135,-1448.94,52.5659),
(177116,4,-552.07,-1463.92,52.5659),
(177116,5,-553.804,-1475.3,52.5659),
(177116,6,-565.997,-1476.44,52.6729),
(177116,7,-573.389,-1463.56,52.9468);
