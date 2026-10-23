-- #635 (epic #569, Phase 6 Sethekk Halls): Anzu's Hawk/Falcon/Eagle spirits get the C++ npc_anzu_spirit (AC boss_anzu.cpp:184-236, ported
-- into src/server/scripts/Outland/Auchindoun/SethekkHalls/boss_anzu.cpp, which now also summons them on engage like AC SummonSpirits).
--
-- Evidence (test DBs, read-only, 2026-10-05; AC acore_world vs MoP world):
--   * AC: ScriptName npc_anzu_spirit on 23134/23135/23136, no AIName, no smart_scripts.
--   * MoP: ScriptName empty on all three; only the Eagle (23136) has AIName SmartAI + smart_scripts row 'On Spellhit - Cast 40240' (same row
--     as upstream), so the Eagle fired on any spell hit and Hawk/Falcon never fired. That SAI row is removed: it would double-fire next to
--     the C++ HoT check. No creature_text for the three spirits on MoP; AC has group 0 '%s returns to stone.' (broadcast text 20980, present
--     in MoP world.broadcast_text), added here for the Say in SAY_STONED.
-- Rollback:
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` IN (23134, 23135, 23136);
--   UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` = 23136;
--   DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 23136;
--   INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES (23136, 0, 0, 0, 8, 0, 100, 4, 0, 7, 0, 0, 0, 11, 40240, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Eagle Spirit - On Spellhit - Cast ''Spite of the Eagle''');
--   DELETE FROM `creature_text` WHERE `CreatureID` IN (23134, 23135, 23136);

UPDATE `creature_template` SET `ScriptName` = 'npc_anzu_spirit' WHERE `entry` IN (23134, 23135, 23136);
UPDATE `creature_template` SET `AIName` = '' WHERE `entry` = 23136;
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` = 23136;

DELETE FROM `creature_text` WHERE `CreatureID` IN (23134, 23135, 23136);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(23134, 0, 0, '%s returns to stone.', 16, 0, 100, 0, 0, 0, 20980, 0, 'Hawk Spirit'),
(23135, 0, 0, '%s returns to stone.', 16, 0, 100, 0, 0, 0, 20980, 0, 'Falcon Spirit'),
(23136, 0, 0, '%s returns to stone.', 16, 0, 100, 0, 0, 0, 20980, 0, 'Eagle Spirit');
