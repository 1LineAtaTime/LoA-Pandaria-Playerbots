-- Western Plaguelands, the first battle for Andorhal: the battle quests only after The Endless Flow.
-- Sourced:
--  * Horde Scholomancer 26924 and Araj the Summoner 26925, Alliance Scholomancer 27162 and Brute Strength 27163: PrevQuestID 26922 /
--    27161 (The Endless Flow) in both TDBs (4.4.2 and 1210).
--  * Horde Brute Strength 27117: both TDBs leave it at 0, but Wowhead MoP Classic's series puts it beside Scholomancer and Araj after The
--    Endless Flow ("1. The Endless Flow 2. Scholomancer, Araj the Summoner, Brute Strength 3. Victory, For Now").
-- Araj 26925 already needed The Endless Flow through 26922's NextQuestID; the PrevQuestID makes it explicit. The Alliance Araj 27164 already
-- needs 27161 the same way (both TDBs: PrevQuestID 0). The groups that make Victory, For Now need all three are not set here.
UPDATE `quest_template_addon` SET `PrevQuestID`=26922 WHERE `ID` IN (26924,26925,27117);
UPDATE `quest_template_addon` SET `PrevQuestID`=27161 WHERE `ID` IN (27162,27163);
