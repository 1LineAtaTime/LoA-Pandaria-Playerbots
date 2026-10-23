-- Refs #453  (lane E1, 2026-10-21; NEW file, applied by the PM with a verified DB backup)
-- 24817 A Goblin in Shark's Clothing: objective 2 = kill The Hammer 36682 (quest_objective 265687, type 0, amount 1).
-- t438 L14 (w76/w77): the Mechashark casts Laser 71659 / Egg-splosive 71658 (SpellEffect 72711/72710: SCHOOL_DAMAGE 359-398 / 531-684,
-- target ENEMY), 12 spell_go reach the client, castFailed 0, bosshp 36682 0.5 = 1560 hp, yet the kill credit stays 0.
-- The Hammer has no AI at all in either DB (AIName '', ScriptName '', no smart_scripts), so the credit relies only on the core
-- reward gate (Unit::Kill -> IsDamageEnoughForLootingAndReward -> KillRewarder). Same pattern the fork already uses for a kill
-- made from a vehicle: SAI 36621 id0 (death -> SMART_ACTION_CALL_KILLEDMONSTER 36145 to SMART_TARGET_PLAYER_RANGE 50 yd).
-- This gives The Hammer the same death credit to every player within 150 yd (the shark rides ~34 yd away), independent of loot
-- recipient / tap bookkeeping. KilledMonsterCredit is capped at the objective amount, so a normal kill credit cannot double count.
-- NOTE: AIName SmartAI makes the Hammer chase its attacker; it is a rank-1 elite (Health_mod 20) that is meant to fight the shark.
-- Apply ONLY after the L14 probe shows the Hammer dies (hp 0 / corpse) with hammer=0/1; if it does not die the cause is elsewhere
-- (see the lane report).
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 36682 AND `ScriptName` = '';
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36682 AND `source_type` = 0 AND `id` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36682, 0, 0, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 33, 36682, 0, 0, 0, 0, 0, 17, 0, 150, 0, 0, 0, 0, 0, 0, 'The Hammer - On Death - Kill Credit 36682 to players within 150 yd (24817, killed from the Mechashark)');
-- UNDO: UPDATE `creature_template` SET `AIName` = '' WHERE `entry` = 36682; DELETE FROM `smart_scripts` WHERE `entryorguid` = 36682 AND `source_type` = 0;
