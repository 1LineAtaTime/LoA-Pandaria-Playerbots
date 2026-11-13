-- Eastern Plaguelands, Fiona's Caravan: Vex'tul 45574, Rimblat Earthshatter 16134 and Beezil Linkspanner 46022 stand at
-- their own spots until they join, then travel with the caravan.
--
--  * Before: Vex'tul (373695) stood on the caravan's own spot at Light's Shield Tower with no aura, visible to everyone for
--    ever, and was nowhere else; Rimblat (173864) and Beezil (173865) stood at the Light's Hope caravan spot with no aura,
--    Beezil visible to a character who had never met the caravan; neither came along to Eastwall or Northpass.
--  * Who stands where (status 1 none, 2 complete, 8 incomplete, 64 rewarded):
--      Vex'tul at his own spot, Light's Shield (moved)  80852/85088 type 18  Zaeldarr the Outcast 27432 rewarded .. Honor and
--                                                                            Strength 27449 rewarded (lights-shield)
--      Vex'tul at Eastwall                              80797/95477 type 13  while the caravan is at Eastwall and 27449 is
--                                                                            rewarded and Into the Flames 27482 is not
--      Vex'tul at Northpass                             80797/102594 type 13 from Nobody to Blame but Myself 27489 complete,
--                                                                            same marker, so 27481 / 27482 keep a giver after
--                                                                            the caravan has moved on
--      the marker: See Quest Invis 20 85810 (type 28, worn by no creature anywhere near the zone), zone-wide, 27449
--      rewarded .. 27482 rewarded; the two Vex'tul rows require it.
--      Rimblat at Light's Hope (exists)                 80852/85355 type 18  until An Opportune Alliance 27457 is rewarded,
--                                                                            and again from Journey's End 27527 taken
--      Rimblat at Eastwall / Northpass                  80852/85805, 85808   27457 rewarded and the caravan at that stop
--                                                                            (aura 49416)
--      Rimblat in the slaughterhouse                    80852/85484          27457 rewarded, with Argus and the table Gidwin
--                                                                            (aura 60922: 27526 complete .. 27527 taken)
--      Beezil at Light's Hope (exists)                  84020/85818 type 21  from Boys Will Be Boys 27455 rewarded, for good
--    Rimblat's windows hang on the stop's own detection, so he is never in two places; between joining and the caravan's
--    next stop he is nowhere, and nothing needs him then (his buff is at the wagon, 27457 is handed to any Fiona).
--  * Each row that hangs on another aura has a detection spell of its own (95477 and 102594 for Vex'tul, both type 13,
--    amount 100/1000, no area lock, no other row in this zone): Aura::HandleAuraSpecificMods strips a row's spell
--    everywhere when the aura lands and that row does not fit, and on entering the zone the zone-wide marker 85810 is
--    cast after the area pass (Player::UpdateZone), so a shared spell would be lost until the next area change.
--    Rimblat's 85805 / 85808 / 85484 already have one row each.
--  * Sourced: the template auras of Rimblat (80852) and Beezil (84020) in both TDBs; the See Quest Invis 10 / 13 spells
--    85355, 85805, 85808, 85484, 85818 are each limited by the DBC to exactly the area used here (2268; 2271 + 5463; 2275;
--    2277; 2268), and 85088 to 5448; Vex'tul's own spot (pin 52.8,51.5, lights-shield); his "disappearing from the tower"
--    after 27449 and being next seen at Eastwall (comments); the pins of Rimblat at Eastwall 61.4,42.8, Northpass 50.4,20.2,
--    slaughterhouse 28.4,26 and of Vex'tul at Eastwall 61.4,42.4.
--  * Ours: every window; Vex'tul's Northpass spot (comment 337422, no pin); Beezil's start at 27455 rewarded (he must be
--    recruitable before the finale, 2011-04-19 [+22]; the retail moment is not recorded); Rimblat moved 8 yd off the
--    Eastwall wagon; facings. Heights terrain + 0.1 or the slaughterhouse floor 149.71; areas computed from the map grid.
--  * Not here: Rimblat's faction (chapel-argent), Vex'tul's quests (lights-shield, eastwall-northpass), Beezil joining
--    (the gossip script of section 2 rewards his flag quest 27563).

DELETE FROM `creature_addon` WHERE `guid` IN (3128015,3128016,3128017,3128020,3128022,373695);
DELETE FROM `creature` WHERE `guid` IN (3128015,3128016,3128017,3128020,3128022);
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3128015,45574,0,139,2271,1,1,0,0,0,0,2564.9,-4773.5,106.25,3.5,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),       -- Vex'tul at Eastwall (pin 61.4,42.4)
(3128016,16134,0,139,2271,1,1,0,0,0,0,2562,-4772,106.17,3.3,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),            -- Rimblat at Eastwall (pin 61.4,42.8)
(3128017,16134,0,139,2275,1,1,0,0,0,0,3160,-4320,131.39,5,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),              -- Rimblat at Northpass (pin 50.4,20.2)
(3128020,45574,0,139,2275,1,1,0,0,0,0,3172,-4320,133.48,3.6,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0),            -- Vex'tul at Northpass (ours)
(3128022,16134,0,139,2277,1,1,0,0,0,0,3001,-3432,149.81,0.2,300,0,0,0,1,0,0,0,0,0,0,0,'',0,0);            -- Rimblat in the slaughterhouse (pin 28.4,26; floor 149.71)
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(3128015,0,0,0,0,0,1,1,0,0,0,0,0,0,'80797'),
(3128020,0,0,0,0,0,1,1,0,0,0,0,0,0,'80797'),
(373695,0,0,0,0,0,1,1,0,0,0,0,0,0,'80852');    -- Vex'tul at his own spot, had no aura
-- Rimblat 3128016, 3128017, 3128022 and 173864 wear the template aura.

-- Vex'tul: off the caravan's spot to his own (pin 52.8,51.5; terrain 95.63; lights-shield).
UPDATE `creature` SET `position_x`=2322.3, `position_y`=-4415.9, `position_z`=95.63, `orientation`=3.31 WHERE `guid`=373695;
-- Template auras of both TDBs (each entry has no other spawn).
UPDATE `creature_template_addon` SET `auras`='80852' WHERE `entry`=16134;
UPDATE `creature_template_addon` SET `auras`='84020' WHERE `entry`=46022;

DELETE FROM `spell_area` WHERE `spell`=85810 AND `area`=139;
DELETE FROM `spell_area` WHERE `spell`=85339 AND `area` IN (2271,2275);
DELETE FROM `spell_area` WHERE `spell`=95477 AND `area`=2271;
DELETE FROM `spell_area` WHERE `spell`=102594 AND `area`=2275;
DELETE FROM `spell_area` WHERE `spell` IN (85088,85805,85808,85484,85355,85818);
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(85088,5448,27432,27449,0,0,2,1,64,11),      -- Vex'tul at his own spot (lights-shield)
(85810,139,27449,27482,0,0,2,1,64,11),       -- marker: Vex'tul travels with the caravan
(95477,2271,27455,27489,85810,0,2,1,64,9),   -- Vex'tul at Eastwall while the caravan is there (See Quest Invis 5)
(102594,2275,27489,0,85810,0,2,1,66,0),      -- Vex'tul at Northpass from the third ride on (Quest Invisibility Detection 5)
(85355,2268,0,27457,0,0,2,1,64,11),          -- Rimblat at Light's Hope until he has joined
(85355,2268,27527,0,0,0,2,1,74,0),           -- Rimblat with the caravan at Light's Hope once Journey's End is taken
(85805,2271,27457,0,49416,0,2,1,64,0),       -- Rimblat with the caravan at Eastwall
(85808,2275,27457,0,49416,0,2,1,64,0),       -- Rimblat with the caravan at Northpass
(85484,2277,27457,0,60922,0,2,1,64,0),       -- Rimblat in the slaughterhouse with Argus and Gidwin
(85818,2268,27455,0,0,0,2,1,64,0);           -- Beezil at Light's Hope from the player's arrival there for the caravan
