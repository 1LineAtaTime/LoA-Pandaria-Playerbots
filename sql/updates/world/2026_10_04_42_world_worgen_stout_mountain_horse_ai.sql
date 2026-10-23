-- Refs #439, #483 (item 3). NEEDS THE BUILD that carries the zone_gilneas.cpp AI npc_stout_mountain_horse (build 6+).
-- Stout Mountain Horse 38765: replace the SmartAI path rows by the C++ ride (both 24677 Lord Hewell -> Livery Outpost,
-- path 387650, and 14465 Duskhaven -> Greymane Manor, path 38765; the `waypoints` rows stay and are read by the AI).
--
-- Evidence: t439 run on build 3/4, cp11: Hewell's gossip and 72772 worked (SMSG_SPELL_GO), the rider sat on the horse
-- (Server.log [PROBE] vehicle=38765) and the horse never moved for 150 s; the run ended 701 yd from the Livery
-- Outpost, i.e. where it started. Vehicle 542's seat 6366 has VEHICLE_SEAT_FLAG_CAN_CONTROL (VehicleSeat.dbc flags
-- 0x4000880B), so the rider charms the horse; the C++ AI keeps its AI under charm, drops client control and drives
-- the ride point by point (same pattern as the working Genn horse in zone_gilneas.cpp).
--
-- Rollback (back to the 2026_10_04_37 SmartAI version):
--   UPDATE `creature_template` SET `ScriptName` = '', `AIName` = 'SmartAI' WHERE `entry` = 38765;
--   then re-run 2026_10_04_37_world_worgen_hewell_horse.sql and, for the 14465 rows it does not create, restore
--   (38765, 0, 0..6) from 2022_08_07_00_gilneas.sql (alexkulya 2d4b1928) with event_param2 = 38765 on rows 1, 2, 3, 5, 6.
-- Re-runnable: UPDATE + DELETE.

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_stout_mountain_horse' WHERE `entry` = 38765;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38765 AND `source_type` = 0;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 38765;
