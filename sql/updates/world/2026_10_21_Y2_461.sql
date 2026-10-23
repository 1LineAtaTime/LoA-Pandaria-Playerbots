-- Refs #461 (lane Y2, 2026-10-21): Lost Isles smaller mechanics, proved in a test run.
--
-- 14001/14474 Goblin Escape Pods: the first pod must free Trade Prince Gallywix 35649, later pods survivors 34748 (Wowpedia 14001,
--   AshamaneCore lost_isles.cpp gob_goblin_escape_pod). 2026_10_04_15 keyed the Gallywix branch on the player NOT having the Trade
--   Prince Controller Aura 67433 and then cast it. But spell_linked_spell 68258 -> 67433 (type 2, present in world and in
--   the MoP 5.4.8 data) applies 67433 together with the Thermohydratic Flippers 68258 that spell_area autocasts at the quest accept,
--   so the player always has it and Gallywix never came (t461 P01, build 87: aura+ 67433 by Gobchain right after the accept,
--   gallywixCast=0, survivorCast=7). Read the aura the way the data gives it: "Gallywix is still in a pod". Gallywix branch when the
--   invoker HAS 67433 (the first pod) and it removes the aura; survivor branch when it is gone.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 195188 AND `SourceId` = 1 AND `SourceGroup` IN (2, 3);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 2, 195188, 1, 0, 1, 0, 67433, 0, 0, 1, 0, 0, '', 'Goblin Escape Pod - survivor branch only once the Trade Prince Controller Aura is gone (Gallywix already freed)'),
(22, 3, 195188, 1, 0, 1, 0, 67433, 0, 0, 0, 0, 0, '', 'Goblin Escape Pod - Gallywix branch only while the invoker has the Trade Prince Controller Aura (68258 -> 67433 at the accept)');
UPDATE `smart_scripts` SET `action_type` = 28, `action_param1` = 67433, `action_param2` = 0,
       `comment` = 'Goblin Escape Pod - Linked - Remove Trade Prince Controller Aura from the invoker (Gallywix freed)'
 WHERE `entryorguid` = 195188 AND `source_type` = 1 AND `id` = 3 AND `action_type` = 85 AND `action_param1` = 67433;
-- Undo (restores 2026_10_04_15):
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 195188 AND `SourceId` = 1 AND `SourceGroup` IN (2, 3);
--   INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
--   (22, 2, 195188, 1, 0, 1, 0, 67433, 0, 0, 0, 0, 0, '', 'Goblin Escape Pod - survivor branch only if the invoker has Trade Prince Controller Aura'),
--   (22, 3, 195188, 1, 0, 1, 0, 67433, 0, 0, 1, 0, 0, '', 'Goblin Escape Pod - Gallywix branch only if the invoker does NOT have Trade Prince Controller Aura');
--   UPDATE `smart_scripts` SET `action_type` = 85, `action_param1` = 67433, `action_param2` = 2, `comment` = 'Goblin Escape Pod - Linked - Invoker Cast Trade Prince Controller Aura'
--    WHERE `entryorguid` = 195188 AND `source_type` = 1 AND `id` = 3;
