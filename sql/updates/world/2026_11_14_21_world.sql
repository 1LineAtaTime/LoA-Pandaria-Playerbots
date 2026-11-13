-- Eastern Plaguelands, quest texts at Thondroril River and Light's Shield Tower, and two dead options on Betina.
--
--  * Gidwin Goldbraids (27367): the completion text asks "Tarenar?" of everyone. Sourced: both TDBs and a Wowhead comment
--    (2011-07-24 [+10]) have "$g Tarenar?: Fiona?;" (Tarenar for male, Fiona for female characters) and a paragraph break.
--  * Honor and Strength (27449) and Just a Drop in the Bucket (27454) had no progress text, so talking to Vex'tul or Betina
--    mid-quest showed an empty page. Sourced: the wiki's Progress sections; TDB 4.4.2 quest_request_items for 27454.
--  * Betina Bigglezink's menu 3651 offered "a replacement Seal / Rune of the Dawn" to everyone with nothing behind the
--    options (no SmartAI, no script). The quests that gave the Seal and the Rune are retired; retail showed the lines only to
--    characters who had done them (NPC comments 2012). Ours: the two options go (lights-shield dossier finding 6).

UPDATE `quest_offer_reward` SET `RewardText` = '$g Tarenar?: Fiona?;  Is that you?$b$bWhoever you are, would you mind helping me out of this thing?' WHERE `ID` = 27367;

DELETE FROM `quest_request_items` WHERE `ID` IN (27449, 27454);
INSERT INTO `quest_request_items` (`ID`, `EmoteOnComplete`, `EmoteOnIncomplete`, `CompletionText`, `VerifiedBuild`) VALUES
(27449, 0, 0, 'Honor means nothing.  Strength is everything.', 0),
(27454, 0, 0, 'The Cult of the Damned still brews plague nearby!  Destroy them!', 0);

DELETE FROM `gossip_menu_option` WHERE `MenuID` = 3651 AND `OptionID` IN (0, 1);
