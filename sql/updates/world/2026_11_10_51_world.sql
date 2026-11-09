-- Western Plaguelands, Chillwind Camp: Uther's Blessing once both Go Fletch! and A Mighty Hunger are done, and A Mighty Hunger after
-- Victory, For Now.
-- Sourced:
--  * Uther's Blessing 27169 needs Go Fletch! 27166 and A Mighty Hunger 27167: Wowhead MoP Classic comments on 27169 ("completed Victory,
--    For Now; and completed Go Fletch!; Valorfist is not giving ... Uther's Blessing ... completed Thurman Grant's quest A Mighty Hunger
--    and now Uther's Blessing is available", 2012; "I had to do the Mighty Hunger as well before it was available", 2012) and
--    warcraft.wiki.gg's quest infobox, as reported by the auditor. Both TDBs leave 27169 at PrevQuestID 0.
--  * A Mighty Hunger 27167 in part 2 of the zone, after Victory, For Now 27165: warcraft.wiki.gg's Alliance storyline, as reported by the
--    auditor. Neither TDB nor Wowhead has a link for it.
-- Ours: the form. 27166 and 27167 point at 27169 with ExclusiveGroup -27166, which in this core means both must be rewarded; 27167 gets
-- PrevQuestID 27165 (27166 already has it). Without this, Valorfist offered Uther's Blessing with Go Fletch! and Thurman offered A Mighty
-- Hunger to a level 34 newcomer before the battle.
UPDATE `quest_template_addon` SET `NextQuestID`=27169, `ExclusiveGroup`=-27166 WHERE `ID`=27166;
DELETE FROM `quest_template_addon` WHERE `ID`=27167;
INSERT INTO `quest_template_addon` (`ID`,`PrevQuestID`,`NextQuestID`,`ExclusiveGroup`) VALUES
(27167,27165,27169,-27166);
