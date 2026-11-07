-- Quest emotes and texts of the Galen's Fall hub quests and the Faldir's Cove chain. Sourced: TDB 442 rows, taken as they are (the $B breaks mop lost, the emotes, the missing incomplete texts).
-- Nothing here is ours. 26083 and 26628 have no request text in the TDB, and none is added.
DELETE FROM `quest_details` WHERE `ID` IN (26027,26029,26050,26051,26052,26053,26083,26428,26628);
INSERT INTO `quest_details` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`VerifiedBuild`) VALUES
(26027,1,1,0,0,0,0,0,0,0),
(26029,0,1,0,0,0,0,0,0,0),
(26050,1,1,0,0,0,0,0,0,0),
(26051,1,1,0,0,0,0,0,0,0),
(26052,1,1,0,0,0,0,0,0,0),
(26053,15,1,1,0,0,0,0,0,0),
(26083,0,1,0,0,0,0,0,0,0),
(26428,15,1,1,0,0,0,0,0,0),
(26628,5,1,5,0,0,0,0,0,0);

DELETE FROM `quest_request_items` WHERE `ID` IN (26027,26029,26050,26051,26052,26053,26083,26428,26628);
INSERT INTO `quest_request_items` (`ID`,`EmoteOnComplete`,`EmoteOnIncomplete`,`CompletionText`,`VerifiedBuild`) VALUES
(26027,0,1,'The ogres lurk in the corner of the ruins, away from the humans. Don''t forget I''m only rewarding you for slaughtering ogres this time, $c. You can do what you want for fun, but don''t expect extra credit.',0),
(26029,0,0,'Speak quickly.',0),
(26050,0,5,'This is no time to talk! The goggles need to be charged with the power of the stone!',0),
(26051,6,6,'How did the goggles work? Did everything function properly? Adjustments can be made if necessary.$b$bOh, and I''ll need those goggles back when you''re done. I am sure Captain O''Breen will have many uses for them later on.',0),
(26052,6,0,'Ahoy, $c.  What brings you to the bridge?',0),
(26053,0,1,'You''ll learn not to bother a Deathstalker unless you have news of success in time. Until then, try to keep your mouth shut in general.',0),
(26428,5,5,'Kill them all and bring me their heads!',0);

DELETE FROM `quest_offer_reward` WHERE `ID` IN (26027,26029,26050,26051,26052,26053,26083,26428,26628);
INSERT INTO `quest_offer_reward` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`RewardText`,`VerifiedBuild`) VALUES
(26027,66,1,1,0,0,0,0,0,'Cleared out properly? Good. I doubt we''ll even bother to lurk in Stromgarde. That ramshackle hole needs to be razed into dust.$B$BThe land would make a decent spot for a new Undercity in its place.',0),
(26029,2,1,0,0,0,0,0,0,'This is the fate of all who would stand in our way.',0),
(26050,0,0,0,0,0,0,0,0,'The goggles work perfectly now! Well done, $n.',0),
(26051,0,0,0,0,0,0,0,0,'Most superb, $n! The Captain will be very pleased! And just wait until word gets back to Fleetmaster Seahorn!',0),
(26052,11,0,0,0,0,0,0,0,'So the little gnomes proved their worth after all? I knew we kept those ankle biters around for a reason!',0),
(26053,66,0,0,0,0,0,0,0,'Excellent. The march of the Forsaken continues.',0),
(26083,0,0,0,0,0,0,0,0,'You friends with Goutgut! Him weird ogre. Smell funny.',0),
(26428,1,2,1,0,0,0,0,0,'You stand before me as a figure of vengeance, and I bow down before you in thanks.$B$BWith that cow''s death, and knowing that their seed will no longer spread over the face of Azeroth, I can put my energy into something more lucrative. I hope you looted their farmstead after you finished them off.',0),
(26628,4,0,0,0,0,0,0,0,'We survived the attack!$b$bWe could not have done it without you, $n.$b$bWe''ll be heading out as soon as the tide rises. If you weren''t such a promising hero I''d offer you a place on my crew. But I can tell you have bigger fish to fry.$b$bEven if you are a puny $r....',0);
