-- Wandering Isle: "Missing Mallet" (29768) can never be taken once "Stronger Than Wood" (29771) is rewarded.
-- 29768 sits in exclusive group -29771 together with 29771, and 29771 has NextQuestID 29768, so 29771 is one of 29768's
-- dependent previous quests. Player::SatisfyQuestDependentPreviousQuests walks those, finds 29771 rewarded and requires every
-- other member of its negative group (29768 itself) to be rewarded as well, so it returns false. The chain 29772, 29774,
-- 29775 ... stays closed. The group -29769 (29769 + 29770) and the group -29771 (29771 alone) are the intended ones.
-- The 5.4.8 reference data has ExclusiveGroup 0 for 29768 (29769/29770 -29769, 29771 -29771, 29773 0 are identical).
-- Backup: SELECT ID, PrevQuestID, NextQuestID, ExclusiveGroup FROM quest_template_addon WHERE ID IN (29768, 29769, 29770, 29771, 29773);
--   before: 29768 29769 29772 -29771
UPDATE `quest_template_addon` SET `ExclusiveGroup` = 0 WHERE `ID` = 29768 AND `ExclusiveGroup` = -29771;
-- Undo: UPDATE `quest_template_addon` SET `ExclusiveGroup` = -29771 WHERE `ID` = 29768;
