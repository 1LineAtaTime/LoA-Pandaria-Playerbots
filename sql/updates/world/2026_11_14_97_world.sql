-- Eastern Plaguelands: Vex'tul at Eastwall Tower stands in the open.
--
-- The caravan copy of Vex'tul at Eastwall (guid 3128015) stood at 2564.9, -4773.5, inside the wall of the tower's base: the
-- player got his "!" on the minimap and could target him, but saw nobody (reported in game). The spot was ours and had not
-- been seen in game. He moves 4 yd east of Fiona (2559, -4763), onto the open ground the caravan stands on, facing her.
--
-- Ours: the position.

UPDATE `creature` SET `position_x`=2563.5, `position_y`=-4763.5, `position_z`=106.4, `orientation`=3.2 WHERE `guid`=3128015 AND `id`=45574;
