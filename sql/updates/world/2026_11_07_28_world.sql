-- Eastpoint and Durnholde quests: emotes and the 28634 request text. Sourced: TDB 4.4.2 (quest_details, quest_offer_reward, quest_request_items); TDB 1210 has the same offer and request rows.
DELETE FROM `quest_details` WHERE `ID` IN (28620,28634,28635,28636,28643,28644,28645,28646,28647);
INSERT INTO `quest_details` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`VerifiedBuild`) VALUES
(28620,1,1,1,0,0,0,0,0,0),(28634,1,1,273,5,0,0,0,0,0),(28635,1,1,6,0,0,0,0,0,0),(28636,1,1,1,273,0,0,0,0,0),(28643,1,1,0,0,0,0,0,0,0),
(28644,1,1,0,0,0,0,0,0,0),(28645,0,1,0,0,0,0,0,0,0),(28646,0,1,0,0,0,0,0,0,0),(28647,0,1,0,0,0,0,0,0,0);
UPDATE `quest_offer_reward` SET `Emote1`=6,`Emote2`=25,`Emote3`=1 WHERE `ID`=28620;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=1 WHERE `ID`=28634;
UPDATE `quest_offer_reward` SET `Emote1`=6,`Emote2`=1 WHERE `ID`=28635;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=1,`Emote3`=273 WHERE `ID`=28636;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=4 WHERE `ID`=28647;
DELETE FROM `quest_request_items` WHERE `ID`=28634;
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES (28634,0,5,'Nothing less than extinction will be accepted!',0);
