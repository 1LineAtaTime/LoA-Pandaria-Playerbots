-- #438 Goblin start, Lost Isles part 2 (zone 4720, map 648): Oomlot -> volcano -> Gallywix.
-- Refs #438.  Dev B2.  Quests 24897 .. 25265.
--
-- THIS FILE NEEDS THE MATCHING C++ (src/server/scripts/Maelstrom/zone_lost_isles_volcano.cpp,
-- AddSC_lost_isles_volcano(), called from AddSC_kezan()) FROM THE NEXT BUILD. The ScriptNames bound here
-- (npc_canon_gob, npc_oom_isle_gob, npc_ceint, npc_izzy_airplane, npc_avion_gob, npc_killag_2,
-- npc_Chariot, npc_Chariot2, npc_Prince, spell_boot_damage) do nothing until that code is built.
-- Apply this file together with the build.
--
-- LoA today (db/loa + sql/updates/world) vs the MoP 5.4.8 data:
--   * MoP 5.4.8 binds C++ ScriptNames (MoPCore lineage) on 38526, 38531, 38387, 38928, 38929, 39066,
--     39329, 39341, 39582 and on spells 72886 / 72971; LoA has none of them.
--   * LoA replaced several of those scripted quest flows with fake-completion hacks in SmartAI:
--       38928 (25023 accept -> teleport),  38387 id 0 (25066 accept -> kill credit 39039),
--       39066 (25100 accept -> teleport),  39341 (25184 accept -> credit 39335 + teleport),
--       39598 (passenger boarded -> credit 39582 = Final Confrontation done on boarding),
--       38124 id 0 (25213 accept -> teleport to the slave pits).
--     They are removed here, in the same file that binds the real replacement (C++ above, or SmartAI for
--     38387 / 39152, or - for 25213 - nothing: Wowpedia says the player walks, MoP does not teleport).
--   * Many Lost Isles NPCs are SmartAI-only in MoP 5.4.8 and empty in LoA (38120, 38124, 38517, 38574/38644,
--     38647, 38696, 38855, 39363, 39152): imported below after validating every row against
--     SmartScriptMgr.h (events 0/1/2/4/6/8/19/20/27/40/52/60/61/62/64, actions 1/5/10/11/12/28/33/41/50/53/72/75/85,
--     targets 1/2/7/8/19/24 exist with the same meaning) and against LoA creature_text / DBC spells.
--
-- Intended design (cited): Wowpedia quest pages (lead's t438/wp/<id>.json), notably 25023 (flight dialogue), 24942
-- (boots only hurt Goblin Zombies), 25066 (gossip "Get me up into the skies, Sassy!"), 25100 (Kilag's Bastia ride),
-- 25184 (mine cart ride), 25251 (Final Confrontation), 25213 (walk to the Slave Pits). Mechanics: MoPCore
-- lost_isles.cpp:1495-2420 (the same lineage) and AshamaneCore lost_isles.cpp:1409-2358.
--
-- Test-DB schema notes: creature_template has no InhabitType (creature_template_movement instead) and smart_scripts
-- has event_param5 / target_param4 (2024_07_24_00, lead SCHEMA-NOTE). Every statement uses explicit column lists.
--
-- Rollback: see the end of the file.

-- =====================================================================
-- 1. Oomlot captives / Izzy / Volcanoth SmartAI (audit rows GQ010, GQ011)
--    Content of the alexkulya audit port 2026_09_23_74_world_lost_isles_oomlot_volcanoth_sai.sql, re-verified here against the MoP data (rows 38574 201/202,
--    38644 201/202, 38647 202/203/206, 38855 201/202/203 are identical to MoP apart from 38647 id 203 link 205 -> 0)
--    and db/loa (texts 38647 groups 0/1 and 38855 group 0 exist; spells 72518, 72522, 73097, 73193, 66987 exist in
--    Spell.dbc). 38647 ids 201/204 are NOT imported: 201 duplicates LoA's own Kaja'Cola credit row, 204 re-summons
--    Izzy on the spell hit on which LoA's row 1 despawns her. LoA's own rows (38574 id 0, 38647 ids 0-1,
--    38855 id 0) are kept; the kill credits 38536 (24901) and 38868 (24958) are in section 3.
-- =====================================================================
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (38574, 38644, 38647, 38855) AND `ScriptName` = '';

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND ((`entryorguid` = 38574 AND `id` IN (201, 202)) OR (`entryorguid` = 38644 AND `id` IN (201, 202)) OR (`entryorguid` = 38647 AND `id` IN (202, 203, 206)) OR (`entryorguid` = 38855 AND `id` IN (201, 202, 203)));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38574, 0, 201, 0, 1, 0, 100, 1, 0, 5000, 0, 0, 0, 11, 72518, 0, 0, 0, 0, 0, 1, 38644, 8, 0, 0, 0, 0, 0, 0, 'Oomlot Shaman - OOC - Cast Enveloping Winds'),
(38574, 0, 202, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 28, 72522, 0, 0, 0, 0, 0, 19, 38643, 0, 0, 0, 0, 0, 0, 0, 'Oomlot Shaman - On Death - Remove Enveloping Winds from closest Goblin Captive'),
(38644, 0, 201, 0, 1, 0, 100, 1, 0, 5000, 0, 0, 0, 11, 72518, 0, 0, 0, 0, 0, 1, 38644, 8, 0, 0, 0, 0, 0, 0, 'Oomlot Shaman - OOC - Cast Enveloping Winds'),
(38644, 0, 202, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 28, 72522, 0, 0, 0, 0, 0, 19, 38643, 0, 0, 0, 0, 0, 0, 0, 'Oomlot Shaman - On Death - Remove Enveloping Winds from closest Goblin Captive'),
(38647, 0, 202, 0, 19, 0, 100, 0, 24925, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Izzy - On Quest 24925 Accepted - Say Line 0'),
(38647, 0, 203, 0, 19, 0, 100, 0, 24937, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Izzy - On Quest 24937 Accepted - Say Line 1'),
(38647, 0, 206, 0, 20, 0, 100, 0, 24924, 0, 0, 0, 0, 28, 66987, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Izzy - On Quest 24924 Rewarded - Remove Pygmy Helmet'),
(38855, 0, 201, 0, 0, 0, 100, 0, 1000, 1000, 7000, 7000, 0, 11, 73097, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Volcanoth - In Combat - Cast Volcanic Breath'),
(38855, 0, 202, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 'Volcanoth - On Aggro - Say Line 0'),
(38855, 0, 203, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 11, 73193, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Volcanoth - On Death - Cast Volcanoth Dead Explosion');

-- =====================================================================
-- 2. SmartAI-only MoP entries imported (validated, see header)
--    38120 Hobart Grapplehammer (both halves of the chain), 38124 Assistant Greely (6 rows), 38517 Slinky Sharpshiv
--    (25109), 38696 Yngwie (24929), 39363 Chip Endale (25203), 38441 Ace (25203 say). (48494 Hobart / 48496 Greely are
--    Kezan HQ spawns, zone 4737: imported by 2026_10_04_14_world_goblin_kezan.sql, not here.)
--    NOT imported: 38120 ids 209-213 (summon creature 479710, a MoP-only entry, audit row GQ016);
--    38643 ids 201/202 (a 'test' row that fires on the aura it should remove); 38409/38746/38441 resummon rows.
--    38120 ids 204-206 in MoP target creature GUID 247818, which is creature 33009 in both DBs (the Greely
--    spawn guids are 232649/232805/375742/375827/375860/375886) so they never fire there; here they target the
--    closest Assistant Greely 38124 within 40 yd (SMART_TARGET_CLOSEST_CREATURE) so the chat chain can run.
--    38124 id 0 (LoA's 25213 teleport hack) is replaced by MoP id 0 (25213 -> say line 8).
-- =====================================================================
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (38120, 38517, 38696, 39363) AND `ScriptName` = '';

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 38120 AND `id` BETWEEN 201 AND 217;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38120, 0, 201, 0, 19, 0, 100, 0, 24671, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, '24671'),
(38120, 0, 202, 0, 19, 0, 100, 0, 24744, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, '24744'),
(38120, 0, 203, 0, 20, 0, 100, 0, 24744, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On quest end 24744'),
(38120, 0, 204, 208, 52, 0, 100, 0, 3, 38120, 0, 0, 0, 1, 0, 12000, 0, 0, 0, 0, 19, 38124, 40, 0, 0, 0, 0, 0, 0, 'Hobart - After text 3 - Assistant Greely talks (group 0)'),
(38120, 0, 205, 0, 52, 0, 100, 0, 0, 38124, 0, 0, 0, 1, 1, 8000, 0, 0, 0, 0, 19, 38124, 40, 0, 0, 0, 0, 0, 0, 'Assistante Grelly talk'),
(38120, 0, 206, 0, 52, 0, 100, 0, 1, 38124, 0, 0, 0, 1, 2, 8000, 0, 0, 0, 0, 19, 38124, 40, 0, 0, 0, 0, 0, 0, 'Assistante say 2'),
(38120, 0, 207, 0, 52, 0, 100, 0, 2, 38124, 0, 0, 0, 1, 4, 8000, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Final text'),
(38120, 0, 208, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 50, 202104, 18, 0, 0, 0, 0, 8, 0, 0, 0, 0, 905.653, 2340.8, 5.8624, 4.22416, 'Link spawn GOB'),
(38120, 0, 214, 0, 19, 0, 100, 0, 24856, 0, 0, 0, 0, 1, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On quest add  24856'),
(38120, 0, 215, 0, 19, 0, 100, 0, 24954, 0, 0, 0, 0, 1, 6, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, '24954'),
(38120, 0, 216, 0, 19, 0, 100, 0, 24958, 0, 0, 0, 0, 1, 7, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Quest start 24958'),
(38120, 0, 217, 0, 19, 0, 100, 0, 25214, 0, 0, 0, 0, 1, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, '25214');

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 38124 AND `id` BETWEEN 0 AND 5;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38124, 0, 0, 0, 19, 0, 100, 0, 24817, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'on quest add 24817'),
(38124, 0, 1, 0, 19, 0, 100, 0, 24946, 0, 0, 0, 0, 1, 4, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '24946'),
(38124, 0, 2, 0, 19, 0, 100, 0, 25200, 0, 0, 0, 0, 1, 6, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '25200'),
(38124, 0, 3, 0, 19, 0, 100, 0, 25204, 0, 0, 0, 0, 1, 7, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '25204'),
(38124, 0, 4, 0, 19, 0, 100, 0, 25213, 0, 0, 0, 0, 1, 8, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '25213'),
(38124, 0, 5, 0, 1, 0, 100, 0, 60000, 180000, 120000, 240000, 0, 1, 5, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'On update say');

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 38517 AND `id` = 201;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38517, 0, 201, 0, 19, 0, 100, 0, 25109, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, '25109');

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 38696 AND `id` IN (201, 203);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38696, 0, 201, 0, 0, 0, 100, 0, 0, 10000, 5000, 15000, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On combat say'),
(38696, 0, 203, 0, 2, 0, 100, 1, 20, 40, 0, 0, 0, 11, 75942, 4, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Type a script description here.');

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 39363 AND `id` IN (201, 202, 203);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(39363, 0, 201, 0, 4, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On aggro say'),
(39363, 0, 202, 0, 0, 0, 100, 0, 1000, 15000, 4000, 15000, 0, 11, 75968, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Attack tourbillon'),
(39363, 0, 203, 0, 0, 0, 100, 0, 2000, 20000, 2000, 18000, 0, 11, 75964, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Massue');

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 38441 AND `id` = 202;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38441, 0, 202, 0, 19, 0, 100, 0, 25203, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'On quest 25203 say 0');

-- =====================================================================
-- 3. Scripted quest flows: ScriptName binds, template fixes, hack removal
-- =====================================================================
-- 24901 Town-In-A-Box: Under Attack. B.C. Eliminator 38526 (npc_canon_gob) + Oomlot Warrior 38531
--   (npc_oom_isle_gob). MoP sets KillCredit1 = 38536 on 38531 (Unit::Kill credits the cannon's driver);
--   LoA gave the same credit with an SAI death row, which is dead once the script is bound -> removed.
UPDATE `creature_template` SET `ScriptName` = 'npc_canon_gob', `AIName` = '' WHERE `entry` = 38526;
UPDATE `creature_template` SET `ScriptName` = 'npc_oom_isle_gob', `AIName` = '', `KillCredit1` = 38536 WHERE `entry` = 38531;
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 38531 AND `id` = 0;

-- 24942 Zombies vs. Super Booster Rocket Boots: spell 72886 target filter (spell_boot_damage,
--   spell_script_names) and the boots clean-up when the quest is turned in to Coach Crosscheck 38738 (npc_ceint,
--   MoPCore lost_isles.cpp:1614). Zombie credit 38807 stays on the existing 38753/38816 death rows (identical in MoP).
DELETE FROM `spell_script_names` WHERE `spell_id` = 72886;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (72886, 'spell_boot_damage');
UPDATE `creature_template` SET `ScriptName` = 'npc_ceint' WHERE `entry` = 38738;
-- 24952 Rocket Boot Boost: item spell 72971 already jumps (spell_target_position 72971 -> 1481.15, 1269.77, 106.779)
--   and gives credit 38842 (its own KILL_CREDIT effect); the MoP spell_boot_gob only repeated both, so it is
--   NOT bound here (it would double the credit and the jump).

-- 25023 Old Friends. Sassy 38928 (npc_izzy_airplane) summons a personal Flying Bomber 38929 (npc_avion_gob).
--   VehicleId 658 has ONE seat (Vehicle.dbc / VehicleSeat.dbc of the test client data), MoP's 657 has three
--   (seats 7566, 7567, 7575): with 658 the player's boarding bumps Sassy out (the reason for the LoA teleport hack).
UPDATE `creature_template` SET `ScriptName` = 'npc_izzy_airplane', `AIName` = '' WHERE `entry` = 38928;
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 38928 AND `id` IN (0, 1);
UPDATE `creature_template` SET `ScriptName` = 'npc_avion_gob', `VehicleId` = 657 WHERE `entry` = 38929;
DELETE FROM `script_waypoint` WHERE `entry` = 38929;
INSERT INTO `script_waypoint` (`entry`, `pointid`, `location_x`, `location_y`, `location_z`, `waittime`, `point_comment`) VALUES
(38929, 0, 1151.07, 1115.43, 129.643, 0, 'Flying Bomber - Old Friends'),
(38929, 1, 1151.07, 1115.43, 129.643, 0, 'Flying Bomber - Old Friends'),
(38929, 2, 1135.73, 1140.05, 142.904, 0, 'Flying Bomber - Old Friends'),
(38929, 3, 1099.58, 1166.04, 160.914, 0, 'Flying Bomber - Old Friends'),
(38929, 4, 1035.83, 1199.63, 140.998, 0, 'Flying Bomber - Old Friends'),
(38929, 5, 985.602, 1262.28, 123.749, 0, 'Flying Bomber - Old Friends'),
(38929, 6, 993.035, 1328.35, 114.86, 0, 'Flying Bomber - Old Friends'),
(38929, 7, 1028.52, 1418.13, 106.693, 0, 'Flying Bomber - Old Friends'),
(38929, 8, 1012.31, 1533.02, 129.84, 0, 'Flying Bomber - Old Friends'),
(38929, 9, 922.436, 1584.99, 168.414, 0, 'Flying Bomber - Old Friends'),
(38929, 10, 798.557, 1588.16, 174.219, 0, 'Flying Bomber - Old Friends'),
(38929, 11, 737.91, 1636.48, 142.386, 0, 'Flying Bomber - Old Friends'),
(38929, 12, 721.896, 1756.58, 120.664, 0, 'Flying Bomber - Old Friends'),
(38929, 13, 763.009, 1888.22, 119.608, 0, 'Flying Bomber - Old Friends'),
(38929, 14, 808.372, 2035.26, 112.913, 0, 'Flying Bomber - Old Friends'),
(38929, 15, 894.958, 2161.13, 93.3024, 0, 'Flying Bomber - Old Friends'),
(38929, 16, 940.938, 2306.29, 39.8023, 0, 'Flying Bomber - Old Friends'),
(38929, 17, 938.337, 2458.16, 23.8301, 0, 'Flying Bomber - Old Friends'),
(38929, 18, 868.55, 2537.89, 11.0801, 0, 'Flying Bomber - Old Friends'),
(38929, 19, 771.257, 2526.76, 11.0801, 0, 'Flying Bomber - Old Friends'),
(38929, 20, 746.307, 2438.62, 11.0801, 0, 'Flying Bomber - Old Friends'),
(38929, 21, 807.429, 2367.57, 30.6635, 0, 'Flying Bomber - Old Friends'),
(38929, 22, 930.785, 2316.55, 45.2003, 0, 'Flying Bomber - Old Friends'),
(38929, 23, 1235.78, 2192.42, 93.2624, 0, 'Flying Bomber - Old Friends'),
(38929, 24, 1534.63, 2529.2, 125.186, 0, 'Flying Bomber - Old Friends'),
(38929, 25, 1584.89, 2684.93, 95.6027, 0, 'Flying Bomber - Old Friends');
DELETE FROM `creature_text` WHERE `CreatureID` = 38928 AND `GroupID` BETWEEN 1 AND 7;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(38928, 1, 0, 'Hang on, $n! We have to get you out of here, NOW!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Old Friends flight (Wowpedia 25023)'),
(38928, 2, 0, 'OH MY GOSH! BEHIND YOU!!!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Old Friends flight (Wowpedia 25023)'),
(38928, 3, 0, 'WE''RE ON FIRE!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Old Friends flight (Wowpedia 25023)'),
(38928, 4, 0, 'YEEEEEEEEEE HAAAAAAAAAA!!!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Old Friends flight (Wowpedia 25023)'),
(38928, 5, 0, 'We''re coming up on the Town-In-A-Box now...', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Old Friends flight (Wowpedia 25023)'),
(38928, 6, 0, 'Get in the back seat. We''re getting out of here. I know where our old friends, the orcs, are!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Old Friends flight (Wowpedia 25023)'),
(38928, 7, 0, 'First Kezan and now here. We are toast if we don''t find a way off of this island soon!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - Old Friends flight (Wowpedia 25023)');

-- 25066 The Pride of Kezan. Sassy 38387: the 'Get me up into the skies, Sassy!' gossip (menu 11146 option 0, shown
--   while 25066 is taken) and the quest accept make the player cast 73430 (summons the Pride of Kezan 39074, which
--   carries the 73446 / 34873 flight auras in creature_template_addon in both DBs); the fake credit row is gone.
--   Option 1 ('set sail for Orgrimmar', shown when 25266 is complete) closes the gossip and makes Sassy cast 74924
--   'Warchief's Emissary: Character Force Cast From Gossip' on the player, triggered; it force-casts 74029
--   (Teleport & Bind to Orgrimmar: bind + teleport to spell_target_position 74029 = map 1, 1468.8 -5012.29 11.7693),
--   replacing LoA's raw teleport (no hearth bind). Why SmartAI and not npc_boot: the fork hands C++ OnGossipSelect
--   sender 0 / action OptionType for DB gossip options (Player.cpp:15613), so the two options cannot be told apart
--   there; SMART_EVENT_GOSSIP_SELECT carries the option id.
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 38387 AND `id` BETWEEN 0 AND 4;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38387, 0, 0, 0, 19, 0, 100, 0, 25066, 0, 0, 0, 0, 85, 73430, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest 25066 Accepted - Invoker Cast Summon the Pride of Kezan'),
(38387, 0, 1, 2, 62, 0, 100, 0, 11146, 0, 0, 0, 0, 85, 73430, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Gossip Option 0 Selected - Invoker Cast Summon the Pride of Kezan'),
(38387, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Link - Close Gossip'),
(38387, 0, 3, 4, 62, 0, 100, 0, 11146, 1, 0, 0, 0, 11, 74924, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Gossip Option 1 Selected - Cast Warchief''s Emissary Force Cast From Gossip'),
(38387, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Link - Close Gossip');

-- 25100 Let's Ride. Kilag Gorefang 39066 (npc_killag_2) summons Bastia 39152 and seats the player; Bastia's
--   SmartAI starts waypoint path 39152 (37 points, `waypoints`) when the player boards and despawns at
--   its end (MoP smart_scripts 39152 ids 0/1 verbatim).
UPDATE `creature_template` SET `ScriptName` = 'npc_killag_2', `AIName` = '' WHERE `entry` = 39066;
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 39066 AND `id` = 0;
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 39152 AND `ScriptName` = '';
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 39152 AND `id` IN (0, 1);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(39152, 0, 0, 0, 27, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 39152, 0, 25100, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bastia - On Passenger Boarded - Start Waypoints (25100)'),
(39152, 0, 1, 0, 40, 0, 100, 0, 37, 39152, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bastia - On Waypoint 37 Reached - Despawn');

DELETE FROM `waypoints` WHERE `entry` = 39152;
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `point_comment`) VALUES
(39152, 1, 1715.19, 2845.8, 16.0096, 0, 0, 'Bastia - Let''s Ride'),
(39152, 2, 1714.3, 2831.23, 23.8402, 0, 0, 'Bastia - Let''s Ride'),
(39152, 3, 1713.93, 2817.76, 31.3169, 0, 0, 'Bastia - Let''s Ride'),
(39152, 4, 1709, 2793.14, 37.5701, 0, 0, 'Bastia - Let''s Ride'),
(39152, 5, 1698.45, 2781.36, 43.3627, 0, 0, 'Bastia - Let''s Ride'),
(39152, 6, 1681.41, 2767.04, 54.7985, 0, 0, 'Bastia - Let''s Ride'),
(39152, 7, 1670.08, 2756.9, 63.8357, 0, 0, 'Bastia - Let''s Ride'),
(39152, 8, 1669.1, 2743.51, 71.9479, 0, 0, 'Bastia - Let''s Ride'),
(39152, 9, 1668.66, 2730.64, 79.8617, 0, 0, 'Bastia - Let''s Ride'),
(39152, 10, 1671.34, 2702.77, 89.0315, 0, 0, 'Bastia - Let''s Ride'),
(39152, 11, 1677.41, 2683.96, 92.415, 0, 0, 'Bastia - Let''s Ride'),
(39152, 12, 1692.25, 2673.87, 91.1998, 0, 0, 'Bastia - Let''s Ride'),
(39152, 13, 1734.1, 2653.13, 90.8206, 0, 0, 'Bastia - Let''s Ride'),
(39152, 14, 1755.76, 2629.7, 91.3455, 0, 0, 'Bastia - Let''s Ride'),
(39152, 15, 1802.48, 2597.55, 90.1697, 0, 0, 'Bastia - Let''s Ride'),
(39152, 16, 1815.78, 2576.96, 91.4506, 0, 0, 'Bastia - Let''s Ride'),
(39152, 17, 1819.65, 2550.1, 93.0821, 0, 0, 'Bastia - Let''s Ride'),
(39152, 18, 1836.43, 2533.84, 94.7833, 0, 0, 'Bastia - Let''s Ride'),
(39152, 19, 1878.38, 2486.61, 93.9704, 0, 0, 'Bastia - Let''s Ride'),
(39152, 20, 1883.54, 2469.96, 97.0218, 0, 0, 'Bastia - Let''s Ride'),
(39152, 21, 1892.88, 2447.82, 100.263, 0, 0, 'Bastia - Let''s Ride'),
(39152, 22, 1911.5, 2407.03, 103.674, 0, 0, 'Bastia - Let''s Ride'),
(39152, 23, 1932.53, 2376.71, 103.134, 0, 0, 'Bastia - Let''s Ride'),
(39152, 24, 1949.42, 2347.31, 102.612, 0, 0, 'Bastia - Let''s Ride'),
(39152, 25, 1948.56, 2322.21, 104.785, 0, 0, 'Bastia - Let''s Ride'),
(39152, 26, 1955.37, 2270.2, 109.834, 0, 0, 'Bastia - Let''s Ride'),
(39152, 27, 1965.47, 2244.69, 111.91, 0, 0, 'Bastia - Let''s Ride'),
(39152, 28, 1967.45, 2212.4, 114.639, 0, 0, 'Bastia - Let''s Ride'),
(39152, 29, 1961.34, 2170.12, 124.047, 0, 0, 'Bastia - Let''s Ride'),
(39152, 30, 1970.13, 2147.06, 130.01, 0, 0, 'Bastia - Let''s Ride'),
(39152, 31, 1970.87, 2119.05, 137.506, 0, 0, 'Bastia - Let''s Ride'),
(39152, 32, 1966.84, 2070.33, 150.293, 0, 0, 'Bastia - Let''s Ride'),
(39152, 33, 1952.72, 2046.1, 161.831, 0, 0, 'Bastia - Let''s Ride'),
(39152, 34, 1932.7, 2036.59, 174.023, 0, 0, 'Bastia - Let''s Ride'),
(39152, 35, 1888.7, 2022.74, 198.632, 0, 0, 'Bastia - Let''s Ride'),
(39152, 36, 1877.39, 2004.51, 207.981, 0, 0, 'Bastia - Let''s Ride'),
(39152, 37, 1856.68, 1977.04, 221.229, 0, 0, 'Bastia - Let''s Ride');

-- 25184 Wild Mine Cart Ride. Mine Cart 39341 (npc_Chariot) summons the cart vehicle 39329 (npc_Chariot2, escort path
--   script_waypoint 39329, 13 points) and seats the player; the fake credit + teleport is removed. 39329 gets the
--   MoP template values (InhabitType 7 = ground+swim+flight(2) -> creature_template_movement, no flightmaster
--   npcflag 8, unit_flags 32768 / unit_flags2 2048, movementId 181, mechanic_immune_mask 1073741695).
UPDATE `creature_template` SET `ScriptName` = 'npc_Chariot', `AIName` = '' WHERE `entry` = 39341;
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 39341 AND `id` IN (0, 1);
UPDATE `creature_template` SET `ScriptName` = 'npc_Chariot2', `npcflag` = 0, `unit_flags` = 32768, `unit_flags2` = 2048, `movementId` = 181, `mechanic_immune_mask` = 1073741695 WHERE `entry` = 39329;
INSERT INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`) VALUES (39329, 1, 1, 2, 0) ON DUPLICATE KEY UPDATE `Ground` = 1, `Swim` = 1, `Flight` = 2;
DELETE FROM `script_waypoint` WHERE `entry` = 39329;
INSERT INTO `script_waypoint` (`entry`, `pointid`, `location_x`, `location_y`, `location_z`, `waittime`, `point_comment`) VALUES
(39329, 0, 2058.63, 1841.98, 139.98, 2000, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 1, 2078.8, 1848.5, 138.05, 0, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 2, 2111.78, 1857.47, 127.66, 0, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 3, 2147.27, 1868.17, 127.27, 0, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 4, 2166.08, 1875.21, 119.73, 0, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 5, 2196.36, 1884.54, 92.42, 0, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 6, 2209.92, 1888.56, 73.5, 0, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 7, 2241.64, 1897.91, 57.52, 0, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 8, 2279.63, 1907.49, 38.2, 0, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 9, 2323.75, 1922.29, 29.68, 0, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 10, 2339.92, 1928.17, 26.91, 0, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 11, 2351.48, 1929.54, 25.42, 0, 'Mine Cart - Wild Mine Cart Ride'),
(39329, 12, 2371.15, 1934.53, 21.01, 0, 'Mine Cart - Wild Mine Cart Ride');

-- 25213 The Slave Pits. LoA's Greely 38124 id 0 teleported the player to the slave pits on accept (replaced in
--   section 2); MoP has no teleport and Wowpedia says 'Head southwest, following the road' (walk).

-- 25251 Final Confrontation. Ultimate Footbomb Uniform vehicle 39598 loses the SmartVehicleAI credit-on-board hack
--   (VehicleAI is the default for vehicles; the uniform credit 39592 comes from spell 73989's KILL_CREDIT effect, the
--   boarding from the 39592 spellclick 73991 -> summon 39598). Gallywix 39582 gets npc_Prince, which gives credit
--   39582 when the fight ends. unit_flags 0 as in MoP 5.4.8.
UPDATE `creature_template` SET `AIName` = '', `unit_flags` = 0 WHERE `entry` = 39598;
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 39598 AND `id` = 0;
UPDATE `creature_template` SET `ScriptName` = 'npc_Prince' WHERE `entry` = 39582;

-- =====================================================================
-- Rollback (restores the pre-file state of everything touched above)
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` IN (38526, 38531, 38738, 38928, 38929, 39066, 39329, 39341, 39582);
--   UPDATE `creature_template` SET `AIName` = 'SmartVehicleAI' WHERE `entry` IN (38526, 39598);
--   UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (38531, 38928, 39066, 39341);
--   UPDATE `creature_template` SET `KillCredit1` = 0 WHERE `entry` = 38531;  UPDATE `creature_template` SET `VehicleId` = 658 WHERE `entry` = 38929;
--   UPDATE `creature_template` SET `AIName` = '' WHERE `entry` IN (38120, 38517, 38644, 38696, 39152, 39363);  (38574, 38647, 38855 were already SmartAI)
--   UPDATE `creature_template` SET `npcflag` = 8, `unit_flags` = 0, `unit_flags2` = 67584, `movementId` = 128, `mechanic_immune_mask` = 16384 WHERE `entry` = 39329;  UPDATE `creature_template` SET `unit_flags` = 32768 WHERE `entry` = 39598;
--   DELETE FROM `creature_template_movement` WHERE `CreatureId` = 39329;  DELETE FROM `spell_script_names` WHERE `spell_id` = 72886;
--   DELETE FROM `script_waypoint` WHERE `entry` IN (38929, 39329);  DELETE FROM `waypoints` WHERE `entry` = 39152;  DELETE FROM `creature_text` WHERE `CreatureID` = 38928 AND `GroupID` BETWEEN 1 AND 7;
--   smart_scripts: DELETE the ids inserted above (38574/38644 201-202, 38647 202/203/206, 38855 201-203, 38120 201-217, 38124 0-5, 38517 201, 38696 201/203, 39363 201-203, 38441 202,
--     38387 0-4, 39152 0-1) and re-insert LoA's removed rows:
--     (38531,0) death -> kill credit 38536; (38928,0) quest 25023 accept -> teleport map 648 1623.55 2692.31 91.595 link 1, (38928,1) cast 73135; (38387,0) quest 25066 -> credit 39039,
--     (38387,1) gossip 11146/1 -> teleport map 1 1470.12 -5012.03 12.386 link 2, (38387,2) close gossip; (39066,0) quest 25100 -> teleport map 648 1832.61 1960.49 220.615;
--     (39341,0) quest 25184 -> credit 39335 link 1, (39341,1) teleport map 648 2366.14 1933.58 21.7338; (39598,0) passenger boarded -> credit 39582; (38124,0) quest 25213 -> teleport map 648 2120.72 2408.69 46.422.
