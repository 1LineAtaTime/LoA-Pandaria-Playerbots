-- Kasha Will Fly Again (28354): 8 Mudsnout Blossoms (1723) at Nethander Stead, on the spots Wowhead MoP Classic shows
-- (no TDB has a sniffed blossom: the 7 rows in TDB 442.26081 and retail 1210.26091 are VerifiedBuild 0)

-- Spawns no Wowhead pin confirms (nearest MoP Classic pin 10.8-28.8 yd away)
DELETE FROM `gameobject` WHERE `guid` IN (158357, 158358, 158371, 158372) AND `id` = 1723;

-- Spawns already on a pinned spot keep their position
UPDATE `gameobject` SET `map` = 0, `zoneId` = 267, `areaId` = 289, `spawnMask` = 1, `phaseMask` = 1, `phaseId` = 0, `phaseGroup` = 0,
  `spawntimesecs` = 120, `animprogress` = 255, `state` = 1
  WHERE `guid` IN (158351, 158352, 158370) AND `id` = 1723;

-- Pinned spots that had no blossom: two at the TDB rows' positions (204266, 204272), three from the pins
DELETE FROM `gameobject` WHERE `guid` BETWEEN 4100011 AND 4100015;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`) VALUES
(4100011, 1723, 0, 267, 289, 1, 1, -919.227, -939.111,  30.479,  0.0174525, 0, 0, 0.00872614, 0.999962, 120, 255, 1),
(4100012, 1723, 0, 267, 289, 1, 1, -952.491, -1015.02,  30.4198, 0.0174525, 0, 0, 0.00872614, 0.999962, 120, 255, 1),
(4100013, 1723, 0, 267, 289, 1, 1, -930.55,  -960.525,  30.356,  0.0174525, 0, 0, 0.00872614, 0.999962, 120, 255, 1),
(4100014, 1723, 0, 267, 289, 1, 1, -914.342, -1023.737, 30.348,  0.0174525, 0, 0, 0.00872614, 0.999962, 120, 255, 1),
(4100015, 1723, 0, 267, 289, 1, 1, -917.583, -1043.188, 30.348,  0.0174525, 0, 0, 0.00872614, 0.999962, 120, 255, 1);
