-- Hillsbrad: turn-in emotes and stage directions of the Cromush story, and the missing request text of Breaking the Hand.
-- Sourced: every text and emote (quest_offer_reward and quest_request_items in TDB 442 and 1210, which agree except for $n / $N
-- in 28616, where mop's $n is kept). Ours: nothing. VerifiedBuild is left alone: the column is a smallint, so TDB 442's 59069 on 28616 would not fit (32305 would).
UPDATE `quest_offer_reward` SET `Emote1`=6,`Emote2`=1,`Emote3`=0,`Emote4`=0,
  `RewardText`='You come from Tarren Mill? You reek of death...' WHERE `ID`=28587;
UPDATE `quest_offer_reward` SET `Emote1`=5,`Emote2`=274,`Emote3`=16,`Emote4`=15,
  `RewardText`='<High Warlord Cromush''s face turns white.>$B$BTHEY HAVE FORSAKEN US? THEY HAVE FORSAKEN THE HORDE? This cannot be!$B$BI...$B$B<Cromush falls to one knee.>$B$BI... I WILL BE FINE. We will do this without their aid. To hell with the Frostwolf!' WHERE `ID`=28600;
UPDATE `quest_offer_reward` SET `Emote1`=273,`Emote2`=15,`Emote3`=0,`Emote4`=0,
  `RewardText`='The time to act is now. The Stormpike must be crushed!' WHERE `ID`=28506;
UPDATE `quest_offer_reward` SET `Emote1`=5,`Emote2`=0,`Emote3`=0,`Emote4`=0,
  `RewardText`='Once Melisara is through with you we will detonate the explosives!' WHERE `ID`=28556;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=273,`Emote3`=1,`Emote4`=5,
  `RewardText`='With the fingers destroyed and the hand shattered, the Stormpike will have no choice but to scurry back to their cave in Alterac Valley.$B$B<Cromush spits.>$B$BWarchief Hellscream will hear about your bravery, $n.$B$BAnd rest assured that he will know of the Frostwolf clan''s treason!' WHERE `ID`=28616;

DELETE FROM `quest_request_items` WHERE `ID`=28556;
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(28556,0,6,'Have you planted the pocket-nukes?',0);
