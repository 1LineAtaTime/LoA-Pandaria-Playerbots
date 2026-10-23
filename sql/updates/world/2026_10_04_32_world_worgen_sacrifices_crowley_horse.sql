-- Refs #439, #474. Quest 14212 "Sacrifices" (Gilneas City, map 654, area 4758).
--
-- Intended (Wowpedia "Sacrifices", MediaWiki API, read 2026-10-04): hop on Crowley's Horse, ride with
-- Lord Darius Crowley through the city and firebomb Bloodfang Stalkers (vehicle ability 1, spell 67063);
-- after the run you can hop back on the horse for another run.
-- Fork today: Crowley's Horse 35231 is a STATIC spawn (guid 371595, phase 8) with spellclick 46598, but its
-- SmartAI only starts the run "On Just Summoned" (event 54 -> actionlist 3523100: cast 67003 Summon Crowley
-- (Mounted) 35230, then WP path 35231). A static spawn is never "just summoned", so the player sits on a
-- horse that never moves and Crowley never mounts. (MoP 5.4.8 has no static horse; its
-- npc_lord_darius_crowley_c2 summons one. mopcore's npc_crowley_horse starts the run in PassengerBoarded:
-- mopcore gilneas.cpp:2332-2390.)
-- Fix: start the same actionlist when a PLAYER boards the static horse (SAI event 27 PASSENGER_BOARDED,
-- not repeatable per spawn, condition: invoker is a player). The path end (WP 33) already ejects the rider and
-- despawns the mounted Crowley; WP_START despawn time 1000 ms despawns the horse, which respawns at home.
--
-- Rollback:
--   DELETE FROM `smart_scripts` WHERE `entryorguid` = 35231 AND `source_type` = 0 AND `id` = 8;
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 9 AND `SourceEntry` = 35231;
-- Re-runnable: DELETE-then-INSERT.

DELETE FROM `smart_scripts` WHERE `entryorguid` = 35231 AND `source_type` = 0 AND `id` = 8;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(35231, 0, 8, 0, 27, 0, 100, 1, 0, 0, 0, 0, 0, 80, 3523100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Crowley''s Horse - On Player Boarded (static spawn) - Start Timed Action Script 3523100');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 9 AND `SourceEntry` = 35231;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 9, 35231, 0, 0, 31, 0, 4, 0, 0, 0, 0, 0, '', 'Crowley''s Horse - start the run only when a player boards');
