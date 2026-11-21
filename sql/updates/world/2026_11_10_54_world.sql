-- Western Plaguelands, the Alliance's second battle for Andorhal: The Battle Resumes! after Combat Training, and its quests in order.
-- Sourced (Wowhead MoP Classic comments; neither TDB links 27197, 27198, 27199 or 27202):
--  * The Battle Resumes! 27197 after the Uther's Blessing line, which ends with Combat Training 27174: "Indirect follow-up of the Uther's
--    Blessing chain, ending with Combat Training. Further opens up more quests in the battle for Andorhal" (27197, 2010); "Before I was
--    able to pick up this quest I had to finish the following quests first: ... Uther's Blessing ... Combat Training. At that points your
--    able to pick up The Battle Resumes!" (27205, 2012); "I'm under the impression that Uther's blessing and the resulting chain have to
--    be completed before getting the battle resumes" (27169, 2010).
--  * Supporting the Troops 27198 and Ashes to Ashes 27199 after 27197, and The Depravity of the Forsaken 27205 after both: "When handing in
--    at Thassarian you can pick up Ashes to Ashes & Supporting the Troops. When handing these in, you can pick up this quest" (27205,
--    2012); "you have to complete both Supporting the Troops and Ashes to Ashes" (27205, 2011).
--  * Aradne 27204 needs Brother Against Brother 27202 as well as Val'kyr Incursion 27201 (both TDBs: PrevQuestID 27201): "a prerequisite
--    for this quest in the chain seems to be brother against brother" (27204, 2014).
--  * Brother Against Brother 27202 beside Val'kyr Incursion: "Since you'll likely be doing Brother Against Brother at the same time" (27201,
--    2010) and warcraft.wiki.gg's storyline, as reported by the auditor.
-- Ours: 27202's PrevQuestID 27205 (the same step as 27201); the groups -27198 (27198 + 27199 -> 27205) and -27201 (27201 + 27202 -> 27204),
-- which in this core need every member rewarded. Until now Durnt Brightfalcon offered The Battle Resumes! to any level 38 Alliance
-- character, and Thassarian, Lang and Thurman offered the second battle's quests during the first.
DELETE FROM `quest_template_addon` WHERE `ID` IN (27197,27198,27199,27202);
INSERT INTO `quest_template_addon` (`ID`,`PrevQuestID`,`NextQuestID`,`ExclusiveGroup`) VALUES
(27197,27174,0,0),
(27198,27197,27205,-27198),
(27199,27197,27205,-27198),
(27202,27205,27204,-27201);
UPDATE `quest_template_addon` SET `ExclusiveGroup`=-27201 WHERE `ID`=27201;
