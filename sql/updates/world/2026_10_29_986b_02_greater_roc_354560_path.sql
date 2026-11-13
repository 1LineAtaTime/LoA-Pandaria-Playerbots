-- #963 / #986b: Greater Roc 51726 spawn 354560 sits in Feralas (-5638, 321, 217) but its reference path 354560 (ported in 963w) has
-- 4 points in Desolace (-1717,1490 .. -1679,1447), ~3,900 yd away. Live test v19: the roc flew ~130 yd toward point 1, then stopped
-- (it never patrols). The path belongs to another location (sniff mix-up in the source data); the other Greater Roc spawns in
-- Feralas stand still (MovementType 0) or have a local path (354383). Make 354560 stand like its siblings.
-- Undo: backup in (not shipped) (re-insert waypoint_data rows, set MovementType 2 / path_id 354560).
UPDATE `creature` SET `MovementType` = 0 WHERE `guid` = 354560 AND `id` = 51726;
UPDATE `creature_addon` SET `path_id` = 0 WHERE `guid` = 354560;
DELETE FROM `waypoint_data` WHERE `id` = 354560;
