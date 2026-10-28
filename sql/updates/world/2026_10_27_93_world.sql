-- Wandering Isle Training Targets (quest 29406 "The Lesson of the Sandy Fist"): use npc_training_target like the other
-- Training Targets (57873). With no AI they fought back; the script keeps them passive and drops combat 5 seconds after
-- the last hit.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_training_target' WHERE `entry` = 53714;
