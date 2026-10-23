-- Duskwood quest 26717 "The Yorgen Worgen": Mound of Loose Dirt ambush (epic #441, audit rows SQ020 / SQ021).
--
-- LoA state: GO 204777 (type 10 goober, quest objective) has no AI, so using it only gives the objective.
-- The Lurking Worgen (43799) exists only as ONE permanent static spawn (guid 171159) at the mound, disguised by
-- creature_template_addon aura 70414 (transform into Invisible Stalker) + emote 433, AIName '', unit_flags 768
-- (IMMUNE_TO_PC|IMMUNE_TO_NPC) - it never does anything.
--
-- Intended design (Wowpedia "The Yorgen Worgen", MediaWiki API, fetched 2026-10-04): "Interact with it and a
-- Lurking Worgen will jump out from the woods and knock the adventurer down, stunning it, before jumping away
-- behind the stables." Completion text: "Sounds like a close call! But he ran off without harming you further..."
-- Stunning Pounce 81957 (Wowhead mop-classic tooltip): "Charges and knocks down the target", 40 yd, instant.
--
-- Mechanism: local patch 0025_skyfire_smartai_quest_fixes.sql (from ProjectSkyfire
-- 2026_08_24_world_03 / 2026_08_25_world_01): the mound summons the worgen at the sniffed spot (attacking the
-- user) and the worgen opens with Stunning Pounce. LoA changes vs the reference, to match the Wowpedia design:
--  * the worgen despawns 4 s after engaging (it "runs off") instead of fighting to the death; 30 s timed
--    summon as a safety net;
--  * unit_flags 768 -> 0 so the summon can engage and be hit without relying on the flag being stripped;
--  * LoA smart_scripts extra columns event_param5 / target_param4 = 0.
-- SMART_EVENT_GO_STATE_CHANGED (70) state 2 = GO_ACTIVATED, raised by GameObject::SetLootState with the user as
-- invoker (SmartAI.cpp:1081 SmartGameObjectAI::OnStateChanged).
-- Static spawn removed by guid 171159, guarded by id + coordinates (no addon/pool/event/formation/SAI refs).
--
-- Rollback: UPDATE `gameobject_template` SET `data3` = 3000, `AIName` = '' WHERE `entry` = 204777;
--   DELETE FROM `smart_scripts` WHERE `entryorguid` IN (204777, 43799); UPDATE `creature_template` SET `AIName` = '',
--   `unit_flags` = 768 WHERE `entry` = 43799; restore creature 171159 + creature_template_addon 43799 from a dump.

-- Mound of Loose Dirt: 30 s auto-close (anti-spam, as the reference) + SmartGameObjectAI.
UPDATE `gameobject_template` SET `data3` = 30000, `AIName` = 'SmartGameObjectAI' WHERE `entry` = 204777;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 204777 AND `source_type` = 1;
INSERT INTO `smart_scripts`
(`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,
 `event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,
 `action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,
 `target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`)
VALUES
(204777,1,0,0,70,0,100,0,2,0,0,0,0,12,43799,1,30000,1,0,0,8,0,0,0,0,-11127.5,-518.424,35.2569,0.436332,'Mound of Loose Dirt - On Activate - Summon Lurking Worgen (attack invoker)');

-- Remove LoA's permanent disguised Lurking Worgen at the summon spot.
DELETE FROM `creature_addon` WHERE `guid` = 171159;
DELETE FROM `creature`
WHERE `guid` = 171159 AND `id` = 43799 AND `map` = 0
  AND ABS(`position_x` - (-11127.5)) < 0.1
  AND ABS(`position_y` - (-518.424)) < 0.1
  AND ABS(`position_z` - 35.2569) < 0.1;

-- Lurking Worgen: drop the Invisible-Stalker disguise, make it engageable, give it SmartAI.
DELETE FROM `creature_template_addon` WHERE `entry` = 43799;
UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = '', `unit_flags` = 0 WHERE `entry` = 43799;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 43799 AND `source_type` = 0;
INSERT INTO `smart_scripts`
(`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,
 `event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,
 `action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,
 `target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`)
VALUES
(43799,0,0,0,0,0,100,1,0,0,0,0,0,11,81957,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Lurking Worgen - In Combat - Cast Stunning Pounce (No Repeat)'),
(43799,0,1,0,0,0,100,1,4000,4000,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Lurking Worgen - In Combat 4 s - Despawn (runs off)');
