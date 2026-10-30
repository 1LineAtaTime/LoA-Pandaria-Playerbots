-- ported from PR 15 2026_10_27_40_world.sql (#986)
-- Silverpine Forest after the Sepulcher: restore the quest chain from To Forsaken Forward Command to Cities in Dust
-- Prerequisites as in the retail and Cataclysm Classic world databases (identical there) and warcraft.wiki.gg: a negative
-- ExclusiveGroup with a shared NextQuestID means every quest of the group is needed for the next one.
-- On Whose Orders? (needs Rutsak's Guard) and A Wolf in Bear's Clothing (needs Only One May Enter) only from the wiki.

INSERT IGNORE INTO `quest_template_addon` (`ID`) VALUES
(27423),(27472),(27475),(27483),(27510),(27542),(27547);

-- Ruins of Gilneas: In Time, All Will Be Revealed, Losing Ground and The F.C.D. all need To Forsaken Forward Command,
-- and all three are needed for Break in Communications: Dreadwatch Outpost
UPDATE `quest_template_addon` SET `PrevQuestID`=27290, `NextQuestID`=27349, `ExclusiveGroup`=-27333 WHERE `ID` IN (27333,27342,27345);
UPDATE `quest_template_addon` SET `PrevQuestID`=0 WHERE `ID`=27349;
UPDATE `quest_template_addon` SET `PrevQuestID`=27350 WHERE `ID`=27364;
-- A Man Named Godfrey and Resistance is Futile both need Fall Back! and are both needed for The Great Escape
UPDATE `quest_template_addon` SET `PrevQuestID`=27405, `NextQuestID`=27438, `ExclusiveGroup`=-27406 WHERE `ID` IN (27406,27423);
-- The Forsaken Front
UPDATE `quest_template_addon` SET `PrevQuestID`=0, `NextQuestID`=27472 WHERE `ID`=27438;
UPDATE `quest_template_addon` SET `PrevQuestID`=27438 WHERE `ID`=27472;
UPDATE `quest_template_addon` SET `PrevQuestID`=27472, `NextQuestID`=27476, `ExclusiveGroup`=-27474 WHERE `ID` IN (27474,27475);
-- Ambermill: Relios the Relic Keeper and Practical Vengeance are both needed for Only One May Enter
UPDATE `quest_template_addon` SET `PrevQuestID`=27476, `NextQuestID`=27484, `ExclusiveGroup`=-27478 WHERE `ID` IN (27478,27483);
UPDATE `quest_template_addon` SET `PrevQuestID`=0 WHERE `ID`=27484;
UPDATE `quest_template_addon` SET `PrevQuestID`=27484 WHERE `ID`=27510;
UPDATE `quest_template_addon` SET `NextQuestID`=27542 WHERE `ID`=27518;
UPDATE `quest_template_addon` SET `PrevQuestID`=27518 WHERE `ID`=27542;
-- Pyrewood: Of No Consequence, Lessons in Fear and Pyrewood's Fall all need Taking the Battlefront and are all needed for
-- Sowing Discord; 7th Legion Battle Plans and Sowing Discord are both needed for On Her Majesty's Secret Service
UPDATE `quest_template_addon` SET `PrevQuestID`=27542, `NextQuestID`=27580, `ExclusiveGroup`=-27550 WHERE `ID` IN (27547,27548,27550);
UPDATE `quest_template_addon` SET `PrevQuestID`=27550, `NextQuestID`=27594, `ExclusiveGroup`=-27580 WHERE `ID` IN (27577,27580);
UPDATE `quest_template_addon` SET `NextQuestID`=27601 WHERE `ID`=27594;
UPDATE `quest_template_addon` SET `PrevQuestID`=27594 WHERE `ID`=27601;

-- Quest givers that exist neither on retail nor in the Cataclysm data (Wowhead agrees):
-- Sylvanas at the Forsaken Front only ends The Great Escape; Only One May Enter comes from Dalar Dawnweaver, Chapter I from
-- Sylvanas, Chapter II from the Ambermill Dimensional Portal; Resistance is Futile from Forward Commander Onslaught;
-- Empire of Dirt from Sylvanas at the Battlefront.
DELETE FROM `creature_queststarter` WHERE (`id`,`quest`) IN ((45525,27438),(45617,27484),(45752,27512),(45617,27513),(45474,27423),(2215,27746));
