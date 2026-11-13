-- #987 wave 1: rerun of the #963 talk-target rule over the imported rows.
-- Mechanism (SmartScript.cpp:121-145): SMART_ACTION_TALK aimed at a creature invoker/victim makes that creature the speaker unless
-- action_param3 (useTalkTarget) = 1. Same idempotent statement as 2026_10_28_963f_talk_callers.sql (whole table, so it also covers
-- the imported rows), plus the 963b (1) rule for the OOC-LoS say-to-invoker rows that wave 1 added (playerOnly via event_param5 = 1).
-- Undo: set action_param3 / event_param5 back to 0 on the same rows.

UPDATE `smart_scripts` SET `action_param3`=1
WHERE `source_type`=0 AND `action_type`=1 AND `target_type` IN (2,7) AND `action_param3`=0
  AND `event_type` NOT IN (19,20,62,38,40,52,54);

UPDATE `smart_scripts` SET `event_param5`=1, `action_param3`=1
WHERE `source_type`=0 AND `event_type`=10 AND `action_type`=1 AND `target_type`=7 AND (`entryorguid`,`id`) IN (
 (23257,0),(25242,0),(25243,0));
