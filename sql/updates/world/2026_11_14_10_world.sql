-- Eastern Plaguelands, Fiona's Caravan: the first two rides (Onward 27373, The Trek Continues 27448) are driven by a script
-- (zone_eastern_plaguelands_caravan.cpp) instead of SmartAI.
--
--  * Before: the parked wagon 45400 had the player cast the ride's summon (84897 / 85066); the private harness 45436 / 45542
--    sat him in the wagon 45423 / 45437 and drove the `waypoints` path; the wagon's "passenger removed" handler looked the
--    harness up as the nearest creature, which never finds a private summon, so a player who jumped off was still credited on
--    arrival, and every moving caravan (wagon, horses, Fiona, Gidwin, Tarenar, Argus) stayed parked at the end of its road for
--    ever. A second click summoned a second caravan; the parked caravan and the party stayed in view of the rider.
--  * Now: npc_epl_caravan_wagon (45400) answers options 0 and 1 of menu 12035, refuses a player already riding or with a ride
--    out, and takes the stop's detections from the rider for the ride; npc_epl_caravan_harness (45436, 45542) seats him, drives
--    the same paths, has the party speak the same lines at the same times (and Fiona laugh twice on the first ride, wiki), and
--    credits 45400, sets him down and goes at the end or as soon as he leaves the wagon, dies or logs out.
--  * Sourced: the spells, entries, seats, accessories and paths are mop's own (not in either TDB) and unchanged; the lines and
--    Fiona's laughs from the wiki's notes of 27373 / 27448. Ours: the script's timing around boarding and arrival.
--  * The wagons 45423 / 45437 lose their SmartAI and run on VehicleAI (they only carry the party).

UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_epl_caravan_wagon' WHERE `entry`=45400;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_epl_caravan_harness' WHERE `entry` IN (45436,45542);
UPDATE `creature_template` SET `AIName`='', `ScriptName`='' WHERE `entry` IN (45423,45437);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (45400,45423,45436,45437,45542) AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (4543600,4554200) AND `source_type`=9;
