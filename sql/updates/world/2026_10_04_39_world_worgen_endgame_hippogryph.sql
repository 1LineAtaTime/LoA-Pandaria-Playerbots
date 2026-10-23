-- Refs #439, #479. Quest 26706 "Endgame" (Keel Harbor). NEEDS THE BUILD that carries the zone_gilneas.cpp scripts
-- npc_hippogryph_endgame, npc_hippogryph and npc_tobias_mistmantle_endgame (without it the quest has no credit
-- source; apply together with that build).
--
-- Intended (Wowhead tooltip API quest=26706, 2026-10-04: "Jump on a Hippogryph when Tobias signals you and destroy
-- the Horde Gunship. Requirements: Gunship destroyed"; Wowpedia "Endgame (quest)": a timed event, Tobias' waiting
-- lines BT 43815/43816/43837/43838/43839, then "Hop on a hippogryph, now's our chance!" BT 43814 and "Time to go!"
-- BT 43813). Wowhead MoP Classic quest page: "Requires level 12" = the fork's MinLevel 12 (unchanged).
-- Fork today: Lorna Crowley 43727 SAI gives 43729 "Finale Credit" on accept (quest done on accept), Hippogryph
-- 43747 has no npcflag and no AI, the vehicle 43751 and Tobias 43749 have no AI.
-- Source: MoP 5.4.8 data (43747 npcflag 1 + ScriptName npc_hippogryph_endgame, 43751 npc_hippogryph, Lorna 43727 SAI
-- "on accept: Tobias says text 0") and the intended behaviour of the two hippogryph
-- scripts (see zone_gilneas.cpp). The flight path (script_waypoint 43751 in the MoP data) is compiled into the
-- C++ (MoveSmoothPath), so no waypoint rows are needed here.
--
-- Rollback:
--   UPDATE `creature_template` SET `npcflag` = 0, `ScriptName` = '' WHERE `entry` = 43747;
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` IN (43751, 43749);
--   DELETE FROM `smart_scripts` WHERE `entryorguid` = 43727 AND `source_type` = 0;
--   INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
--   (43727, 0, 0, 1, 19, 0, 100, 0, 26706, 0, 0, 0, 0, 33, 43729, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Lorna Crowley - Quest Accept - Killcredit');
--   DELETE FROM `creature_text` WHERE `CreatureID` = 43749;
--   INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
--   (43749, 0, 0, 'We must wait for the right time to strike!', 12, 0, 100, 0, 0, 0, 0, 43815, 0, 'Tobias Mistmantle'),
--   (43749, 1, 0, 'Let''s give everyone time to gather.  We''ll need a handful of people to come with us.', 12, 0, 100, 0, 0, 0, 0, 43840, 0, 'Tobias Mistmantle'),
--   (43749, 2, 0, 'Time to go!', 12, 0, 100, 0, 0, 0, 0, 43813, 0, 'Tobias Mistmantle');
-- Re-runnable: UPDATE + DELETE-then-INSERT.

UPDATE `creature_template` SET `npcflag` = `npcflag` | 1, `AIName` = '', `ScriptName` = 'npc_hippogryph_endgame' WHERE `entry` = 43747;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_hippogryph' WHERE `entry` = 43751;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_tobias_mistmantle_endgame' WHERE `entry` = 43749;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 43727 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(43727, 0, 0, 0, 19, 0, 100, 0, 26706, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 43749, 60, 0, 0, 0, 0, 0, 0, 'Lorna Crowley - On Quest 26706 Accept - Tobias Mistmantle says a waiting line');

-- Group 0 = waiting lines (random), 1 = the signal, 2 = "Time to go!".
DELETE FROM `creature_text` WHERE `CreatureID` = 43749;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(43749, 0, 0, 'We must wait for the right time to strike!', 12, 0, 100, 0, 0, 0, 0, 43815, 0, 'Tobias Mistmantle - Endgame waiting'),
(43749, 0, 1, 'We must bide our time.  They won''t know what hit them.', 12, 0, 100, 0, 0, 0, 0, 43816, 0, 'Tobias Mistmantle - Endgame waiting'),
(43749, 0, 2, 'We''ll set flight once everyone is ready.', 12, 0, 100, 0, 0, 0, 0, 43837, 0, 'Tobias Mistmantle - Endgame waiting'),
(43749, 0, 3, 'Patience, friends.  We will strike soon.', 12, 0, 100, 0, 0, 0, 0, 43838, 0, 'Tobias Mistmantle - Endgame waiting'),
(43749, 0, 4, 'Hold your horses, friend.  We''ll hit them when we''re all ready.', 12, 0, 100, 0, 0, 0, 0, 43839, 0, 'Tobias Mistmantle - Endgame waiting'),
(43749, 0, 5, 'Let''s give everyone time to gather.  We''ll need a handful of people to come with us.', 12, 0, 100, 0, 0, 0, 0, 43840, 0, 'Tobias Mistmantle - Endgame waiting'),
(43749, 1, 0, 'Hop on a hippogryph, now''s our chance!', 12, 0, 100, 0, 0, 0, 0, 43814, 0, 'Tobias Mistmantle - Endgame signal'),
(43749, 2, 0, 'Time to go!', 12, 0, 100, 0, 0, 0, 0, 43813, 0, 'Tobias Mistmantle - Endgame signal');
