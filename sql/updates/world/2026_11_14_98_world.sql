-- Eastern Plaguelands: Vex'tul at Eastwall Tower stands where a player stood.
--
-- 2026_11_14_97 moved the Eastwall copy of Vex'tul (guid 3128015) out of the tower wall to a spot beside Fiona that turned out
-- to be inside the hillside (reported in game). This position is the tester's own (.gps on open ground beside Rimblat
-- Earthshatter); he faces Fiona.
--
-- Ours: the position.

UPDATE `creature` SET `position_x`=2564.04, `position_y`=-4769.21, `position_z`=106.634, `orientation`=2.25 WHERE `guid`=3128015 AND `id`=45574;
