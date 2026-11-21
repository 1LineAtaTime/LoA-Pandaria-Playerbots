-- Eastern Plaguelands, Fiona's Caravan: Gidwin Goldbraids and Tarenar Sunstrike stand where the story has them, one copy
-- of each at a time.
--
--  * Before: Gidwin and Tarenar never came back to the wagon at Thondroril River after their quests; nobody stood by the
--    statue of Light's Hope (Tarenar), the chapel, Eastwall Tower, Northpass Tower, the Plaguewood fork or the
--    slaughterhouse; the one Tarenar 45729 (369660) and Gidwin 45730 (369661) stood at the Light's Hope caravan spot with
--    no aura, visible from a character's first visit, and gave or took 27466-27527 there instead of along the road.
--  * Who stands where, in quest states (status 1 none, 2 complete, 8 incomplete, 64 rewarded):
--      Gidwin 45431 at the wagon, river camp      82343/85311 type 12  Greasing the Wheel 27369 rewarded .. Onward 27373 complete
--      Tarenar 45429 at the wagon, river camp     80797/85339 type 13  A Gift For Fiona 27372 rewarded .. 27373 complete
--      Tarenar 45429, Marris Stead (exists)       49414/49416 type 7   .. 27372 rewarded, hidden while his 27371 follower
--                                                                      84902 is out (deaths-step-crown-guard's aura_spell)
--      statue pair: Gidwin 45431 (exists) and     49415/49417 type 8   The Trek Continues 27448 complete .. A Boyhood Dream 27463
--      Tarenar 46178                                                   taken (their followers take over; back on an abandon)
--      chapel pair: Tarenar 45729 (exists, moved  60921/85077, later 60922, type 9   The Noxious Glade 27465 rewarded .. Northdale 27466 taken,
--      in) and Gidwin 45730                                            and again from Journey's End 27527 rewarded, for good;
--                                                                      the first window also closes for good once the third
--                                                                      ride is done (aura -85315, the zone-wide Northpass
--                                                                      light detection of 2026_11_14_06, 27489 complete), so
--                                                                      a player who abandons Northdale later re-takes it
--                                                                      from the Tarenar he is travelling with, not from a
--                                                                      second Tarenar in the chapel
--      Tarenar 45729 at Eastwall                  60921/60922 type 9   27466 taken .. 27489 complete, hidden while his 27487
--                                                                      follower 85405 is out (eastwall-northpass's aura_spell)
--      Gidwin 45730 in the Eastwall tower         49415/84909 type 8   27466 taken .. Ix'lar 27487 taken ("While you and Tarenar
--                                                                      were out, Giddy ran north"), only while the caravan
--                                                                      is at Eastwall (aura 49416), so a player who skips
--                                                                      27487 does not find him there for ever
--      Tarenar 45729 at Northpass                 49415/49417 type 8   27489 complete .. Blind Fury 27524 taken
--      Tarenar 45729 at the Plaguewood fork       49414/49416 type 7   27524 taken .. Gidwin's Fate Revealed 27526 complete
--      Tarenar 45729 in the slaughterhouse        49415/49417 type 8   27526 complete .. 27527 rewarded (takes Guardians of
--                                                                      Stratholme 27525 if still open)
--      Gidwin 45730 on the slaughterhouse table   60921/60922 type 9   27526 complete .. 27527 taken (row in 2026_11_14_00,
--                                                                      shared with Argus)
--      Gidwin 45730 at the Light's Hope wagon     82343/85311 type 12  while 27527 is in the log ("jumping up and down in front
--      (exists)                                                        of Fiona" at the hand-in)
--    So every quest these two give or take has one visible giver and one visible ender in every state, and neither is
--    ever shown twice.
--  * Rows that hang on another aura (aura_spell) use a detection spell no other row uses: Aura::HandleAuraSpecificMods
--    removes a row's spell everywhere when that aura lands and the row does not fit, even if another row of the same spell
--    fits, and on entering the zone the area pass runs before the zone pass (Player::UpdateZone), so a zone-wide aura
--    cast last would strip a shared detection until the next area change. Hence the tower Gidwin on See Quest Invis 2
--    84909 (type 8) and the chapel pair's first window on See Quest Invis 3 85077 (type 9), both used by nothing else.
--  * Sourced: the Wowhead pins (Gidwin and Tarenar at the Thondroril caravan 9.0,66.4; statue 74.2,53.2; chapel 75.6,52.2;
--    Eastwall 61.4-61.6,43.2 and tower 62.2,42.4; Northpass 50.4,20.2; road 27.4-27.6,21.2; slaughterhouse 28.4-28.6,26;
--    Light's Hope caravan 73.8,52.2); the table spot 3008.63,-3434.42,151.157 o 2.857 (TDB spell_target_position of 85649);
--    Gidwin dancing on the table and jumping at the wagon, the second Gidwin in the chapel facing where the hearthstone lands
--    (comments, plaguewood-finale dossier); Tarenar beside the statue (wiki 27455); the tower Gidwin pacing (wiki 27466).
--  * Ours: every window boundary; that the slaughterhouse Gidwin leaves when Journey's End is taken and the chapel pair
--    returns only once it is handed in (retail showed a table Gidwin, a wagon Gidwin and a chapel Gidwin at once; here one
--    at a time, so an abandoned 27527 is taken again on the table, not in the chapel); statue Tarenar's entry 46178 (the only
--    flagless pre-tabard Tarenar, spawned by nothing in either TDB); positions from 8 yd pins, every facing. Heights:
--    terrain + 0.1, or the model floor (vmap) inside the Eastwall tower (108.83), the chapel (90.75) and the slaughterhouse
--    (149.71). Areas computed from the map grid; Guardtower, Redridgechapel and Slaughterhouse map to no area of their own.

DELETE FROM `creature_addon` WHERE `guid` IN (3128010,3128011,3128012,3128013,3128014,3128018,3128019,3129400,3129401,3129402,373696,369660,369661);
DELETE FROM `creature` WHERE `guid` IN (3128010,3128011,3128012,3128013,3128014,3128018,3128019,3129400,3129401,3129402);
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3128010,45431,0,139,2619,1,1,0,0,0,1,1921,-2646,59.64,2.5,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),            -- Gidwin back at the river camp (pin 9.0,66.4)
(3128011,45429,0,139,2619,1,1,0,0,0,1,1911,-2649,61.34,0.9,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),            -- Tarenar back at the river camp (pin 9.0,66.4)
(3128012,46178,0,139,2268,1,1,0,0,0,1,2266,-5304,82.27,4,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),              -- Tarenar beside the statue
(3128013,45729,0,139,2271,1,1,0,0,0,1,2543.3,-4777.6,106.79,0.6,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),       -- Tarenar at Eastwall (pins 61.4-61.6,43.2; tower base 106.69)
(3128014,45730,0,139,2271,1,1,0,0,0,1,2564.9,-4806,108.93,3.1,300,0,3,0,1,0,1,0,0,0,0,0,'',0,0),         -- Gidwin pacing in the Eastwall tower (pin 62.2,42.4; floor 108.83)
(3128018,45729,0,139,2275,1,1,0,0,0,1,3158,-4333,132.89,0.8,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),           -- Tarenar at Northpass (pin 50.4,20.2)
(3128019,45730,0,139,2268,1,1,0,0,0,1,2297,-5343,90.85,0.4,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),            -- Gidwin in the chapel (pin 75.6,52.2; floor 90.75)
(3129400,45729,0,139,2277,1,1,0,0,0,1,3138.5,-3397.5,140.02,0,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),         -- Tarenar at the Plaguewood fork (pins 27.4-27.6,21.2)
(3129401,45729,0,139,2277,1,1,0,0,0,1,3004.5,-3437,149.81,0.5,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),         -- Tarenar in the slaughterhouse (pins 28.4-28.6,26; floor 149.71)
(3129402,45730,0,139,2277,1,1,0,0,0,1,3008.63,-3434.42,151.157,2.85679,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0); -- Gidwin on the table (TDB 85649 spot)
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(3128010,0,0,0,0,0,1,1,0,0,0,0,0,0,'82343'),
(3128011,0,0,0,0,0,1,1,0,0,0,0,0,0,'79976 80797'),  -- Retribution Aura as on his other copies (both TDBs' template)
(3128012,0,0,0,0,0,1,1,0,0,0,0,0,0,'49415'),
(3128013,0,0,0,0,0,1,1,0,0,0,0,0,0,'60921'),
(3128014,0,0,0,0,0,1,1,0,0,0,0,0,0,'49415'),
(3128018,0,0,0,0,0,1,1,0,0,0,0,0,0,'49415'),
(3128019,0,0,0,0,0,1,1,0,0,0,0,0,0,'60921'),
(3129400,0,0,0,0,0,1,1,0,0,0,0,0,0,'49414'),
(3129401,0,0,0,0,0,1,1,0,0,0,0,0,0,'49415'),
(3129402,0,0,0,0,0,1,1,0,10,0,0,0,0,'60921'),       -- emote 10: dancing on the table
(373696,0,0,0,0,0,1,1,0,0,0,0,0,0,'49415'),         -- statue Gidwin, had no aura
(369660,0,0,0,0,0,1,1,0,0,0,0,0,0,'60921'),         -- chapel Tarenar, had no aura
(369661,0,0,0,0,0,1,1,0,0,0,0,0,0,'82343');         -- wagon Gidwin at Light's Hope, had no aura

-- Tarenar 369660: from the Light's Hope caravan spot into the chapel (pin 75.6,52.2; chapel floor 90.75).
UPDATE `creature` SET `position_x`=2301, `position_y`=-5340, `position_z`=90.85, `orientation`=0.4 WHERE `guid`=369660;

DELETE FROM `spell_area` WHERE `spell`=49416 AND `area` IN (5446,2277);
DELETE FROM `spell_area` WHERE `spell`=49417 AND `area` IN (2619,2271,2275,2268,2277);
DELETE FROM `spell_area` WHERE `spell`=60922 AND `area` IN (2619,2271,2268);
DELETE FROM `spell_area` WHERE `spell`=85311 AND `area` IN (2619,2268);
DELETE FROM `spell_area` WHERE `spell`=85339 AND `area`=2619;
DELETE FROM `spell_area` WHERE `spell`=84909 AND `area`=2271;
DELETE FROM `spell_area` WHERE `spell`=85077 AND `area`=2268;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(49416,5446,0,27372,-84902,0,2,1,64,11),     -- Tarenar at the Marris Stead, not while his follower is out (deaths-step-crown-guard)
(49416,2277,27524,27526,0,0,2,1,74,9),       -- Tarenar at the Plaguewood fork
(49417,2619,0,27368,0,0,2,1,64,11),          -- Gidwin 45428 in his cocoon (unchanged)
(84909,2271,27466,27487,49416,0,2,1,74,1),   -- Gidwin in the Eastwall tower, while the caravan is at Eastwall (own spell, see below)
(49417,2275,27489,27524,0,0,2,1,66,1),       -- Tarenar at Northpass
(49417,2268,27448,27463,0,0,2,1,66,1),       -- statue pair
(49417,2277,27526,27527,0,0,2,1,66,11),      -- Tarenar in the slaughterhouse
(60922,2619,27368,27369,0,0,2,1,64,11),      -- Gidwin 45431 freed, north of the river (unchanged)
(60922,2271,27466,27489,-85405,0,2,1,74,9),  -- Tarenar at Eastwall, not while his 27487 follower is out (eastwall-northpass)
(85077,2268,27465,27466,-85315,0,2,1,64,1),  -- chapel pair before Northdale; not once the caravan has reached Northpass (own spell)
(60922,2268,27527,0,0,0,2,1,64,0),           -- chapel pair after Journey's End
(85311,2619,27369,27373,0,0,2,1,64,9),       -- Gidwin at the river camp
(85311,2268,27527,0,0,0,2,1,10,0),           -- Gidwin at the Light's Hope wagon while Journey's End is in the log
(85339,2619,27372,27373,0,0,2,1,64,9);       -- Tarenar at the river camp
