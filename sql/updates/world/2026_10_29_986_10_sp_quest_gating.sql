-- ported from PR 15 2026_10_25_47_world.sql (#986)
-- Silverpine Forest: Lady Sylvanas Windrunner and the other quest givers stand at their posts from the start (Cataclysm summons
-- or phases them in), so every quest without a previous quest was offered straight away. Previous quests from the
-- warcraft.wiki.gg Silverpine Forest storyline and quest pages, quest givers from Wowhead (MoP Classic) and TCPP.
INSERT IGNORE INTO `quest_template_addon` (`ID`) VALUES (26989), (26992), (27073), (27195);

-- Forsaken High Command: Agony Abounds, Guts and Gore and The Gilneas Liberation Front come after The Warchief Cometh
UPDATE `quest_template_addon` SET `PrevQuestID` = 26965 WHERE `ID` IN (26989, 26992, 26995);
-- Dangerous Intentions comes after Iterating Upon Success
UPDATE `quest_template_addon` SET `PrevQuestID` = 26998 WHERE `ID` = 27039;

-- Forsaken Rear Guard: Give 'em Hell! and Playing Dirty come after The Warchief's Fleet
UPDATE `quest_template_addon` SET `PrevQuestID` = 27065 WHERE `ID` IN (27073, 27082);
-- Skitterweb Menace and Lost in the Darkness need both Steel Thunder and Give 'em Hell!
UPDATE `quest_template_addon` SET `PrevQuestID` = 27069 WHERE `ID` IN (27093, 27095);

-- Rise, Forsaken comes after Orcs are in Order
UPDATE `quest_template_addon` SET `PrevQuestID` = 27096 WHERE `ID` = 27097;
-- Honor the Dead, Hair of the Dog and Reinforcements from Fenris come after Lordaeron
UPDATE `quest_template_addon` SET `PrevQuestID` = 27098 WHERE `ID` IN (27180, 27226, 27231);
-- Nowhere to Run comes after Cornered and Crushed!
UPDATE `quest_template_addon` SET `PrevQuestID` = 27194 WHERE `ID` = 27195;
-- To Forsaken Forward Command needs Honor the Dead, Hair of the Dog, The Waters Run Red... and Nowhere to Run
UPDATE `quest_template_addon` SET `PrevQuestID` = 27180 WHERE `ID` = 27290;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` IN (27093, 27095, 27290);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 27093, 0, 0, 8, 0, 27073, 0, 0, 0, 0, 0, '', 'Lost in the Darkness requires Give ''em Hell!'),
(19, 0, 27095, 0, 0, 8, 0, 27073, 0, 0, 0, 0, 0, '', 'Skitterweb Menace requires Give ''em Hell!'),
(19, 0, 27290, 0, 0, 8, 0, 27226, 0, 0, 0, 0, 0, '', 'To Forsaken Forward Command requires Hair of the Dog'),
(19, 0, 27290, 0, 0, 8, 0, 27232, 0, 0, 0, 0, 0, '', 'To Forsaken Forward Command requires The Waters Run Red...'),
(19, 0, 27290, 0, 0, 8, 0, 27195, 0, 0, 0, 0, 0, '', 'To Forsaken Forward Command requires Nowhere to Run');

-- Lady Sylvanas Windrunner does not give Orcs are in Order or Nowhere to Run, and Admiral Hatchet does not give The Warchief's Fleet
DELETE FROM `creature_queststarter` WHERE (`id` = 44365 AND `quest` IN (27096, 27195)) OR (`id` = 44916 AND `quest` = 27065);
