-- #963 (lane d): Fleeing Dreghood Warrior (19477) and the Illidari Taskmasters (17058), Ruins of Sha'naar, Hellfire Peninsula.
--
-- INTENDED (sources read 2026-10-28):
--   * Wowpedia API, page "Fleeing Dreghood Warrior" (redirect to "Dreghood Brute", pageid 119729): Dreghood Brutes "are Broken
--     located at Sha'naari Wastes and the Ruins of Sha'naar ... When a nearby Illidari Taskmaster is killed, they turn into
--     Fleeing Dreghood Warriors", with the fleeing quotes "Flee! Flee!" / "The taskmaster's dead! Quick, Dreghoods! Run!" /
--     "This is our chance! Run for it!" (our creature_text 19477 group 0). Wowpedia "Cruel Taskmasters" (quest 9399, Ikan):
--     "slay the Illidari taskmasters so my people may be free!".
--   * Wowhead tooltip API nether.wowhead.com/cata/tooltip/npc/19477: "map":{} - no spawn location; npc/17058 has 34 spawn points.
--   * AzerothCore (the port source): data/sql/old/db_world/8.x/2022_11_09_00.sql, also in base smart_scripts.sql:18085-18092 -
--     Taskmaster On Just Died: brutes (16938) and geomancers (16937) within 20 yd -> Update Template 19477, Set Faction 15,
--     Set Emote State 470, Evade, Say Line 0, Start Random Movement 10, Despawn In 10000 ms. creature_template 19477 faction 35,
--     no 19477 spawns (local acore_world).
--   * TrinityCore TDB 335.26091 and TDB 434.22011 (local dumps, (not shipped), x-5): the same idea through
--     Set Data on the brutes (smart_scripts 16938 ids 2-14, 17058 ids 4-5); 19477 faction 35, no 19477 spawns.
--     SkyFire SFDB 548 26.002: 19477 faction 35, no 19477 spawns.
--
-- WHAT OUR SERVER DID: 19477 had faction 1802 (FactionTemplate 1802 = Alliance, hostile to monsters) and nine static spawns
--   (guids 124280-124288, same as the reference DB/a second 5.4.8 world DB) standing between the Taskmasters (faction 1813, hostile to players). The
--   two sides fought forever (the Taskmaster aggro row 17058/2 talked hundreds of times per hour, see 963b_smartai_talk_targets),
--   Horde players and bots saw them as hostile targets, and killing a Taskmaster did nothing to the brutes.
--
-- CHANGE (AC behaviour, adapted to this core): this core's SMART_ACTION_UPDATE_TEMPLATE only changes the script owner
--   (SmartScript.cpp:951-960, `me`), so AC's "Taskmaster updates the brutes' template" would turn the dead Taskmaster itself
--   into 19477. The Taskmaster therefore sets data 2 2 on the brutes/geomancers within 20 yd (TrinityCore's mechanism), and each
--   living one runs AC's sequence on itself: Update Template 19477, Set Faction 15, Set Emote State 470, Evade, Say Line 0,
--   Random Movement 10, Despawn 10 s. A condition keeps dead brutes (corpses) out (AC target_param4 = 1 "alive only"; this
--   core's CREATURE_RANGE target has no alive filter). The brute respawns as itself (Creature.cpp:1865 restores the original entry).
--   Static 19477 spawns removed; 19477 template faction 35 like every other core.
--
-- Idempotent. UNDO: UPDATE creature_template SET faction=1802 WHERE entry=19477; re-insert the nine creature rows from
--   the reference 5.4.8 world DB (guids 124280-124288); DELETE the smart_scripts / conditions rows inserted below.

-- 1) Template faction as AC / TC335 / TC434 / SFDB548 (35, friendly). The fleeing brute gets 15 from the script like AC.
UPDATE `creature_template` SET `faction`=35 WHERE `entry`=19477;

-- 2) No static Fleeing Dreghood Warriors (none in AC, TC335, TC434, SFDB548; Wowhead lists no location).
DELETE FROM `creature_addon` WHERE `guid` BETWEEN 124280 AND 124288;
DELETE FROM `creature` WHERE `guid` BETWEEN 124280 AND 124288 AND `id`=19477;

-- 3) Illidari Taskmaster: on death, free the brutes and geomancers within 20 yd.
DELETE FROM `smart_scripts` WHERE `entryorguid`=17058 AND `source_type`=0 AND `id` IN (4,5);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(17058,0,4,0,6,0,100,0,0,0,0,0,0,45,2,2,0,0,0,0,9,16938,0,20,0,0,0,0,0,'Illidari Taskmaster - On Just Died - Set Data 2 2 (Dreghood Brutes within 20 yd)'),
(17058,0,5,0,6,0,100,0,0,0,0,0,0,45,2,2,0,0,0,0,9,16937,0,20,0,0,0,0,0,'Illidari Taskmaster - On Just Died - Set Data 2 2 (Dreghood Geomancers within 20 yd)');

-- 4) Dreghood Brute / Dreghood Geomancer: on data 2 2 become a Fleeing Dreghood Warrior and run off (AC 2022_11_09_00 sequence).
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (16937,16938) AND `source_type`=0 AND `id` BETWEEN 2 AND 8;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(16938,0,2,3,38,0,100,0,2,2,0,0,0,36,19477,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Brute - On Data 2 2 Set (Taskmaster died) - Update Template To ''Fleeing Dreghood Warrior'''),
(16938,0,3,4,61,0,100,0,0,0,0,0,0,2,15,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Brute - Linked - Set Faction 15'),
(16938,0,4,5,61,0,100,0,0,0,0,0,0,17,470,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Brute - Linked - Set Emote State 470'),
(16938,0,5,6,61,0,100,0,0,0,0,0,0,24,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Brute - Linked - Evade'),
(16938,0,6,7,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Brute - Linked - Say Line 0 (as Fleeing Dreghood Warrior)'),
(16938,0,7,8,61,0,100,0,0,0,0,0,0,89,10,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Brute - Linked - Start Random Movement 10'),
(16938,0,8,0,61,0,100,0,0,0,0,0,0,41,10000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Brute - Linked - Despawn In 10000 ms'),
(16937,0,2,3,38,0,100,0,2,2,0,0,0,36,19477,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Geomancer - On Data 2 2 Set (Taskmaster died) - Update Template To ''Fleeing Dreghood Warrior'''),
(16937,0,3,4,61,0,100,0,0,0,0,0,0,2,15,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Geomancer - Linked - Set Faction 15'),
(16937,0,4,5,61,0,100,0,0,0,0,0,0,17,470,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Geomancer - Linked - Set Emote State 470'),
(16937,0,5,6,61,0,100,0,0,0,0,0,0,24,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Geomancer - Linked - Evade'),
(16937,0,6,7,61,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Geomancer - Linked - Say Line 0 (as Fleeing Dreghood Warrior)'),
(16937,0,7,8,61,0,100,0,0,0,0,0,0,89,10,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Geomancer - Linked - Start Random Movement 10'),
(16937,0,8,0,61,0,100,0,0,0,0,0,0,41,10000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Dreghood Geomancer - Linked - Despawn In 10000 ms');

-- 5) Only a living brute/geomancer flees (SourceGroup = smart_scripts id + 1, ConditionTarget 1 = the script owner).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceGroup`=3 AND `SourceEntry` IN (16937,16938) AND `SourceId`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,3,16938,0,0,36,1,0,0,0,0,0,0,'','Dreghood Brute - flee on Taskmaster death only if alive'),
(22,3,16937,0,0,36,1,0,0,0,0,0,0,'','Dreghood Geomancer - flee on Taskmaster death only if alive');
