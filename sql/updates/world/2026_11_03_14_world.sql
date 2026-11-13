-- Hillsbrad Foothills, the Sludge Fields: For Science! (28199) follows No One Here Gets Out Alive (28192).
-- Neither TDB (4.4.2, retail) has an addon row for 28199, so Flesh-Shaper Arnauld offered it on arrival. warcraft.wiki.gg
-- gives Previous: No One Here Gets Out Alive, and its Sludge Fields chain lists Discretion is Key and For Science! together
-- as step 3, after 28192 (TDB zero filled from the wiki). No quest, condition or script needs 28199 rewarded, and no
-- quest offers it as RewardNextQuest; a player who already has it in the log keeps it (only accepting checks this).
DELETE FROM `quest_template_addon` WHERE `ID`=28199;
INSERT INTO `quest_template_addon` (`ID`,`MaxLevel`,`AllowableClasses`,`SourceSpellID`,`PrevQuestID`,`NextQuestID`,`ExclusiveGroup`,`RewardMailTemplateID`,`RewardMailDelay`,`RequiredSkillID`,`RequiredSkillPoints`,`RequiredMinRepFaction`,`RequiredMaxRepFaction`,`RequiredMinRepValue`,`RequiredMaxRepValue`,`ProvidedItemCount`,`SpecialFlags`,`ScriptName`) VALUES
(28199,0,0,0,28192,0,0,0,0,0,0,0,0,0,0,0,0,''); -- For Science!
