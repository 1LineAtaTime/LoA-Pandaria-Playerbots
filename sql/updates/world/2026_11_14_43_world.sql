-- Eastern Plaguelands, Eastwall, Northpass and Terrordale quests: progress texts mop lacks, turn-in emotes, a gendered
-- completion text.
-- Sourced: both TDBs unless noted. 27482, 27522: TDB 4.4.2 only. 27487's progress text: the wiki only (neither TDB has a
-- row). 27534's emotes: TDB 4.4.2. 27535: TDB 4.4.2 "$g mister: lady;" (mop says "mister" to everyone).
DELETE FROM `quest_request_items` WHERE `ID` IN (27477,27482,27487,27521,27522,27533);
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(27477,0,6,'Have I told you lately that I hate you?',0),
(27482,0,0,'Throw the body into the flames, $N.  You can do it.  You must do it.',0),
(27487,0,0,'I hope Gidwin doesn''t mind waiting.',0),
(27521,0,0,'You have no idea how difficult it was for me to pull myself away, $r.  I nearly ended up one of them.',0),
(27522,0,0,'I wish I could join you, but I''m not much for fighting.  I''ve protected Gidwin as much as I could... he needs you to protect him now, $N.',0),
(27533,0,18,'I cry for the poor pigs.',0);
UPDATE `quest_request_items` SET `EmoteOnComplete`=1,`EmoteOnIncomplete`=1 WHERE `ID`=27523;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=1 WHERE `ID` IN (27477,27523);
UPDATE `quest_offer_reward` SET `Emote1`=1 WHERE `ID` IN (27479,27481,27522,27531);
UPDATE `quest_offer_reward` SET `Emote1`=22,`Emote2`=5,`Emote3`=18 WHERE `ID`=27487;
UPDATE `quest_offer_reward` SET `Emote1`=6,`Emote2`=20 WHERE `ID`=27488;
UPDATE `quest_offer_reward` SET `Emote1`=396,`Emote2`=396 WHERE `ID`=27532;
UPDATE `quest_offer_reward` SET `Emote1`=70 WHERE `ID`=27533;
UPDATE `quest_offer_reward` SET `Emote1`=4,`Emote2`=2 WHERE `ID`=27534;
UPDATE `quest_offer_reward` SET `RewardText`='You''ve come to the right place, $g mister: lady;!  No place better than Terrordale, yes siree, that''s right!  The jewel of Lordaeron!' WHERE `ID`=27535;
