-- #569 (Phase 6 dungeons): two more strategy-relevant entries with AC SmartAI and nothing on MoP.
--
--   * Sliver 24552 (Magisters' Terrace, Delrissa's add; AC tbc-mgt MgTShared.cpp:99, :1376 targets it): AC SmartAI 'Thrash' 12787
--     (initial 5-10 s, repeat 15-20 s). MoP: no AI/script, ScriptName npc_sliver (a C++ script not in the fork:
--     grep src/server/scripts finds only the summon in boss_priestess_delrissa.cpp:1002,1043). Port = AC's SAI row.
--   * Web Wrap 28619 (Azjol-Nerub, Krik'thir's cocoon; AC wotlk-an ANActions.cpp:26, ANTriggers.cpp:24 attack it to free the wrapped ally):
--     AC SmartAI 'On AI Init - passive' + 'On Death - remove the owner's Web Wrap stun 52087'. MoP: none. The fork's Krik'thir
--     script only re-applies the stun when the root expires (boss_krikthir_the_gatewatcher.cpp:574-596) and nothing removes it when the cocoon dies.
--   AC unit_flags differ (Web Wrap AC 131076 vs MoP 0) - NOT touched here, flag semantics changed in 5.x; tracked in the issue.
-- AC event_flags 0x200 (WHILE_CHARMED) is RAID40 on this fork (SmartScriptMgr.h:1427): written as 0.
-- Rollback:
--   DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` IN (24552, 28619);
--   UPDATE `creature_template` SET `AIName` = '' WHERE `entry` IN (24552, 28619);

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (24552, 28619);

DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` IN (24552, 28619);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(24552, 0, 0, 0, 0, 0, 100, 0, 5000, 10000, 15000, 20000, 11, 12787, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Sliver - In Combat - Cast ''Thrash'''),
(28619, 0, 0, 0, 37, 0, 100, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Web Wrap - On AI Init - Set React Passive'),
(28619, 0, 1, 0, 6, 0, 100, 0, 0, 0, 0, 0, 28, 52087, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 'Web Wrap - On Death - Remove Aura From Owner');
