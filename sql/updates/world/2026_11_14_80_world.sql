-- Eastern Plaguelands, Tyr's Hand: order of "The Brotherhood of Light" (27459-27462, 27612-27620).
-- Before: only Soft Landing (27460) had a prerequisite. The Crusader's Flare offered To Take the Abbey / To Take the Barracks to
-- anyone, Eligor and Korfax offered their six quests to any level-39 character, and each wrap-up opened after one feeder
-- (a rewarded "dependent previous quest" with ExclusiveGroup 0 is enough: Player::SatisfyQuestDependentPreviousQuests).
-- Sourced (warcraft.wiki.gg zone progression; Wowhead comment 2010-12-04 "you must first complete ... Soft Landing, To Take the
-- Abbey"): Soft Landing -> To Take the Abbey + To Take the Barracks; Abbey -> Victory From Within + The Assassin -> Befouled No
-- More; Barracks -> Scarlet Salvage + The Wrathcaster + The Huntsman + The Commander -> Argent Upheaval; both -> Like Rats.
-- Both TDBs' addon rows for the chain are unusable (every feeder -> 27617 in a positive group, which makes parallel quests
-- exclusive) and are not ported.
-- Mechanism (this core): a negative ExclusiveGroup on quests that share a NextQuestID means "all of them" for that next quest.
INSERT IGNORE INTO `quest_template_addon` (`ID`) VALUES (27461), (27462);
UPDATE `quest_template_addon` SET `PrevQuestID` = 27460, `NextQuestID` = 0, `ExclusiveGroup` = 0 WHERE `ID` IN (27461, 27462);
UPDATE `quest_template_addon` SET `PrevQuestID` = 27461, `NextQuestID` = 27617, `ExclusiveGroup` = -27612 WHERE `ID` IN (27612, 27613);
UPDATE `quest_template_addon` SET `PrevQuestID` = 27462, `NextQuestID` = 27618, `ExclusiveGroup` = -27614 WHERE `ID` IN (27614, 27615, 27616, 27619);
UPDATE `quest_template_addon` SET `PrevQuestID` = 0, `NextQuestID` = 27620, `ExclusiveGroup` = -27617 WHERE `ID` IN (27617, 27618);
