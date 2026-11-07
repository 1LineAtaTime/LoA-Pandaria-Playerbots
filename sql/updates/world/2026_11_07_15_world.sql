-- Quest emotes and texts of the Tarren Mill side quests and Warchief's Command: Arathi Highlands! 28619.
-- Sourced: TDB 4.4.2 (quest_details; TDB 1210 has no rows for these), and both TDBs for quest_offer_reward and quest_request_items.
-- Ours: 28489's reward text without the trailing newline mop had and without the TDBs' typo (".n" at the end).
DELETE FROM `quest_details` WHERE `ID` IN (28484,28485,28487,28489,28495);
INSERT INTO `quest_details` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`VerifiedBuild`) VALUES
(28484,274,1,1,0,0,0,0,0,0),
(28485,1,1,5,1,0,0,0,0,0),
(28487,24,1,6,273,0,0,0,0,0),
(28489,6,1,1,273,0,0,0,0,0),
(28495,5,1,1,0,0,0,0,0,0);
UPDATE `quest_offer_reward` SET `Emote1`=24,`Emote2`=274,`Emote3`=6,`Emote4`=0 WHERE `ID`=28484;
UPDATE `quest_offer_reward` SET `Emote1`=273,`Emote2`=4,`Emote3`=1,`Emote4`=0 WHERE `ID`=28485;
UPDATE `quest_offer_reward` SET `Emote1`=66,`Emote2`=1,`Emote3`=0,`Emote4`=0 WHERE `ID`=28487;
UPDATE `quest_offer_reward` SET `Emote1`=4,`Emote2`=1,`Emote3`=25,`Emote4`=0,`RewardText`='That''s a hefty load you''re carrying. It should keep my customers satisfied for the time being.$B$BHere is your pay, as promised.' WHERE `ID`=28489;
UPDATE `quest_offer_reward` SET `Emote1`=5,`Emote2`=20,`Emote3`=274,`Emote4`=1 WHERE `ID`=28495;
UPDATE `quest_offer_reward` SET `Emote1`=66,`Emote2`=1,`Emote3`=5,`Emote4`=0 WHERE `ID`=28619;
UPDATE `quest_request_items` SET `EmoteOnComplete`=25,`EmoteOnIncomplete`=25 WHERE `ID`=28484;
UPDATE `quest_request_items` SET `EmoteOnComplete`=5,`EmoteOnIncomplete`=5 WHERE `ID` IN (28485,28489);
UPDATE `quest_request_items` SET `EmoteOnComplete`=6,`EmoteOnIncomplete`=6 WHERE `ID`=28487;
DELETE FROM `quest_request_items` WHERE `ID`=28495;
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(28495,0,6,'So... did it work?',0);
