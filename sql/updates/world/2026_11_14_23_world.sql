-- Eastern Plaguelands, Light's Shield Tower: Catalysm (27453) must wait for both To Kill With Purpose (27451) and Dark Garb
-- (27452).
--
-- Both had NextQuestID 27453 and ExclusiveGroup 0, which in this core makes 27453 available as soon as EITHER is rewarded
-- (Player::SatisfyQuestDependentPreviousQuests). Sourced: the wiki ("previous: To Kill With Purpose, Dark Garb"), Wowhead's
-- list of Betina's quests and the texts (27453 uses the rot, 27454 hands out the disguise made from the garb). Both TDBs
-- have +27451 (only one of the two may be taken), which contradicts every source. Ours: the each-from-all group -27451.
-- With it the auto-offer of 27453 (RewardNextQuest on both) fails quietly at the first hand-in and succeeds at the second.
--
-- Not here: RewardNextQuest 27373 on Greasing the Wheel (27369) and A Gift For Fiona (27372) (both TDBs) is written by the
-- caravan staging file 2026_11_14_05, which owns Fiona's rows.

UPDATE `quest_template_addon` SET `ExclusiveGroup` = -27451 WHERE `ID` IN (27451, 27452);
