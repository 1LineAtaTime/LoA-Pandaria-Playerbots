-- Issue #963: achievement table load errors (all rows are already ignored by the core; this removes the dead rows).
--  * achievement_reward for achievements 1681,1682,2145,3656,4784,4785 which are not in the achievement store
--    ("has wrong achievement", 6 lines) -> 6 rows; their achievement_reward_locale rows ("contains locale strings for a
--    non-existing achievement reward", 28 lines) -> 28 rows.
--  * achievement_criteria_data data type 12 (not supported by this core, "non-supported data type (12)", 17 lines) -> 17 rows.
--  * achievement_criteria_data type 11 (SCRIPT) with empty ScriptName for criteria 16081,16848,7231,7321
--    ("does not have ScriptName set", 4 lines) -> 4 rows.
-- EXPECTED AFTER: 0 lines of these five types (55 lines). No behaviour change.
DELETE FROM `achievement_reward_locale` WHERE `ID` IN (1681, 1682, 2145, 3656, 4784, 4785);
DELETE FROM `achievement_reward` WHERE `entry` IN (1681, 1682, 2145, 3656, 4784, 4785);
DELETE FROM `achievement_criteria_data` WHERE `type` = 12 AND `criteria_id` IN (6803,7315,7318,7319,7320,7361,7363,7579,7583,7587,7590,7593,7598,11789,12752,12976,12993);
DELETE FROM `achievement_criteria_data` WHERE `type` = 11 AND `ScriptName` = '' AND `criteria_id` IN (7231, 7321, 16081, 16848);
