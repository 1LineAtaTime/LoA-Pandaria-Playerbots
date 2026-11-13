-- #955 (2026-10-28): flight-master / innkeeper travel nodes that still stand at WotLK positions (playerbots DB).
-- Audit (test a local DB, 404 flightMaster / innkeeper nodes): each node was compared with the nearest MoP
-- world.creature spawn of an NPC carrying the same npcflag (template | spawn, 0x2000 flight master / 0x10000
-- innkeeper; the same flags TravelNodeMap::generateNpcNodes uses, TravelNode.cpp:1763). 339 nodes are within 5 yd,
-- 19 within 40 yd (left alone), 46 further away; those 46 are handled below.
--  * MOVE (20 nodes -> 19 after the collision rule): no other node stands at that NPC; the node takes the spawn
--    x / y / z. Nodes that moved more than 300 yd (FM only) also lose their walk links (type 1, both directions):
--    the stored walk paths end at the old position and would send bots on an off-graph hike; their flight (type 4)
--    links stay, so the flight network is untouched. Re-walk those nodes by regenerating the links later.
--  * DELETE (25 nodes): another node already stands at the NPC the node stood for (duplicate of a correct node, or a
--    start-zone innkeeper node whose NPC no longer exists nearby). The node, every link to / from it and every path
--    row to / from it go together (TravelNodeMap::loadNodeStore dereferences saveNodes.find() without a check, so a
--    link row pointing at a deleted node must never remain). Checked offline on the full graph: removing these nodes
--    and the dropped walk links creates no new isolated node and splits no connected component.
--  * Left alone on purpose: 620 Camp Narache innkeeper and 629 Shadowglen innkeeper (the only connectors of the
--    start nodes 62 / 3), and the ship nodes 1962 / 1968 (see the #955 report).
-- Every statement is an absolute-value UPDATE or a DELETE by id, so the file can be re-run safely.

-- ---- MOVE ----
UPDATE `playerbots_travelnode` SET `x` = -51.95, `y` = -967.899, `z` = 53.535 WHERE `id` = 10; -- Tarren Mill innkeeper: (-6, -942) -> Innkeeper Shay (53 yd)
UPDATE `playerbots_travelnode` SET `x` = 2990.2, `y` = -4164.11, `z` = 101.422 WHERE `id` = 39; -- Talrendis Point flightMaster: (2718, -3881) -> Kroum (393 yd)
UPDATE `playerbots_travelnode` SET `x` = 471.135, `y` = 1500.55, `z` = 135.605 WHERE `id` = 86; -- The Sepulcher innkeeper: (511, 1636) -> Innkeeper Bates (142 yd)
UPDATE `playerbots_travelnode` SET `x` = 2285.95, `y` = 282.613, `z` = 33.334 WHERE `id` = 143; -- Brill innkeeper: (2235, 252) -> Innkeeper Renee (60 yd)
UPDATE `playerbots_travelnode` SET `x` = 3349.5, `y` = 1056.16, `z` = 3.032 WHERE `id` = 176; -- Zoramgar Outpost flightMaster: (3374, 994) -> Andruk (66 yd)
UPDATE `playerbots_travelnode` SET `x` = 2147.09, `y` = 1535.16, `z` = 341.322 WHERE `id` = 211; -- Stonetalon Peak flightMaster: (2683, 1466) -> Teloren (540 yd)
UPDATE `playerbots_travelnode` SET `x` = -10553.6, `y` = 1034.42, `z` = 57.039 WHERE `id` = 262; -- Sentinel Hill flightMaster: (-10628, 1037) -> Thor (75 yd)
UPDATE `playerbots_travelnode` SET `x` = -6960.24, `y` = -1098.88, `z` = -272.567 WHERE `id` = 271; -- Marshals Refuge flightMaster: (-6111, -1140) -> Flizzy Coilspanner (851 yd)
UPDATE `playerbots_travelnode` SET `x` = -7098.42, `y` = -3816.02, `z` = 9.573 WHERE `id` = 303; -- Gadgetzan flightMaster: (-7045, -3779) -> Bulkrek Ragefist (65 yd)
UPDATE `playerbots_travelnode` SET `x` = -7188.6, `y` = -3766.39, `z` = 9.249 WHERE `id` = 304; -- Gadgetzan flightMaster: (-7225, -3738) -> Bera Stonehammer (46 yd)
UPDATE `playerbots_travelnode` SET `x` = -2148.18, `y` = -1723.44, `z` = 93.074 WHERE `id` = 308; -- Camp Taurajo flightMaster: (-2384, -1881) -> Lognah (284 yd)
UPDATE `playerbots_travelnode` SET `x` = -665.03, `y` = -533.721, `z` = 28.297 WHERE `id` = 403; -- Southshore flightMaster: (-715, -512) -> Darla Harris (55 yd)
UPDATE `playerbots_travelnode` SET `x` = 1939.44, `y` = -2694.39, `z` = 62.147 WHERE `id` = 416; -- Thondroril River flightMaster: (1942, -2561) -> Frax Bucketdrop (133 yd)
UPDATE `playerbots_travelnode` SET `x` = 1573.27, `y` = -4439.16, `z` = 16.14 WHERE `id` = 426; -- Orgrimmar innkeeper: (1634, -4439) -> Innkeeper Gryshka (61 yd)
UPDATE `playerbots_travelnode` SET `x` = -3865.31, `y` = -11642.2, `z` = -137.566 WHERE `id` = 451; -- The Exodar flightMaster: (-4057, -11789) -> Stephanos (241 yd)
UPDATE `playerbots_travelnode` SET `x` = 320.059, `y` = -2119.22, `z` = 121.922 WHERE `id` = 605; -- Wildhammer Keep innkeeper: (400, -2120) -> Innkeeper Thulfram (80 yd)
UPDATE `playerbots_travelnode` SET `x` = 6215.3, `y` = -1871.17, `z` = 566.083 WHERE `id` = 645; -- Talonbranch Glade  flightMaster: (6204, -1951) -> Mishellena (81 yd)
UPDATE `playerbots_travelnode` SET `x` = 4969.77, `y` = 145.836, `z` = 51.948 WHERE `id` = 646; -- Bloodvenom Post  flightMaster: (5065, -339) -> Delanea (494 yd)
UPDATE `playerbots_travelnode` SET `x` = -4467.67, `y` = 2191.13, `z` = 17.979 WHERE `id` = 669; -- Feathermoon Stronghold flightMaster: (-4371, 3340) -> Irela Moonfeather (1153 yd)

-- ---- walk links dropped for FM nodes that moved > 300 yd (flight / transport links kept) ----
DELETE FROM `playerbots_travelnode_path` WHERE (`node_id` IN (39,211,271,646,669) OR `to_node_id` IN (39,211,271,646,669)) AND (`node_id`,`to_node_id`) IN (SELECT `node_id`,`to_node_id` FROM `playerbots_travelnode_link` WHERE `type` = 1);
DELETE FROM `playerbots_travelnode_link` WHERE (`node_id` IN (39,211,271,646,669) OR `to_node_id` IN (39,211,271,646,669)) AND `type` = 1;

-- ---- DELETE ----
--  21 Valormok flightMaster (dup of [4013])
--  78 Stormwind City innkeeper (dup of [382])
--  177 Thunder Bluff innkeeper (dup of [257])
--  198 Stonetalon Peak innkeeper (inn, NPC 621yd away, uncovered)
--  229 Valley of Trials innkeeper (dup of [388])
--  244 Dun Morogh innkeeper (dup of [46])
--  273 Caverns of Time innkeeper (inn, NPC 952yd away, uncovered)
--  328 Freewind Post flightMaster (dup of [310])
--  329 Freewind Post innkeeper (inn, NPC 1109yd away, uncovered)
--  404 Camp Taurajo innkeeper (inn, NPC 855yd away, uncovered)
--  419 Southshore innkeeper (inn, NPC 569yd away, uncovered)
--  456 Silvermoon City innkeeper (dup of [40])
--  458 The Exodar innkeeper (dup of [535])
--  460 Coldridge Valley innkeeper (dup of [31])
--  478 Brill innkeeper (collides with moved 143 (same spawn))
--  599 Lights Hope Chapel flightMaster (dup of [598])
--  612 Deathknell innkeeper (inn, NPC 796yd away, uncovered)
--  614 Undercity innkeeper (dup of [591])
--  618 Sunstrider Isle innkeeper (dup of [428])
--  621 Razor Hill innkeeper (dup of [418])
--  623 Kharanos innkeeper (dup of [31])
--  624 Northshire Valley innkeeper (dup of [525])
--  626 Crash Site innkeeper (dup of [536])
--  627 Warriors Terrace innkeeper (dup of [192])
--  628 Dolanaar innkeeper (dup of [664])
DELETE FROM `playerbots_travelnode_path` WHERE `node_id` IN (21,78,177,198,229,244,273,328,329,404,419,456,458,460,478,599,612,614,618,621,623,624,626,627,628) OR `to_node_id` IN (21,78,177,198,229,244,273,328,329,404,419,456,458,460,478,599,612,614,618,621,623,624,626,627,628);
DELETE FROM `playerbots_travelnode_link` WHERE `node_id` IN (21,78,177,198,229,244,273,328,329,404,419,456,458,460,478,599,612,614,618,621,623,624,626,627,628) OR `to_node_id` IN (21,78,177,198,229,244,273,328,329,404,419,456,458,460,478,599,612,614,618,621,623,624,626,627,628);
DELETE FROM `playerbots_travelnode` WHERE `id` IN (21,78,177,198,229,244,273,328,329,404,419,456,458,460,478,599,612,614,618,621,623,624,626,627,628);
