-- #956 (2026-10-28): the Deeprun Tram transport links name the WotLK subway cars (playerbots DB; beyond AC).
-- The six tram links (type 3, map 369) of the AC graph carry the AC / WotLK gameobject entries 176080-176085 ("Subway").
-- This core has no spawn of those entries: the 5.4.8 world DB spawns the same six cars, at the same positions, as
-- 218203-218208 (Doodad_SubwayCar01..13, Transport.cpp CreateLocal gives them their sniffed parent rotation). So
-- RoamTransport never found the car ("wait-why not-found transport=176085"), and 10 of the 16 dock-wait-timeouts of the
-- 2 h run at 1000 bots were at the Ironforge-side platform (-3.3,-12.6,-4.1) (Astias 6x, Kruregrouth 2x, Eliani 2x).
-- Mapping by spawn position (acore_world.gameobject map 369 vs world.gameobject map 369, test a local DB):
--   176080 (4.58065, 28.2097, 7.01107)  -> 218203 guid 5865   nodes 1945 <-> 1946
--   176081 (4.52807, 8.43529, 7.01107)  -> 218204 guid 5866   nodes 1947 <-> 1948
--   176082 (-45.4005, 2492.79, 6.9886)  -> 218205 guid 5863   nodes 1949 <-> 1950
--   176083 (-45.4007, 2512.15, 6.9886)  -> 218206 guid 16394  nodes 1951 <-> 1952
--   176084 (-45.3934, 2472.93, 6.9886)  -> 218207 guid 5864   nodes 1953 <-> 1954
--   176085 (4.49883, -11.3475, 7.01107) -> 218208 guid 15363  nodes 1955 <-> 1956
-- The stop nodes stay as they are: each pair is the car origin at its two stations, and the 5.4.8 TransportAnimation
-- of the MoP entry ends there (218208: y -2481.65, z -1.03 at 59500-70733 ms, rotated 180 degrees by the parent
-- rotation -> (4.5, 2470.3, 5.98) = node 1956; 218203: y -2482.28, z +0.19 -> node 1945 (4.58, 2510.49, 7.20)).
-- Idempotent: UPDATE by node pair to the absolute MoP entry. Undo: the same UPDATEs back to 176080-176085.
-- TravelNode.cpp (#956) also skips, in roam route searches, any static transport link whose entry has no spawn
-- (boot line ">> Travel node static transport stops: ... not spawned"): expected after this file 0 tram stops among them.

UPDATE `playerbots_travelnode_link` SET `object` = 218203 WHERE `type` = 3 AND ((`node_id` = 1945 AND `to_node_id` = 1946) OR (`node_id` = 1946 AND `to_node_id` = 1945));  -- was 176080
UPDATE `playerbots_travelnode_link` SET `object` = 218204 WHERE `type` = 3 AND ((`node_id` = 1947 AND `to_node_id` = 1948) OR (`node_id` = 1948 AND `to_node_id` = 1947));  -- was 176081
UPDATE `playerbots_travelnode_link` SET `object` = 218205 WHERE `type` = 3 AND ((`node_id` = 1949 AND `to_node_id` = 1950) OR (`node_id` = 1950 AND `to_node_id` = 1949));  -- was 176082
UPDATE `playerbots_travelnode_link` SET `object` = 218206 WHERE `type` = 3 AND ((`node_id` = 1951 AND `to_node_id` = 1952) OR (`node_id` = 1952 AND `to_node_id` = 1951));  -- was 176083
UPDATE `playerbots_travelnode_link` SET `object` = 218207 WHERE `type` = 3 AND ((`node_id` = 1953 AND `to_node_id` = 1954) OR (`node_id` = 1954 AND `to_node_id` = 1953));  -- was 176084
UPDATE `playerbots_travelnode_link` SET `object` = 218208 WHERE `type` = 3 AND ((`node_id` = 1955 AND `to_node_id` = 1956) OR (`node_id` = 1956 AND `to_node_id` = 1955));  -- was 176085
