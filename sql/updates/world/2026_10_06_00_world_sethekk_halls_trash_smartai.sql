-- #569 (Phase 6 dungeons, wave C port of AC tbc-seth, issue #565): Sethekk Halls trash the strategy depends on has no script on MoP.
--
-- Evidence (test DBs, read-only, 2026-10-05; AC acore_world vs MoP world):
--   * Time-Lost Controller 18327: AC AIName SmartAI + 3 smart_scripts rows (aggro Say, Summon Charming Totem 32764, Shrink 35013).
--     MoP: AIName empty, ScriptName generic_creature (no-op), no rows.
--   * Charming Totem 20343: AC SmartAI row 'Cast Charm 35120 on the closest player' + faction 58. MoP: no AI, faction 16.
--     AC's Seth strategy (SethActions.cpp:23 'charming totem' reaction, SethShared.h:36) assumes the totem charms.
--   * Sethekk Prophet 18325: MoP already has AC's Fear 27641 cast as row 0 (the MoP row, event_flags 6 = normal+heroic dungeon,
--     1-3 s initial / 8-12 s repeat; AC 8.7-17.7 s / 13.2-24.1 s). It is KEPT as it is (MoP row preferred). Missing vs AC:
--     the aggro Say (chance 15) and the death cast 'Summon Arakkoa Spirit' 32692. creature_text for 18325/18327 is missing on MoP.
--   * No fork C++ replaces any of these (grep 18327/20343/32692 in src/server/scripts: nothing in Outland/Auchindoun).
--
-- 5.4.8 flag trap: AC event_flags 0x200 (WHILE_CHARMED) is RAID40 in this fork's SmartScriptMgr.h:1427, so AC rows are written with flags 0
-- (or 0x1 NOT_REPEATABLE as in AC). Spell ids 32764/35013/35120/32692 are AC's; their existence in the 5.4.8 Spell.dbc is NOT verified
-- here (no DBC reader in this lane) - watch for 'SmartScript: Spell %u not exist' in DBErrors/Server.log on boot.
-- BroadcastTextIds 16716-16720 exist in MoP world.broadcast_text.
-- Rollback:
--   DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND ((`entryorguid` IN (18327, 20343)) OR (`entryorguid` = 18325 AND `id` IN (1, 2)));
--   UPDATE `creature_template` SET `AIName` = '' WHERE `entry` IN (18327, 20343);
--   UPDATE `creature_template` SET `faction` = 16 WHERE `entry` = 20343;
--   DELETE FROM `creature_text` WHERE `CreatureID` IN (18325, 18327);

DELETE FROM `creature_text` WHERE `CreatureID` IN (18325, 18327);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(18325, 0, 0, 'In Terokk''s name!', 12, 0, 100, 0, 0, 0, 16716, 0, 'Sethekk Halls Trash'),
(18325, 0, 1, 'Protect the Veil!', 12, 0, 100, 0, 0, 0, 16717, 0, 'Sethekk Halls Trash'),
(18325, 0, 2, 'Darkfire -- avenge us!', 12, 0, 100, 0, 0, 0, 16718, 0, 'Sethekk Halls Trash'),
(18325, 0, 3, 'Ssssekk-sara Rith-nealaak!', 12, 0, 100, 0, 0, 0, 16719, 0, 'Sethekk Halls Trash'),
(18325, 0, 4, 'Arak-ha!', 12, 0, 100, 0, 0, 0, 16720, 0, 'Sethekk Halls Trash'),
(18327, 0, 0, 'In Terokk''s name!', 12, 0, 100, 0, 0, 0, 16716, 0, 'Sethekk Halls Trash'),
(18327, 0, 1, 'Protect the Veil!', 12, 0, 100, 0, 0, 0, 16717, 0, 'Sethekk Halls Trash'),
(18327, 0, 2, 'Darkfire -- avenge us!', 12, 0, 100, 0, 0, 0, 16718, 0, 'Sethekk Halls Trash'),
(18327, 0, 3, 'Ssssekk-sara Rith-nealaak!', 12, 0, 100, 0, 0, 0, 16719, 0, 'Sethekk Halls Trash'),
(18327, 0, 4, 'Arak-ha!', 12, 0, 100, 0, 0, 0, 16720, 0, 'Sethekk Halls Trash');

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (18325, 18327, 20343);
UPDATE `creature_template` SET `faction` = 58 WHERE `entry` = 20343;

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND ((`entryorguid` IN (18327, 20343)) OR (`entryorguid` = 18325 AND `id` IN (1, 2)));
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(18325, 0, 1, 0, 4, 0, 15, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Sethekk Prophet - On Aggro - Say Line 0'),
(18325, 0, 2, 0, 6, 0, 100, 0, 0, 0, 0, 0, 11, 32692, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Sethekk Prophet - On Death - Cast ''Summon Arakkoa Spirit'''),
(18327, 0, 0, 0, 4, 0, 15, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Controller - On Aggro - Say Line 0'),
(18327, 0, 1, 0, 0, 0, 100, 0, 9100, 24100, 27800, 48300, 11, 32764, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Controller - In Combat - Cast ''Summon Charming Totem'''),
(18327, 0, 2, 0, 0, 0, 100, 0, 8400, 23200, 9700, 32600, 11, 35013, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 'Time-Lost Controller - In Combat - Cast ''Shrink'''),
(20343, 0, 0, 0, 60, 0, 100, 1, 2000, 2000, 0, 0, 11, 35120, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 0, 'Charming Totem - Update - Cast Charm');
