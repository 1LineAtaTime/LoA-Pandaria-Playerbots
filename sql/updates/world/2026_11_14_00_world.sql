-- Eastern Plaguelands, Fiona's Caravan: the caravan stands at all six stops of its journey, one stop at a time per player,
-- and Argus Highbeacon 45451 stands with it from Crown Guard on.
--
--  * Before: harness 45416 (wagon 45400 and the horses as accessories) and Fiona 45417 stood only at Thondroril River,
--    Crown Guard Tower and Light's Shield Tower; detection 49416 in Light's Shield Tower had no end, so after the second
--    ride every hand-in to Fiona (27457, 27466, 27487, 27522, 27527) and every buff quest happened at Light's Shield, and
--    Argus (the giver of 27532 and a repairer) stood only on top of Crown Guard Tower and was hidden for good after it.
--  * Mechanism (mop's own, extended): every copy wears Generic Quest Invisibility 1 49414 (type 7, amount 1; the harness
--    gives it to itself and its accessories by SmartAI), and the player holds Generic Quest Invisibility Detection 1 49416
--    (type 7, amount 1000) from spell_area in the stop's area only. The windows tile the main chain without gap or overlap:
--      stop 1 Thondroril River 2619    until Onward 27373 is complete
--      stop 2 Crown Guard Tower 2263   27373 complete/rewarded, until The Trek Continues 27448 is complete
--      stop 3 Light's Shield Tower 5448 27448 complete/rewarded, until Boys Will Be Boys 27455 is rewarded
--      stop 4 Eastwall Tower 2271      27455 rewarded, until Nobody to Blame but Myself 27489 is complete
--      stop 5 Northpass Tower 2275     27489 complete/rewarded, until Gidwin's Fate Revealed 27526 is complete
--      stop 6 Light's Hope Chapel 2268 27526 complete/rewarded, no end
--    so in every quest state exactly one Fiona and one wagon are visible, and abandoning a ride quest after its credit puts
--    the player back at the earlier stop with everything in place.
--  * Argus: one copy per stop. His copies at stops 2-5 share the stop's window. In the slaughterhouse (Plaguewood) he stands
--    with Gidwin from 27526 complete until Journey's End 27527 is taken (60921 / 60922, type 9, the same row as the table
--    Gidwin of 2026_11_14_01), and at stop 6 he joins once 27527 is taken (80797 / 85339 See Quest Invis 5, type 13), so he
--    is never in two places at once.
--  * Sourced: the five TDB parking spots of Fiona's Caravan 45423 (Thondroril, Crown Guard, Light's Shield, Eastwall
--    2556.2,-4768.71,106.08, Northpass 3163.43,-4327.63,132.83; both TDBs); Argus' Crown Guard spot 1841.74,-3726.59,195.68
--    o 3.016 (both TDBs; mop had him 20 yd off); the Wowhead pins of Fiona, the wagon and Argus at each stop (Argus at
--    Light's Shield 53.8,53.5, Eastwall 62.2,42.6, Northpass 50.6,20.2, slaughterhouse 28.2,25.8); 49416 from the west bank
--    of the Thondroril (area 2620): the DBC's own stop-1 detection 84893 is limited to 2620. Light's Hope as the end of the
--    road ("the carriage will always be found at Light's Hope Chapel from then on", wiki).
--  * Ours: every window boundary (no source records when retail moved the caravan; 27455 rewarded for Eastwall is on the
--    user's question list), the Light's Hope wagon and Fiona spots (pin 73.6-73.8,52, 8 yd grid), Argus at Light's Hope
--    (no pin: ruling R5), every facing. Heights: terrain + 0.1, or the Guardtower floor (vmap) for Argus in the Light's
--    Shield and Eastwall towers, the slaughterhouse floor for Argus there. Areas computed from the map grid for each spot;
--    none of the three building models (Guardtower, Slaughterhouse) maps to an area of its own (WMOAreaTable nameset 0 ->
--    area 0), so the grid area holds inside too.
--  * Hand-offs: the third ride (27489, Eastwall -> Northpass) is section 2's; this file only gives it the parked caravan at
--    Eastwall and Northpass.

-- Harness (wagon + horses as accessories) and Fiona at stops 4-6, Argus at stops 3-6 and in the slaughterhouse.
DELETE FROM `creature_addon` WHERE `guid` IN (3128000,3128001,3128002,3128003,3128004,3128005,3128006,3128007,3128008,3128009,3128021);
DELETE FROM `creature` WHERE `guid` IN (3128000,3128001,3128002,3128003,3128004,3128005,3128006,3128007,3128008,3128009,3128021);
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3128000,45416,0,139,2271,1,1,0,0,0,0,2556.2,-4768.71,106.076,2.25,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),     -- stop 4 Eastwall, TDB 45423 guid 300067
(3128001,45416,0,139,2275,1,1,0,0,0,0,3163.43,-4327.63,132.83,0,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),        -- stop 5 Northpass, TDB 45423 guid 322599
(3128002,45416,0,139,2268,1,1,0,0,0,0,2303,-5268,83.05,1.6,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),             -- stop 6 Light's Hope (pin 73.6,52)
(3128003,45417,0,139,2271,1,1,0,0,0,0,2559,-4763,106.31,3.9,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),            -- Fiona, stop 4
(3128004,45417,0,139,2275,1,1,0,0,0,0,3166,-4322,132.23,3.9,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),            -- Fiona, stop 5
(3128005,45417,0,139,2268,1,1,0,0,0,0,2306,-5273,82.96,1.6,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),             -- Fiona, stop 6 (pin 73.8,52)
(3128006,45451,0,139,5448,1,1,0,0,0,0,2266,-4466,115.14,1.3,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),            -- Argus, stop 3, tower ground floor (pin 53.8,53.5; floor 115.04)
(3128007,45451,0,139,2271,1,1,0,0,0,0,2559.5,-4806,108.93,1.6,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),          -- Argus, stop 4, tower ground floor (pin 62.2,42.6; floor 108.83)
(3128008,45451,0,139,2275,1,1,0,0,0,0,3170,-4336,133.44,2.4,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),            -- Argus, stop 5 (pin 50.6,20.2)
(3128009,45451,0,139,2268,1,1,0,0,0,0,2298,-5276,81.95,0.6,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),             -- Argus, stop 6 (no pin; ruling R5)
(3128021,45451,0,139,2277,1,1,0,0,0,0,3011,-3424,149.81,4,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0);              -- Argus in the slaughterhouse (pin 28.2,25.8; floor 149.71)
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(3128003,0,0,0,0,0,1,1,0,0,0,0,0,0,'49414'),
(3128004,0,0,0,0,0,1,1,0,0,0,0,0,0,'49414'),
(3128005,0,0,0,0,0,1,1,0,0,0,0,0,0,'49414'),
(3128006,0,0,0,0,0,1,1,0,0,0,0,0,0,'49414'),
(3128007,0,0,0,0,0,1,1,0,0,0,0,0,0,'49414'),
(3128008,0,0,0,0,0,1,1,0,0,0,0,0,0,'49414'),
(3128009,0,0,0,0,0,1,1,0,0,0,0,0,0,'80797'),
(3128021,0,0,0,0,0,1,1,0,0,0,0,0,0,'60921');

-- Argus on Crown Guard Tower: to the spot both TDBs have.
UPDATE `creature` SET `position_x`=1841.74, `position_y`=-3726.59, `position_z`=195.68, `orientation`=3.0161 WHERE `guid`=172780;

-- The stop windows (49416 type 7). Statuses: 1 none, 2 complete, 8 incomplete, 64 rewarded; quest_end_status lists the
-- states of the end quest that still allow the aura.
DELETE FROM `spell_area` WHERE `spell`=49416 AND `area` IN (2619,2620,2263,5448,2271,2275,2268);
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(49416,2619,0,27373,0,0,2,1,64,9),       -- stop 1 Thondroril River (unchanged)
(49416,2620,0,27373,0,0,2,1,64,9),       -- stop 1 seen from the west bank of the river (sourced: 84893 is limited to 2620)
(49416,2263,27373,27448,0,0,2,1,66,9),   -- stop 2 Crown Guard Tower (unchanged)
(49416,5448,27448,27455,0,0,2,1,66,11),  -- stop 3 Light's Shield Tower, now ends when Boys Will Be Boys is handed in (R1)
(49416,2271,27455,27489,0,0,2,1,64,9),   -- stop 4 Eastwall Tower
(49416,2275,27489,27526,0,0,2,1,66,9),   -- stop 5 Northpass Tower
(49416,2268,27526,0,0,0,2,1,66,0);       -- stop 6 Light's Hope Chapel, for good
-- Argus in the slaughterhouse from Gidwin's rescue until Journey's End is taken (shared with the table Gidwin), and at
-- stop 6 from then on.
DELETE FROM `spell_area` WHERE `spell`=60922 AND `area`=2277;
DELETE FROM `spell_area` WHERE `spell`=85339 AND `area`=2268;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(60922,2277,27526,27527,0,0,2,1,66,1),   -- slaughterhouse: Argus 3128021, Gidwin on the table 3129402 (60921, type 9)
(85339,2268,27527,0,0,0,2,1,74,0);       -- Argus at stop 6 3128009 (80797, type 13) once Journey's End is taken
