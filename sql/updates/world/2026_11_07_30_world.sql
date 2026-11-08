-- Melisara and Lilith's field quests (Cry of the Banshee 28538, Decimation 28566, Infiltration 28603, Deception and Trickery 28604,
-- Domination 28605): the stage directions and $B breaks mop_world lost, and the missing emotes and request texts.
-- Sourced: everything here, TDB 442 (quest_offer_reward, quest_request_items, quest_details). VerifiedBuild left at 0.
UPDATE `quest_offer_reward` SET `RewardText`='<Melisara''s voice pervades your thoughts.>$B$BThe humans will serve us in life and in death. I will be in contact with you shortly.' WHERE `ID`=28538;
UPDATE `quest_offer_reward` SET `RewardText`='<Melisara''s voice pervades your thoughts.>$B$BThey will see the bloodshed and they will hesitate. Hesitation will make them think twice about ever attacking us again.' WHERE `ID`=28566;
UPDATE `quest_offer_reward` SET `RewardText`='<Melisara''s voice pervades your thoughts.>$B$BPerfectly handled. You could easily be a Deathstalker. And what could they have even done to you? You acted in self defense!$B$BNow to deal with the final finger.' WHERE `ID`=28603;
UPDATE `quest_offer_reward` SET `Emote1`=1 WHERE `ID`=28605;

DELETE FROM `quest_request_items` WHERE `ID` IN (28538,28604);
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(28538,0,0,'<Melisara''s thoughts intermingle with your thoughts.>',0),
(28604,0,0,'Deception and trickery - these are your tools!',0);

DELETE FROM `quest_details` WHERE `ID` IN (28538,28566,28603,28604,28605);
INSERT INTO `quest_details` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`VerifiedBuild`) VALUES
(28538,1,1,1,0,0,0,0,0,0),
(28566,0,1,0,0,0,0,0,0,0),
(28603,0,1,0,0,0,0,0,0,0),
(28604,1,1,0,0,0,0,0,0,0),
(28605,0,1,0,0,0,0,0,0,0);
