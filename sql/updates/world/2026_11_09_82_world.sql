-- Darkcleric Marnal 26384 (Eliza Darkgrin -> Darkcleric Marnal) after Hunt the Keeper 26369.
-- 26384 hands the player the Tablet of Shadra 58241 on accept (StartItem), the very item Hunt the Keeper asks for, and its
-- text ("We can't toss that tablet aside just yet") follows that quest. With no prerequisite, Eliza offered it at once, and
-- taking it completed Hunt the Keeper without the kill (seen in game 2026-11-09). Neither TDB has a link; the gate is ours,
-- from the text and the wiki's zone progression.
UPDATE `quest_template_addon` SET `PrevQuestID`=26369 WHERE `ID`=26384;
INSERT IGNORE INTO `quest_template_addon` (`ID`,`PrevQuestID`) VALUES (26384,26369);
