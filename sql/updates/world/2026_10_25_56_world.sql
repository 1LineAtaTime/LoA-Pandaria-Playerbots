-- Ruul the Darkener: one stationary spawn on his drake over Shadowmoon Valley, as in AzerothCore. 73715 was set to waypoint movement
-- without a path ("doesn't have waypoint path id: 0"), and 77079 was a second copy of the rare at -4148, 590.
UPDATE `creature_template` SET `MovementType` = 0 WHERE `entry` = 21315;

DELETE FROM `creature` WHERE `guid` IN (73715, 77079);
DELETE FROM `creature_addon` WHERE `guid` IN (73715, 77079);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`,
`spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`) VALUES
(73715, 21315, 530, 3520, 3821, 1, 1, -3761.74, 1081.67, 125.316, 4.5238, 300, 0, 0, 86172, 0, 0);
