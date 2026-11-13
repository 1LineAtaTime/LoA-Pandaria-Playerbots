-- #1005 quests(kezan): 14125 "447" - Gasbot Control Panel 201736 not visible during the quest.
--
-- Root cause: Kezan (zone 4737) phase_definitions entry 5 (phasemask 96 = 32|64) requires 14120 rewarded AND 14125
-- QUEST_NONE (condition 14) AND 14125 not complete; entry 6 (phasemask 128) requires 14125 rewarded OR complete.
-- While 14125 is INCOMPLETE (taken, objectives open) neither entry passes, so the player falls back to phasemask 1.
-- Everything the quest needs outside KTC HQ lives in 16/32/64/128: the Gasbot Control Panel 201736 (176), its collision
-- 188215 (224), Sassy Hardwrench 34668 (240), the explosives crates 201740 (64). Only the three indoor objective GOs
-- (201733/201734/201735, phasemask 255) stayed visible. Same rows in the reference 5.4.8 world DB; TDB 4.3.4 / SFDB 5.4.8 have no
-- Kezan phase data; the quest text says "come back outside and use the control panel to send in the gasbot", so the
-- panel must be visible while the quest is in progress.
--
-- Fix: entry 5 gets a second OR group (ElseGroup 1): 14120 rewarded AND 14125 taken (condition 9 = INCOMPLETE only).
-- The reference DB rows of ElseGroup 0 are left as they are. Takes effect at worldserver start (phase definition
-- conditions are loaded with the conditions table).
--
-- Undo:
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 25 AND `SourceGroup` = 4737 AND `SourceEntry` = 5 AND `ElseGroup` = 1;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 25 AND `SourceGroup` = 4737 AND `SourceEntry` = 5 AND `ElseGroup` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(25, 4737, 5, 0, 1, 8, 0, 14120, 0, 0, 0, 0, 0, '', 'Kezan phase 96: 14120 rewarded (#1005, 447 in progress)'),
(25, 4737, 5, 0, 1, 9, 0, 14125, 0, 0, 0, 0, 0, '', 'Kezan phase 96: 14125 taken (#1005, Gasbot Control Panel visible)');
