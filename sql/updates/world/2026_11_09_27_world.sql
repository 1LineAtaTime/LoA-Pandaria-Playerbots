-- The Hinterlands: quest dialog emotes and the missing "incomplete" texts.
-- Sourced: TDB 4.4.2, copied as it is (VerifiedBuild 0: the 4.4.2 build numbers do not fit mop's columns); TDB 1210 has the same texts
-- and the same reward emotes, but fewer rows. mop had a quest_details row only for 26430 and no reward emote for any quest of the zone, and 27 quests
-- had no quest_request_items row, so their NPC said nothing while the quest was incomplete.
-- Ours: the stray backtick at the end of Ongo'longo's Revenge's incomplete text (both TDBs) is left out.
DELETE FROM `quest_details` WHERE `ID` IN (836,26210,26211,26212,26223,26224,26225,26238,26240,26263,26267,26268,26283,26306,26307,26308,26309,26310,26357,26363,26366,26367,26368,26369,26381,26382,26384,26387,26406,26418,26419,26430,26432,26462,26483,26485,26486,26490,26491,26492,26496,26497,26498,26515,26516,26517,26518,26521,26523,26524,26525,26526,26528,26529,26530,26531,26532,26546,26547,26548,26558,26641,26643,27625,27626,27725);
INSERT INTO `quest_details` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`VerifiedBuild`) VALUES
(836,0,1,0,0,0,0,0,0,0),
(26210,1,1,1,0,0,0,0,0,0),
(26211,1,1,1,5,0,0,0,0,0),
(26212,0,1,0,0,0,0,0,0,0),
(26223,1,1,5,0,0,0,0,0,0),
(26224,1,1,1,0,0,0,0,0,0),
(26225,0,1,0,0,0,0,0,0,0),
(26238,0,1,0,0,0,0,0,0,0),
(26240,0,1,0,0,0,0,0,0,0),
(26263,0,1,0,0,0,0,0,0,0),
(26267,1,1,0,0,0,0,0,0,0),
(26268,1,1,0,0,0,0,0,0,0),
(26283,1,1,0,0,0,0,0,0,0),
(26306,25,1,0,0,0,0,0,0,0),
(26307,0,1,0,0,0,0,0,0,0),
(26308,0,1,0,0,0,0,0,0,0),
(26309,1,1,1,1,0,0,0,0,0),
(26310,0,1,0,0,0,0,0,0,0),
(26357,0,1,0,0,0,0,0,0,0),
(26363,0,1,0,0,0,0,0,0,0),
(26366,0,1,0,0,0,0,0,0,0),
(26367,0,1,0,0,0,0,0,0,0),
(26368,1,1,0,0,0,0,0,0,0),
(26369,0,1,0,0,0,0,0,0,0),
(26381,1,1,0,0,0,0,0,0,0),
(26382,0,1,0,0,0,0,0,0,0),
(26384,0,1,0,0,0,0,0,0,0),
(26387,0,1,0,0,0,0,0,0,0),
(26406,1,1,0,0,0,0,0,0,0),
(26418,0,1,0,0,0,0,0,0,0),
(26419,0,1,0,0,0,0,0,0,0),
(26430,1,1,0,0,0,0,0,0,0),
(26432,0,1,0,0,0,0,0,0,0),
(26462,0,1,0,0,0,0,0,0,0),
(26483,0,1,0,0,0,0,0,0,0),
(26485,1,1,1,5,0,0,0,0,0),
(26486,0,1,0,0,0,0,0,0,0),
(26490,1,1,0,0,0,0,0,0,0),
(26491,1,1,0,0,0,0,0,0,0),
(26492,1,1,0,0,0,0,0,0,0),
(26496,1,1,0,0,0,0,0,0,0),
(26497,25,1,0,0,0,0,0,0,0),
(26498,0,1,0,0,0,0,0,0,0),
(26515,0,1,0,0,0,0,0,0,0),
(26516,0,1,0,0,0,0,0,0,0),
(26517,0,1,0,0,0,0,0,0,0),
(26518,0,1,0,0,0,0,0,0,0),
(26521,0,1,0,0,0,0,0,0,0),
(26523,0,1,0,0,0,0,0,0,0),
(26524,1,1,1,1,0,0,0,0,0),
(26525,0,1,0,0,0,0,0,0,0),
(26526,0,1,0,0,0,0,0,0,0),
(26528,1,1,0,0,0,0,0,0,0),
(26529,0,1,0,0,0,0,0,0,0),
(26530,0,1,0,0,0,0,0,0,0),
(26531,0,1,0,0,0,0,0,0,0),
(26532,0,1,0,0,0,0,0,0,0),
(26546,1,1,0,0,0,0,0,0,0),
(26547,1,1,0,0,0,0,0,0,0),
(26548,0,1,0,0,0,0,0,0,0),
(26558,0,1,0,0,0,0,0,0,0),
(26641,0,1,0,0,0,0,0,0,0),
(26643,0,1,0,0,0,0,0,0,0),
(27625,5,1,0,0,0,0,0,0,0),
(27626,1,1,0,0,0,0,0,0,0),
(27725,1,1,0,0,0,0,0,0,0);

-- Incomplete texts that had no row
DELETE FROM `quest_request_items` WHERE `ID` IN (26211,26223,26224,26238,26240,26263,26267,26306,26307,26308,26357,26367,26387,26462,26483,26491,26497,26498,26515,26516,26521,26525,26531,26546,26547,26643,27625);
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(26211,0,6,'How goes the hunt?',0),
(26223,0,5,'Show them the might of Revantusk!',0),
(26224,0,5,'Clear your head of impure thoughts, $r. Become one with the wilds!',0),
(26238,0,0,'Don''t be loitering'' around the village all day now. If you''re staying with us, I expect you to fight for us.',0),
(26240,0,0,'Back so soon? Don''t be shy, now.',0),
(26263,0,0,'I''m not as old as I look, $r... I can still hear that dwarf from here.',0),
(26267,0,1,'Don''t get caught offguard. Even if one slime doesn''t seem like much, once one gets you pinned down... the others won''t be far behind.',0),
(26306,0,0,'You''d better not get cold feet on me.',0),
(26307,0,0,'What are you waiting for?',0),
(26308,0,0,'You''ll be quite busy if you want to spill as much blood as they have.',0),
(26357,0,0,'I don''t see enough lights burnin'', $c. Better get goin''.',0),
(26367,0,0,'Ongo''longo still lies in chains.',0),
(26387,0,0,'Don''t get squeamish on me. I''d be over there ripping open cocoons myself if I didn''t have to keep all this equipment running.',0),
(26462,0,0,'Still here? I hope you''re not expectin'' a pot o'' tea an'' a footrub, ''cause some of us have work to do.',0),
(26483,0,0,'Don''t try and tell me you''re done already? I don''t like askin'' people to bring back heads as proof, it''s always a damn hassle.',0),
(26491,0,1,'Don''t get caught offguard. Even if one slime doesn''t seem like much, once one gets you pinned down... the others won''t be far behind.',0),
(26497,0,0,'Not afraid of a few trolls, are we?',0),
(26498,0,0,'Well, get on with it!',0),
(26515,0,0,'I ain''t seen a ruckus up there yet!',0),
(26516,0,0,'Keep at it, $glad:lass;!',0),
(26521,0,0,'I don''t see enough lights burnin'', $c. Better get goin''.',0),
(26525,0,0,'Don''t fear the horrid things, $n.',0),
(26531,0,0,'Are y''ready?',0),
(26546,0,0,'Don''t go tryin'' to give those to the gryphons inside here, now. They''ve already been fed plenty.',0),
(26547,0,25,'You be sportin'' with those wolves now.',0),
(26643,0,0,'Gettin'' cold feet now? Don''t let the spiders win.',0),
(27625,0,1,'Please, you must assist the Highvale!',0);
-- Emotes of the incomplete texts that had a row
UPDATE `quest_request_items` SET `EmoteOnComplete`=6,`EmoteOnIncomplete`=6 WHERE `ID`=26210;
UPDATE `quest_request_items` SET `EmoteOnComplete`=1,`EmoteOnIncomplete`=1 WHERE `ID`=26212;
UPDATE `quest_request_items` SET `EmoteOnComplete`=1,`EmoteOnIncomplete`=1 WHERE `ID`=26268;
UPDATE `quest_request_items` SET `EmoteOnComplete`=1,`EmoteOnIncomplete`=1 WHERE `ID`=26283;
UPDATE `quest_request_items` SET `EmoteOnComplete`=1,`EmoteOnIncomplete`=1 WHERE `ID`=26309;
UPDATE `quest_request_items` SET `EmoteOnComplete`=6,`EmoteOnIncomplete`=6 WHERE `ID`=26381;
UPDATE `quest_request_items` SET `EmoteOnComplete`=6,`EmoteOnIncomplete`=6 WHERE `ID`=26485;
UPDATE `quest_request_items` SET `EmoteOnComplete`=1,`EmoteOnIncomplete`=1 WHERE `ID`=26490;
UPDATE `quest_request_items` SET `EmoteOnComplete`=1,`EmoteOnIncomplete`=1 WHERE `ID`=26492;
UPDATE `quest_request_items` SET `EmoteOnComplete`=1,`EmoteOnIncomplete`=1 WHERE `ID`=26524;
UPDATE `quest_request_items` SET `EmoteOnComplete`=6,`EmoteOnIncomplete`=6 WHERE `ID`=26528;

-- Reward emotes
UPDATE `quest_offer_reward` SET `Emote1`=4,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=836;
UPDATE `quest_offer_reward` SET `Emote1`=5,`Emote2`=1,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26210;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=1,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26211;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=1,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26212;
UPDATE `quest_offer_reward` SET `Emote1`=66,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26223;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26224;
UPDATE `quest_offer_reward` SET `Emote1`=4,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26240;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26267;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26268;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26306;
UPDATE `quest_offer_reward` SET `Emote1`=11,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26307;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=1,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26309;
UPDATE `quest_offer_reward` SET `Emote1`=432,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26310;
UPDATE `quest_offer_reward` SET `Emote1`=11,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26357;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26366;
UPDATE `quest_offer_reward` SET `Emote1`=4,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26367;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26381;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26406;
UPDATE `quest_offer_reward` SET `Emote1`=432,`Emote2`=11,`Emote3`=273,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26418;
UPDATE `quest_offer_reward` SET `Emote1`=3,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26432;
UPDATE `quest_offer_reward` SET `Emote1`=4,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26462;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=1,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26485;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26491;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26492;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26497;
UPDATE `quest_offer_reward` SET `Emote1`=11,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26498;
UPDATE `quest_offer_reward` SET `Emote1`=4,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26515;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26518;
UPDATE `quest_offer_reward` SET `Emote1`=11,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26521;
UPDATE `quest_offer_reward` SET `Emote1`=432,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26523;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=1,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26524;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26525;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26528;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26529;
UPDATE `quest_offer_reward` SET `Emote1`=432,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26530;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26542;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=11,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26546;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=26547;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=27626;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=5,`Emote3`=0,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=27725;
UPDATE `quest_offer_reward` SET `Emote1`=66,`Emote2`=1,`Emote3`=5,`Emote4`=0,`EmoteDelay1`=0,`EmoteDelay2`=0,`EmoteDelay3`=0,`EmoteDelay4`=0 WHERE `ID`=28574;

-- Reward texts that lost their gender choice ("lad" for every player) or their last lines.
UPDATE `quest_offer_reward` SET `RewardText`='Looks like it''s still good inside! Huh... is that banana bread? Still fresh!$B$BHere, $glad:lass;, take your share of goods.' WHERE `ID`=26492;
UPDATE `quest_offer_reward` SET `RewardText`='There it is... only one left, $glad:lass;.' WHERE `ID`=26529;
UPDATE `quest_offer_reward` SET `RewardText`='This is it, $glad:lass;! We''re ready!' WHERE `ID`=26530;
UPDATE `quest_offer_reward` SET `RewardText`='Yoro sends warning of the Horde? The Highvale have been cautious of an attack for some time now. In fact, we have been--$B$B<Screams and the clash of metal break out to the south.>$B$BBy the light... they are here!' WHERE `ID`=27725;
