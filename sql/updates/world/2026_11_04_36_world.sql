-- Ruins of Southshore: Green Living 28325, Helcular's Rod Giveth... 28331 and Helcular's Command 28332 follow Trouble at
-- Southshore 28251. Wiki-sourced, TDB zero: warcraft.wiki.gg lists all three (with Studies in Lethality 28324) as "Next" of
-- Trouble at Southshore and gives it as their "Previous"; neither TDB (442, 1210) links them and Wowhead's Series starts
-- with them, so Serge Hinott and Helcular offered them to any Horde player from level 20.
-- 28331/28332 keep NextQuestID 28344 and ExclusiveGroup -28331 ("both needed" for Lok'tar, 2026_10_28_40; agrees with the
-- wiki's single parallel step, the TDBs' +28331 would hide the second quest). No quest's RewardNextQuest offers these
-- three, and the gate is only checked when a quest is taken, so quests already in a log or rewarded are unaffected.
-- 28325 had no addon row (both TDBs have none either).
INSERT INTO `quest_template_addon` (`ID`,`PrevQuestID`) VALUES (28325,28251) ON DUPLICATE KEY UPDATE `PrevQuestID`=28251;
UPDATE `quest_template_addon` SET `PrevQuestID`=28251 WHERE `ID` IN (28331,28332);
