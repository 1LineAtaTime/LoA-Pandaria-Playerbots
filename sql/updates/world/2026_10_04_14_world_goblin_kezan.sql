-- #438 dev A (Kezan, zone 4737, goblin level 1-5) - everything the Kezan quests still lacked.
-- REQUIRES the matching C++ from the next build (src/server/scripts/Maelstrom/zone_kezan.cpp,
-- namespace KezanGoblin): every ScriptName and spell_script_names row below binds a class that
-- only exists after that build. Do not apply this file to a worldserver without that build: the
-- hack SmartAI rows it removes would be gone and nothing would replace them.
--
-- Contents (source for every block in its header):
--  1. 24502 Necessary Roughness / 24503+28414 Fourth and Goal (Coach Crosscheck 37106, shredders 37179/37213,
--     sharks 37114, goal target 37203, Deathwing 48572).
--  2. 14122 The Great Bank Heist (vault vehicle 35486, spell 67555).
--  3. 14125 447 (Gasbot control panel spell 70253, Gasbot 37598).
--  4. 14124 Kablooey Bombs (67682), 14121 Robbing Hoods citizens (35063/35075/35234), Fireworks (66912),
--     14126 Life Savings reward spell (91847) -> Lost Isles hand-off.
--  5. 14069 Defiant Troll 34830 (spell-hit trigger + sleep aura, as intended).
--  6. SmartAI import for Kezan NPCs that are SmartAI-only in the MoP data and have none here
--     (class-quest trainers, Do it Yourself targets, Dampwick, Pirate Party Crashers, Hobart, Greely)
--     + Sassy Hardwrench quest lines.
--
-- Fake-completion hacks removed here, each in the same block that binds its working replacement:
--   35486 vault: SAI 3548600 (kill credit + create Personal Riches + say lines on summon),
--   37213 shredder: SAI 3721300 (kill credit 37203 on summon),
--   37114 shark: SAI id2 (every shark sent a data-set to the shredder 4-16 s after spawning, which credited 37114 by itself),
--   37179: SAI 3717900 (shark spawns, now done in C++).
-- Left untouched on purpose: Sassy 34668 gossip 12493 SAI (teleport to the yacht dock, see block 6 note).
--
-- Rollback (restores the pre-file state; the SAI rows deleted below are listed so they can be re-inserted):
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` IN (37106, 37114, 37179, 37203, 37213, 35486, 37598, 35063, 35075, 35234);
--   UPDATE `creature_template` SET `AIName` = 'SmartVehicleAI' WHERE `entry` IN (37179, 37213, 35486);
--   UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 37114;
--   UPDATE `creature_template` SET `AIName` = '' WHERE `entry` IN (34673, 34689, 34693, 34695, 34696, 34835, 34872, 34876, 34877, 34878, 35200, 48494, 48496, 48572);
--   UPDATE `quest_template_addon` SET `ScriptName` = '' WHERE `ID` IN (24503, 28414);
--   UPDATE `quest_template` SET `RewardNextQuest` = 24503 WHERE `ID` = 24502;
--   DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_fourth_and_goal_kick_footbomb', 'spell_great_bank_heist_vault_interact', 'spell_summon_gasbot', 'spell_kabummbomb', 'spell_kezan_fireworks', 'spell_life_saving_complete');
--   DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` IN (34673, 34689, 34693, 34695, 34696, 34835, 34872, 34876, 34877, 34878, 35200, 48494, 48496, 48572);
--   DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 34668 AND `id` >= 2;
--   DELETE FROM `creature_text` WHERE (`CreatureID` = 37106 AND `GroupID` IN (3, 4)) OR (`CreatureID` IN (34689, 34696) AND `GroupID` = 0) OR (`CreatureID` IN (34695, 35200) AND `GroupID` = 1) OR `CreatureID` = 48572;
--   DELETE FROM `waypoints` WHERE `entry` = 4857200;
--   DELETE FROM `creature_template_movement` WHERE `CreatureId` = 48572;
--   UPDATE `creature_template_addon` SET `auras` = NULL WHERE `entry` = 34830;
--   34830 SAI: UPDATE `smart_scripts` SET `event_type` = 73, `event_param1` = 0 WHERE `entryorguid` = 34830 AND `source_type` = 0 AND `id` = 1; set `link` = 0 on id 3; delete id 6.
--   Removed hack rows (re-insert after the DELETEs above if rolling back):
--   INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`)
--   VALUES
--   (37179, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 0, 80, 3717900, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bilgewater Buccaneer'),
--   (37179, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 64, 1, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 'Bilgewater Buccaneer'),
--   (37179, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 103, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bilgewater Buccaneer'),
--   (37179, 0, 3, 0, 38, 0, 100, 0, 1, 0, 0, 0, 0, 33, 37114, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 0, 'Bilgewater Buccaneer'),
--   (37213, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 0, 80, 3721300, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bilgewater Buccaneer'),
--   (37213, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 64, 1, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Bilgewater Buccaneer'),
--   (37213, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 103, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bilgewater Buccaneer'),
--   (37114, 0, 0, 1, 8, 0, 100, 1, 69993, 0, 0, 0, 0, 33, 37114, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Steamwheedle Shark'),
--   (37114, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 37, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Steamwheedle Shark'),
--   (37114, 0, 2, 3, 1, 0, 100, 1, 4000, 16000, 0, 0, 0, 45, 1, 0, 0, 0, 0, 0, 9, 37179, 0, 100, 0, 0, 0, 0, 0, 'Steamwheedle Shark'),
--   (37114, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 37, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Steamwheedle Shark'),
--   (35486, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 0, 80, 3548600, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'First Bank of Kezan Vault - On Respawn - Run Script'),
--   (35486, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 64, 1, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'First Bank of Kezan Vault'),
--   (35486, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 103, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'First Bank of Kezan Vault - On Respawn - Disallow Movement'),
--   (3717900, 9, 0, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 0, 12, 37114, 2, 60000, 0, 0, 0, 8, 0, 0, 0, 0, -8286.64, 1490.95, 43.7856, 6.27674, 'Bilgewater Buccaneer'),
--   (3717900, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 12, 37114, 2, 60000, 0, 0, 0, 8, 0, 0, 0, 0, -8286.68, 1476.79, 43.6459, 0.00141, 'Bilgewater Buccaneer'),
--   (3717900, 9, 2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 12, 37114, 2, 60000, 0, 0, 0, 8, 0, 0, 0, 0, -8286.68, 1481.47, 43.6484, 6.26103, 'Bilgewater Buccaneer'),
--   (3717900, 9, 3, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 12, 37114, 2, 60000, 0, 0, 0, 8, 0, 0, 0, 0, -8286.57, 1486.48, 43.7861, 6.25709, 'Bilgewater Buccaneer'),
--   (3717900, 9, 4, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 12, 37114, 2, 60000, 0, 0, 0, 8, 0, 0, 0, 0, -8291.25, 1477.05, 43.9937, 6.23907, 'Bilgewater Buccaneer'),
--   (3717900, 9, 5, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 12, 37114, 2, 60000, 0, 0, 0, 8, 0, 0, 0, 0, -8291.18, 1491.21, 44.0269, 6.23515, 'Bilgewater Buccaneer'),
--   (3717900, 9, 6, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 12, 37114, 2, 60000, 0, 0, 0, 8, 0, 0, 0, 0, -8291.06, 1486.55, 43.9885, 6.26263, 'Bilgewater Buccaneer'),
--   (3717900, 9, 7, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 12, 37114, 2, 60000, 0, 0, 0, 8, 0, 0, 0, 0, -8291.13, 1481.69, 43.9892, 6.26656, 'Bilgewater Buccaneer'),
--   (3721300, 9, 0, 0, 0, 0, 100, 0, 1, 1, 0, 0, 0, 86, 46598, 0, 12, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bilgewater Buccaneer'),
--   (3721300, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 33, 37203, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 0, 'Bilgewater Buccaneer'),
--   (3548600, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 33, 35486, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 0, 'First Bank of Kezan Vault - On Script - Give Kill Credit'),
--   (3548600, 9, 1, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 0, 'First Bank of Kezan Vault - On Script - Say Text 0'),
--   (3548600, 9, 2, 0, 0, 0, 100, 0, 10000, 10000, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 0, 'First Bank of Kezan Vault - On Script - Say Text 1'),
--   (3548600, 9, 3, 0, 0, 0, 100, 0, 10000, 10000, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 0, 'First Bank of Kezan Vault - On Script - Say Text 2'),
--   (3548600, 9, 4, 0, 0, 0, 100, 0, 10000, 10000, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 0, 'First Bank of Kezan Vault - On Script - Say Text 3'),
--   (3548600, 9, 5, 0, 0, 0, 100, 0, 10000, 10000, 0, 0, 0, 56, 46858, 1, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 0, 'First Bank of Kezan Vault - On Script - Create Item Personal Riches'),
--   (3548600, 9, 6, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 12, 1, 0, 0, 0, 0, 0, 0, 0, 'First Bank of Kezan Vault - On Script - Say Text 4'),
--   (3548600, 9, 7, 0, 0, 0, 100, 0, 500, 500, 0, 0, 0, 28, 67476, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'First Bank of Kezan Vault - On Script - Forced Despawn');

-- ===========================================================================
-- 1. Footbomb: 24502 Necessary Roughness, 24503/28414 Fourth and Goal
-- ===========================================================================
-- Source: AshamaneCore kezan.cpp (npc_coach_crosscheck, npc_boucanier_gob, npc_shark_gob,
-- npc_fourth_and_goal_target, quest_fourth_and_goal) and MoPCore kezan.cpp npc_gw/npc_boucanier_gob/
-- npc_shark_gob; ScriptNames are npc_gw, npc_boucanier_gob, npc_shark_gob,
-- npc_fourth_and_goal_target. The retail wiring is already
-- data: creature 48526 (parked shredder, spellclick 70015 for 24502 / 70075 for 24503+28414, quest
-- conditions in `conditions`); the summon spell rides the player and 70016 gives credit 48271.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_gw' WHERE `entry` = 37106;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_boucanier_gob' WHERE `entry` IN (37179, 37213);
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_shark_gob' WHERE `entry` = 37114;
UPDATE `creature_template` SET `ScriptName` = 'npc_fourth_and_goal_target' WHERE `entry` = 37203;

-- Fake-completion hack SAI on the shredders/sharks (see header) - the C++ above replaces it.
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (37179, 37213, 37114) AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (3717900, 3721300) AND `source_type` = 9;

-- 28414 (goblin) and 24503 (the other races' copy): swap shredder, earthquake + Deathwing on completion.
UPDATE `quest_template_addon` SET `ScriptName` = 'quest_fourth_and_goal' WHERE `ID` IN (24503, 28414);

-- 24502 is goblin-only (AllowableRaces 256) but chained to 24503, which goblins cannot take (races 33555378,
-- no goblin); the goblin follow-up is 28414 (AllowableRaces 768, ExclusiveGroup 24503). Wowpedia 24502: "Fourth and
-- Goal starts automatically after" -> with 28414 flagged auto-accept (3670016) the reward now hands it over.
-- AshamaneCore 005_kezan_quest_chains.sql sets the same chain (24502 -> 28414 -> 24520).
UPDATE `quest_template` SET `RewardNextQuest` = 28414 WHERE `ID` = 24502;

-- 70052 Kick Footbomb: credit the kick (the DB's target condition names the shredder, not the goal target).
DELETE FROM `spell_script_names` WHERE `spell_id` = 70052;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(70052, 'spell_fourth_and_goal_kick_footbomb');

-- Coach boss-emote hints shown when boarding (Wowpedia 24502 "On getting in the vehicle"); MoP 5.4.8 has groups 3-4, LoA only 0-2.
DELETE FROM `creature_text` WHERE `CreatureID` = 37106 AND `GroupID` IN (3, 4);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`)
VALUES
(37106, 3, 0, 'Throw your Footbombs at those Steamwheedle Sharks!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'say Text'),
(37106, 4, 0, 'Kick the Footbomb between the smokestacks up and behind the opposing goal!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'say Text');

-- 48572 Deathwing (Fourth and Goal fly-over, summoned by 66322): MoP SmartAI + path 4857200 + 2 lines.
-- The MoP row 4 is an orphaned LINK event: here it is linked from the wp-6 pause so the second line plays.
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 48572;
DELETE FROM `creature_template_movement` WHERE `CreatureId` = 48572;
INSERT INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`) VALUES
(48572, 0, 0, 1, 0);
DELETE FROM `creature_text` WHERE `CreatureID` = 48572;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`)
VALUES
(48572, 0, 0, 'The sun has set on this mortal world, fools. Make peace with your end, for the hour of twilight falls!', 14, 0, 100, 0, 0, 23228, 0, 0, 0, 'Deathwing Fourth and Goal'),
(48572, 1, 0, 'What did that dragon do to Mount Kajaro?!!!', 41, 0, 100, 0, 0, 0, 0, 0, 0, 'Deathwing Fourth and Goal');
DELETE FROM `waypoints` WHERE `entry` = 4857200;
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `point_comment`) VALUES
(4857200, 1, -8389.72, 1482.54, 166.736, 0, 0, NULL),
(4857200, 2, -8435.36, 1506.08, 175.315, 0, 0, NULL),
(4857200, 3, -8428.3, 1555.11, 166.445, 0, 0, NULL),
(4857200, 4, -8400.05, 1550.83, 146.242, 0, 0, NULL),
(4857200, 5, -8306.45, 1508.1, 85.1214, 0, 0, NULL),
(4857200, 6, -8290.1, 1498.44, 74.3326, 0, 0, NULL),
(4857200, 7, -7905.62, 1334.69, 38.9364, 0, 0, NULL);
DELETE FROM `smart_scripts` WHERE `entryorguid` = 48572 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`)
VALUES
(48572, 0, 0, 1, 54, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On summoned talk 0'),
(48572, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 4857200, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Do wp start '),
(48572, 0, 2, 4, 40, 0, 100, 0, 6, 4857200, 0, 0, 0, 54, 3000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Wp 6 make a pause end finish fly after that'),
(48572, 0, 3, 0, 40, 0, 100, 0, 7, 4857200, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On wp 7 reached force despawn'),
(48572, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On link say text 1');

-- ===========================================================================
-- 2. 14122 The Great Bank Heist
-- ===========================================================================
-- Source: AshamaneCore kezan.cpp spell_great_bank_heist_vault_interact / npc_first_bank_of_kezan_vault.
-- Vault doors = gameobject 195449 (type 22, spell 67555). No script was bound to 67555 (spell_bank_67495-99 /
-- gob_bank_gobelin are not bound either), so the quest could not start.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_first_bank_of_kezan_vault' WHERE `entry` = 35486;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35486 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 3548600 AND `source_type` = 9;
DELETE FROM `spell_script_names` WHERE `spell_id` = 67555;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(67555, 'spell_great_bank_heist_vault_interact');

-- ===========================================================================
-- 3. 14125 447
-- ===========================================================================
-- Source: AshamaneCore kezan.cpp spell_summon_gasbot / npc_kezan_gasbot (MoPCore: gob_control_gobelin /
-- npc_robot_gob). The three sabotage objectives are plain
-- quest goobers (201733/201734/201735, data1 = 14125 -> GameObject::Use KillCreditGO) and already work.
-- Gasbot Control Panel 201736 (type 22) casts 70253 -> summons the Gasbot (70252) at the panel.
UPDATE `creature_template` SET `ScriptName` = 'npc_kezan_gasbot' WHERE `entry` = 37598;
DELETE FROM `spell_script_names` WHERE `spell_id` = 70253;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(70253, 'spell_summon_gasbot');

-- ===========================================================================
-- 4. 14124 Kablooey Bombs, 14121 Robbing Hoods, Life of the Party fireworks, 14126 Life Savings
-- ===========================================================================
-- 67682 Kablooey Bombs: blows a Kaja'mite Deposit (195488) into 3 Kaja'mite Chunks (195492)
-- (AshamaneCore/MoPCore spell_kabummbomb; the same name is bound on the same spell id in
-- spell_script_names, LoA had none). The deposit's linked trap spell 67507 is not in the DBC.
DELETE FROM `spell_script_names` WHERE `spell_id` = 67682;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(67682, 'spell_kabummbomb');

-- 35075 Kezan Citizen / 35234 Hired Looter: ScriptName npc_citoyen_gob (AshamaneCore binds
-- 35063 + 35234); 35063 (the other citizen entry, 433 spawns) gets it too. Reacts to the Hot Rod
-- knockback 66301 (14121: looters die and drop Stolen Loot).
UPDATE `creature_template` SET `ScriptName` = 'npc_citoyen_gob' WHERE `entry` IN (35063, 35075, 35234);

-- 66912 Fireworks (party bar): cosmetic firework volley (AshamaneCore spell_kezan_fireworks).
DELETE FROM `spell_script_names` WHERE `spell_id` = 66912;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(66912, 'spell_kezan_fireworks');

-- 91847 Life Savings: Quest Complete (quest 14126 reward spell): movie 22 then 74100 (bind + teleport to
-- Shipwreck Shore, Lost Isles; spell_target_position 74100 -> 534.8, 3272.9, 0.17 exists). AshamaneCore spell_life_saving_complete.
DELETE FROM `spell_script_names` WHERE `spell_id` = 91847;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(91847, 'spell_life_saving_complete');

-- ===========================================================================
-- 5. 14069 Good Help is Hard to Find - Defiant Troll 34830
-- ===========================================================================
-- LoA triggered the troll on SMART_EVENT_ON_SPELLCLICK (73); MoP 5.4.8 triggers on the
-- 66306 spell hit (event 8), removes the sleep aura 62248 (link 6) and spawns the troll under
-- Enrage 45111 + Sleeping 62248 (creature_template_addon.auras). Same here.
UPDATE `smart_scripts` SET `event_type` = 8, `event_param1` = 66306, `event_param2` = 0, `event_param3` = 0, `event_param4` = 0, `comment` = 'Defiant Troll - On Spell Hit (Goblin All-In-1-Der Belt Shocker) - Remove Aura Enrage' WHERE `entryorguid` = 34830 AND `source_type` = 0 AND `id` = 1;
UPDATE `smart_scripts` SET `link` = 6 WHERE `entryorguid` = 34830 AND `source_type` = 0 AND `id` = 3;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 34830 AND `source_type` = 0 AND `id` = 6;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`)
VALUES
(34830, 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 28, 62248, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On link remove sleep from self');
UPDATE `creature_template_addon` SET `auras` = '45111 62248' WHERE `entry` = 34830;

-- ===========================================================================
-- 6. SmartAI import (Kezan NPCs that are SmartAI-only in the MoP data and have no AI here)
-- ===========================================================================
-- Source: MoP 5.4.8 world data smart_scripts (every row validated against SmartScriptMgr.h: all
-- event/action/target ids exist in this fork; every cast spell is in Spell.dbc; every talk group exists
-- or is imported below; GUID targets 228870/228873/228921 exist in `creature`).
--   34673 Bamm Megabomb (class-quest dummy shooter; MoP row 1 "say group 0" skipped: no text),
--   34689 Fizz Lighter / 34696 Evol Fingers / 34693 Slinky Sharpshiv / 34695 Maxx Avalanche (class trainers 14008/14012/14010+14123/14011),
--   34835 Bruno / 34876 Frankie / 34877 Jack / 34878 Sudsy (14070 Do it Yourself: at 30 percent health credit + faction 35, at 20 percent stop combat),
--   34872 Foreman Dampwick (14138 gift reaction, 25473/14124 lines), 35200 Pirate Party Crasher (14115 lines + Swashbuckling Slice),
--   48494 Hobart Grapplehammer / 48496 Assistant Greely (HQ ambient lines).
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (34673, 34689, 34693, 34695, 34696, 34835, 34872, 34876, 34877, 34878, 35200, 48494, 48496);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (34673, 34689, 34693, 34695, 34696, 34835, 34872, 34876, 34877, 34878, 35200, 48494, 48496) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`)
VALUES
(34673, 0, 0, 0, 1, 0, 100, 0, 1000, 9000, 5000, 25000, 0, 11, 73389, 0, 0, 0, 0, 0, 10, 228921, 48304, 0, 0, 0, 0, 0, 0, 'Attack Auto 73389'),
(34689, 0, 0, 0, 1, 0, 100, 0, 5000, 5000, 15000, 15000, 0, 11, 69608, 0, 0, 0, 0, 0, 10, 228873, 34696, 0, 0, 0, 0, 0, 0, 'Cast Fireball OOC'),
(34689, 0, 1, 0, 19, 0, 100, 0, 14008, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on quest 14008 say0'),
(34693, 0, 0, 0, 1, 0, 100, 0, 500, 1000, 240000, 240000, 0, 11, 1784, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Stealth on Spawn'),
(34693, 0, 1, 0, 19, 0, 100, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on quest accept 14010 say1'),
(34693, 0, 2, 0, 19, 0, 100, 0, 14123, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on quest accept 14123 say 0'),
(34693, 0, 3, 0, 20, 0, 100, 0, 14010, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on questcomplete set emote4'),
(34693, 0, 4, 0, 20, 0, 100, 0, 14123, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on quest complete 14123 emote4'),
(34693, 0, 5, 0, 1, 0, 100, 0, 5000, 30000, 10000, 50000, 0, 28, 1784, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Remove aura 1784 after some time'),
(34695, 0, 0, 0, 1, 0, 100, 0, 5100, 71000, 1000, 610000, 0, 11, 69626, 0, 0, 0, 0, 0, 9, 24656, 0, 100, 0, 0, 0, 0, 0, 'Cast 69626'),
(34695, 0, 1, 0, 1, 0, 100, 0, 500, 1000, 600000, 600000, 0, 11, 78273, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Flametongue Weapon on Spawn'),
(34695, 0, 2, 0, 19, 0, 100, 0, 14011, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on quest add 14011 say0'),
(34695, 0, 3, 0, 1, 0, 50, 0, 50000, 120000, 50000, 120000, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Parl'),
(34696, 0, 0, 0, 1, 0, 100, 0, 10000, 10000, 20000, 20000, 0, 11, 69607, 0, 0, 0, 0, 0, 10, 228870, 34689, 0, 0, 0, 0, 0, 0, 'Cast Shadow bolt OOC'),
(34696, 0, 16, 0, 11, 0, 100, 1, 3000, 5000, 0, 0, 0, 11, 44163, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Summon Imp Company'),
(34696, 0, 17, 0, 19, 0, 100, 0, 14012, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on quest add 14012 say0'),
(34835, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On aggro say.'),
(34835, 0, 1, 2, 2, 0, 100, 1, 0, 30, 0, 0, 0, 33, 34835, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'At 20% set kill credit'),
(34835, 0, 2, 3, 61, 0, 100, 1, 0, 0, 0, 0, 0, 2, 35, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Faction 35 stop combat'),
(34835, 0, 3, 4, 61, 0, 100, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say on finish'),
(34835, 0, 4, 0, 1, 0, 100, 1, 5000, 5000, 0, 0, 0, 2, 7, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Reprend sa faction'),
(34835, 0, 201, 0, 2, 0, 100, 1, 0, 20, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'At 20% stop combat'),
(34835, 0, 202, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 20, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on 3 sec ooc set autoattack state to 1'),
(34872, 0, 0, 1, 20, 0, 100, 0, 14138, 0, 0, 0, 0, 1, 0, 3000, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on quest saytext0'),
(34872, 0, 1, 2, 52, 0, 100, 0, 0, 34872, 0, 0, 0, 11, 87226, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'spellhit cadeau'),
(34872, 0, 2, 3, 52, 0, 100, 0, 0, 34872, 0, 0, 0, 1, 1, 3000, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'link say text1'),
(34872, 0, 3, 0, 52, 0, 100, 0, 1, 0, 0, 0, 0, 1, 2, 3000, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'link say text2'),
(34872, 0, 4, 0, 19, 0, 100, 0, 25473, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on quest 25473 say text'),
(34872, 0, 5, 0, 19, 0, 100, 0, 14124, 0, 0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on quest 14124 say text 4'),
(34872, 0, 6, 0, 1, 0, 50, 0, 45000, 245000, 45000, 245000, 0, 1, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'criez say5'),
(34876, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On aggro say.'),
(34876, 0, 1, 2, 2, 0, 100, 1, 0, 30, 0, 0, 0, 33, 34876, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'At 20% set kill credit'),
(34876, 0, 2, 3, 61, 0, 100, 1, 0, 0, 0, 0, 0, 2, 35, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Flee'),
(34876, 0, 3, 4, 61, 0, 100, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say on finish'),
(34876, 0, 4, 0, 1, 0, 100, 1, 5000, 5000, 0, 0, 0, 2, 7, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Type a script description here.'),
(34876, 0, 5, 0, 0, 0, 100, 0, 2000, 2000, 15000, 15000, 0, 11, 75775, 0, 0, 0, 0, 0, 18, 15, 0, 0, 0, 0, 0, 0, 0, 'cast spell Throw Gears'),
(34876, 0, 201, 0, 2, 0, 100, 1, 0, 20, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'at 20% stop combat'),
(34876, 0, 202, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 20, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on 3 sec ooc set autoattack state to 1'),
(34877, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On aggro say.'),
(34877, 0, 1, 2, 2, 0, 100, 1, 0, 30, 0, 0, 0, 33, 34877, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'At 20% set kill credit'),
(34877, 0, 2, 201, 61, 0, 100, 1, 0, 0, 0, 0, 0, 2, 35, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Flee'),
(34877, 0, 3, 0, 1, 0, 100, 1, 5000, 5000, 0, 0, 0, 2, 7, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Type a script description here.'),
(34877, 0, 4, 0, 0, 0, 100, 0, 2000, 2000, 15000, 15000, 0, 11, 75986, 0, 0, 0, 0, 0, 18, 20, 0, 0, 0, 0, 0, 0, 0, 'cast spell repousser'),
(34877, 0, 201, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Type a script description here.'),
(34877, 0, 202, 0, 2, 0, 100, 1, 0, 20, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'at 20% stop combat'),
(34877, 0, 203, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 20, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on 3 sec ooc set autoattack state to 1'),
(34878, 0, 0, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On aggro say.'),
(34878, 0, 1, 2, 2, 0, 100, 1, 0, 30, 0, 0, 0, 33, 34878, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'At 20% set kill credit'),
(34878, 0, 2, 3, 61, 0, 100, 1, 0, 0, 0, 0, 0, 2, 35, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Flee'),
(34878, 0, 3, 4, 61, 0, 100, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say on finish'),
(34878, 0, 4, 0, 1, 0, 100, 1, 5000, 5000, 0, 0, 0, 2, 7, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Type a script description here.'),
(34878, 0, 5, 0, 0, 0, 100, 0, 2000, 2000, 12000, 12000, 0, 11, 75761, 0, 0, 0, 0, 0, 18, 20, 0, 0, 0, 0, 0, 0, 0, ''),
(34878, 0, 201, 0, 2, 0, 100, 1, 0, 20, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on healt 20% stop combat'),
(34878, 0, 202, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 20, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'on 3 sec ooc set autoattack state to 1'),
(35200, 0, 0, 0, 1, 0, 20, 0, 10000, 80000, 10000, 320000, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Out of combat'),
(35200, 0, 1, 0, 4, 0, 60, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say on aggro'),
(35200, 0, 2, 0, 0, 0, 100, 0, 5000, 8900, 11000, 17800, 0, 11, 75361, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Cast Swashbuckling Slice'),
(48494, 0, 0, 0, 60, 0, 100, 0, 5000, 5000, 360000, 360000, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 1, 0, 60, 0, 100, 0, 25000, 25000, 380000, 380000, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 2, 0, 60, 0, 100, 0, 50000, 50000, 405000, 405000, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 3, 0, 60, 0, 100, 0, 60000, 60000, 415000, 415000, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 4, 0, 60, 0, 100, 0, 75000, 75000, 430000, 430000, 0, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 5, 0, 60, 0, 100, 0, 85000, 85000, 440000, 440000, 0, 1, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 6, 0, 60, 0, 100, 0, 90000, 90000, 445000, 445000, 0, 1, 6, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 7, 0, 60, 0, 100, 0, 110000, 110000, 465000, 465000, 0, 1, 7, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 8, 0, 60, 0, 100, 0, 115000, 115000, 470000, 470000, 0, 1, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 9, 0, 60, 0, 100, 0, 120000, 120000, 475000, 475000, 0, 1, 9, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 10, 0, 60, 0, 100, 0, 125000, 125000, 480000, 480000, 0, 1, 10, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 11, 0, 60, 0, 100, 0, 135000, 135000, 490000, 490000, 0, 1, 11, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 12, 0, 60, 0, 100, 0, 140000, 140000, 495000, 495000, 0, 1, 12, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 13, 0, 60, 0, 100, 0, 145000, 145000, 500000, 500000, 0, 1, 13, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 14, 0, 60, 0, 100, 0, 165000, 165000, 520000, 520000, 0, 1, 14, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 15, 0, 60, 0, 100, 0, 185000, 185000, 540000, 540000, 0, 1, 15, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 16, 0, 60, 0, 100, 0, 195000, 195000, 550000, 550000, 0, 1, 16, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 17, 0, 60, 0, 100, 0, 200000, 200000, 555000, 555000, 0, 1, 17, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 18, 0, 60, 0, 100, 0, 210000, 210000, 565000, 565000, 0, 1, 18, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 19, 0, 60, 0, 100, 0, 230000, 230000, 585000, 585000, 0, 1, 19, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 20, 0, 60, 0, 100, 0, 235000, 235000, 590000, 590000, 0, 1, 20, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 21, 0, 60, 0, 100, 0, 270000, 270000, 625000, 625000, 0, 1, 21, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48494, 0, 22, 0, 64, 0, 100, 0, 0, 0, 0, 0, 0, 10, 1, 3, 5, 6, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Play Emote on Gossip'),
(48496, 0, 0, 0, 60, 0, 100, 0, 10000, 10000, 365000, 365000, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 1, 0, 60, 0, 100, 0, 20000, 20000, 375000, 375000, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 2, 0, 60, 0, 100, 0, 27000, 27000, 382000, 382000, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 3, 0, 60, 0, 100, 0, 35000, 35000, 390000, 390000, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 4, 0, 60, 0, 100, 0, 55000, 55000, 410000, 410000, 0, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 5, 0, 60, 0, 100, 0, 95000, 95000, 450000, 450000, 0, 1, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 6, 0, 60, 0, 100, 0, 100000, 100000, 455000, 455000, 0, 1, 6, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 7, 0, 60, 0, 100, 0, 155000, 155000, 510000, 510000, 0, 1, 7, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 8, 0, 60, 0, 100, 0, 175000, 175000, 530000, 530000, 0, 1, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 9, 0, 60, 0, 100, 0, 195000, 195000, 550000, 550000, 0, 1, 9, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 10, 0, 60, 0, 100, 0, 220000, 220000, 575000, 575000, 0, 1, 10, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 11, 0, 60, 0, 100, 0, 245000, 245000, 600000, 600000, 0, 1, 11, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 12, 0, 60, 0, 100, 0, 250000, 250000, 605000, 605000, 0, 1, 12, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 13, 0, 60, 0, 100, 0, 260000, 260000, 615000, 615000, 0, 1, 13, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 14, 0, 60, 0, 100, 0, 280000, 280000, 635000, 635000, 0, 1, 14, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Say Text on Update'),
(48496, 0, 15, 0, 64, 0, 100, 0, 0, 0, 0, 0, 0, 10, 1, 3, 5, 6, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Play Emote on Gossip');

-- creature_text groups the imported rows need (MoP 5.4.8 has them, LoA does not)
DELETE FROM `creature_text` WHERE (`CreatureID` IN (34689, 34696) AND `GroupID` = 0) OR (`CreatureID` IN (34695, 35200) AND `GroupID` = 1);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`)
VALUES
(34689, 0, 0, 'All you gotta do is learn how to cast Frost Nova. Then, head over to the training dummies that Bamm is shooting at and let ''er rip.', 12, 0, 100, 0, 0, 0, 0, 0, 0, ''),
(34696, 0, 0, 'All you gotta do is learn how to cast Corruption, and then go use it on these dummies that Bamm is shooting at. Corrupt''em!', 12, 0, 100, 0, 0, 0, 0, 0, 0, ''),
(34695, 1, 0, 'Would one of you hurry up and finish off the other? Some of us are trying to make some moolah here!', 12, 0, 100, 1, 0, 0, 0, 0, 0, 'say Text'),
(35200, 1, 0, 'Yarr!', 14, 0, 100, 22, 0, 0, 0, 0, 0, ''),
(35200, 1, 1, 'It be gold we''re after!', 14, 0, 100, 22, 0, 0, 0, 0, 0, '');

-- 34668 Sassy Hardwrench: quest lines + emotes. Rows 0-1 (LoA: gossip option 12493 "ready to go" -> teleport) are kept.
-- The MoP SAI maps the lines one quest off (14122 -> "break your moolah" is group 5, not 4, etc.); the mapping
-- below follows the creature_text content and the Wowpedia notes (14126 on accept: "The Trade Prince's yacht is about to set sail...").
DELETE FROM `smart_scripts` WHERE `entryorguid` = 34668 AND `source_type` = 0 AND `id` >= 2;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`)
VALUES
(34668, 0, 2, 0, 19, 0, 100, 0, 14138, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14138 Accepted - Say Line 0'),
(34668, 0, 3, 0, 19, 0, 100, 0, 14070, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14070 Accepted - Say Line 2'),
(34668, 0, 4, 0, 19, 0, 100, 0, 14115, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14115 Accepted - Say Line 3'),
(34668, 0, 5, 0, 19, 0, 100, 0, 14116, 0, 0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14116 Accepted - Say Line 4'),
(34668, 0, 6, 0, 19, 0, 100, 0, 14122, 0, 0, 0, 0, 1, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14122 Accepted - Say Line 5'),
(34668, 0, 7, 0, 19, 0, 100, 0, 14125, 0, 0, 0, 0, 1, 6, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14125 Accepted - Say Line 6'),
(34668, 0, 8, 0, 19, 0, 100, 0, 14126, 0, 0, 0, 0, 1, 7, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14126 Accepted - Say Line 7'),
(34668, 0, 9, 10, 20, 0, 100, 0, 25473, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 25473 Rewarded - Play Emote 4'),
(34668, 0, 10, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Link - Say Line 1'),
(34668, 0, 11, 0, 20, 0, 100, 0, 14070, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14070 Rewarded - Play Emote 4'),
(34668, 0, 12, 0, 20, 0, 100, 0, 14115, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14115 Rewarded - Play Emote 4'),
(34668, 0, 13, 0, 20, 0, 100, 0, 14113, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14113 Rewarded - Play Emote 4'),
(34668, 0, 14, 0, 20, 0, 100, 0, 14122, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14122 Rewarded - Play Emote 4'),
(34668, 0, 15, 0, 20, 0, 100, 0, 14125, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14125 Rewarded - Play Emote 4'),
(34668, 0, 16, 0, 20, 0, 100, 0, 14120, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14120 Rewarded - Play Emote 4'),
(34668, 0, 17, 0, 20, 0, 100, 0, 14153, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 14153 Rewarded - Play Emote 4'),
(34668, 0, 18, 0, 20, 0, 100, 0, 24520, 0, 0, 0, 0, 5, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 24520 Rewarded - Play Emote 4'),
(34668, 0, 19, 0, 64, 0, 100, 0, 0, 0, 0, 0, 0, 5, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Gossip Hello - Play Emote 1');

-- NOTE (14126 Life Savings ride): retail drives the player to the yacht in a Hot Rod; no waypoint data for
-- it exists in either DB (script_waypoint/waypoints/waypoint_data: nothing for 37676/37680/900000), so LoA's
-- teleport-to-the-dock gossip (SAI 34668 id 0) stays. The hand-off proper (turn in at Gallywix 35222 on the
-- dock -> 91847 -> movie 22 -> 74100 Lost Isles) is the C++ spell_life_saving_complete bound above.
