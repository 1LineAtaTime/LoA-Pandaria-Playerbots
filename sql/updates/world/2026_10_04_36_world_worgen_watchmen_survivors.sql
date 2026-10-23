-- Refs #439, #483 (items 2 and 5). NEEDS THE BUILD that carries the zone_gilneas.cpp AIs npc_drowning_watchman and
-- npc_crash_survivor (without it the two creatures fall back to no AI: 14395 then has no credit at all).
--
-- Item 2: 14395 "Gasping for Breath". Intended (Wowpedia "Gasping for Breath", MediaWiki API 2026-10-04): pick up
-- a drowning watchman, swim him to shore, he thanks you there ("Make sure to get them out of the water and onto
-- shore"). Fork today: spellclick 68735 runs SAI actionlist 3644000 that gives the credit 5 s after the click
-- wherever the player is (MoP 5.4.8 has the same SAI plus a C++
-- npc_drowning_watchman: SpellHit 68735 -> EnterVehicle(clicker)). The C++ AI here carries the
-- watchman and credits 36450 once the carrier stands out of the water (4.3.4 reference: CPP gilneas_chapter_2.cpp
-- at_gasping_for_breath / spell 68737 credit on the shore). The fork's creature_text 36440 groups 0-2 are kept.
--
-- Item 5: 24468 "Stranded at the Marsh". Intended (Wowpedia, MediaWiki API 2026-10-04): the survivors are locked in
-- combat with Swamp Crocolisks; kill the crocolisk, the survivor says thanks and runs back to the crash site.
-- Source: MoP 5.4.8 data smart_scripts 37067 / 3706700 / 37078 and creature_text 37067 (MoP keeps only 4 static
-- crocolisks - guids 221468, 221850, 221874, 221878 - and summons one per survivor). The behaviour is in the C++
-- AI npc_crash_survivor; this file binds it, adds the texts (BT 36909-36912) and removes the 11 static crocolisks
-- that stand next to each survivor (the summon replaces them; the objective still counts 37078 kills).
--
-- Rollback:
--   UPDATE `creature_template` SET `ScriptName` = '', `AIName` = 'SmartAI' WHERE `entry` = 36440;
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` = 37067;
--   DELETE FROM `creature_text` WHERE `CreatureID` = 37067;
--   INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
--   (36440, 0, 0, 0, 73, 0, 100, 0, 0, 0, 0, 0, 0, 80, 3644000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Drowning Watchman - On Spellclick - Run Script'),
--   (3644000, 9, 0, 0, 0, 0, 100, 0, 1, 1, 0, 0, 0, 11, 46598, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Drowning Watchman - Script - '),
--   (3644000, 9, 1, 0, 0, 0, 100, 0, 5000, 5000, 0, 0, 0, 33, 36450, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Drowning Watchman - Script - Killcredit'),
--   (3644000, 9, 2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 28, 46598, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Drowning Watchman - Script - Remove Ride Vehicle Hardcoded'),
--   (3644000, 9, 3, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 28, 68735, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Drowning Watchman - Script - Remove Rescue Drowning Watchman'),
--   (3644000, 9, 4, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Drowning Watchman - Script - Despawn');
--   INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `modelid`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `curhealth`, `curmana`, `MovementType`) VALUES
--   (221486, 37078, 654, 4714, 4714, 1, 131072, 33450, -2035.11, 1822.58, 6.39407, 5.55015, 60, 0, 156, 0, 0),
--   (221848, 37078, 654, 4714, 4714, 1, 131072, 33450, -2090.77, 1769.82, 5.60029, 3.12414, 60, 0, 156, 0, 0),
--   (221852, 37078, 654, 4714, 4714, 1, 131072, 33450, -2062.98, 1811.81, 6.76907, 4.67748, 60, 0, 156, 0, 0),
--   (221854, 37078, 654, 4714, 4714, 1, 131072, 33450, -2078.92, 1802.41, 5.99517, 4.67748, 60, 0, 156, 0, 0),
--   (221868, 37078, 654, 4714, 4714, 1, 131072, 33450, -2041.48, 1753.8, 7.13064, 4.36332, 60, 0, 156, 0, 0),
--   (221870, 37078, 654, 4714, 4714, 1, 131072, 33450, -1974.44, 1754.43, 6.22959, 4.67748, 60, 0, 156, 0, 0),
--   (221882, 37078, 654, 4714, 4714, 1, 131072, 33450, -1972.23, 1796.59, 6.9872, 0.174533, 60, 0, 156, 0, 0),
--   (221919, 37078, 654, 4714, 4714, 1, 131072, 33450, -2078.83, 1805.24, 6.24517, 4.67748, 60, 0, 156, 0, 0),
--   (221920, 37078, 654, 4714, 4714, 1, 131072, 33450, -2087.94, 1769.77, 5.60029, 3.12414, 60, 0, 156, 0, 0),
--   (221923, 37078, 654, 4714, 4714, 1, 131072, 33450, -2062.89, 1814.64, 6.64407, 4.67748, 60, 0, 156, 0, 0),
--   (221924, 37078, 654, 4714, 4714, 1, 131072, 33450, -2037.21, 1824.47, 6.64407, 5.55015, 60, 0, 156, 0, 0);
-- Re-runnable: UPDATE + DELETE-then-INSERT.

-- Item 2: the watchman AI replaces the 5-second SAI.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_drowning_watchman' WHERE `entry` = 36440;
DELETE FROM `smart_scripts` WHERE (`entryorguid` = 36440 AND `source_type` = 0) OR (`entryorguid` = 3644000 AND `source_type` = 9);

-- Item 5: survivors fight their own summoned crocolisk.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_crash_survivor' WHERE `entry` = 37067;
DELETE FROM `creature` WHERE `id` = 37078 AND `guid` IN (221486, 221848, 221852, 221854, 221868, 221870, 221882, 221919, 221920, 221923, 221924);
DELETE FROM `creature_addon` WHERE `guid` IN (221486, 221848, 221852, 221854, 221868, 221870, 221882, 221919, 221920, 221923, 221924);

DELETE FROM `creature_text` WHERE `CreatureID` = 37067;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(37067, 0, 0, 'You got here just in time.  Thanks!', 12, 0, 100, 0, 0, 0, 0, 36909, 0, 'Crash Survivor - saved'),
(37067, 0, 1, 'I wasn''t ready to die in a marsh.  Thanks again!', 12, 0, 100, 0, 0, 0, 0, 36910, 0, 'Crash Survivor - saved'),
(37067, 0, 2, 'Time to run!', 12, 0, 100, 0, 0, 0, 0, 36911, 0, 'Crash Survivor - saved'),
(37067, 0, 3, 'Let''s get out of this marsh!', 12, 0, 100, 0, 0, 0, 0, 36912, 0, 'Crash Survivor - saved');
