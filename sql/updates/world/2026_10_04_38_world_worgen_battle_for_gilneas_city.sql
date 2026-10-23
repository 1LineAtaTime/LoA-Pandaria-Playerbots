-- Refs #439, #478. Quest 24904 "The Battle for Gilneas City". NEEDS THE BUILD that carries the zone_gilneas.cpp
-- scripts npc_krennan_aranas, npc_prince_liam_greymane_gilneas and npc_lady_sylvanas_gilneas (until then the
-- quest has no source of its credit; apply this file together with that build).
--
-- Intended (Wowpedia "The Battle for Gilneas City", MediaWiki API 2026-10-04): Krennan yells "It's time to join the
-- fray, <name>!" on accept; speaking to Krennan ("Let us take back our city!") starts the battle led by Liam,
-- which ends with Sylvanas' arrow killing Liam; Wowhead tooltip API (mop-classic quest=24904): requirement
-- "Battle for Gilneas City Complete" (38854).
-- Fork today: Lorna Crowley 37783 SAI gives 38854 on accept and teleports the player to Greymane Court, so the quest
-- completes on accept and the battle does not exist; Krennan 38553, Liam 38218 and Sylvanas 38469 have no AI
-- (the intended scripts are C++ classes of the same names; mopcore gilneas.cpp:5051-5870).
-- This file: binds the three ScriptNames, replaces Lorna's placeholder by Krennan's accept yell (target: closest
-- 38553), adds the missing finale lines (Liam "FATHER!!!" BT 38369 with sound 19612 as on Liam 38474, "We did it,
-- father..." BT 38371, "We took back our city..." BT 38372; Sylvanas "Such a waste..." BT 38370), and the implicit
-- target condition of 72116 "Shoot Liam" (TARGET_UNIT_NEARBY_ENTRY) -> 38218.
--
-- Rollback:
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` IN (38553, 38218, 38469);
--   DELETE FROM `smart_scripts` WHERE `entryorguid` = 37783 AND `source_type` = 0;
--   INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
--   (37783, 0, 0, 1, 19, 0, 100, 0, 24904, 0, 0, 0, 0, 33, 38854, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Lorna Crowley - Quest Accept - Killcredit'),
--   (37783, 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 62, 654, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, -1681.73, 1619.83, 20.5728, 0, 'Lorna Crowley - Quest Accept - Teleport');
--   DELETE FROM `creature_text` WHERE (`CreatureID` = 38218 AND `GroupID` IN (12, 13, 14)) OR (`CreatureID` = 38469 AND `GroupID` = 2);
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 72116;
-- Re-runnable: UPDATE + DELETE-then-INSERT.

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_krennan_aranas' WHERE `entry` = 38553;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_prince_liam_greymane_gilneas' WHERE `entry` = 38218;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_lady_sylvanas_gilneas' WHERE `entry` = 38469;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 37783 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(37783, 0, 0, 0, 19, 0, 100, 0, 24904, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 19, 38553, 60, 0, 0, 0, 0, 0, 0, 'Lorna Crowley - On Quest 24904 Accept - Krennan Aranas yells "It''s time to join the fray"');

DELETE FROM `creature_text` WHERE (`CreatureID` = 38218 AND `GroupID` IN (12, 13, 14)) OR (`CreatureID` = 38469 AND `GroupID` = 2);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(38218, 12, 0, 'FATHER!!!', 14, 0, 100, 0, 0, 19612, 0, 38369, 0, 'Prince Liam Greymane - battle finale'),
(38218, 13, 0, 'We did it, father...', 12, 0, 100, 0, 0, 0, 0, 38371, 0, 'Prince Liam Greymane - battle finale'),
(38218, 14, 0, 'We took back our city... we took back...', 12, 0, 100, 0, 0, 0, 0, 38372, 0, 'Prince Liam Greymane - battle finale'),
(38469, 2, 0, 'Such a waste.  That arrow''s poison was not meant to be wasted on your whelp.  We''ll meet again, Genn!', 12, 0, 100, 0, 0, 0, 0, 38370, 0, 'Lady Sylvanas Windrunner - battle finale');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 72116;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 72116, 0, 0, 31, 0, 3, 38218, 0, 0, 0, 0, '', 'Shoot Liam - targets Prince Liam Greymane (battle)');
