-- Andorhal: the Alliance moves its camp for the second battle; Thurman Grant at Chillwind Camp, the Writhing Haunt and
-- Andorhal in turn; the Alliance townsfolk gone once Andorhal falls
-- Sourced: Wowhead (MoP Classic) map pins: Thassarian 44453 (39.8 69.4, 39.8 69.6), Lurid 44471 (39.8 69.4, 40.0 69.6) and
-- Lang Loosegrip 44467 (40.0 69.2) at a second camp 60 yd north-east of their first-battle spots, which the pins also show;
-- Thurman Grant 45165 at Chillwind Camp, the Writhing Haunt and that second camp (39.8 69.6). Every second-battle turn-in
-- of the Alliance (27197-27205, quest_poi objective -1) lies at that camp, and Brother Against Brother 27202's log text sends
-- the player back "to Thurman Grant at Andorhal". Thurman's quests: A Mighty Hunger 27167 at Chillwind; The Writhing Haunt
-- 27172 (from Valorfist) ends at the Writhing Haunt, where This Is Our Army 27173, Combat Training 27174 and The Menders'
-- Stead 27233 start; Wowhead 27202: the player picks it up "where the farmers" are. Wowhead comments: 27206 "Completing this
-- quest phases out the flight path for alliance"; Lurid 45129 "after the final quests (27206/27090) in Andorhal, the Horde
-- will take over the town"; Thassarian 44453 "Thassarian is gone" after the Alliance quests.
-- Ours:
--   - the second camp's spawns: the first camp's sniffed layout moved by (+27.1, +54.3), which puts each NPC inside his
--     pins' cells (Thassarian 3126402, Lurid 3126403, Lang 3126404, same facings), and Thurman 3126405 in his pin's cell
--     beside them, facing as they do; z from the terrain
--   - 172 the first camp (Thassarian 212699, Lurid 212700, Lang 212701): until any second-battle quest is taken, or while
--     an Alliance first-battle quest is in the log
--   - 179 the second camp: once any second-battle quest is taken, until Ace in the Hole 27089 or Andorhal, Once and For All
--     27090 is taken (the Horde's hunt ends at Lurid's body) or Alas, Andorhal 27206 is rewarded (Thassarian goes west)
--   - 183 Thurman at Chillwind Camp 212047: until The Writhing Haunt is taken, or while A Mighty Hunger is in the log
--   - 184 Thurman at the Writhing Haunt 317148: from The Writhing Haunt taken until The Battle Resumes! 27197 is taken, or
--     while 27172/27173/27174 is in the log
--   - 185 the Alliance townsfolk by the second camp (Ginny Goodwin's flight point, Benjamin Henderson, Alexis Torry, Foreman
--     Milos): until 27206 or the Horde's 27090 is rewarded
-- All granted zone-wide in Western Plaguelands (28); 172, 179, 183, 184, 185 are Phase.dbc's generic "Quest Zone-Specific"
-- ids, unused in mop_world.
DELETE FROM `creature` WHERE `guid` BETWEEN 3126402 AND 3126405;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3126402,44453,0,28,193,1,1,179,0,33911,0,1374.40,-1294.72,57.47,4.11898,60,0,0,0,542430,0,0,0,0,0,0,0,'',0,0), -- Thassarian, second camp
(3126403,44471,0,28,193,1,1,179,0,5231,0,1373.11,-1292.26,57.65,4.20624,300,0,0,0,3048,0,0,0,0,0,0,0,'',0,0), -- Lurid, second camp
(3126404,44467,0,28,193,1,1,179,0,33918,0,1385.20,-1303.27,57.50,2.61799,300,0,0,0,2452,1283,0,0,0,0,0,0,'',0,0), -- Lang Loosegrip, second camp
(3126405,45165,0,28,193,1,1,179,0,34287,0,1369.00,-1296.00,57.14,4.11898,300,0,0,0,1995,1534,0,0,0,0,0,0,'',0,0); -- Thurman Grant, second camp

UPDATE `creature` SET `phaseId`=172, `phaseGroup`=0 WHERE (`guid`,`id`) IN ((212699,44453),(212700,44471),(212701,44467));
UPDATE `creature` SET `phaseId`=183, `phaseGroup`=0 WHERE `guid`=212047 AND `id`=45165;
UPDATE `creature` SET `phaseId`=184, `phaseGroup`=0 WHERE `guid`=317148 AND `id`=45165;
UPDATE `creature` SET `phaseId`=185, `phaseGroup`=0 WHERE (`guid`,`id`) IN ((212088,46006),(212073,47717),(212072,47719),(212071,47721));

DELETE FROM `phase_area` WHERE `AreaId`=28 AND `PhaseId` IN (172,179,183,184,185);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(28,172,'Western Plaguelands - the Alliance camp in Andorhal for the first battle'),
(28,179,'Western Plaguelands - the Alliance camp in Andorhal for the second battle, until Ace in the Hole / Alas, Andorhal'),
(28,183,'Western Plaguelands - Thurman Grant at Chillwind Camp until The Writhing Haunt'),
(28,184,'Western Plaguelands - Thurman Grant at the Writhing Haunt from The Writhing Haunt until The Battle Resumes!'),
(28,185,'Western Plaguelands - the Alliance townsfolk of Andorhal until Alas, Andorhal / Andorhal, Once and For All');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=28 AND `SourceGroup` IN (172,179,183,184,185);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,172,28,0,0,47,0,27083,74,0,1,0,0,'','Alliance camp (first battle) - The Battle Resumes! (H) not taken'),
(26,172,28,0,0,47,0,27084,74,0,1,0,0,'','Alliance camp (first battle) - The Farmers'' Militia not taken'),
(26,172,28,0,0,47,0,27085,74,0,1,0,0,'','Alliance camp (first battle) - Supporting the Troops (H) not taken'),
(26,172,28,0,0,47,0,27086,74,0,1,0,0,'','Alliance camp (first battle) - Ashes to Ashes (H) not taken'),
(26,172,28,0,0,47,0,27087,74,0,1,0,0,'','Alliance camp (first battle) - Lindsay Ravensun, Revealed not taken'),
(26,172,28,0,0,47,0,27089,74,0,1,0,0,'','Alliance camp (first battle) - Ace in the Hole not taken'),
(26,172,28,0,0,47,0,27090,74,0,1,0,0,'','Alliance camp (first battle) - Andorhal, Once and For All not taken'),
(26,172,28,0,0,47,0,27144,74,0,1,0,0,'','Alliance camp (first battle) - The Reckoning not taken'),
(26,172,28,0,0,47,0,27197,74,0,1,0,0,'','Alliance camp (first battle) - The Battle Resumes! (A) not taken'),
(26,172,28,0,0,47,0,27198,74,0,1,0,0,'','Alliance camp (first battle) - Supporting the Troops (A) not taken'),
(26,172,28,0,0,47,0,27199,74,0,1,0,0,'','Alliance camp (first battle) - Ashes to Ashes (A) not taken'),
(26,172,28,0,0,47,0,27201,74,0,1,0,0,'','Alliance camp (first battle) - Val''kyr Incursion not taken'),
(26,172,28,0,0,47,0,27202,74,0,1,0,0,'','Alliance camp (first battle) - Brother Against Brother not taken'),
(26,172,28,0,0,47,0,27204,74,0,1,0,0,'','Alliance camp (first battle) - Aradne not taken'),
(26,172,28,0,0,47,0,27205,74,0,1,0,0,'','Alliance camp (first battle) - The Depravity of the Forsaken not taken'),
(26,172,28,0,0,47,0,27206,74,0,1,0,0,'','Alliance camp (first battle) - Alas, Andorhal not taken'),
(26,172,28,0,1,47,0,27159,10,0,0,0,0,'','Alliance camp (first battle) - Scourge First... Horde Later in the log'),
(26,172,28,0,2,47,0,27160,10,0,0,0,0,'','Alliance camp (first battle) - War Machines (A) in the log'),
(26,172,28,0,3,47,0,27161,10,0,0,0,0,'','Alliance camp (first battle) - The Endless Flow (A) in the log'),
(26,172,28,0,4,47,0,27162,10,0,0,0,0,'','Alliance camp (first battle) - Scholomancer (A) in the log'),
(26,172,28,0,5,47,0,27163,10,0,0,0,0,'','Alliance camp (first battle) - Brute Strength (A) in the log'),
(26,172,28,0,6,47,0,27164,10,0,0,0,0,'','Alliance camp (first battle) - Araj the Summoner (A) in the log'),
(26,179,28,0,0,47,0,27083,74,0,0,0,0,'','Alliance camp (second battle) - The Battle Resumes! (H) taken'),
(26,179,28,0,0,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,0,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,0,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,1,47,0,27084,74,0,0,0,0,'','Alliance camp (second battle) - The Farmers'' Militia taken'),
(26,179,28,0,1,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,1,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,1,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,2,47,0,27085,74,0,0,0,0,'','Alliance camp (second battle) - Supporting the Troops (H) taken'),
(26,179,28,0,2,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,2,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,2,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,3,47,0,27086,74,0,0,0,0,'','Alliance camp (second battle) - Ashes to Ashes (H) taken'),
(26,179,28,0,3,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,3,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,3,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,4,47,0,27087,74,0,0,0,0,'','Alliance camp (second battle) - Lindsay Ravensun, Revealed taken'),
(26,179,28,0,4,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,4,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,4,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,5,47,0,27144,74,0,0,0,0,'','Alliance camp (second battle) - The Reckoning taken'),
(26,179,28,0,5,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,5,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,5,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,6,47,0,27197,74,0,0,0,0,'','Alliance camp (second battle) - The Battle Resumes! (A) taken'),
(26,179,28,0,6,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,6,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,6,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,7,47,0,27198,74,0,0,0,0,'','Alliance camp (second battle) - Supporting the Troops (A) taken'),
(26,179,28,0,7,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,7,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,7,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,8,47,0,27199,74,0,0,0,0,'','Alliance camp (second battle) - Ashes to Ashes (A) taken'),
(26,179,28,0,8,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,8,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,8,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,9,47,0,27201,74,0,0,0,0,'','Alliance camp (second battle) - Val''kyr Incursion taken'),
(26,179,28,0,9,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,9,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,9,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,10,47,0,27202,74,0,0,0,0,'','Alliance camp (second battle) - Brother Against Brother taken'),
(26,179,28,0,10,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,10,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,10,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,11,47,0,27204,74,0,0,0,0,'','Alliance camp (second battle) - Aradne taken'),
(26,179,28,0,11,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,11,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,11,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,12,47,0,27205,74,0,0,0,0,'','Alliance camp (second battle) - The Depravity of the Forsaken taken'),
(26,179,28,0,12,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,12,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,12,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,179,28,0,13,47,0,27206,74,0,0,0,0,'','Alliance camp (second battle) - Alas, Andorhal taken'),
(26,179,28,0,13,47,0,27089,74,0,1,0,0,'','Alliance camp (second battle) - Ace in the Hole not taken'),
(26,179,28,0,13,47,0,27090,74,0,1,0,0,'','Alliance camp (second battle) - Andorhal, Once and For All not taken'),
(26,179,28,0,13,47,0,27206,64,0,1,0,0,'','Alliance camp (second battle) - Alas, Andorhal not rewarded'),
(26,183,28,0,0,47,0,27172,74,0,1,0,0,'','Thurman Grant (Chillwind Camp) - The Writhing Haunt not taken'),
(26,183,28,0,1,47,0,27167,10,0,0,0,0,'','Thurman Grant (Chillwind Camp) - A Mighty Hunger in the log'),
(26,184,28,0,0,47,0,27172,74,0,0,0,0,'','Thurman Grant (Writhing Haunt) - The Writhing Haunt taken'),
(26,184,28,0,0,47,0,27197,74,0,1,0,0,'','Thurman Grant (Writhing Haunt) - The Battle Resumes! (A) not taken'),
(26,184,28,0,1,47,0,27172,10,0,0,0,0,'','Thurman Grant (Writhing Haunt) - The Writhing Haunt in the log'),
(26,184,28,0,2,47,0,27173,10,0,0,0,0,'','Thurman Grant (Writhing Haunt) - This Is Our Army in the log'),
(26,184,28,0,3,47,0,27174,10,0,0,0,0,'','Thurman Grant (Writhing Haunt) - Combat Training (A) in the log'),
(26,185,28,0,0,47,0,27206,64,0,1,0,0,'','Alliance townsfolk - Alas, Andorhal not rewarded'),
(26,185,28,0,0,47,0,27090,64,0,1,0,0,'','Alliance townsfolk - Andorhal, Once and For All not rewarded');
