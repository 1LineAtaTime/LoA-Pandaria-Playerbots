-- Eastern Plaguelands, "Rewriting the Battle of Darrowshire" (27383-27392): order and givers of the chain.
--
-- Sourced: the wiki's chain template ("Both of the following, in either order": 27384 and 27392 before 27385; "All of the
-- following, in any order": 27387-27389 before 27390); Wowhead comments 2011-07-07 on 27385 ("after you complete Pamela's Doll and
-- I'm Not Supposed to Tell You This"), 2010-11-28 [+47] on 27387 ("Pre-Q is A Strange Historian") and 2011-09-29 [+21]; both TDBs
-- have Pamela 10926 alone as starter of 27385 and Carlin 11063 alone as starter of 27386.
-- Before: either of Pamela's two quests opened Uncle Carlin; Carlin also offered 27385 (which ends at himself) and Chromie 27386 (which
-- ends at herself); Chromie's three relic quests were offered at level 39 without A Strange Historian, and any one of them opened 27390.
-- This core's semantics (Player::SatisfyQuestDependentPreviousQuests): a negative ExclusiveGroup on the quests whose NextQuestID points
-- at a quest means all of them must be rewarded. Little Pamela 27383 stays optional (Wowhead's series table starts at 27384/27392;
-- neither TDB has a PrevQuestID).
-- After it: RewardNextQuest still offers 27385 (and 27390) after the first quest of the pair (trio); the offer fails silently until
-- the others are done.
UPDATE `quest_template_addon` SET `ExclusiveGroup`=-27384 WHERE `ID` IN (27384,27392);
UPDATE `quest_template_addon` SET `PrevQuestID`=27386, `ExclusiveGroup`=-27387 WHERE `ID` IN (27387,27388,27389);
DELETE FROM `creature_queststarter` WHERE (`id`=11063 AND `quest`=27385) OR (`id`=10667 AND `quest`=27386);
