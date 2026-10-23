-- Refs #439, #475. Follow-up of 2026_10_04_33 (The Hunt For Sylvanas 24902): Tobias Mistmantle 38507 gets pulled
-- into combat right after Lorna summons him, and his path never starts.
--
-- Evidence (t439 run on build 3/4): Server.log "ERROR SmartAI::StartPath: Creature entry 38507 wanted to start
-- waypoint movement while in combat, ignoring." (SmartAI.cpp:125-129), cp8.log shows only his first line, the
-- Sylvanas scene never runs, and 24902 stays at status 3.
-- Intended (Wowpedia "The Hunt For Sylvanas"): Tobias sneaks to the cathedral unseen, with the player following
-- him. The fix makes the summoned Tobias passive and immune to players and NPCs, so nothing in the phase-2097152
-- city can engage him and the timed path/jump script from _33 runs as written.
--
-- Rollback:
--   DELETE FROM `smart_scripts` WHERE `entryorguid` = 38507 AND `source_type` = 0 AND `id` IN (19, 20);
-- Re-runnable: DELETE-then-INSERT.

DELETE FROM `smart_scripts` WHERE `entryorguid` = 38507 AND `source_type` = 0 AND `id` IN (19, 20);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(38507, 0, 19, 20, 54, 0, 100, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - On Just Summoned - Set Passive'),
(38507, 0, 20, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 18, 768, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Tobias Mistmantle - On Just Summoned - Immune to PC and NPC (he sneaks, nothing may engage him)');
