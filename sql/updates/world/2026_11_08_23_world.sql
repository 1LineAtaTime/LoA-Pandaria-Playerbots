-- Hammerfall quests: missing quest emotes, Crush the Witherbark's incomplete text, and the $B breaks of Raising Spirits and Guile of the Raptor.
-- Sourced: TDB 442 quest_details, quest_request_items and quest_offer_reward rows for 26107, 26108, 26429, 26430 and 6622, copied as they are
-- (VerifiedBuild 0: the TDB's 4.4.2 build numbers do not fit mop's column). Also drops the trailing spaces mop had on these texts. Nothing here is ours.
DELETE FROM `quest_details` WHERE `ID` IN (26107,26108,26429,26430,6622);
INSERT INTO `quest_details` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`VerifiedBuild`) VALUES
(26107,0,1,0,0,0,0,0,0,0),
(26108,0,1,0,0,0,0,0,0,0),
(26429,1,1,1,0,0,0,0,0,0),
(26430,1,1,0,0,0,0,0,0,0),
(6622,0,1,0,0,0,0,0,0,0);
DELETE FROM `quest_request_items` WHERE `ID` IN (26108,26429);
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(26108,0,0,'My preparations are made, all I need are the raptor hearts.',0),
(26429,0,1,'Haven''t completed my task yet? Are you scared, coward?',0);
DELETE FROM `quest_offer_reward` WHERE `ID` IN (26107,26108,26429,6622);
INSERT INTO `quest_offer_reward` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`RewardText`,`VerifiedBuild`) VALUES
(26107,1,432,432,0,0,0,0,0,'The lethargy that overcame me even after the camps fell was so strong, I refused this gift when it was first given to me.$B$BNow, I am proud to do my part in the name of the Horde.',0),
(26108,1,0,0,0,0,0,0,0,'As Tor''gan taught me, I transfer the power and the spirit of the raptors into this orb. It will imbue our warriors with the fiery strength and intelligence of the raptor.$B$BThe guile of the raptor is yours now as well, $c. Take it with pride, for the Horde!',0),
(26429,1,11,0,0,0,0,0,0,'The signs of war are upon your garments, and sweat drips from your brow. Good, very good indeed.',0),
(6622,1,2,0,0,0,0,0,0,'Doctor $N, welcome to Horde Trauma.',0);
