-- Eastern Plaguelands, Light's Hope Chapel: Lord Raymond George 49856's two Argent Dawn quests, 28755 Annals of the Silver
-- Hand and 28756 Aberrations of Bone, become repeatable, offered only after Argent Call: The Trial of the Crypt 27464 is
-- handed in, and not offered at maximum Argent Dawn reputation.
-- Sourced (Wowhead comments on quests 28755 and 28756 and NPC 49856):
--  * Repeatable, not daily: "2000 argent dawn rep, repeatable." (28755, 2010-11-27 [+31]); "PICK UP THE QUEST AGAIN after
--    turning it in each time" (28755, reply to 2010-12-04 [+119], [+28]); "you can do it as many times as you want in a day"
--    (28755, 2011-08-08 [0]); "Seems to be a repeatable quest, I didn't get any experience for it, just 2000 rep" (28756,
--    2010-11-26 [+33]); "you can do it as many times as you want" (28756, reply to 2012-01-26 [+2]); "the quests he gives
--    out are not daily, but are repeatable" (49856, reply to 2010-12-06 [+12]). The first turn-in gives no XP either.
--  * Offered after the Trial of the Crypt: "After completing the latter of these two quests [A Boyhood Dream -> Argent Call:
--    The Trial of the Crypt] ... Lord Raymond George ... will provide you with the two repeatable quests" (28755 and 28756,
--    2011-03-19 [+55] / [+89]); "After you succeed in killing this guy in the crypt and turn in the quest, he'll have his
--    two quests available" (49856, reply to 2010-12-06 [+4]); "Before this guy will give you his repeatable quests, you need
--    to do Fiona's Caravan chain" (49856, 2010-12-06 [+56]); "After 2nd hub is done, you get some quests in Light's Hope
--    Chapel that open up the 2 quests" (28756, 2010-11-27 [+12]); "I was already Revered ... I was not able to pick up the
--    repeatable quest of topic until I completed this line" (28755, reply [+2]).
--  * Not offered at maximum Argent Dawn: "You will not be able to get this quest (or the other one) if you are max exalted
--    with the Argent Dawn" (28756, 2010-12-23 [+28]); "Maybe it's because I was already Exalted (999/1000) with Argent Dawn?
--    Edit: Confirmed ... I'm now 874/1000 Exalted and I'm able to pick up these quests." (28755, reply to 2010-11-27 [+18]);
--    "was exalted with Argent Dawn and quest did not show up ... lost 125 rep then was able to get quest" (28755, reply [0]).
-- Both TDBs (4.4.2, 12.1.0) have no quest_template_addon row for either quest: no repeatable flag, no prerequisite and no
-- reputation cap there; their quest_template agrees with ours (Flags 8, no XP or money, Argent Dawn 529 RewardFactionValue1 8).
-- Ours: the numbers that express the sourced behaviour in this core.
--  * SpecialFlags 1 (QUEST_SPECIAL_FLAGS_REPEATABLE; Quest::IsRepeatable). Not daily (QuestFlags unchanged at 8).
--  * PrevQuestID 27464 (rewarded; Player::SatisfyQuestPreviousQuest). 27464's own reward raises Argent Dawn to Revered.
--  * RequiredMaxRepFaction 529, RequiredMaxRepValue 42999: Player::SatisfyQuestReputation refuses at standing >= the value;
--    42999 is ReputationMgr::Reputation_Cap, Exalted 999/1000, so 998/1000 and below can still take them, as the comments say.
-- No other rows touch these quests (no conditions, disables, spell_area or prerequisite links to them); the starter and ender
-- rows, quest_poi and the quest_template rows stay as they are.

DELETE FROM `quest_template_addon` WHERE `ID` IN (28755, 28756);
INSERT INTO `quest_template_addon` (`ID`, `MaxLevel`, `AllowableClasses`, `SourceSpellID`, `PrevQuestID`, `NextQuestID`, `ExclusiveGroup`, `RewardMailTemplateID`, `RewardMailDelay`, `RequiredSkillID`, `RequiredSkillPoints`, `RequiredMinRepFaction`, `RequiredMaxRepFaction`, `RequiredMinRepValue`, `RequiredMaxRepValue`, `ProvidedItemCount`, `SpecialFlags`, `ScriptName`) VALUES
(28755, 0, 0, 0, 27464, 0, 0, 0, 0, 0, 0, 0, 529, 0, 42999, 0, 1, ''),
(28756, 0, 0, 0, 27464, 0, 0, 0, 0, 0, 0, 0, 529, 0, 42999, 0, 1, '');
