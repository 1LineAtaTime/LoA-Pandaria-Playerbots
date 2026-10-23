-- Refs #439, #481. Follow-up of 2026_10_04_35: the Losing Your Tail trap fired twice for one player.
--
-- Evidence (t439 run on build 3/4, state/t439/cp10.log): two "Such easy prey" + two boss emotes in the same
-- update (EVT 18217/18218). The trigger's OOC_LOS event runs on every relocation; the "no Dark Scout within 30 yd"
-- condition is still true until the triggered 70795 summon lands, so a second 70794 went out before the first
-- scout existed. Fix: also require that the invoker is not already under 70794 (the stun is applied by the
-- INVOKER_CAST itself, before the summon), and raise the event cooldown to 5 s.
--
-- Rollback:
--   DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = -219500 AND `SourceGroup` = 1 AND `ConditionTypeOrReference` = 1;
--   UPDATE `smart_scripts` SET `event_param3` = 1000, `event_param4` = 1000 WHERE `entryorguid` = -219500 AND `source_type` = 0 AND `id` = 0;
-- Re-runnable: UPDATE + DELETE-then-INSERT.

UPDATE `smart_scripts` SET `event_param3` = 5000, `event_param4` = 5000 WHERE `entryorguid` = -219500 AND `source_type` = 0 AND `id` = 0;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = -219500 AND `SourceGroup` = 1 AND `ConditionTypeOrReference` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 1, -219500, 0, 0, 1, 0, 70794, 0, 0, 1, 0, 0, '', 'Losing Your Tail trap - invoker not already trapped (Freezing Trap Effect)');
