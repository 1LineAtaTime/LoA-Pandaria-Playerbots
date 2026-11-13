-- Hillsbrad Foothills, Purgation Isle: The Road to Purgation (28375) and They Will Never Expect This... (28397) as both TDBs
-- have them (Cataclysm Classic 442 and retail 1210, sniffed 4.3.4 build 15595/32305): 28397's progress text "We will kill them
-- all!" (also warcraft.wiki.gg, Progress) with Orkus' exclamation emote was missing, and Orkus' turn-in emotes were 0. 28397's
-- reward text gets its paragraph break back ($B$B, both TDBs and the wiki).
DELETE FROM `quest_request_items` WHERE `ID` = 28397;
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `CompletionText`, `VerifiedBuild`) VALUES
(28397, 0, 5, 'We will kill them all!', 15595);

UPDATE `quest_offer_reward` SET `Emote1` = 1, `Emote2` = 273, `Emote3` = 0, `Emote4` = 0, `EmoteDelay1` = 0, `EmoteDelay2` = 0, `EmoteDelay3` = 0, `EmoteDelay4` = 0 WHERE `ID` = 28375;
UPDATE `quest_offer_reward` SET `Emote1` = 25, `Emote2` = 5, `Emote3` = 273, `Emote4` = 0, `EmoteDelay1` = 0, `EmoteDelay2` = 0, `EmoteDelay3` = 0, `EmoteDelay4` = 0,
`RewardText` = 'Look upon the carnage, $n. You and I make quite the team!$B$BNow to find those plans and deal with their leaders.' WHERE `ID` = 28397;
