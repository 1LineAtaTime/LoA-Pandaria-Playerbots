-- Galen's Fall story quests: the emotes and $B breaks mop_world lost. Sourced: TDB 442 (quest_details, quest_offer_reward,
-- quest_request_items), VerifiedBuild left at 0 on the new rows. The "pehaps" in 26081 is in TDB and mop alike.
-- Ours: 28572 has no TDB offer row; it gets the emotes of 28619, which has the same text.
DELETE FROM `quest_details` WHERE `ID` IN (26022,26023,26025,26030,26031,26032,26033,26081,26084);
INSERT INTO `quest_details` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`VerifiedBuild`) VALUES
(26022,0,1,0,0,0,0,0,0,0),
(26023,1,1,0,0,0,0,0,0,0),
(26025,0,1,0,0,0,0,0,0,0),
(26030,1,1,0,0,0,0,0,0,0),
(26031,1,1,0,0,0,0,0,0,0),
(26032,1,1,0,0,0,0,0,0,0),
(26033,1,1,0,0,0,0,0,0,0),
(26081,1,1,0,0,0,0,0,0,0),
(26084,0,1,0,0,0,0,0,0,0);
UPDATE `quest_offer_reward` SET `Emote1`=0,`Emote2`=0,`Emote3`=0,`Emote4`=0,`RewardText`='This is but a prelude to the slaughter we will visit upon these lands. All will know our name.$B$BYou have done well, $n. Victory for Sylvanas!' WHERE `ID`=26022;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`RewardText`='Yes... I was once Prince of the fallen kingdom of Stromgarde. I was cut down by the one seeking Trol''kalar.$B$BI rue my foolish life now. I serve...only Sylvanas.' WHERE `ID`=26023;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`RewardText`='My restless nights will turn to peaceful slumber when I have destroyed this orb. Thank you, $n. It is a chilling reminder of the terrible power the demons held over the orcs before Hellscream released them from their curse.' WHERE `ID`=26025;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`RewardText`='I hated the Syndicate in life as much as I do in death. To see the sigil again is a small comfort.$B$BBut we have much more work to do... for Sylvanas.' WHERE `ID`=26030;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=432,`Emote3`=432,`Emote4`=0,`RewardText`='Thoradin was the founder of Arathor. Just as I join these fragments together, he joined the scattered tribes of humans into their first great nation.$B$BBut I''m sure you''re not interested in such drivel. Forgive me. Old memories are often hard to bury.' WHERE `ID`=26031;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`RewardText`='Valorcall was loyal to me in life. His death will not be something I bear lightly.$B$BBut bear it I must. I am Forsaken. My old existence means nothing.$B$BThe sigils are ours; Trol''kalar awaits.' WHERE `ID`=26032;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`RewardText`='Trol''kalar, my father''s blade... it seems like it''s been an eternity since I saw it.' WHERE `ID`=26033;
UPDATE `quest_offer_reward` SET `Emote1`=113,`Emote2`=0,`Emote3`=0,`Emote4`=0,`RewardText`='The news of your victory satisfies me, $c... a rare sensation. The Dark Lady herself has mused that our conquest of Lordaeron may be the only way the Forsaken can find true satisfaction, and pehaps even relief.$B$BBut it matters not. Our victory is inevitable. For your loyal service to the Forsaken, I award you in the name of Sylvanas.' WHERE `ID`=26081;
UPDATE `quest_offer_reward` SET `Emote1`=1,`Emote2`=0,`Emote3`=0,`Emote4`=0,`RewardText`='Throm''ka, $c. If your deeds for the undead are as great as you claim, I am honored to help you serve the Horde.' WHERE `ID`=26084;
UPDATE `quest_request_items` SET `EmoteOnComplete`=1,`EmoteOnIncomplete`=1 WHERE `ID`=26031;
UPDATE `quest_request_items` SET `EmoteOnComplete`=1,`EmoteOnIncomplete`=1 WHERE `ID`=26033;
UPDATE `quest_offer_reward` SET `Emote1`=66,`Emote2`=1,`Emote3`=5,`Emote4`=0 WHERE `ID`=28572;
