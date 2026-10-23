-- #847 (lane Z, 2026-10-21): make the stored travel graph what TravelNodeMap::loadNodeStore keeps in memory (playerbots DB;
-- beyond AC - the AC graph is its own 3.3.5 data).
--
-- W86 boot: >> Loaded 3913 travelNodes, >> Loaded 15757 travelNode paths, >> Travel node graph in memory: 3909 nodes,
-- 15749 links, 110 isolated. Root cause (an offline replay of the loader over the test DB gives 3909 / 15749 / 110 exactly):
--  - loadNodeStore adds every row with addNode(pos, name, true) (TravelNode.cpp:2474; AC TravelNode.cpp:2278 the same), and
--    addNode's checkDuplicate default (TravelNode.h:488) returns an EXISTING node within 5 yd (3D, same map) instead of a new one.
--    Lane X phase 1 moved four instance-entrance nodes onto the 5.4.8 navmesh, within 5 yd of their exit twin, so the twin (the
--    higher id) is folded into the entrance node: 4 nodes. The walk link between each pair becomes a self link, which
--    TravelNode::setPathTo drops (TravelNode.h:264, this != node): 8 links. Every other link of a twin already lands on the
--    entrance node in memory (no pair collides).
--  - 110 isolated (no outgoing link): 104 are the AC data itself (the AC acore_playerbots rows replay to 104 the same way: lone
--    boss / entrance nodes on instance maps, 532 x27, 603 x7, 578 x6 ...); +2 Tempest Keep exits 1787 / 1789 (map 530: lane X
--    dropped their walk twins 1541 / 1542 - no navmesh, the fortress is flying-only; they keep their inbound instance-exit portal
--    links); +4 ferry stops 1959 1960 1965 1966 (Moonspray 176244 / Feathermoon Ferry 177233 have no transports row on 5.4.8;
--    2026_10_15_00.sql deleted all their link and path rows and left the node rows).
-- Fix (the merge changes nothing in memory - it is what the loader already builds; the ferry rows are dead data):
--  - each twin is merged into its entrance node: the pair's walk links (and their path rows) are deleted, every other link and
--    path row of the twin is re-pointed to the entrance node, the twin's node row is deleted;
--  - the 4 orphan ferry stop rows are deleted (no link or path row refers to them; the DELETE re-checks that).
-- Expected boot after it: >> Loaded 3905 travelNodes, >> Loaded 15749 travelNode paths, in memory: 3905 nodes, 15749 links,
--   106 isolated (104 AC-identical + the 2 Tempest Keep exits); stored totals == in-memory totals.
-- Apply after 2026_10_21_laneX_847_phase3.sql. Every remaining row stays linked / calculated = 1 (no regeneration).
-- Undo: the matching undo script (restores every touched node, link and path row with
--   the exact stored float values of the test DB on 2026-10-21, after lane X phase 3).

-- 1677 -> 1403: Stormwind Stockade exit -> Stormwind Stockade entrance (4.68 yd apart)
-- 1689 -> 1435: Stormwind City portal (arrival of portal 1434) -> Stormwind City portal (departure to 1690) (4.23 yd apart)
-- 1691 -> 1437: Uldaman exit -> Uldaman entrance (2.85 yd apart)
-- 1841 -> 1609: Gundrak exit -> Gundrak entrance (3.67 yd apart)
DELETE FROM `playerbots_travelnode_link` WHERE ((`node_id`=1403 AND `to_node_id`=1677) OR (`node_id`=1435 AND `to_node_id`=1689) OR (`node_id`=1437 AND `to_node_id`=1691) OR (`node_id`=1609 AND `to_node_id`=1841) OR (`node_id`=1677 AND `to_node_id`=1403) OR (`node_id`=1689 AND `to_node_id`=1435) OR (`node_id`=1691 AND `to_node_id`=1437) OR (`node_id`=1841 AND `to_node_id`=1609));
DELETE FROM `playerbots_travelnode_path` WHERE ((`node_id`=1403 AND `to_node_id`=1677) OR (`node_id`=1435 AND `to_node_id`=1689) OR (`node_id`=1437 AND `to_node_id`=1691) OR (`node_id`=1609 AND `to_node_id`=1841) OR (`node_id`=1677 AND `to_node_id`=1403) OR (`node_id`=1689 AND `to_node_id`=1435) OR (`node_id`=1691 AND `to_node_id`=1437) OR (`node_id`=1841 AND `to_node_id`=1609));
UPDATE `playerbots_travelnode_link` SET `node_id`=1403 WHERE `node_id`=1677;
UPDATE `playerbots_travelnode_link` SET `to_node_id`=1403 WHERE `to_node_id`=1677;
UPDATE `playerbots_travelnode_path` SET `node_id`=1403 WHERE `node_id`=1677;
UPDATE `playerbots_travelnode_path` SET `to_node_id`=1403 WHERE `to_node_id`=1677;
UPDATE `playerbots_travelnode_link` SET `node_id`=1435 WHERE `node_id`=1689;
UPDATE `playerbots_travelnode_link` SET `to_node_id`=1435 WHERE `to_node_id`=1689;
UPDATE `playerbots_travelnode_path` SET `node_id`=1435 WHERE `node_id`=1689;
UPDATE `playerbots_travelnode_path` SET `to_node_id`=1435 WHERE `to_node_id`=1689;
UPDATE `playerbots_travelnode_link` SET `node_id`=1437 WHERE `node_id`=1691;
UPDATE `playerbots_travelnode_link` SET `to_node_id`=1437 WHERE `to_node_id`=1691;
UPDATE `playerbots_travelnode_path` SET `node_id`=1437 WHERE `node_id`=1691;
UPDATE `playerbots_travelnode_path` SET `to_node_id`=1437 WHERE `to_node_id`=1691;
UPDATE `playerbots_travelnode_link` SET `node_id`=1609 WHERE `node_id`=1841;
UPDATE `playerbots_travelnode_link` SET `to_node_id`=1609 WHERE `to_node_id`=1841;
UPDATE `playerbots_travelnode_path` SET `node_id`=1609 WHERE `node_id`=1841;
UPDATE `playerbots_travelnode_path` SET `to_node_id`=1609 WHERE `to_node_id`=1841;
DELETE FROM `playerbots_travelnode` WHERE `id` IN (1677,1689,1691,1841);

-- orphan ferry stops (Moonspray 176244: 1959 1960; Feathermoon Ferry 177233: 1965 1966)
DELETE FROM `playerbots_travelnode` WHERE `id` IN (1959,1960,1965,1966) AND `id` NOT IN (SELECT `node_id` FROM `playerbots_travelnode_link`) AND `id` NOT IN (SELECT `to_node_id` FROM `playerbots_travelnode_link`);
