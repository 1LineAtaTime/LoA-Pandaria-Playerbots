-- Arathi Highlands: put the Witherbark war party west of Hammerfall where every sniff has it
-- The Cataclysm Classic TDB 4.4.2, the retail TDB 1210, TCPP's 4.3.4 sniff file and Legion Core all have one group of
-- three Witherbark Trolls, a Witherbark Shadowcaster and a Witherbark Headhunter at about -1225 -3531, standing,
-- always up, in no game event. mop had the same five 75 yd east at about -1155 -3557, four of them in game event 43
-- "Hammerfall Under Attack" (ended 2020-12-30, so they never spawned; no source has that event).
-- Move the five onto the TDB spots with the TDB's facing, equipment, 120 s respawn, no wander, and take the four out
-- of event 43. Ours: which mop guid goes on which TDB spot of the same entry. The event's three trolls near the gate
-- (82836, 82866, 82883) are left in it.
UPDATE `creature` SET `position_x`=-1228.61, `position_y`=-3534.62, `position_z`=46.2944, `orientation`=6.12611, `equipment_id`=1, `phaseMask`=1, `areaId`=321, `spawntimesecs`=120, `wander_distance`=0, `MovementType`=0 WHERE `guid`=161462 AND `id`=2556;
UPDATE `creature` SET `position_x`=-1228.29, `position_y`=-3532.14, `position_z`=45.8627, `orientation`=5.98648, `equipment_id`=1, `phaseMask`=1, `spawntimesecs`=120, `wander_distance`=0, `MovementType`=0 WHERE `guid`=161463 AND `id`=2553;
UPDATE `creature` SET `position_x`=-1226.78, `position_y`=-3531.17, `position_z`=45.6478, `orientation`=5.89921, `phaseMask`=1, `spawntimesecs`=120, `wander_distance`=0, `MovementType`=0 WHERE `guid`=161464 AND `id`=2552;
UPDATE `creature` SET `position_x`=-1222.76, `position_y`=-3530.78, `position_z`=45.4764, `orientation`=5.74213, `phaseMask`=1, `spawntimesecs`=120, `wander_distance`=0, `MovementType`=0 WHERE `guid`=161465 AND `id`=2552;
UPDATE `creature` SET `position_x`=-1224.47, `position_y`=-3533.49, `position_z`=46.0107, `orientation`=5.98648, `phaseMask`=1, `spawntimesecs`=120, `wander_distance`=0, `MovementType`=0 WHERE `guid`=161466 AND `id`=2552;
DELETE FROM `game_event_creature` WHERE `eventEntry`=43 AND `guid` IN (161463,161464,161465,161466);
