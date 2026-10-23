-- #438 Goblin start, Lost Isles part 1 (crash landing -> Town-In-A-Box): the missing quest mechanics.
-- Refs #438. Needs the matching C++ from the next build (src/server/scripts/Maelstrom/zone_lost_isles.cpp,
-- registered through AddSC_kezan -> AddSC_lost_isles); the ScriptName bindings below do nothing without it.
--
-- What this file does, per quest (source in brackets; "lead research #438" = t438/objs.txt, mech.txt, diffent.txt):
--  14239 Don't Go Into the Light!  : Doc Zapnozzle 36608 gets the intro/outro script (npc_Zapnozzle) and its creature_text
--                                    (Wowpedia 14239 notes, fetched 2026-10-04); Gizmo 36600 gets the "Sorry, Doc" line.
--  14001/14474 Goblin Escape Pods  : GO 195188 SAI rewritten. LoA summoned Trade Prince Gallywix (35649) from EVERY pod; the
--                                    the MoP 5.4.8 data summons a survivor (spell 66137); Wowpedia 14001: only the FIRST rescue
--                                    is Gallywix. Implemented with the Ashamane lost_isles.cpp:189-216 logic as SAI + conditions.
--  14014/14019/14233-14237/14303   : accept/complete talk lines (creature_text already in LoA, no AI used it); sources: Wowpedia
--                                    quest pages "On accept"/"On completion" notes, MoP smart_scripts for 35893/36112/36470.
--  14021 Miner Troubles            : hack row (35769 credit-on-accept) removed, replaced by npc_Mechumide (35769) +
--                                    npc_mineur_gob (35813) + script_waypoint (path of 35810, 19 rows).
--  14238/14240 Kilag Gorefang      : hack row (35917 teleport) removed, replaced by npc_killag_sangrecroc + Bastia 36585
--                                    (npc_pant_gob) + script_waypoint 36585 (18 rows). Aura script for 68338.
--  14236 Weed Whacker              : spell script spell_weed_whacker on item spell 68211 (spell_script_names).
--  14242 Precious Cargo            : hack row (36127 teleport) removed, replaced by npc_gyrocoptere_quest_giver (36127) +
--                                    npc_girocoptere (36143) + script_waypoint 36143 (6 rows). Thrall 36145 gets
--                                    the MoP 14326 rows + its missing creature_text.
--  14243 Warchief's Revenge        : hack rows (36161 credit + teleport) removed; Thrall 36161 -> npc_li_thrall_warchief,
--                                    cyclone 36178 -> npc_tornade_gob + script_waypoint 36178 (32 rows);
--                                    quest_objective amount 1 -> 50 (MoP 50, Wowpedia: slay 50 Alliance Sailors).
--  14244 Up, Up & Away!            : GO 196439 teleport+credit hack removed, replaced by gob_fronde_gobelin (rocket 36505,
--                                    npc_li_sling_rocket, 4 waypoints copied from 36514); Sassy 36425 summons Gallywix
--                                    36513 (MoP SAI) whose rocket 36514 (npc_fusee_gob) flies the same path.
--  14245 Town-In-A-Box             : plunger GO 201938 (type 22, spell caster) had no spell; Data0 = 68938 ("Plunger - Cover"
--                                    = force-cast 68935 knock-back/parachute/phase + KILL_CREDIT 38024). Terrain swap untouched.
--  24817 A Goblin in Shark's Clothing : controller GO 202108 credited BOTH objectives on use (hack); now it casts 71662 (summon
--                                    + ride Mechashark 38318, credit 38318). The Hammer 36682 must be killed from the shark.
--  24864 Irresistible Pool Pony    : hatchlings 38412/44578/44579/44580 -> npc_young_naga_gob (spellclick 71916 is not in the
--                                    DBC set, so the LoA SAI could never fire); Megs 38432 -> npc_megs_isle_gob.
--  24868 Surrender or Else!        : Ace 39198 -> npc_crack_isle_gob (+ creature_text from Wowpedia 24868), Faceless 38448 gets
--                                    the MoP two SAI rows.
-- Spells/summon properties checked in the active DBC set (SummonProperties 827 = category 4 vehicle), CreatureText loader and the
-- fork's SmartScriptMgr.h (events 0/1/19/20/54/61/64/70, actions 1/10/11/12/28/33/41/69/80/85, targets 1/4/7/8; condition source 22
-- keyed (SourceEntry, SourceId=source_type), SourceGroup = event id + 1).
--
-- Rollback: see the block at the end of this file.

-- 14243: 50 Alliance Sailors, not 1 (LoA quest_objective id 265514 has amount 1; MoP 50).
UPDATE `quest_objective` SET `amount` = 50 WHERE `questId` = 14243 AND `id` = 265514;

-- creature_text: Doc Zapnozzle 36608 (14239). Wowpedia 14239 'Notes' (fetched 2026-10-04). BroadcastTextId unknown (offline), left 0.
DELETE FROM `creature_text` WHERE `CreatureID` = 36608;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(36608, 0, 0, 'Gizmo, what are you doing just sitting there? Don''t you recognize who that is laying next to you?!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - 14239'),
(36608, 1, 0, 'That''s $n! $g He : She;''s the only reason we''re still breathing and not crispy fried critters back on Kezan.', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - 14239'),
(36608, 2, 0, 'Stay back, I''m going to resuscitate $g him : her;! I hope these wet jumper cables don''t kill us all!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - 14239'),
(36608, 3, 0, 'Come on! Clear!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - 14239'),
(36608, 4, 0, 'That''s all I''ve got. It''s up to $g him : her; now. You hear me, $n? Come on, snap out of it! Don''t go into the Light!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - 14239'),
(36608, 5, 0, 'You made the right choice. We all owe you a great deal, $n. Try not to get yourself killed out here.', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - 14239'),
(36608, 6, 0, 'There are more survivors to tend to. I''ll see you on the shore.', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Doc Zapnozzle - 14239');

-- Geargrinder Gizmo 36600 group 1 (spoken by npc_Zapnozzle during the intro).
DELETE FROM `creature_text` WHERE `CreatureID` = 36600 AND `GroupID` = 1;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(36600, 1, 0, 'That''s $n?! Sorry, Doc, I thought $g he : she; was dead!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Geargrinder Gizmo - 14239 intro');

-- Goblin Survivor 34748: the seven lines were seven single-line groups (nothing picks a group at random); Wowpedia 14001 says
-- a random one is spoken. Merge them into group 0 (ID 0..6) so a Talk(0) picks one at random.
UPDATE `creature_text` SET `ID` = `GroupID`, `GroupID` = 0 WHERE `CreatureID` = 34748 AND `GroupID` BETWEEN 1 AND 6;

-- Thrall 36145 (Precious Cargo): the 14326 accept line exists in MoP creature_text (36145 group 0) but not in LoA.
DELETE FROM `creature_text` WHERE `CreatureID` = 36145;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(36145, 0, 0, 'I will see you up top, $n. Thank you again for my freedom!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Thrall - 14326 accept');

-- Ace 39198 (24868). Wowpedia 24868 'Notes' (fetched 2026-10-04). Group 0 (existing movie-idea line) is left alone.
DELETE FROM `creature_text` WHERE `CreatureID` = 39198 AND `GroupID` BETWEEN 1 AND 5;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(39198, 1, 0, 'I got these little hellions on a short leash, $n. The naga won''t attack us while we have their hatchlings.', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Ace - 24868'),
(39198, 2, 0, 'You ready to make their leader surrender, buddy? Okay, here we go.', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Ace - 24868'),
(39198, 3, 0, 'We''re planning on having a special naga appreciation sale after your surrender is all wrapped up! Thirty percent off most goods we have in stock. Red light savings on Kaldorei artifacts today only!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Ace - 24868'),
(39198, 3, 1, 'You naga keep your distance. We''re not kidding around here!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Ace - 24868'),
(39198, 3, 2, 'We''ve got your hatchlings. You better not attack us if you know what''s good for you!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Ace - 24868'),
(39198, 3, 3, 'Prepare to surrender to the Bilgewater Cartel!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Ace - 24868'),
(39198, 4, 0, 'Alright, naga leader, come on out from hiding and surrender in the name of $n and the Bilgewater Cartel!', 14, 0, 100, 0, 0, 0, 0, 0, 0, 'Ace - 24868'),
(39198, 5, 0, 'Um, dude, this does not look good. I''m out of here!', 12, 0, 100, 0, 0, 0, 0, 0, 0, 'Ace - 24868');

-- ===================== smart_scripts =====================
-- Hack rows that C++ replaces (whole script of these entries: the hack was the only row).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35769 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35917 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36127 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36161 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 196439 AND `source_type` = 1;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38412 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 44578 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 44579 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 44580 AND `source_type` = 0;

-- GO 195188 Goblin Escape Pod. LoA: credit + always summon 35649 (30 s). MoP: credit + cast 66137 (summon survivor 34748). Wowpedia 14001 + Ashamane lost_isles.cpp:195-215: first rescue is Gallywix, later ones survivors.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 195188 AND `source_type` = 1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(195188, 1, 0, 0, 70, 0, 100, 0, 2, 0, 0, 0, 0, 33, 34748, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Escape Pod - On State Changed (Active) - Quest Credit Goblin Survivor'),
(195188, 1, 1, 0, 70, 0, 100, 0, 2, 0, 0, 0, 0, 85, 66137, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Escape Pod - On State Changed (Active) - Invoker Cast Summon Live Goblin Survivor (only if Gallywix was already rescued)'),
(195188, 1, 2, 3, 70, 0, 100, 0, 2, 0, 0, 0, 0, 85, 67845, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Escape Pod - On State Changed (Active) - Invoker Cast Summon Trade Prince Gallywix (first rescue)'),
(195188, 1, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 85, 67433, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Escape Pod - Linked - Invoker Cast Trade Prince Controller Aura');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 195188 AND `SourceId` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 2, 195188, 1, 0, 1, 0, 67433, 0, 0, 0, 0, 0, '', 'Goblin Escape Pod - survivor branch only if the invoker has Trade Prince Controller Aura'),
(22, 3, 195188, 1, 0, 1, 0, 67433, 0, 0, 1, 0, 0, '', 'Goblin Escape Pod - Gallywix branch only if the invoker does NOT have Trade Prince Controller Aura');

-- Goblin Survivor 34748 (summoned by 66137): thanks the rescuer, swims to the beach (Ashamane lost_isles.cpp:226-245) and despawns. Static survivors are unaffected (JUST_SUMMONED only).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 34748 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(34748, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 0, 80, 3474800, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Goblin Survivor - On Just Summoned - Run Script');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 3474800 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3474800, 9, 0, 0, 0, 0, 100, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Survivor - On Script - Say Line 0'),
(3474800, 9, 1, 0, 0, 0, 100, 0, 2500, 2500, 0, 0, 0, 69, 1, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 617.317, 3142.259, -0.114589, 2.176555, 'Survivor - On Script - Move To Beach'),
(3474800, 9, 2, 0, 0, 0, 100, 0, 14000, 14000, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Survivor - On Script - Despawn');

-- Trade Prince Gallywix 35649 (summoned by 67845): same behaviour as the survivors (Ashamane lost_isles.cpp:235-239).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35649 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35649, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 0, 80, 3564900, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Trade Prince Gallywix - On Just Summoned - Run Script');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 3564900 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3564900, 9, 0, 0, 0, 0, 100, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Survivor - On Script - Say Line 0'),
(3564900, 9, 1, 0, 0, 0, 100, 0, 2500, 2500, 0, 0, 0, 69, 1, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 617.317, 3142.259, -0.114589, 2.176555, 'Survivor - On Script - Move To Beach'),
(3564900, 9, 2, 0, 0, 0, 100, 0, 14000, 14000, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Survivor - On Script - Despawn');

-- Geargrinder Gizmo 36600: accept line of 14001 (Wowpedia 14001 'On accept'; MoP smart_scripts 36600 id 0).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36600 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36600, 0, 0, 0, 19, 0, 100, 0, 14001, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Geargrinder Gizmo - On Quest Accept 14001 - Say Line 0'),
(36600, 0, 1, 0, 19, 0, 100, 0, 14474, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Geargrinder Gizmo - On Quest Accept 14474 - Say Line 0');

-- GO 202108: LoA credited 38318 AND 36682 (The Hammer) on use = auto-complete. 71662 = force-cast 71648 (summon+ride 38318) + kill credit 38318; The Hammer has to be killed from the shark (Wowpedia 24817).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 202108 AND `source_type` = 1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(202108, 1, 0, 0, 70, 0, 100, 0, 2, 0, 0, 0, 0, 85, 71662, 2, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Mechashark X-Steam Controller - On State Changed (Active) - Invoker Cast Controller (summons and boards the Mechashark, credit 38318)');

-- Sassy Hardwrench 36425 (Wild Overlook): MoP smart_scripts 36425 (accept 14244: 'Uh oh' + summon Gallywix 36513); spell 68815 has the same destination.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36425 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36425, 0, 0, 1, 19, 0, 100, 0, 14244, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest Accept 14244 - Say Line 0'),
(36425, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 12, 36513, 3, 45000, 0, 0, 0, 8, 0, 0, 0, 0, 856.168, 2751.02, 118.867, 2.27097, 'Sassy Hardwrench - Linked - Summon Trade Prince Gallywix');

-- Trade Prince Gallywix 36513: MoP rows (talk 0/1/2, then leave). Instead of the wrongly keyed 68804 the exit is spell 68819 (summons his rocket 36514 at the sling, spell_target_position 68817 exists).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36513 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36513, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 0, 80, 3651300, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Trade Prince Gallywix - On Just Summoned - Run Script');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 3651300 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3651300, 9, 0, 0, 0, 0, 100, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Trade Prince Gallywix - On Script - Say Line 0'),
(3651300, 9, 1, 0, 0, 0, 100, 0, 8000, 8000, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Trade Prince Gallywix - On Script - Say Line 1'),
(3651300, 9, 2, 0, 0, 0, 100, 0, 20000, 20000, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Trade Prince Gallywix - On Script - Say Line 2'),
(3651300, 9, 3, 0, 0, 0, 100, 0, 21000, 21000, 0, 0, 0, 11, 68819, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Trade Prince Gallywix - On Script - Cast Force Cast from Trade Prince Gallywix'),
(3651300, 9, 4, 0, 0, 0, 100, 0, 23000, 23000, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Trade Prince Gallywix - On Script - Despawn');

-- Foreman Dampwick 36470 (Landing Site): MoP smart_scripts 36470 (hello emote, accept 14245 line).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36470 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36470, 0, 0, 0, 64, 0, 100, 0, 0, 0, 0, 0, 0, 10, 1, 3, 5, 6, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Foreman Dampwick - On Gossip Hello - Random Emote'),
(36470, 0, 1, 0, 19, 0, 100, 0, 14245, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Foreman Dampwick - On Quest Accept 14245 - Say Line 0');

-- Scout Brax 36112: MoP smart_scripts 36112 id 0 (accept 14241 yell).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36112 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36112, 0, 0, 0, 19, 0, 100, 0, 14241, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Scout Brax - On Quest Accept 14241 - Say Line 0');

-- Kilag Gorefang 35893 (Vicious Vale): MoP id 0 (reward 14236 removes aura 68212); Wowpedia 14303 'On accept' = group 1; group 0 'Use your weed whacker' is the 14236 accept line (text content).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35893 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35893, 0, 0, 0, 19, 0, 100, 0, 14236, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kilag Gorefang - On Quest Accept 14236 - Say Line 0'),
(35893, 0, 1, 0, 20, 0, 100, 0, 14236, 0, 0, 0, 0, 28, 68212, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Kilag Gorefang - On Quest Reward 14236 - Remove Weed Whacker Aura'),
(35893, 0, 2, 0, 19, 0, 100, 0, 14303, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kilag Gorefang - On Quest Accept 14303 - Say Line 1');

-- Aggra 35875: Wowpedia 14235 / 14237 'Notes: On accept' lines (creature_text groups 0/1 already in LoA).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35875 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35875, 0, 0, 0, 19, 0, 100, 0, 14235, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Aggra - On Quest Accept 14235 - Say Line 0'),
(35875, 0, 1, 0, 19, 0, 100, 0, 14237, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Aggra - On Quest Accept 14237 - Say Line 1');

-- Sassy Hardwrench 35650: Wowpedia 14014 'On accept' (group 0). Groups 1 and 2 are not wired (no source ties them to a quest).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35650 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35650, 0, 0, 0, 19, 0, 100, 0, 14014, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Sassy Hardwrench - On Quest Accept 14014 - Say Line 0');

-- Bamm Megabomb 35758: Wowpedia 14019 'On accept' / 'On completion'.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 35758 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35758, 0, 0, 0, 19, 0, 100, 0, 14019, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bamm Megabomb - On Quest Accept 14019 - Say Line 0'),
(35758, 0, 1, 0, 20, 0, 100, 0, 14019, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Bamm Megabomb - On Quest Reward 14019 - Say Line 1');

-- Thrall 36145: MoP smart_scripts 36145 ids 0-2 (the credit for 14242 comes from Alliance Wizard 36621 SAI, identical in both data sets).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36145 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36145, 0, 0, 0, 19, 0, 100, 0, 14326, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Thrall - On Quest Accept 14326 - Say Line 0'),
(36145, 0, 1, 0, 19, 0, 100, 0, 14326, 0, 0, 0, 0, 12, 36622, 1, 10000, 0, 0, 0, 8, 0, 0, 0, 0, 993.146, 3852.02, 3.227, 0, 'Thrall - On Quest Accept 14326 - Summon Thrall (10 s)'),
(36145, 0, 2, 0, 19, 0, 100, 0, 14326, 0, 0, 0, 0, 28, 59073, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Thrall - On Quest Accept 14326 - Remove Phase Aura 59073 from invoker');

-- Orc Scout 36100 (summoned by 68336 on 14238 accept): Wowpedia 14238 'On accept' scout line.
DELETE FROM `smart_scripts` WHERE `entryorguid` = 36100 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(36100, 0, 0, 0, 54, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Orc Scout - On Just Summoned - Say Line 0');

-- Faceless of the Deep 38448: MoP smart_scripts 38448 ids 201/202 (cast flags 0; the MoP flag 4 is undefined here).
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38448 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38448, 0, 0, 0, 1, 0, 100, 1, 0, 5000, 0, 0, 0, 11, 72055, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Faceless of the Deep - Out of Combat - Cast Absorption Shield'),
(38448, 0, 1, 0, 0, 0, 100, 0, 5000, 5000, 5000, 5000, 0, 11, 75903, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 'Faceless of the Deep - In Combat - Cast Shadow Crash');

-- ===================== script bindings =====================
-- The MoP ScriptNames are reused where MoP binds the same entry (npc_Mechumide 35769, npc_killag_sangrecroc 35917,
-- npc_pant_gob 36585, npc_gyrocoptere_quest_giver 36127, npc_girocoptere 36143, npc_tornade_gob 36178, npc_fusee_gob 36514,
-- npc_young_naga_gob 38412/44578, gob_fronde_gobelin 196439, spell_weed_whacker 68211). Entries where MoP binds a
-- different entry than the one the DBC-correct flow uses get the same name on the LoA entry (npc_mineur_gob -> 35813 instead of
-- 35810; npc_Zapnozzle, npc_megs_isle_gob, npc_crack_isle_gob exist as ScriptNames but are unbound in the data).
UPDATE `creature_template` SET `ScriptName` = 'npc_Zapnozzle' WHERE `entry` = 36608;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_Mechumide' WHERE `entry` = 35769;
UPDATE `creature_template` SET `ScriptName` = 'npc_mineur_gob' WHERE `entry` = 35813;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_killag_sangrecroc' WHERE `entry` = 35917;
UPDATE `creature_template` SET `ScriptName` = 'npc_pant_gob' WHERE `entry` = 36585;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_gyrocoptere_quest_giver' WHERE `entry` = 36127;
UPDATE `creature_template` SET `ScriptName` = 'npc_girocoptere' WHERE `entry` = 36143;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_li_thrall_warchief' WHERE `entry` = 36161;
UPDATE `creature_template` SET `ScriptName` = 'npc_tornade_gob' WHERE `entry` = 36178;
UPDATE `creature_template` SET `ScriptName` = 'npc_li_sling_rocket' WHERE `entry` = 36505;
UPDATE `creature_template` SET `ScriptName` = 'npc_fusee_gob' WHERE `entry` = 36514;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_young_naga_gob' WHERE `entry` = 38412;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_young_naga_gob' WHERE `entry` = 44578;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_young_naga_gob' WHERE `entry` = 44579;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_young_naga_gob' WHERE `entry` = 44580;
UPDATE `creature_template` SET `ScriptName` = 'npc_megs_isle_gob' WHERE `entry` = 38432;
UPDATE `creature_template` SET `ScriptName` = 'npc_crack_isle_gob' WHERE `entry` = 39198;

-- SmartAI for the entries that got SAI rows above (only where no C++ script is bound).
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (34748, 35649, 35650, 35758, 35875, 35893, 36100, 36112, 36145, 36425, 36470, 36513, 36600, 38448) AND `ScriptName` = '';

-- Gameobjects.
UPDATE `gameobject_template` SET `AIName` = '', `ScriptName` = 'gob_fronde_gobelin' WHERE `entry` = 196439;
UPDATE `gameobject_template` SET `data0` = 68938 WHERE `entry` = 201938;

DELETE FROM `spell_script_names` WHERE `spell_id` IN (68211, 68338);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(68211, 'spell_weed_whacker'),
(68338, 'aura_infrared_orc_scout');

-- ===================== script_waypoint =====================
-- Miner Troubles path: 35810 path, keyed to the DBC-correct quest miner 35813 (the entry that has the creature_text) (19 rows)
DELETE FROM `script_waypoint` WHERE `entry` = 35813;
INSERT INTO `script_waypoint` (`entry`, `pointid`, `location_x`, `location_y`, `location_z`, `waittime`) VALUES
(35813, 0, 490.17, 2979.84, 7.96, 2000),
(35813, 1, 504.64, 2975.71, 7.26, 0),
(35813, 2, 518.08, 2970.58, 8.91, 0),
(35813, 3, 553.21, 2937.13, 1.47, 0),
(35813, 4, 567.74, 2936.75, 0.01, 0),
(35813, 5, 577.63, 2956.31, -1.43, 0),
(35813, 6, 588.16, 2964.23, -2.41, 5000),
(35813, 7, 595.49, 2957.88, -4.17, 0),
(35813, 8, 590.34, 2938.9, -7.53, 0),
(35813, 9, 575.81, 2906.65, -7.26, 5000),
(35813, 10, 576.52, 2889.07, -7.69, 0),
(35813, 11, 598.07, 2869.78, -6.33, 0),
(35813, 12, 608.96, 2858.01, -7.05, 5000),
(35813, 13, 612.71, 2866.21, -5.9, 0),
(35813, 14, 623.74, 2907.45, -2.62, 0),
(35813, 15, 634.83, 2922.38, -0.71, 0),
(35813, 16, 653.01, 2931.0, 0.13, 0),
(35813, 17, 665.7, 2949.08, -0.03, 5000),
(35813, 18, 658.23, 2960.95, 1.45, 2000);

-- To the Cliffs: Bastia ride (18 rows)
DELETE FROM `script_waypoint` WHERE `entry` = 36585;
INSERT INTO `script_waypoint` (`entry`, `pointid`, `location_x`, `location_y`, `location_z`, `waittime`) VALUES
(36585, 0, 867.63, 2824.62, 107.43, 3000),
(36585, 1, 869.24, 2845.64, 102.66, 0),
(36585, 2, 877.27, 2863.22, 101.38, 0),
(36585, 3, 911.58, 2886.83, 99.89, 0),
(36585, 4, 946.2, 2936.15, 108.51, 0),
(36585, 5, 968.47, 2946.36, 110.17, 0),
(36585, 6, 1050.07, 2978.12, 112.72, 0),
(36585, 7, 1083.81, 3039.47, 122.96, 0),
(36585, 8, 1079.34, 3081.42, 124.75, 0),
(36585, 9, 1061.9, 3108.49, 125.42, 0),
(36585, 10, 1031.07, 3137.44, 124.52, 0),
(36585, 11, 1023.36, 3159.06, 121.16, 0),
(36585, 12, 1029.44, 3171.27, 119.18, 0),
(36585, 13, 1050.68, 3193.39, 114.94, 0),
(36585, 14, 1056.43, 3204.82, 110.42, 0),
(36585, 15, 1059.2, 3220.41, 99.73, 0),
(36585, 16, 1065.47, 3227.1, 93.97, 0),
(36585, 17, 1079.17, 3238.69, 81.86, 3000);

-- Precious Cargo: Gyrochoppa flight (6 rows)
DELETE FROM `script_waypoint` WHERE `entry` = 36143;
INSERT INTO `script_waypoint` (`entry`, `pointid`, `location_x`, `location_y`, `location_z`, `waittime`) VALUES
(36143, 1, 846.743, 3335.54, 10.1433, 0),
(36143, 2, 755.885, 3374.22, 14.3933, 0),
(36143, 3, 780.326, 3449.16, 14.3933, 0),
(36143, 4, 1071.62, 3589.42, 26.2227, 0),
(36143, 5, 1098.78, 3729.42, 93.0283, 0),
(36143, 6, 971.514, 3802.34, 14.3616, 0);

-- Warchief's Revenge: Cyclone of the Elements loop (32 rows)
DELETE FROM `script_waypoint` WHERE `entry` = 36178;
INSERT INTO `script_waypoint` (`entry`, `pointid`, `location_x`, `location_y`, `location_z`, `waittime`) VALUES
(36178, 1, 982.576, 3826.71, 23.0219, 0),
(36178, 2, 982.576, 3826.71, 34.1519, 0),
(36178, 3, 973.276, 3832.12, 46.5758, 0),
(36178, 4, 950.23, 3829.0, 39.8924, 0),
(36178, 5, 925.56, 3825.7, 34.0517, 0),
(36178, 6, 901.331, 3831.65, 32.4751, 0),
(36178, 7, 889.159, 3842.11, 32.4296, 0),
(36178, 8, 881.847, 3868.53, 32.4174, 0),
(36178, 9, 885.848, 3895.51, 30.3379, 0),
(36178, 10, 899.286, 3922.17, 33.991, 0),
(36178, 11, 925.813, 3935.75, 41.2059, 0),
(36178, 12, 949.887, 3930.92, 44.487, 0),
(36178, 13, 964.961, 3907.52, 44.0504, 0),
(36178, 14, 964.414, 3892.4, 43.0649, 0),
(36178, 15, 957.613, 3867.64, 42.1959, 0),
(36178, 16, 946.854, 3829.93, 50.6892, 0),
(36178, 17, 945.911, 3815.27, 54.7268, 0),
(36178, 18, 960.21, 3781.94, 54.3537, 0),
(36178, 19, 981.227, 3770.49, 49.5633, 0),
(36178, 20, 1006.44, 3777.55, 47.8623, 0),
(36178, 21, 1018.82, 3800.6, 48.865, 0),
(36178, 22, 1027.22, 3825.35, 51.828, 0),
(36178, 23, 1040.08, 3840.14, 56.5372, 0),
(36178, 24, 1066.98, 3845.72, 60.3242, 0),
(36178, 25, 1084.22, 3824.06, 59.1033, 0),
(36178, 26, 1085.46, 3796.7, 53.7313, 0),
(36178, 27, 1076.84, 3777.56, 50.3004, 0),
(36178, 28, 1050.62, 3768.04, 43.8942, 0),
(36178, 29, 1035.78, 3780.65, 40.744, 0),
(36178, 30, 1019.06, 3808.61, 38.9818, 0),
(36178, 31, 1010.13, 3817.01, 39.4104, 0),
(36178, 32, 979.786, 3830.44, 41.544, 0);

-- Up, Up & Away!: Gallywix's rocket (4 rows)
DELETE FROM `script_waypoint` WHERE `entry` = 36514;
INSERT INTO `script_waypoint` (`entry`, `pointid`, `location_x`, `location_y`, `location_z`, `waittime`) VALUES
(36514, 1, 916.37, 2591.61, 207.502, 0),
(36514, 2, 928.458, 2538.97, 196.308, 0),
(36514, 3, 939.998, 2386.25, 4.63964, 1500),
(36514, 4, 939.998, 2386.25, 4.63964, 0);

-- Up, Up & Away!: the player's sling rocket (the player flew with 36514; same four points) (4 rows)
DELETE FROM `script_waypoint` WHERE `entry` = 36505;
INSERT INTO `script_waypoint` (`entry`, `pointid`, `location_x`, `location_y`, `location_z`, `waittime`) VALUES
(36505, 1, 916.37, 2591.61, 207.502, 0),
(36505, 2, 928.458, 2538.97, 196.308, 0),
(36505, 3, 939.998, 2386.25, 4.63964, 1500),
(36505, 4, 939.998, 2386.25, 4.63964, 0);

-- ===================== Rollback =====================
--   UPDATE `quest_objective` SET `amount` = 1 WHERE `questId` = 14243 AND `id` = 265514;
--   DELETE FROM `creature_text` WHERE `CreatureID` IN (36608, 36145) OR (`CreatureID` = 36600 AND `GroupID` = 1) OR (`CreatureID` = 39198 AND `GroupID` BETWEEN 1 AND 5);
--   UPDATE `creature_text` SET `GroupID` = `ID`, `ID` = 0 WHERE `CreatureID` = 34748 AND `GroupID` = 0 AND `ID` BETWEEN 1 AND 6;
--   DELETE FROM `smart_scripts` WHERE (`entryorguid` IN (34748, 35649, 35650, 35758, 35875, 35893, 36100, 36112, 36145, 36425, 36470, 36513, 36600, 38448, 3474800, 3564900, 3651300) AND `source_type` IN (0, 9)) OR (`entryorguid` IN (195188, 202108) AND `source_type` = 1);
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 195188 AND `SourceId` = 1;
--   Re-insert the removed LoA rows (hacks): 35769 id0 (ev19 14021 -> credit 35816), 35917 id0 (ev19 14240 -> teleport 648 1078.49,3238.95,82.3846),
--   36127 id0 (ev19 14242 -> teleport 648 971.731,3802.93,14.9325), 36161 id0/id1 (ev19 14243 -> credit 36176 -> teleport 871.797,2768.51,119.17),
--   196439 source 1 id0-2 (gossip 10808 -> teleport 945.498,2395.88,3.64052 + credit 50046 + close gossip), 202108 id1 (credit 36682),
--   195188 id1 (summon 35649, 30 s), 38412/44578/44579/44580 id0-1 (spellclick -> credit 38413 -> despawn); then restore AIName = 'SmartAI' on them.
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` IN (36608, 35769, 35813, 35917, 36585, 36127, 36143, 36161, 36178, 36505, 36514, 38412, 44578, 44579, 44580, 38432, 39198);
--   UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (35769, 35917, 36127, 36161, 38412, 44578, 44579, 44580);
--   UPDATE `creature_template` SET `AIName` = '' WHERE `entry` IN (34748, 35649, 35650, 35758, 35875, 35893, 36100, 36112, 36145, 36425, 36470, 36513, 36600, 38448);
--   UPDATE `gameobject_template` SET `AIName` = 'SmartGameObjectAI', `ScriptName` = '' WHERE `entry` = 196439;
--   UPDATE `gameobject_template` SET `data0` = 0 WHERE `entry` = 201938;
--   DELETE FROM `spell_script_names` WHERE `spell_id` IN (68211, 68338);
--   DELETE FROM `script_waypoint` WHERE `entry` IN (35813, 36585, 36143, 36178, 36514, 36505);
