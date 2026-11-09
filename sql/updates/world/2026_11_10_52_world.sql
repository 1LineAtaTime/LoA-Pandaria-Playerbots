-- Western Plaguelands, the Bulwark: Lower the Boom after the three Bulwark lines, Strange New Faces after Lower the Boom.
-- Sourced (Wowhead MoP Classic comments; neither TDB links these quests):
--  * Lower the Boom 26936: "you need to have finished both After the Crusade and Who Needs Cauldrons?" (2012), "This also requires Foes
--    Before Hoes" (2014), and on 26979 "Lower the Boom also requires After the Crusade" / "also requires the quest line from Priestess
--    Vandis" (2012). After the Crusade 26930 ends Derrington's line, Foes Before Hoes 26933 Vandis's and Who Needs Cauldrons? 26978
--    Dithers's. Derrington's reward text: "Felstone is cleared and the bombs are set."
--  * Strange New Faces 26979: "Prerequisites are: Latent Disease, Who Needs Cauldrons?, and Lower the Boom" (2012); its text opens "Now
--    that Felstone is ours".
-- Ours: the form. 26930, 26933 and 26978 point at 26936 with ExclusiveGroup -26930, which in this core means all three must be rewarded;
-- 26979 gets PrevQuestID 26936. Until now Derrington offered both to a newcomer on arrival.
UPDATE `quest_template_addon` SET `NextQuestID`=26936, `ExclusiveGroup`=-26930 WHERE `ID` IN (26930,26933,26978);
UPDATE `quest_template_addon` SET `PrevQuestID`=26936 WHERE `ID`=26979;
