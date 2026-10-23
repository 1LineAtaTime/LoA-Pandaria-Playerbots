-- #787 follow-up (lane F, 2026-10-21): the last two travel-graph walk links that start on one elevator floor and walk on the
-- other (playerbots DB; beyond AC - the graph is AC's 3.3.5 data).
--
-- 2026_10_15_00.sql Part A fixed the walk links INTO a stop (the "leg-reversed" ValidateRoute rejection: 0 such links left,
-- checked over every type-3 stop on the test DB). Checked the other direction too: the walk link OUT of an arrival stop must
-- start on that stop's floor, or a bot that rode up is sent to walk the bottom floor (and the reverse). Two left, both with a
-- sibling stop of the same elevator on the floor the path walks:
--  - 1937 -> 786 (Elevator 152614, Kalimdor (2263.7,-5565.6)): starts at the top stop 1937 z 33.9, then walks at z 21.5 =
--    the bottom stop 1938 z 21.1. Re-pointed to 1938 -> 786 (no such link yet).
--  - 2018 -> 2017 (Doodad_Nexus_Elevator_BaseStructure_01 188514, Coldarra (3978.4,7266.5)): starts at the top stop 2018
--    z 255.5, then walks at z 200.1 = the bottom stop 2019 z 200.0. Re-pointed to 2019 -> 2017 (no such link yet).
-- The other 22 "first point off the stop's level" links have no sibling stop on that level: they are boat / zeppelin /
-- gondola / pump-lift origins above their deck (the 2026_10_15_00 Part C class), not floor swaps.
-- Undo: swap the node ids back (1938->786 to 1937->786, 2019->2017 to 2018->2017) and restore nr 0 to the old stop position
-- (1937: 2263.7,-5565.56,33.9385; 2018: 3978.39,7266.48,255.5 - see playerbots_travelnode).
UPDATE `playerbots_travelnode_link` SET `node_id`=1938 WHERE `node_id`=1937 AND `to_node_id`=786;
UPDATE `playerbots_travelnode_path` SET `node_id`=1938 WHERE `node_id`=1937 AND `to_node_id`=786;
UPDATE `playerbots_travelnode_path` SET `x`=2263.26, `y`=-5566.1, `z`=21.1112 WHERE `node_id`=1938 AND `to_node_id`=786 AND `nr`=0;
UPDATE `playerbots_travelnode_link` SET `node_id`=2019 WHERE `node_id`=2018 AND `to_node_id`=2017;
UPDATE `playerbots_travelnode_path` SET `node_id`=2019 WHERE `node_id`=2018 AND `to_node_id`=2017;
UPDATE `playerbots_travelnode_path` SET `x`=3978.39, `y`=7266.48, `z`=200.037 WHERE `node_id`=2019 AND `to_node_id`=2017 AND `nr`=0;
