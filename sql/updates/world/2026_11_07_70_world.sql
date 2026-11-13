-- The Real Threat comes after Call to Arms. Sourced: warcraft.wiki.gg infobox "Previous: Call to Arms" and its progression (Clearing the Highlands, Call to Arms, The Real Threat);
-- Wowhead comment of 2010-12-05 on quest 26029 "becomes available after you complete Call to Arms". Neither TDB has a row for 26029, so they are silent, not against.
DELETE FROM `quest_template_addon` WHERE `ID` = 26029;
INSERT INTO `quest_template_addon` (`ID`,`MaxLevel`,`AllowableClasses`,`SourceSpellID`,`PrevQuestID`,`NextQuestID`,`ExclusiveGroup`,`RewardMailTemplateID`,`RewardMailDelay`,`RequiredSkillID`,`RequiredSkillPoints`,`RequiredMinRepFaction`,`RequiredMaxRepFaction`,`RequiredMinRepValue`,`RequiredMaxRepValue`,`ProvidedItemCount`,`SpecialFlags`,`ScriptName`) VALUES
(26029,0,0,0,26027,0,0,0,0,0,0,0,0,0,0,0,0,'');
