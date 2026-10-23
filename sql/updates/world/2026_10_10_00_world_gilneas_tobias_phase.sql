-- Refs #475, #439. cp8 24902 "The Hunt For Sylvanas": Tobias Mistmantle 38507 is in combat again and never walks.
-- SQL only, no build needed.
--
-- Evidence (wave 30, build 30, state/t439/cp8.log + cp8.server.log): after the accept Tobias repeats Talk 0 ("Let us go
-- at once...") every ~5 s and the server logs "SmartAI::StartPath: Creature entry 38507 wanted to start waypoint
-- movement while in combat, ignoring." x10. The player is not in combat (PROBE attackers=0, flags=-), no bot is on map
-- 654, and 2026_10_09_11 is still applied (creature_template 38507 unit_flags 33536 = IMMUNE_TO_PC|IMMUNE_TO_NPC).
-- Waves 27-29 passed with the same data; wave 25 failed the same way before _11.
--
-- Cause: the immune flags do not keep him out of combat. Unit::Attack (Unit.cpp:8745-8766) checks neither
-- IMMUNE_TO_PC nor IMMUNE_TO_NPC, so any AttackStart/threat path that skips IsValidAttackTarget still puts him in
-- combat. He is summoned by Lorna (smart_scripts 38611 id 0, at -1654.18 1604.09 23.13) and inherits her phaseMask
-- 2883584 (0x40000 | 0x80000 | 0x200000), which shares 0x40000 with the Greymane Court static spawns 20-32 yd away:
-- the three elite Soultethered Banshees 38473 (222310-222312), Lady Sylvanas 222313 (npc_lady_sylvanas_gilneas,
-- aggressive) and Prince Liam 222991. Which of them engages him depends on the state cp7's battle leaves behind
-- (finale despawn/respawn timers), hence the intermittent failure. Not proven which one: no NPC probe.
--
-- Fix: on Just Summoned, Tobias leaves the shared phases and keeps only 0x200000 (2097152, phase_definitions 4714/10,
-- "after 24904", the phase a player on 24902 is in). Nothing at Greymane Court in 0x40000 can see or reach him any
-- more. His scene summons (Sylvanas 38530, General 38533, Crenshaw 38537) inherit his phase and only target one another
-- (their SAI rows), and the Forsaken General 38617 his WP2 row talks to is a 2097152 spawn (guid 219454).
-- The phase change runs inside JustSummoned (TempSummon::InitSummon), before his first update, so he is never in
-- 0x40000 during a grid visit.
--
-- Backup: smart_scripts 38507 rows 19 and 20 as they are now:
--   (38507,0,19,20,54,0,100,0,0,0,0,0,0, 8,0,0,0,0,0,0,1,...,'Tobias Mistmantle - On Just Summoned - Set Passive')
--   (38507,0,20, 0,61,0,100,0,0,0,0,0,0,18,768,...,1,...,'Tobias Mistmantle - On Just Summoned - Immune to PC and NPC ...')
-- Rollback:
--   DELETE FROM `smart_scripts` WHERE `entryorguid` = 38507 AND `source_type` = 0 AND `id` = 21;
--   UPDATE `smart_scripts` SET `link` = 0 WHERE `entryorguid` = 38507 AND `source_type` = 0 AND `id` = 20;
-- Re-runnable: UPDATE + DELETE-then-INSERT.

UPDATE `smart_scripts` SET `link` = 21 WHERE `entryorguid` = 38507 AND `source_type` = 0 AND `id` = 20;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 38507 AND `source_type` = 0 AND `id` = 21;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38507, 0, 21, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 44, 2097152, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - On Just Summoned - Phase 2097152 only (out of the Greymane Court 0x40000 spawns)');
