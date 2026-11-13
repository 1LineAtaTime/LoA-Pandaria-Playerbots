-- The Hinterlands: the order of the Jintha'Alor, Venomous Secrets and Quel'Danil Lodge quests.
-- Sourced:
--  * In Defense of Quel'Danil 27625 and The Highvale Documents 27626 after Quel'Danil Lodge 27725: both TDBs (4.4.2 and 1210).
--  * Venomous Secrets: both TDBs give the Horde 26368 PrevQuestID 26524 (the Alliance Dark Vessels, which no Horde character can do) and
--    the Alliance Dark Vessels 26524 NextQuestID 26368. warcraft.wiki.gg gives both Venomous Secrets "Previous: Summit of Fate", and a
--    Wowhead MoP Classic comment on the Alliance Dark Vessels: "the quest chain should point to Venomous Secrets - the alliance version.
--    It is not available on completing this quest but becomes available after Summit of Fate". So each Venomous Secrets follows its own
--    faction's Summit of Fate (26368 after 26363, 26525 after 26517), and each Dark Vessels points at its own Venomous Secrets (26309 and
--    26524), as the TDB meant; in this core a NextQuestID link is one of several dependents, so Summit of Fate is what gates it.
--  * Death to the Vilebranch 26498 after Vilebranch Scum 26497, Summit of Fate 26517 after Ongo'longo's Revenge 26515, Hunt the Keeper
--    26526 after Summit of Fate: Wowhead's series, warcraft.wiki.gg, and the Horde twins' rows in both TDBs (26307, 26363, 26369).
--  * Quel'Danil Lodge 27725 once the Jintha'Alor camp's quests are done: warcraft.wiki.gg ("Requires quests from Fraggar and Doran") and
--    a Wowhead MoP Classic comment ("you have to complete a series of quests for Fraggar Thundermantle and Doran Steelwing, ending with
--    Hunt the Keeper"). Each line's last quest, It's Ours Now 26516 (Doran), Venomous Secrets 26525 and Hunt the Keeper 26526 (Fraggar),
--    points at 27725 with ExclusiveGroup -26516, which in this core means all three must be rewarded.
--  * Gilda Cloudcaller's The Eye of Shadra 26528 and Can't Make An Omelette Without... 26641 once both of Traska's quests are done: Wowhead
--    MoP Classic comments ("she wouldn't give me any quests", "you pick up two quests there, get phased, do those quests, and then you're
--    able to pick the starter"). Retail gated Gilda with a phase mop does not have; both TDBs leave them at Prev 0.
-- Ours: the three end quests chosen as the 27725 gate; for Gilda, 27625 and 27626 get ExclusiveGroup -27625 and her two quests PrevQuestID
-- 27625, which in this core needs the whole group rewarded. Retail hid Tracker Yoro behind quest invisibility (template aura 92292 in both TDBs)
-- whose detection no TDB row grants; the prerequisite stands in for it.
UPDATE `quest_template_addon` SET `PrevQuestID`=26363 WHERE `ID`=26368;
UPDATE `quest_template_addon` SET `NextQuestID`=26368 WHERE `ID`=26309;
UPDATE `quest_template_addon` SET `NextQuestID`=26525 WHERE `ID`=26524;
UPDATE `quest_template_addon` SET `PrevQuestID`=26497 WHERE `ID`=26498;
DELETE FROM `quest_template_addon` WHERE `ID` IN (26517,26525,26526,27725,27625,27626);
INSERT INTO `quest_template_addon` (`ID`,`PrevQuestID`) VALUES
(26517,26515),
(26525,26517),
(26526,26517),
(27625,27725),
(27626,27725);
UPDATE `quest_template_addon` SET `NextQuestID`=27725,`ExclusiveGroup`=-26516 WHERE `ID` IN (26516,26525,26526);
UPDATE `quest_template_addon` SET `ExclusiveGroup`=-27625 WHERE `ID` IN (27625,27626);
UPDATE `quest_template_addon` SET `PrevQuestID`=27625 WHERE `ID` IN (26528,26641);
