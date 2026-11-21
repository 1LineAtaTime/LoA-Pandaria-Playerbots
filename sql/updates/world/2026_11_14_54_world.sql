-- Eastern Plaguelands, Light's Hope Chapel quests: texts and emotes as both TDBs (VerifiedBuild 32722, the same in 4.4.2 and
-- 12.1.0). Sourced throughout; nothing ours.
--  * Progress texts that mop lacked: Argent Call: The Trial of the Crypt 27464 "Your test is not yet complete.", Argent Call:
--    The Noxious Glade 27465 "The three of you still have a job to do.", Buried Blades 27467 "Are you still here?".
--  * Gender switches: 27464 called every player "brother" ($g brother: sister;), Smokey and the Bandage 27458 every player
--    "chief" ($g chief: sweetie;; comment 2011-02-17).
--  * Paragraph breaks ($b$b) that the completion texts of 27457, 27464, 27465, 27466 and 27467 had lost.
--  * Completion emotes: 27456 bow (2), 27458 talk (1), 27464 salute (66), 27466 wave then question (3, 6), 27467 talk (1).
DELETE FROM `quest_request_items` WHERE `ID` IN (27464,27465,27467);
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(27464,0,0,'Your test is not yet complete.',0),
(27465,0,0,'The three of you still have a job to do.',0),
(27467,0,0,'Are you still here?',0);

UPDATE `quest_offer_reward` SET `Emote1`=2, `Emote2`=0, `RewardText`='May the Earthmother watch over you! Bless you, young one.' WHERE `ID`=27456;
UPDATE `quest_offer_reward` SET `Emote1`=0, `Emote2`=0, `RewardText`='You brought us a shaman!  From the Earthen Ring, no less!$b$bI have no idea how we''ll fit him in the caravan, but we''ll find a way.  Welcome aboard, Rimblat Earthshatter!' WHERE `ID`=27457;
UPDATE `quest_offer_reward` SET `Emote1`=1, `Emote2`=0, `RewardText`='Well well well!  Someone who goes and gets things done!  Here''s your cut, $g chief: sweetie;.  I''ll be seein'' you around.' WHERE `ID`=27458;
UPDATE `quest_offer_reward` SET `Emote1`=66, `Emote2`=0, `RewardText`='Remember that your ancestors always watch over you.  As a $g brother: sister; of the Argent Crusade, you will join a family that has a long line of such fallen ancestors.  Their vigil and protection will be your strength as you press forward against the Scourge.$b$bCongratulations Tarenar, Gidwin, and $n.  Welcome to the Argent Crusade.' WHERE `ID`=27464;
UPDATE `quest_offer_reward` SET `Emote1`=0, `Emote2`=0, `RewardText`='You''ve returned sooner than I expected.  Excellent.$b$bWe will continue our path into the north.  Next, you will be completing a similar task - this time, in Northdale.' WHERE `ID`=27465;
UPDATE `quest_offer_reward` SET `Emote1`=3, `Emote2`=6, `RewardText`='Hello, $n!  Glad to see you''ve rejoined the caravan!$b$bTarenar and Gidwin arrived here before you.  Tarenar''s just right there, and Gidwin''s... well... he''s in a little bit of a mood right now.  I think he''s in the tower.' WHERE `ID`=27466;
UPDATE `quest_offer_reward` SET `Emote1`=1, `Emote2`=0, `RewardText`='Our duty for the day is done.$b$bIt is only a matter of time before we defeat the Scourge.  Today, we made progress.' WHERE `ID`=27467;
