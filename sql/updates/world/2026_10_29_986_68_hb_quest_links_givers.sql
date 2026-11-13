-- ported from PR 15 2026_10_28_40_world.sql (#986)
-- Hillsbrad Foothills: quest links, givers and breadcrumb groups (whole zone).

-- Hillsbrad Foothills, Azurelode Mine and the Sludge Fields: quest links and quest givers.
-- Sources: Cataclysm Classic TDB 442 and retail TDB 1210 (quest_template_addon, creature_queststarter), Wowhead MoP Classic
-- Quick Facts and Series, warcraft.wiki.gg zone progression.

-- The two Warchief's Command breadcrumbs exclude each other (TDB: ExclusiveGroup 28089 on both).
UPDATE `quest_template_addon` SET `ExclusiveGroup`=28089 WHERE `ID` IN (28089,28571);

-- Empire of Dirt is the other optional breadcrumb into Welcome to the Machine (wiki: "Empire of Dirt or Warchief's Command").
-- 28089/28571 point NextQuestID at 28096, which in this core makes one of them required for 28096; a player who came from
-- Silverpine with Empire of Dirt only could never get Welcome to the Machine. Ours, not in the TDB.
UPDATE `quest_template_addon` SET `NextQuestID`=28096 WHERE `ID`=27746;

-- The Battle for Hillsbrad and Discretion is Key follow No One Here Gets Out Alive (TDB PrevQuestID 28192 on both; Wowhead
-- series 28192 -> 28196/28197 -> 28206). Either of them opens Little Girl Lost ("A Lost Lad" on the wiki) (mop NextQuestID 28206 on both, unchanged).
UPDATE `quest_template_addon` SET `PrevQuestID`=28192 WHERE `ID` IN (28196,28197);

-- Lydon has to be freed before he (or Johnny) hands out the next quests (wiki: Freedom for Lydon follows A Lost Lad; Protocol
-- and Do it for Twinkles follow Freedom for Lydon). Neither TDB links them; without this, caged Lydon 47900 offers Protocol
-- before he is freed.
DELETE FROM `quest_template_addon` WHERE `ID` IN (28209,28231);
INSERT INTO `quest_template_addon` (`ID`,`MaxLevel`,`AllowableClasses`,`SourceSpellID`,`PrevQuestID`,`NextQuestID`,`ExclusiveGroup`,`RewardMailTemplateID`,`RewardMailDelay`,`RequiredSkillID`,`RequiredSkillPoints`,`RequiredMinRepFaction`,`RequiredMaxRepFaction`,`RequiredMinRepValue`,`RequiredMaxRepValue`,`ProvidedItemCount`,`SpecialFlags`,`ScriptName`) VALUES
(28209,0,0,0,28206,0,0,0,0,0,0,0,0,0,0,0,0,''), -- Freedom for Lydon
(28231,0,0,0,28209,0,0,0,0,0,0,0,0,0,0,0,0,''); -- Do it for Twinkles
UPDATE `quest_template_addon` SET `PrevQuestID`=28209 WHERE `ID`=28230; -- Protocol

-- Quest givers neither TDB nor Wowhead has.
DELETE FROM `creature_queststarter` WHERE (`id`,`quest`) IN (
(2215,28089),   -- High Executor Darthalia only ends Warchief's Command (starters: Cromush 44640/45631/46125, the command boards)
(47554,28111),  -- Captain Keyton ends Trouble at Azurelode; Darthalia offers it (retail starter, and 28096's RewardNextQuest)
(47551,28114),  -- Spider-Handler Sarus ends Glorious Harvest; only Apothecary Underhill 47432 starts it
(47551,28115),  -- same for Past Their Prime
(47781,28168)); -- Warden Stillwater ends Trouble at the Sludge Fields; Keyton (442 + retail) and Darthalia (retail) start it

-- Hillsbrad hub B (Southshore / Purgation Isle / Alterac): quest links, starters and flags
-- Sources: TDB 442 quest_template_addon / creature_queststarter / quest_template.Flags, warcraft.wiki.gg chain
-- ("Hillsbrad Foothills storyline" and the quest infoboxes).

-- Trouble at Southshore 28251: only Master Apothecary Lydon 47900 starts it (TDB 442, Wowhead); Serge Hinott is its ender.
DELETE FROM `creature_queststarter` WHERE `id`=2391 AND `quest`=28251;

-- Helcular's Rod Giveth... 28331 + Helcular's Command 28332: both are needed for Lok'tar 28344 (wiki: one parallel step).
-- TDB 442 has ExclusiveGroup +28331 (take only one), which would hide the second quest; -28331 = all of the group.
UPDATE `quest_template_addon` SET `ExclusiveGroup`=-28331 WHERE `ID` IN (28331,28332);

-- The Road to Purgation 28375 follows Kasha Will Fly Again 28354 (wiki order; TDB leaves 0, so Orkus offered it at once).
UPDATE `quest_template_addon` SET `PrevQuestID`=28354 WHERE `ID`=28375;

-- Heroes of the Horde! 28400: started by the Purgation Isle Orkus 48470 too (TDB 442 row, VerifiedBuild 59069; 48503 is
-- the summoned event actor and has no spawn), after They Will Never Expect This... 28397 (wiki "Previous").
DELETE FROM `creature_queststarter` WHERE `id`=48470 AND `quest`=28400;
INSERT INTO `creature_queststarter` (`id`,`quest`) VALUES (48470,28400);
UPDATE `quest_template_addon` SET `PrevQuestID`=28397 WHERE `ID`=28400;

-- Aid of the Frostwolf 28587 follows Heroes of the Horde! 28400 (wiki "Previous"); Cromush offered it before you met him.
UPDATE `quest_template_addon` SET `PrevQuestID`=28400 WHERE `ID`=28587;

-- Cry of the Banshee 28538 follows March of the Stormpike 28506 (wiki "Previous"), in parallel with Breaking the Hand.
UPDATE `quest_template_addon` SET `PrevQuestID`=28506 WHERE `ID`=28538;

-- Infiltration 28603 + Deception and Trickery 28604 need Decimation 28566 and are both needed for Domination 28605
-- (TDB 442 prev 28566; wiki: Domination's previous quests are both). TDB's +28603 group would make them exclusive.
UPDATE `quest_template_addon` SET `PrevQuestID`=28566, `NextQuestID`=28605, `ExclusiveGroup`=-28603 WHERE `ID` IN (28603,28604);

-- Stormpike Apocalypse 28616 needs both Breaking the Hand 28556 and Domination 28605 (wiki "Previous quests").
UPDATE `quest_template_addon` SET `NextQuestID`=28616, `ExclusiveGroup`=-28556 WHERE `ID` IN (28556,28605);

-- Tending the Garden (28751), Brazie's daily: warcraft.wiki.gg "You must complete the Plants vs. Ghouls quest chain first"
-- (the TDBs have no addon row for it). mop offers it from the start. Require Lawn of the Dead (28748).
DELETE FROM `quest_template_addon` WHERE `ID`=28751;
INSERT INTO `quest_template_addon` (`ID`,`MaxLevel`,`AllowableClasses`,`SourceSpellID`,`PrevQuestID`,`NextQuestID`,`ExclusiveGroup`,`RewardMailTemplateID`,`RewardMailDelay`,`RequiredSkillID`,`RequiredSkillPoints`,`RequiredMinRepFaction`,`RequiredMaxRepFaction`,`RequiredMinRepValue`,`RequiredMaxRepValue`,`ProvidedItemCount`,`SpecialFlags`,`ScriptName`) VALUES
(28751,0,0,0,28748,0,0,0,0,0,0,0,0,0,0,0,1,'');

-- Warchief's Command breadcrumbs out of Hillsbrad: the Tarren Mill/Eastpoint board quest (28619) and the capital board quest
-- (28572) for Arathi are one exclusive group in TDB 4.4.2 and retail (ExclusiveGroup 28572), and the Western Plaguelands board
-- quest (28575) is exclusive with The Battle for Andorhal (26920, ExclusiveGroup 26920): both lead to 26921. mop has 0, so a
-- player can take and hand in both Arathi breadcrumbs. Absolute updates of the sourced values.
UPDATE `quest_template_addon` SET `ExclusiveGroup`=28572 WHERE `ID` IN (28572,28619);
UPDATE `quest_template_addon` SET `ExclusiveGroup`=26920 WHERE `ID` IN (26920,28575);
