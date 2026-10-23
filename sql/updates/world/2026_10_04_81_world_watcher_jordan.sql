-- Epic #441 (alexkulya audit rows WU012); staged 2026-09-23,
-- re-checked 2026-10-04 against the LoA base world dump + sql/updates and LoA SmartAI enums.
-- LoA port: Watcher Jordan (entry 887, Duskwood) - this named NPC had 3 spawns in LoA (170441 random-move, 170824 and
-- 170926 on short paths). Keep one (170824) on an 18-point patrol with aura 18950 (Invisibility and Stealth Detection).
-- Source: alexkulya ref commit b62e92a, sql/updates/world/2026_08_24_00_world_watcher_jordan.sql.
-- Adaptations for LoA:
--  * creature.movement_type -> creature.MovementType (UPDATE additionally guarded by `id` = 887).
--  * creature_addon bytes1 0 / bytes2 4097 -> SheathState 1, PvPFlags 16.
--  * guids 170441/170824/170926 verified: all id 887 in LoA world with the same positions as ak_world; no pool/formation/
--    game_event references.
-- Re-runnable: every INSERT is preceded by a DELETE of the same keys.

DELETE FROM `creature` WHERE `guid` IN (170441, 170926) AND `id` = 887;
UPDATE `creature` SET `position_x` = -10624.589, `position_y` = -1185.5638, `position_z` = 28.968943, `wander_distance` = 0, `MovementType` = 2 WHERE `guid` = 170824 AND `id` = 887;

DELETE FROM `creature_template_addon` WHERE `entry` = 887;
DELETE FROM `creature_addon` WHERE `guid` IN (170824, 170441, 170926);
INSERT INTO `creature_addon` (`guid`, `path_id`, `mount`, `SheathState`, `PvPFlags`, `emote`, `auras`) VALUES
(170824, 170824*10, 0, 1, 16, 0, '18950');

DELETE FROM `waypoint_data` WHERE `id` IN (170824*10, 170824, 170441, 170926);
INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_flag`) VALUES
(170824*10, 1, -10624.589, -1185.5638, 28.968943, 0, 0, 0),
(170824*10, 2, -10648.632, -1192.0426, 28.565548, 0, 0, 0),
(170824*10, 3, -10683.194, -1193.247, 27.458822, 0, 0, 0),
(170824*10, 4, -10709.024, -1176.6018, 26.58296, 0, 0, 0),
(170824*10, 5, -10736.007, -1159.654, 26.561907, 0, 0, 0),
(170824*10, 6, -10755.747, -1144.1693, 26.936907, 0, 0, 0),
(170824*10, 7, -10768.78, -1129.4673, 27.876637, 0, 0, 0),
(170824*10, 8, -10779.995, -1110.3231, 30.857838, 0, 0, 0),
(170824*10, 9, -10784.427, -1099.5131, 33.14368, 0, 0, 0),
(170824*10, 10, -10787.088, -1079.9607, 36.6554, 0, 0, 0),
(170824*10, 11, -10784.427, -1099.5131, 33.14368, 0, 0, 0),
(170824*10, 12, -10779.995, -1110.3231, 30.857838, 0, 0, 0),
(170824*10, 13, -10768.78, -1129.4673, 27.876637, 0, 0, 0),
(170824*10, 14, -10755.747, -1144.1693, 26.936907, 0, 0, 0),
(170824*10, 15, -10736.007, -1159.654, 26.561907, 0, 0, 0),
(170824*10, 16, -10709.024, -1176.6018, 26.58296, 0, 0, 0),
(170824*10, 17, -10683.194, -1193.247, 27.458822, 0, 0, 0),
(170824*10, 18, -10648.632, -1192.0426, 28.565548, 0, 0, 0);
