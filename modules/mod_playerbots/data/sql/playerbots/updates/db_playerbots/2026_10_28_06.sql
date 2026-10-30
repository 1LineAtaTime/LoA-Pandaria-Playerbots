-- #975 B (2026-10-28): drop the Sister Mercy (pirate boat, transport 187038, Howling Fjord) roam transport links.
--
-- Run mop1k-v10-1028 (1000 bots): Barreno waited at the 187038 dock node 1998 (81.6,-3673.6), was sent to the #948
-- pier point (97.2,-3682.6,0.5) in the water (dock-approach-pier go ... reach=1 swim=1), retried twice while the boat
-- was docked (dock-pier-retry n=1 / n=2 ... docked) and abandoned at 812 s (dock-wait-timeout). Nobody boarded 187038
-- in this run or in mop1k-v3..v9-1028 (no board / ride-arrived / leave line for it).
-- Offline navmesh (5.4.8 mmaps, (not shipped): every Sister Mercy node lies on open
-- water polys; the nearest non-water walkable point to each node is listed below. At the dock node 1998 it is a
-- sea-level shore poly 17 yd away (z 0.6, the same spot the server calls water) and the first point 1.5 yd over the
-- water is 30 yd away on the beach; there is no pier at deck height at either stop (1998, 1992), so no dry stand
-- point from which a bot can walk onto the deck exists.
-- So the 12 type-3 links of object 187038 and their path rows are removed; trips route around the boat. The
-- walk links into the ship nodes stay (AC data). Graph check over all links: no other node loses a reachable node.
-- Nearest dry walkable point per Sister Mercy node (2D distance):
--    1992 (9.7,-4098.0,11.9): 32.4 yd at (14.1,-4130.1,-5.1)
--    1993 (-6.8,-3973.5,0.2): 47.7 yd at (-34.8,-4012.1,1.9)
--    1994 (67.9,-3853.9,0.2): 33.6 yd at (83.5,-3883.7,0.2)
--    1995 (144.5,-3779.4,0.2): 58.3 yd at (166.7,-3833.3,0.8)
--    1996 (241.8,-3739.7,0.2): 22.2 yd at (241.8,-3761.9,0.8)
--    1997 (355.0,-3698.9,0.2): 57.2 yd at (387.7,-3745.9,0.2)
--    1998 (81.6,-3673.6,0.6): 17.3 yd at (96.0,-3683.2,0.6)
-- Apply after 2026_10_28_05.sql. Re-runnable.
-- Undo: (not shipped) (the rows as they were on the test
-- a local DB on 2026-10-28, before this file). Generator: (not shipped)

DELETE FROM `playerbots_travelnode_path` WHERE (`node_id`,`to_node_id`) IN ((1992,1998),(1993,1992),(1993,1998),(1994,1993),(1994,1998),(1995,1994),(1995,1998),(1996,1995),(1996,1998),(1997,1996),(1997,1998),(1998,1997));
DELETE FROM `playerbots_travelnode_link` WHERE `type` = 3 AND `object` = 187038 AND (`node_id`,`to_node_id`) IN ((1992,1998),(1993,1992),(1993,1998),(1994,1993),(1994,1998),(1995,1994),(1995,1998),(1996,1995),(1996,1998),(1997,1996),(1997,1998),(1998,1997));
