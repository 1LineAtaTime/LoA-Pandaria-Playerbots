-- Victory, For Now (Horde 26926, Alliance 27165) needs all three of Scholomancer, Araj the Summoner and Brute Strength
-- Sourced: warcraft.wiki.gg Western Plaguelands storyline, "All of the following" before Victory, For Now; both TDBs (4.4.2, 1210)
-- give the three quests NextQuestID 26926 / 27165 and ExclusiveGroup 26924 / 27162. Wowhead's series puts the three side by side
-- before Victory, For Now.
-- Ours: the negative sign. In this core a positive group lets a player take only one quest of the group; a negative group lets him
-- hold all three and makes the quest after them need every one of them rewarded.
-- Needs the Gandling fix (zone_western_plaguelands.cpp, Scholomancer credits the Horde 26924 too), or 26926 can't be reached.
UPDATE `quest_template_addon` SET `NextQuestID`=26926, `ExclusiveGroup`=-26924 WHERE `ID` IN (26924,26925,27117);
UPDATE `quest_template_addon` SET `NextQuestID`=27165, `ExclusiveGroup`=-27162 WHERE `ID` IN (27162,27163,27164);
