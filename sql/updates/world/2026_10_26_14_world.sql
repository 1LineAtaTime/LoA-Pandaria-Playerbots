-- Farmstead Slave: cheer once when a fellow slave dies
-- Its death row set emote state 4 (Cheer) on the slaves around it, but Cheer is a one-shot emote, which
-- SMART_ACTION_SET_EMOTE_STATE refuses ("HandleEmoteStateCommand cant`t be used for oneshot emotes"). Play it instead.
UPDATE `smart_scripts` SET `action_type` = 5 WHERE `entryorguid` = 59577 AND `source_type` = 0 AND `id` = 0;
