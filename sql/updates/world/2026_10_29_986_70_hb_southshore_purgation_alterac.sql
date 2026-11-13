-- ported from PR 15 2026_10_28_42_world.sql (#986)
-- Hillsbrad Foothills, Ruins of Southshore, Purgation Isle and the Alterac uplands: make the quests completable.

-- Studies in Lethality 28324: "Recover 250 Blight Samples" (Blight Sample Kill Credit 48290)
-- The Lethality Analyzer (63426) casts Collect Specimen 89752 on a Blight Slime 48136 (condition 17 exists): a channel
-- whose periodic aura (amplitude 250 ms) triggers Depleted 89756 on the slime. The 5.4.8 spell that gives the credit is
-- Specimen Collected 88769 (energize alternate power +1, KILL_CREDIT 48290 bp 1, target the caster's target): nothing cast
-- it, so the quest could not progress. We let the slime cast it back on whoever drains it (SmartAI, ours): one sample per
-- Depleted tick, 250 ticks = about a minute of channelling, matching the 250 count and the +1 anger per sample of 88769.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=48136;
DELETE FROM `smart_scripts` WHERE `entryorguid`=48136 AND `source_type`=0 AND `id`=1;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(48136,0,1,0,8,0,100,0,89756,0,0,0,0,11,88769,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Blight Slime - On Spellhit ''Depleted'' - Cast ''Specimen Collected'' on Invoker');

-- Blight Slime 48136: 5 spawns in mop, 15 in the 4.4.2 sniff (Cataclysm Classic, unchanged zone). Add the 11 sniffed
-- spots with no mop slime within 25 yd (mop's wander 3 / 300 s kept). Angry Blight Slime 48319 (Angry Scrubbing Bubbles):
-- 7 in mop, 9 in 4.4.2; add the 2 missing. guids 3110000-3110010, 3110020-3110021.
DELETE FROM `creature` WHERE `guid` BETWEEN 3110000 AND 3110010 OR `guid` IN (3110020,3110021);
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3110000,48136,0,267,271,1,1,0,0,0,0,-896.304,-471.444,11.6994,4.82464,300,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322768
(3110001,48136,0,267,271,1,1,0,0,0,0,-825.496,-574.882,14.2441,2.18059,300,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322786
(3110002,48136,0,267,271,1,1,0,0,0,0,-731.306,-605.821,21.5482,0.565901,300,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322810
(3110003,48136,0,267,271,1,1,0,0,0,0,-733.497,-581.306,23.0348,5.96903,300,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322992
(3110004,48136,0,267,271,1,1,0,0,0,0,-697.405,-539.378,26.6113,5.00452,300,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322993
(3110005,48136,0,267,271,1,1,0,0,0,0,-763.991,-557.391,17.6357,0.892825,300,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322996
(3110006,48136,0,267,271,1,1,0,0,0,0,-721.846,-516.096,24.2938,5.35816,300,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323000
(3110007,48136,0,267,271,1,1,0,0,0,0,-746.719,-506.665,23.5201,2.05027,300,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323001
(3110008,48136,0,267,271,1,1,0,0,0,0,-714.69,-520.838,22.9479,1.21656,300,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323022
(3110009,48136,0,267,271,1,1,0,0,0,0,-805.73,-471.936,13.8484,3.13787,300,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323025
(3110010,48136,0,267,271,1,1,0,0,0,0,-845.899,-409.88,12.8118,5.60483,300,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323056
(3110020,48319,0,267,271,1,1,0,0,0,0,-824.71,-468.62,15.3922,5.28835,300,0,3.0,0,562,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323026
(3110021,48319,0,267,271,1,1,0,0,0,0,-847.507,-423.247,10.9537,2.73252,300,0,3.0,0,562,0,1,0,0,0,0,0,'',0,0); -- 442 guid 323032

-- Helcular's Command 28332 (kill 15 Bloodfang Forsaken Hunter 48272): replace the stale 48260 layer with the sniffed one.
-- mop: 8 spawns of 48272 (all on 4.4.2 spots) plus 71 of a second "Bloodfang Forsaken Hunter" 48260 (KillCredit1 48272,
-- Shadowstep instead of Leaping Rake, 300 s respawn, guids 176720-176970) that neither the 4.4.2 sniff nor the retail TDB
-- spawns (both: 40 x 48272, 0 x 48260; the 48260 template exists in 4.4.2 with no spawns). 61 of the 71 stand on no sniffed
-- spot and three pairs are exact duplicates (176798/176834, 176801/176835, 176824/176852). The quest already works through
-- the KillCredit; this only makes the field match the sniff. Optional.
DELETE FROM `waypoint_data` WHERE `id` IN (SELECT `path_id` FROM `creature_addon` WHERE `guid` IN (SELECT `guid` FROM `creature` WHERE `id`=48260) AND `path_id`<>0);
DELETE FROM `creature_addon` WHERE `guid` IN (SELECT `guid` FROM (SELECT `guid` FROM `creature` WHERE `id`=48260) AS g);
DELETE FROM `creature` WHERE `id`=48260;
-- The 32 sniffed 48272 spots mop lacks (the other 8 are already there), guids 3110030-3110061; mop's 48272 wander 3 kept.
DELETE FROM `creature` WHERE `guid` BETWEEN 3110030 AND 3110061;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3110030,48272,0,267,267,1,1,0,0,0,0,-612.804,-705.998,29.9039,1.65325,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322814
(3110031,48272,0,267,267,1,1,0,0,0,0,-654.682,-660.75,31.0397,0.663225,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322816
(3110032,48272,0,267,267,1,1,0,0,0,0,-519.564,-767.411,50.7063,1.18682,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322822
(3110033,48272,0,267,267,1,1,0,0,0,0,-513.012,-705.017,47.5419,0.471239,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322967
(3110034,48272,0,267,267,1,1,0,0,0,0,-568.84,-643.33,40.0874,3.43586,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322975
(3110035,48272,0,267,267,1,1,0,0,0,0,-525.816,-623.102,41.2022,6.24828,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322976
(3110036,48272,0,267,267,1,1,0,0,0,0,-556.488,-625.797,39.7866,3.14159,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 322977
(3110037,48272,0,267,267,1,1,0,0,0,0,-459.271,-647.215,48.5923,1.8326,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323010
(3110038,48272,0,267,267,1,1,0,0,0,0,-465.189,-631.193,45.8339,3.40339,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323011
(3110039,48272,0,267,267,1,1,0,0,0,0,-520.078,-585.375,41.4875,4.7822,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323020
(3110040,48272,0,267,271,1,1,0,0,0,0,-765.056,-431.606,21.7529,0.698132,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323027
(3110041,48272,0,267,267,1,1,0,0,0,0,-855.575,-348.958,27.7635,2.54818,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323057
(3110042,48272,0,267,267,1,1,0,0,0,0,-457.05,-566.467,50.0642,2.54818,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323068
(3110043,48272,0,267,267,1,1,0,0,0,0,-559.28,-646.198,42.6334,4.4855,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323121
(3110044,48272,0,267,267,1,1,0,0,0,0,-872.405,-290.179,39.6242,5.84685,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323152
(3110045,48272,0,267,267,1,1,0,0,0,0,-821.96,-304.002,26.5831,3.29867,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323153
(3110046,48272,0,267,267,1,1,0,0,0,0,-758.559,-313.693,20.5489,5.88176,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323157
(3110047,48272,0,267,267,1,1,0,0,0,0,-736.017,-364.413,25.5272,4.74729,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323158
(3110048,48272,0,267,271,1,1,0,0,0,0,-674.738,-357.924,39.5456,4.32842,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323159
(3110049,48272,0,267,271,1,1,0,0,0,0,-660.151,-369.231,38.6797,5.68977,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323163
(3110050,48272,0,267,271,1,1,0,0,0,0,-645.295,-348.788,47.868,4.66003,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323164
(3110051,48272,0,267,267,1,1,0,0,0,0,-619.674,-373.087,38.8262,4.59022,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323165
(3110052,48272,0,267,267,1,1,0,0,0,0,-572.644,-434.983,40.0888,2.26893,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323166
(3110053,48272,0,267,267,1,1,0,0,0,0,-568.009,-447.073,40.6071,4.74729,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323167
(3110054,48272,0,267,267,1,1,0,0,0,0,-553.924,-387.148,43.3357,2.11185,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323172
(3110055,48272,0,267,267,1,1,0,0,0,0,-526.227,-439.12,43.2163,1.65829,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323173
(3110056,48272,0,267,267,1,1,0,0,0,0,-488.639,-444.927,46.8237,3.76991,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323175
(3110057,48272,0,267,267,1,1,0,0,0,0,-489.983,-501.813,41.4,3.97935,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323176
(3110058,48272,0,267,267,1,1,0,0,0,0,-539.571,-351.734,47.0261,0.907571,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323205
(3110059,48272,0,267,267,1,1,0,0,0,0,-600.233,-346.587,63.9694,4.31096,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323206
(3110060,48272,0,267,267,1,1,0,0,0,0,-587.736,-354.391,52.7703,4.38078,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0), -- 442 guid 323208
(3110061,48272,0,267,267,1,1,0,0,0,0,-691.674,-270.606,19.7553,4.34587,120,0,3.0,0,521,0,1,0,0,0,0,0,'',0,0); -- 442 guid 323210

-- Terrible Little Creatures 28355 (Horde side quest at Nethander Stead)
-- wiki: "gained by killing a Mudsnout Gnoll or Mudsnout Shaman at Nethander Stead during Kasha Will Fly Again";
-- start and end automatic. TDB 442 lists the gnoll 2372 as its starter (the sniffed quest push) and no ender.
-- mop had no way to get it (the gnoll is hostile, nothing pushed it) and no way to hand it in (no ender and no
-- QUEST_FLAGS_AUTO_COMPLETE). The push on death is ours (SmartAI); the flags are TDB 442's (1900544 = +0x10000).
UPDATE `quest_template` SET `Flags`=1900544 WHERE `ID`=28355;

DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid` IN (2372,2373) AND `id`=10;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(2372,0,10,0,6,0,100,0,0,0,0,0,0,7,28355,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Mudsnout Gnoll - On Death - Add Quest ''Terrible Little Creatures'' to killer'),
(2373,0,10,0,6,0,100,0,0,0,0,0,0,7,28355,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Mudsnout Shaman - On Death - Add Quest ''Terrible Little Creatures'' to killer');

-- Only while Kasha Will Fly Again 28354 is in the log (mask 10 = complete|incomplete) and 28355 was never taken (mask 1).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry` IN (2372,2373) AND `SourceGroup`=11;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,11,2372,0,0,47,0,28354,10,0,0,0,0,'','Mudsnout Gnoll: push 28355 only with Kasha Will Fly Again in the log'),
(22,11,2372,0,0,47,0,28355,1,0,0,0,0,'','Mudsnout Gnoll: push 28355 only if never taken'),
(22,11,2373,0,0,47,0,28354,10,0,0,0,0,'','Mudsnout Shaman: push 28355 only with Kasha Will Fly Again in the log'),
(22,11,2373,0,0,47,0,28355,1,0,0,0,0,'','Mudsnout Shaman: push 28355 only if never taken');

-- Kasha and Kingslayer Orkus: who is visible at which stage of the Purgation Isle story (Hillsbrad hub B)
-- The sniffed Kasha spawns carry "Apply Quest Invis Zone N" auras and nothing gave the matching detection, so they were
-- never seen. Each detection spell sits next to its Apply spell in Spell.dbc (89276/89277; 90115-90119 next to Ride to
-- Purgation 90122 and Summon Kasha 90123). The quest bounds are ours, from the story (wiki):
--   Kasha 48217 guid 312689, hovering by Orkus at the Southshore beach (area 271), Apply Zone 2 89276: healed by Kasha Will
--     Fly Again 28354 ("Kasha gets back up and roars"), gone once she has carried you to Purgation Isle (The Road to
--     Purgation 28375 complete). -> Detect 89277 while 28354 rewarded and 28375 not taken or in progress.
--   Kasha 48471 guid 312259, landed on Purgation Isle (area 896), Apply Zone 4 90117 (template): there from the end of the
--     ride (28375 complete) until Heroes of the Horde! 28400 is complete (Kasha flies you out). -> Detect 90115.
--   Kasha 48576 guid 375403, perched above Melisara and Cromush in Tarren Mill (area 272), Apply Zone 5 90119: Kasha brings
--     you to Tarren Mill at the end of 28400 and stays (Orkus: "Please take care of Kasha"). -> Detect 90116 from 28400
--     complete on.
-- The dead Kasha 48217 guid 312260 (StandState dead, next to Orkus 48218) has no invisibility and stays as she is.
DELETE FROM `spell_area` WHERE `spell` IN (89277,90115,90116);
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(89277,271,28354,28375,0,0,2,1,64,9),
(90115,896,28375,28400,0,0,2,1,66,9),
(90116,272,28400,0,0,0,2,1,66,0);

-- Tarren Mill: Kasha 48576 guid 375402 (waypoint path from 7 yd above Orkus 48575 down to the perch of 375403) and Orkus
-- 48575 guid 375404 (40 yd above the ground at -124,-973, z 104; 48576 is a vehicle, VehicleId 1337) are the sniffed
-- flight of Kasha carrying Orkus' body in, i.e. a vehicle and its passenger caught in the air. 375403 is the landed Kasha.
-- Orkus 48575 had no invisibility and hung in the sky over Tarren Mill from the start.
DELETE FROM `waypoint_data` WHERE `id`=375402;
DELETE FROM `creature_addon` WHERE `guid` IN (375402,375404);
DELETE FROM `creature` WHERE `guid` IN (375402,375404);

-- Purgation Isle: Kingslayer Orkus 48470 ends The Road to Purgation, starts and ends They Will Never Expect This... 28397 and
-- starts Heroes of the Horde! 28400 (TDB 442). In 5.4.8 he is a personal summon (Summon Orkus 90132, from Purgation Isle
-- Summon Controller I 90130), which this core has no script for, and mop had no spawn: the chain stopped here. A static
-- Orkus beside the landed Kasha with her invisibility (Apply Quest Invis Zone 4 90117), so the same Detect 90115 bounds
-- show the two together. Ours; guid 3110070.
DELETE FROM `creature_addon` WHERE `guid`=3110070;
DELETE FROM `creature` WHERE `guid`=3110070;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3110070,48470,0,267,896,1,1,0,0,0,0,-1230.74,425.02,3.05,3.34,120,0,0,0,1,0,0,0,0,0,0,0,'',0,0);
INSERT INTO `creature_addon` (`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(3110070,0,0,0,0,0,0,1,0,0,0,0,0,0,'90117');

-- Deception and Trickery 28604: "Use the Barrel of Water to Poison 30 Stormpike Trainees" (Stormpike Trainee Poison Credit 49037)
-- 5.4.8 design (Spell.dbc + wiki): Lilith 49035 is a summon (Summon Lilith 91393, SummonProperties 3081) that comes with
-- Lilith Summon Controller 91391, whose FORCE_REACTION aura makes Stormpike Guard (faction 730: the trainees' template 1534)
-- friendly, "so the enemy sees you as one of their own". The Water Barrel item (64583) casts Offer Water 91400 (summons the
-- barrel 49036); the barrel's Water! 91399 (dummy, 12 yd area) calls the trainees, who drink and die over time.
-- mop: Lilith had no spawn and nothing summoned her, so neither this quest nor Domination 28605 (which needs it, wiki) could
-- be had; nothing gave 49037. Minimal version (ours):
--  * a static Lilith beside Melisara in Tarren Mill (she starts and ends 28604 and starts 28605); guid 3110072,
--  * Lilith Summon Controller 91391 on the player in The Headland (area 1680) while 28604 is in progress,
--  * the barrel casts Water! on spawn; each trainee it reaches gives the barrel's summoner one credit (SPELLHIT_TARGET),
--    walks to the barrel, drinks and dies 8 s later.
-- Caveat: while the controller is up Instructor Rufus and Drill Sergeant Magnus (Infiltration 28603, also faction 1534) are
-- friendly too; finish one quest before the other there.
DELETE FROM `creature` WHERE `guid`=3110072;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3110072,49035,0,267,272,1,1,0,0,0,0,-24.6,-931.3,55.4,0.35,120,0,0,0,1,0,0,0,0,0,0,0,'',0,0);

DELETE FROM `spell_area` WHERE `spell`=91391;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(91391,1680,28604,0,0,0,2,1,8,0);

-- Water! 91399 only reaches living Stormpike Trainees.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=91399;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,91399,0,0,31,0,3,48912,0,0,0,0,'','Water! - Stormpike Trainee only'),
(13,1,91399,0,0,36,0,0,0,0,0,0,0,'','Water! - alive targets only');

UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=49036;
DELETE FROM `smart_scripts` WHERE `entryorguid`=49036 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=48912 AND `source_type`=0 AND `id`=2;
DELETE FROM `smart_scripts` WHERE `entryorguid`=4891200 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(49036,0,0,1,54,0,100,0,0,0,0,0,0,11,91399,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Water Barrel - On Just Summoned - Cast ''Water!'''),
(49036,0,1,0,61,0,100,0,0,0,0,0,0,41,30000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Water Barrel - Linked - Despawn in 30 s'),
(49036,0,2,0,31,0,100,0,91399,0,0,0,0,33,49037,0,0,0,0,0,23,0,0,0,0,0,0,0,0,'Water Barrel - On ''Water!'' Hit Target - Kill Credit ''Stormpike Trainee Poison Credit'' to Summoner'),
(48912,0,2,0,8,0,100,1,91399,0,0,0,0,80,4891200,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Stormpike Trainee - On Spellhit ''Water!'' - Run Script'),
(4891200,9,0,0,0,0,100,0,0,0,0,0,0,8,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Stormpike Trainee - Script - Set Passive'),
(4891200,9,1,0,0,0,100,0,0,0,0,0,0,69,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Stormpike Trainee - Script - Move to Barrel'),
(4891200,9,2,0,0,0,100,0,4000,4000,0,0,0,5,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Stormpike Trainee - Script - Emote Eat (drink)'),
(4891200,9,3,0,0,0,100,0,8000,8000,0,0,0,37,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Stormpike Trainee - Script - Die');
