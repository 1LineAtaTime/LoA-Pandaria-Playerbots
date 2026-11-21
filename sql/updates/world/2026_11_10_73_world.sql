-- The Battle for Andorhal by stage: the battle against the Scourge, the second battle, the val'kyr attacks, and Koltira,
-- Lindsay Ravensun, Lady Sylvanas and Lurid's corpse each in their own state
-- Sourced: Phase.dbc 259 "Andorhal 01 - Horde/Alliance vs. Scourge", 261 "Andorhal 03 - Horde vs. Alliance", 268 "Andorhal
-- 04H - Val'kyr Attack (H)", 272 "Andorhal 04A - Val'kyr Attack (A)", group 519 = [261, 272]; 84252 "The Reckoning: Force
-- Phase" (phase 269, the cutscene) and 84525 (272). warcraft.wiki.gg (Battle for Andorhal): Lindsay Ravensun is Sylvanas in
-- disguise; the Horde finds only Lurid's body at the Alliance camp; Sylvanas drags Koltira to the Undercity. Wowhead (MoP
-- Classic) map pins: Lindsay Ravensun 44466 at her camp spot, at the Charred Outpost and in the basement beside Lady
-- Sylvanas's spot (1543.5, -1690.3); Koltira 44452 at his camp and beside Darkmaster Gandling, where both TDBs have him
-- (TDB 1210 guid 328338) next to Thassarian 213211. Wowhead comments: 27087 "Upon accepting it, I couldn't find Lindsay
-- anywhere ... a new phase with the whole camp empty"; Lurid 45129 "after the final quests (27206/27090) in Andorhal, the
-- Horde will take over the town, and Lurid's corpse ... will be found near the Inn"; Thassarian 44453 "after completing all
-- quests ... as alliance ... Thassarian is gone, only the corpse(!) of his minion Lurid remains". Neither TDB phases
-- Andorhal (every spawn PhaseId 0, no phase_area), so which spawn goes where is ours.
-- Ours: the keys, all granted zone-wide in Western Plaguelands (28) so the town looks the same from the road:
--   259 first battle (Scourge, its bosses, the stage-one soldiers, Thassarian and Koltira at Gandling): until Victory, For
--       Now 26926/27165 is rewarded and no second-battle quest is taken, or while a first-battle quest is in the log
--   261 second battle (the Forsaken army; the Alliance army and militia take group 519 = [261, 272], which in practice is
--       261, since every player with 272 has 261): once any second-battle quest is taken. 261 never ends: after Andorhal,
--       Once and For All 27090 and Alas, Andorhal 27206 the town stays at the second battle (the Lurid comment's Horde-held
--       town is not modelled)
--   268 Horde val'kyr layer: Lindsay Ravensun, Revealed 27087 rewarded, or a later Horde quest taken
--   272 Alliance val'kyr layer: The Depravity of the Forsaken 27205 rewarded, or a later Alliance quest taken
--   170 Lindsay at the camp: until 27087 is taken, except while she is at the Charred Outpost (Strange New Faces 26979
--       taken, Combat Training 26938 not yet rewarded); also while War Machines 26923, which she gives and ends, is in
--       the log, unless 27087 is (the basement Lindsay ends it then)
--   188 Lindsay at the Charred Outpost (3126000): from 26979 taken until 26938 is rewarded. All three quests she has there
--       end at her; 27087 needs 26938, so it can never be handed in at the outpost
--   182 Lindsay in the basement (new spawn on her pin, z the basement floor, facing as Sylvanas): while 27087 is in the log
--   181 Lady Sylvanas: from 27087 rewarded (or 27089/27090/27144 taken), not during The Reckoning scene (aura 84252: the
--       scene summons its own Sylvanas 45111 on her exact spot)
--   171 Koltira at the camp: until The Reckoning 27144 is complete, not during its scene (it summons its own Koltira 45112)
--   180 Lurid's corpse: from Ace in the Hole 27089 taken (it is the ender), or Alas, Andorhal 27206 rewarded
-- Phase ids 170, 171, 180, 181, 182, 188 are Phase.dbc's generic "Quest Zone-Specific" ids, unused in mop_world. The
-- Scourge's Soulless Ghoul 1794 (15154) and Putridius 1850 (15146) go into 259 with the rest of the Scourge. Koltira at
-- Gandling is a new spawn on the TDB 1210 spot (guid 3126400); Lindsay's basement spawn is 3126401.
-- 260 "Andorhal 02 - Horde/Alliance Prep" gets no layer: between the battles the town shows only its camps and fires.
DELETE FROM `creature` WHERE `guid` IN (3126400,3126401);
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3126400,44452,0,28,193,1,1,259,0,33910,0,1288.65,-1536.47,58.7686,1.18098,300,0,0,0,1549800,178160,0,0,0,0,0,0,'',0,0), -- Koltira Deathweaver at Gandling (TDB 1210 guid 328338)
(3126401,44466,0,28,193,1,1,182,0,33917,0,1544.00,-1689.00,57.41,1.71042,300,0,0,0,3048,0,0,0,0,0,0,0,'',0,0); -- Lindsay Ravensun in the basement (Wowhead pin 49.0 63.6)

UPDATE `creature` SET `phaseId`=259, `phaseGroup`=0 WHERE `map`=0 AND `zoneId`=28 AND `areaId`=193 AND `id` IN (44315,44316,44562,44329,44326,44328,44331,44332,44317,44318,1852,44323,46558,44360);
UPDATE `creature` SET `phaseId`=259, `phaseGroup`=0 WHERE `guid`=213211 AND `id`=44453;
UPDATE `creature` SET `phaseId`=259, `phaseGroup`=0 WHERE (`guid`,`id`) IN ((15154,1794),(15146,1850));
UPDATE `gameobject` SET `phaseId`=259, `phaseGroup`=0 WHERE `guid` IN (165911,165912) AND `id`=204966;
UPDATE `creature` SET `phaseId`=261, `phaseGroup`=0 WHERE `map`=0 AND `zoneId`=28 AND `areaId`=193 AND `id` IN (44447,44449,44946);
UPDATE `creature` SET `phaseId`=0, `phaseGroup`=519 WHERE `map`=0 AND `zoneId`=28 AND `areaId`=193 AND `id` IN (44448,44450,44944);
UPDATE `creature` SET `phaseId`=268, `phaseGroup`=0 WHERE `map`=0 AND `zoneId`=28 AND `areaId`=193 AND `id`=45101;
UPDATE `creature` SET `phaseId`=272, `phaseGroup`=0 WHERE `map`=0 AND `zoneId`=28 AND `areaId`=193 AND `id` IN (45239,45240,45242,45235);
UPDATE `creature` SET `phaseId`=170, `phaseGroup`=0 WHERE `guid`=212084 AND `id`=44466;
UPDATE `creature` SET `phaseId`=188, `phaseGroup`=0 WHERE `guid`=3126000 AND `id`=44466;
UPDATE `creature` SET `phaseId`=181, `phaseGroup`=0 WHERE `guid`=311562 AND `id`=44451;
UPDATE `creature` SET `phaseId`=171, `phaseGroup`=0 WHERE `guid`=212083 AND `id`=44452;
UPDATE `creature` SET `phaseId`=180, `phaseGroup`=0 WHERE `guid`=212037 AND `id`=45129;

DELETE FROM `phase_area` WHERE `AreaId`=28 AND `PhaseId` IN (259,261,268,272,170,171,180,181,182,188);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(28,259,'Western Plaguelands - Andorhal 01: the battle against the Scourge, until Victory, For Now'),
(28,261,'Western Plaguelands - Andorhal 03: Horde vs. Alliance, from the first second-battle quest'),
(28,268,'Western Plaguelands - Andorhal 04H: Val''kyr attack (Horde), from Lindsay Ravensun, Revealed'),
(28,272,'Western Plaguelands - Andorhal 04A: Val''kyr attack (Alliance), from The Depravity of the Forsaken'),
(28,170,'Western Plaguelands - Lindsay Ravensun at the Forsaken camp until Lindsay Ravensun, Revealed'),
(28,171,'Western Plaguelands - Koltira at the Forsaken camp until The Reckoning'),
(28,180,'Western Plaguelands - Lurid''s corpse from Ace in the Hole or Alas, Andorhal'),
(28,181,'Western Plaguelands - Lady Sylvanas in Andorhal from Lindsay Ravensun, Revealed'),
(28,182,'Western Plaguelands - Lindsay Ravensun in the basement during Lindsay Ravensun, Revealed'),
(28,188,'Western Plaguelands - Lindsay Ravensun at the Charred Outpost from Strange New Faces to Combat Training');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=28 AND `SourceGroup` IN (259,261,268,272,170,171,180,181,182,188);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,259,28,0,0,47,0,26926,64,0,1,0,0,'','Andorhal 01 - Victory, For Now (H) not rewarded'),
(26,259,28,0,0,47,0,27165,64,0,1,0,0,'','Andorhal 01 - Victory, For Now (A) not rewarded'),
(26,259,28,0,0,47,0,27083,74,0,1,0,0,'','Andorhal 01 - The Battle Resumes! (H) not taken'),
(26,259,28,0,0,47,0,27084,74,0,1,0,0,'','Andorhal 01 - The Farmers'' Militia not taken'),
(26,259,28,0,0,47,0,27085,74,0,1,0,0,'','Andorhal 01 - Supporting the Troops (H) not taken'),
(26,259,28,0,0,47,0,27086,74,0,1,0,0,'','Andorhal 01 - Ashes to Ashes (H) not taken'),
(26,259,28,0,0,47,0,27087,74,0,1,0,0,'','Andorhal 01 - Lindsay Ravensun, Revealed not taken'),
(26,259,28,0,0,47,0,27089,74,0,1,0,0,'','Andorhal 01 - Ace in the Hole not taken'),
(26,259,28,0,0,47,0,27090,74,0,1,0,0,'','Andorhal 01 - Andorhal, Once and For All not taken'),
(26,259,28,0,0,47,0,27144,74,0,1,0,0,'','Andorhal 01 - The Reckoning not taken'),
(26,259,28,0,0,47,0,27197,74,0,1,0,0,'','Andorhal 01 - The Battle Resumes! (A) not taken'),
(26,259,28,0,0,47,0,27198,74,0,1,0,0,'','Andorhal 01 - Supporting the Troops (A) not taken'),
(26,259,28,0,0,47,0,27199,74,0,1,0,0,'','Andorhal 01 - Ashes to Ashes (A) not taken'),
(26,259,28,0,0,47,0,27201,74,0,1,0,0,'','Andorhal 01 - Val''kyr Incursion not taken'),
(26,259,28,0,0,47,0,27202,74,0,1,0,0,'','Andorhal 01 - Brother Against Brother not taken'),
(26,259,28,0,0,47,0,27204,74,0,1,0,0,'','Andorhal 01 - Aradne not taken'),
(26,259,28,0,0,47,0,27205,74,0,1,0,0,'','Andorhal 01 - The Depravity of the Forsaken not taken'),
(26,259,28,0,0,47,0,27206,74,0,1,0,0,'','Andorhal 01 - Alas, Andorhal not taken'),
(26,259,28,0,1,47,0,26921,10,0,0,0,0,'','Andorhal 01 - Scourge First... Alliance Later in the log'),
(26,259,28,0,2,47,0,26922,10,0,0,0,0,'','Andorhal 01 - The Endless Flow (H) in the log'),
(26,259,28,0,3,47,0,26923,10,0,0,0,0,'','Andorhal 01 - War Machines (H) in the log'),
(26,259,28,0,4,47,0,26924,10,0,0,0,0,'','Andorhal 01 - Scholomancer (H) in the log'),
(26,259,28,0,5,47,0,26925,10,0,0,0,0,'','Andorhal 01 - Araj the Summoner (H) in the log'),
(26,259,28,0,6,47,0,27117,10,0,0,0,0,'','Andorhal 01 - Brute Strength (H) in the log'),
(26,259,28,0,7,47,0,27159,10,0,0,0,0,'','Andorhal 01 - Scourge First... Horde Later in the log'),
(26,259,28,0,8,47,0,27160,10,0,0,0,0,'','Andorhal 01 - War Machines (A) in the log'),
(26,259,28,0,9,47,0,27161,10,0,0,0,0,'','Andorhal 01 - The Endless Flow (A) in the log'),
(26,259,28,0,10,47,0,27162,10,0,0,0,0,'','Andorhal 01 - Scholomancer (A) in the log'),
(26,259,28,0,11,47,0,27163,10,0,0,0,0,'','Andorhal 01 - Brute Strength (A) in the log'),
(26,259,28,0,12,47,0,27164,10,0,0,0,0,'','Andorhal 01 - Araj the Summoner (A) in the log'),
(26,261,28,0,0,47,0,27083,74,0,0,0,0,'','Andorhal 03 - The Battle Resumes! (H) taken'),
(26,261,28,0,1,47,0,27084,74,0,0,0,0,'','Andorhal 03 - The Farmers'' Militia taken'),
(26,261,28,0,2,47,0,27085,74,0,0,0,0,'','Andorhal 03 - Supporting the Troops (H) taken'),
(26,261,28,0,3,47,0,27086,74,0,0,0,0,'','Andorhal 03 - Ashes to Ashes (H) taken'),
(26,261,28,0,4,47,0,27087,74,0,0,0,0,'','Andorhal 03 - Lindsay Ravensun, Revealed taken'),
(26,261,28,0,5,47,0,27089,74,0,0,0,0,'','Andorhal 03 - Ace in the Hole taken'),
(26,261,28,0,6,47,0,27090,74,0,0,0,0,'','Andorhal 03 - Andorhal, Once and For All taken'),
(26,261,28,0,7,47,0,27144,74,0,0,0,0,'','Andorhal 03 - The Reckoning taken'),
(26,261,28,0,8,47,0,27197,74,0,0,0,0,'','Andorhal 03 - The Battle Resumes! (A) taken'),
(26,261,28,0,9,47,0,27198,74,0,0,0,0,'','Andorhal 03 - Supporting the Troops (A) taken'),
(26,261,28,0,10,47,0,27199,74,0,0,0,0,'','Andorhal 03 - Ashes to Ashes (A) taken'),
(26,261,28,0,11,47,0,27201,74,0,0,0,0,'','Andorhal 03 - Val''kyr Incursion taken'),
(26,261,28,0,12,47,0,27202,74,0,0,0,0,'','Andorhal 03 - Brother Against Brother taken'),
(26,261,28,0,13,47,0,27204,74,0,0,0,0,'','Andorhal 03 - Aradne taken'),
(26,261,28,0,14,47,0,27205,74,0,0,0,0,'','Andorhal 03 - The Depravity of the Forsaken taken'),
(26,261,28,0,15,47,0,27206,74,0,0,0,0,'','Andorhal 03 - Alas, Andorhal taken'),
(26,268,28,0,0,47,0,27087,64,0,0,0,0,'','Andorhal 04H - Lindsay Ravensun, Revealed rewarded'),
(26,268,28,0,1,47,0,27089,74,0,0,0,0,'','Andorhal 04H - Ace in the Hole taken'),
(26,268,28,0,2,47,0,27090,74,0,0,0,0,'','Andorhal 04H - Andorhal, Once and For All taken'),
(26,268,28,0,3,47,0,27144,74,0,0,0,0,'','Andorhal 04H - The Reckoning taken'),
(26,272,28,0,0,47,0,27205,64,0,0,0,0,'','Andorhal 04A - The Depravity of the Forsaken rewarded'),
(26,272,28,0,1,47,0,27201,74,0,0,0,0,'','Andorhal 04A - Val''kyr Incursion taken'),
(26,272,28,0,2,47,0,27202,74,0,0,0,0,'','Andorhal 04A - Brother Against Brother taken'),
(26,272,28,0,3,47,0,27204,74,0,0,0,0,'','Andorhal 04A - Aradne taken'),
(26,272,28,0,4,47,0,27206,74,0,0,0,0,'','Andorhal 04A - Alas, Andorhal taken'),
(26,170,28,0,0,47,0,27087,74,0,1,0,0,'','Lindsay Ravensun (camp) - Lindsay Ravensun, Revealed not taken'),
(26,170,28,0,0,47,0,26979,74,0,1,0,0,'','Lindsay Ravensun (camp) - Strange New Faces not taken'),
(26,170,28,0,1,47,0,27087,74,0,1,0,0,'','Lindsay Ravensun (camp) - Lindsay Ravensun, Revealed not taken'),
(26,170,28,0,1,47,0,26938,64,0,0,0,0,'','Lindsay Ravensun (camp) - Combat Training (H) rewarded'),
(26,170,28,0,2,47,0,26923,10,0,0,0,0,'','Lindsay Ravensun (camp) - War Machines (H) in the log'),
(26,170,28,0,2,47,0,27087,10,0,1,0,0,'','Lindsay Ravensun (camp) - Lindsay Ravensun, Revealed not in the log'),
(26,188,28,0,0,47,0,26979,74,0,0,0,0,'','Lindsay Ravensun (Charred Outpost) - Strange New Faces taken'),
(26,188,28,0,0,47,0,26938,64,0,1,0,0,'','Lindsay Ravensun (Charred Outpost) - Combat Training (H) not rewarded'),
(26,182,28,0,0,47,0,27087,10,0,0,0,0,'','Lindsay Ravensun (basement) - Lindsay Ravensun, Revealed in the log'),
(26,181,28,0,0,47,0,27087,64,0,0,0,0,'','Lady Sylvanas - Lindsay Ravensun, Revealed rewarded'),
(26,181,28,0,0,1,0,84252,0,0,1,0,0,'','Lady Sylvanas - not in The Reckoning scene'),
(26,181,28,0,1,47,0,27089,74,0,0,0,0,'','Lady Sylvanas - Ace in the Hole taken'),
(26,181,28,0,1,1,0,84252,0,0,1,0,0,'','Lady Sylvanas - not in The Reckoning scene'),
(26,181,28,0,2,47,0,27090,74,0,0,0,0,'','Lady Sylvanas - Andorhal, Once and For All taken'),
(26,181,28,0,2,1,0,84252,0,0,1,0,0,'','Lady Sylvanas - not in The Reckoning scene'),
(26,181,28,0,3,47,0,27144,74,0,0,0,0,'','Lady Sylvanas - The Reckoning taken'),
(26,181,28,0,3,1,0,84252,0,0,1,0,0,'','Lady Sylvanas - not in The Reckoning scene'),
(26,171,28,0,0,47,0,27144,66,0,1,0,0,'','Koltira (camp) - The Reckoning not complete or rewarded'),
(26,171,28,0,0,1,0,84252,0,0,1,0,0,'','Koltira (camp) - not in The Reckoning scene'),
(26,180,28,0,0,47,0,27089,74,0,0,0,0,'','Lurid''s corpse - Ace in the Hole taken'),
(26,180,28,0,1,47,0,27090,74,0,0,0,0,'','Lurid''s corpse - Andorhal, Once and For All taken'),
(26,180,28,0,2,47,0,27206,64,0,0,0,0,'','Lurid''s corpse - Alas, Andorhal rewarded');
