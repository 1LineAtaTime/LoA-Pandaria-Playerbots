-- Reviewed nonzero training prerequisites and next-quest links.
-- Adapted from the seven source files in starting-training-batch-2026-11-05.md.
-- Preserve existing tutorial gates instead of copying upstream prerequisite removals.
-- Correct upstream Simple Sigil3116 (not Dwarf3106) and human Corruption26914 (not mage26916).
UPDATE `quest_template_addon` SET `NextQuestId`=31142 WHERE `ID`=31141 AND `NextQuestId`=26913;
UPDATE `quest_template_addon` SET `PrevQuestId`=3115 WHERE `ID`=26904 AND `PrevQuestId`=0;
UPDATE `quest_template_addon` SET `PrevQuestId`=3116 WHERE `ID`=26945 AND `PrevQuestId`=0;
UPDATE `quest_template_addon` SET `PrevQuestId`=3117 WHERE `ID`=26947 AND `PrevQuestId`=0;
UPDATE `quest_template_addon` SET `PrevQuestId`=3118 WHERE `ID`=26946 AND `PrevQuestId`=0;
UPDATE `quest_template_addon` SET `PrevQuestId`=3119 WHERE `ID`=26949 AND `PrevQuestId`=0;
UPDATE `quest_template_addon` SET `PrevQuestId`=26841 WHERE `ID`=26940 AND `PrevQuestId`=0;
UPDATE `quest_template_addon` SET `PrevQuestId`=3120 WHERE `ID`=26948 AND `PrevQuestId`=0;
UPDATE `quest_template_addon` SET `PrevQuestId`=31135 WHERE `ID`=31138 AND `PrevQuestId`=0;
