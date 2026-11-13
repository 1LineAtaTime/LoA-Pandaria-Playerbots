-- #988 wave 2: rerun of the #963 talk-target rule over the whole table (covers the rows added by the 988 abilities files).
-- Mechanism (SmartScript.cpp:121-145): SMART_ACTION_TALK aimed at a creature invoker/victim makes that creature the speaker unless action_param3 (useTalkTarget) = 1.
-- Same idempotent statement as 2026_10_28_963f_talk_callers.sql / 2026_10_29_987_abilities_999_talk_target_rerun.sql. Undo: set action_param3 back to 0 on the affected rows.

UPDATE `smart_scripts` SET `action_param3`=1
WHERE `source_type`=0 AND `action_type`=1 AND `target_type` IN (2,7) AND `action_param3`=0
  AND `event_type` NOT IN (19,20,62,38,40,52,54);
