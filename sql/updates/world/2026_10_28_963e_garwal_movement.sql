-- #963 (2026-10-28): Garwal 24277 (spawn 45211) has MovementType 2 (waypoint) but no creature_addon path and no
-- waypoint_data, so WaypointMovementGenerator::LoadPath logged "doesn't have waypoint path id: 0" at every load
-- (same treatment as the 17 stationary spawns of 2026_10_28_963b_waypoint_paths.sql). Idempotent; undo: MovementType 2.
UPDATE `creature` SET `MovementType` = 0 WHERE `guid` = 45211 AND `id` = 24277 AND `MovementType` = 2;
