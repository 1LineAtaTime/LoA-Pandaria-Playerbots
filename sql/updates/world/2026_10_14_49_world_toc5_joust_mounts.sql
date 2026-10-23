-- #561 Trial of the Champion (650) joust mounts: the Alliance Argent Warhorse 35644 was faction 14 (hostile),
-- NOT_SELECTABLE|IMMUNE_TO_PC and VehicleId 529 (seat 6126 has no CAN_CONTROL/CAN_CAST), so no Alliance player or bot could
-- ride and joust; the Horde Battleworg 36558 was faction 35 with unit_flags 16777216. AC acore_world (the WotLK spec):
-- 35644 faction 1 / unit_flags 2 / VehicleId 486, 36558 faction 2 / unit_flags 2 / VehicleId 486 (seat 5406 CAN_CAST).
-- Undo: 35644 faction=14, unit_flags=33554688, VehicleId=529; 36558 faction=35, unit_flags=16777216.
UPDATE `creature_template` SET `faction`=1, `unit_flags`=2, `VehicleId`=486 WHERE `entry`=35644;
UPDATE `creature_template` SET `faction`=2, `unit_flags`=2 WHERE `entry`=36558;
