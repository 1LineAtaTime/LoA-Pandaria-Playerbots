-- Arathi Highlands, Stromgarde Keep: Solid Chest 2857 (guid 12882) off the stairs.
-- Problem: the chest stands on a stair landing (-1699.68 -1797.74 93.13), a spot that matches none of the nine chest
--   pins of the source around the keep.
-- Change: move it to the nearest pin, map 25.8 65.7 (Arathi Highlands: x = -133.333 - 24*y%, y = -866.667 - 36*x%),
--   a room inside the keep; floor height 88.24 measured in game (the outdoor ground below the pin is 80.64).
-- Source: Wowhead MoP Classic object 2857 map pins (zone 45).
-- Rollback:
--   UPDATE `gameobject` SET `position_x`=-1699.68,`position_y`=-1797.74,`position_z`=93.1325 WHERE `guid`=12882 AND `id`=2857;
UPDATE `gameobject` SET `position_x`=-1710.1,`position_y`=-1796.9,`position_z`=88.24 WHERE `guid`=12882 AND `id`=2857;
