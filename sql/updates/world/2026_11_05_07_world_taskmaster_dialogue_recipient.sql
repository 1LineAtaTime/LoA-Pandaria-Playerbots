-- The taskmaster owns group1: "You dare interfere with the Master's
-- endeavors?" / orders to the Dreghood slaves. ACTION_INVOKER is the addressee,
-- not the speaker. Creature attackers otherwise request group1 on their own
-- template (observed 19477), despite the taskmaster's four authored lines.
-- Keep the target, aggro event, dialogue and timer; use the target as recipient.
UPDATE smart_scripts SET action_param3=1 WHERE entryorguid=17058 AND source_type=0 AND id=2 AND link=0 AND event_type=4 AND event_phase_mask=0 AND event_chance=100 AND event_flags=0 AND event_param1=0 AND event_param2=0 AND event_param3=0 AND event_param4=0 AND action_type=1 AND action_param1=1 AND action_param2=0 AND action_param3=0 AND action_param4=0 AND action_param5=0 AND action_param6=0 AND target_type=7 AND target_param1=0 AND target_param2=0 AND target_param3=0 AND target_x=0 AND target_y=0 AND target_z=0 AND target_o=0 AND event_param5=0 AND target_param4=0;
