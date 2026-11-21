-- Eastern Plaguelands, the Plaguewood and the finale (27525, 27526, 27527, 27528, 27529, 27530, 27539, 27551): the progress
-- texts and quest-window emotes of TDB 4.4.2 that mop lacks.
--
--  * Sourced: every row and value is TDB 4.4.2's (quest_request_items, quest_offer_reward Emote1-3); the wiki pages carry the
--    same progress wording. 27539 and 27551 already have their progress text here (unchanged). 27526's progress text
--    ("Wh... what happened?", wiki) has no TDB 4.4.2 row and is never seen: Gidwin is visible only once the quest is complete.
--  * Emotes (Emotes.dbc of 5.4.8 has each): 1 talk, 2 bow, 4 cheer, 5 exclamation, 24 shy, 66 salute, 113 salute (no sheathe),
--    397 point (no sheathe).

DELETE FROM `quest_request_items` WHERE `ID` IN (27525,27528,27529,27530);
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(27525,0,0,'We''re getting close.  I only hope it''s not too late.',0),
(27528,0,0,'Hold tightly onto your rage, $r.  Let it guide your strikes against these beasts.',0),
(27529,0,5,'The poor defenders of Darrowshire cry out for freedom.',0),
(27530,0,0,'This is my final task for you.  I wish you luck.',0);

UPDATE `quest_offer_reward` SET `Emote1` = 5, `Emote2` = 0, `Emote3` = 0 WHERE `ID` IN (27525,27551);
UPDATE `quest_offer_reward` SET `Emote1` = 5, `Emote2` = 24, `Emote3` = 0 WHERE `ID` = 27526;
UPDATE `quest_offer_reward` SET `Emote1` = 2, `Emote2` = 1, `Emote3` = 4 WHERE `ID` = 27527;
UPDATE `quest_offer_reward` SET `Emote1` = 397, `Emote2` = 0, `Emote3` = 0 WHERE `ID` = 27528;
UPDATE `quest_offer_reward` SET `Emote1` = 1, `Emote2` = 0, `Emote3` = 0 WHERE `ID` = 27529;
UPDATE `quest_offer_reward` SET `Emote1` = 113, `Emote2` = 0, `Emote3` = 0 WHERE `ID` = 27530;
UPDATE `quest_offer_reward` SET `Emote1` = 66, `Emote2` = 0, `Emote3` = 0 WHERE `ID` = 27539;
