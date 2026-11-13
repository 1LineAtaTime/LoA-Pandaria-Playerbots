-- #963 (2026-10-28): spawns with MovementType 2 (waypoint) but no path in creature_addon / creature_template_addon logged
-- "WaypointMovementGenerator::LoadPath: creature X doesn't have waypoint path id: 0" whenever their grid loaded (one at a
-- time in every run: Garwal, Valorcall, Putridus, Gjalerhorn Scavenger 399980 ...). Bulk pass over the whole table:
-- * 98271 (entry 36830) has its 15 waypoint_data rows under its own guid but no creature_addon path: link them.
-- * the other 46 have no waypoint data anywhere: stationary (MovementType 0), as 963b/963e/963i did one by one.
-- Idempotent. Undo: MovementType 2 for the listed guids; path_id 0 / delete the 98271 addon row.
INSERT INTO `creature_addon` (`guid`, `path_id`) VALUES (98271, 98271) ON DUPLICATE KEY UPDATE `path_id` = 98271;
UPDATE `creature` SET `MovementType` = 0 WHERE `MovementType` = 2 AND `guid` IN (7058,47510,48298,48338,51974,51982,52032,94392,137620,300017,300020,345424,345538,345540,345541,345542,345543,345560,345561,345562,345563,345564,345565,345566,345568,345569,345570,345571,345572,345573,345574,345643,345644,345645,345652,345656,345669,345676,345677,345678,345685,399980,570891,570910,571055,571056);
