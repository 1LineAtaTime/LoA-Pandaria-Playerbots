-- Eastern Plaguelands, Fiona's Caravan: the eight caravan buffs (27555-27562) can be taken again and again, only one is held
-- at a time, Hidden Stashes drop only with Fiona's Lucky Charm, and Beezil's Cog waits for Beezil.
--
--  * Before: each buff quest could be taken once per character (SpecialFlags 0; Player::CanRewardQuest refuses a rewarded
--    non-repeatable quest), all eight auras stacked, Hidden Stash 61387 dropped from 95 loot tables for everyone, and
--    Beezil's Cog 27562 was offered from the first visit although Beezil joins the caravan last.
--  * Sourced: "repeatable = Yes" on all eight wiki pages and "I can click this question mark at the caravan endless times"
--    (881665); "you can only have one buff from the caravan at a time" (956654 [+7], newhon63, Duvvani); the charm "adds item
--    61387 to the loot-table of mobs" (Vlad [+85]; Mr. Grubbs "can now drop" after Onward, 177789 [+70]); Beezil's Cog only
--    once Beezil has joined (wiki).
--  * Ours: the spell_group id 51500 (free; groups end at 51442) with stack rule 1 EXCLUSIVE, under which
--    Unit::_RemoveNoStackAurasDueToAura removes the old buff when the new one lands; the gate of 27562 on the flag quest
--    "[FLAG] Beezil's Flag" 27563, which section 2's gossip script on Beezil 46022 rewards when he joins (until that script
--    is in, Beezil's Cog cannot be taken). The stash rates are left as they are (0.08-13.7 %; drop-rate rule: no certain
--    retail rate, so the less grindy value stays); a stash still holds no money (no item_template_addon row; amount unknown).
--  * The 95 loot rows are the whole of creature_loot_template for 61387 (no reference or other loot table has it). Two of
--    them are not only Eastern Plaguelands loot: 8596 (Plaguehound Runt, also spawned in Western Plaguelands) and 10820
--    (Duke Ragereaver, Stratholme). The charm is a zone-139 aura, so no stash drops there any more, as on retail, where the
--    stash came only from the charm.

-- One buff at a time.
DELETE FROM `spell_group` WHERE `id`=51500;
INSERT INTO `spell_group` (`id`,`spell_id`,`comment`) VALUES
(51500,85612,'Fiona''s Caravan buff: Fiona''s Lucky Charm'),
(51500,85613,'Fiona''s Caravan buff: Gidwin''s Weapon Oil'),
(51500,85614,'Fiona''s Caravan buff: Tarenar''s Talisman'),
(51500,85615,'Fiona''s Caravan buff: Pamela''s Doll'),
(51500,85616,'Fiona''s Caravan buff: Vex''tul''s Armbands'),
(51500,85617,'Fiona''s Caravan buff: Argus'' Journal'),
(51500,85618,'Fiona''s Caravan buff: Rimblat''s Stone'),
(51500,85619,'Fiona''s Caravan buff: Beezil''s Cog');
DELETE FROM `spell_group_stack_rules` WHERE `group_id`=51500;
INSERT INTO `spell_group_stack_rules` (`group_id`,`stack_rule`,`comment`) VALUES
(51500,1,'Fiona''s Caravan buffs: exclusive, the new one replaces the old');

-- Repeatable (QUEST_SPECIAL_FLAGS_REPEATABLE); 27562 had no addon row.
UPDATE `quest_template_addon` SET `SpecialFlags`=`SpecialFlags`|1 WHERE `ID` BETWEEN 27555 AND 27561;
DELETE FROM `quest_template_addon` WHERE `ID`=27562;
INSERT INTO `quest_template_addon` (`ID`,`MaxLevel`,`AllowableClasses`,`SourceSpellID`,`PrevQuestID`,`NextQuestID`,`ExclusiveGroup`,`RewardMailTemplateID`,`RewardMailDelay`,`RequiredSkillID`,`RequiredSkillPoints`,`RequiredMinRepFaction`,`RequiredMaxRepFaction`,`RequiredMinRepValue`,`RequiredMaxRepValue`,`ProvidedItemCount`,`SpecialFlags`,`ScriptName`) VALUES
(27562,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,'');

-- Beezil's Cog once Beezil has joined.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceGroup`=0 AND `SourceEntry`=27562;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(19,0,27562,0,0,8,0,27563,0,0,0,0,0,'','Beezil''s Cog: Beezil has joined the caravan ([FLAG] Beezil''s Flag rewarded)');

-- Hidden Stash only for a looter with Fiona's Lucky Charm.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=1 AND `SourceEntry`=61387;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(1,8519,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Blighted Surge)'),
(1,8520,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Plague Ravager)'),
(1,8521,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Blighted Horror)'),
(1,8522,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Plague Monstrosity)'),
(1,8523,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scourge Soldier)'),
(1,8524,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Cursed Mage)'),
(1,8525,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scourge Warder)'),
(1,8526,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Dark Caster)'),
(1,8527,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scourge Guard)'),
(1,8528,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Dread Weaver)'),
(1,8529,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scourge Champion)'),
(1,8530,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Cannibal Ghoul)'),
(1,8531,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Gibbering Ghoul)'),
(1,8532,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Diseased Flayer)'),
(1,8534,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Putrid Gargoyle)'),
(1,8535,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Putrid Shrieker)'),
(1,8537,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Interloper)'),
(1,8538,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Unseen Servant)'),
(1,8540,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Torn Screamer)'),
(1,8541,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Hate Shrieker)'),
(1,8542,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Death Singer)'),
(1,8543,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Stitched Horror)'),
(1,8544,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Gangled Golem)'),
(1,8545,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Stitched Golem)'),
(1,8546,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Dark Adept)'),
(1,8547,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Death Cultist)'),
(1,8548,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Vile Tutor)'),
(1,8550,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Shadowmage)'),
(1,8551,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Dark Summoner)'),
(1,8553,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Necromancer)'),
(1,8555,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Crypt Stalker)'),
(1,8556,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Crypt Walker)'),
(1,8557,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Crypt Horror)'),
(1,8558,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Crypt Slayer)'),
(1,8560,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Mossflayer Scout)'),
(1,8561,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Mossflayer Shadowhunter)'),
(1,8562,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Mossflayer Cannibal)'),
(1,8563,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Wretched Woodsman)'),
(1,8564,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Wretched Ranger)'),
(1,8565,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Wretched Pathstrider)'),
(1,8596,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Plaguehound Runt)'),
(1,8597,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Plaguehound)'),
(1,8598,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Frenzied Plaguehound)'),
(1,8600,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Plaguebat)'),
(1,8601,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Noxious Plaguebat)'),
(1,8602,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Monstrous Plaguebat)'),
(1,8603,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Carrion Grub)'),
(1,8605,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Carrion Devourer)'),
(1,9447,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scarlet Warder)'),
(1,9448,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scarlet Praetorian)'),
(1,9449,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scarlet Cleric)'),
(1,9450,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scarlet Curate)'),
(1,9451,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scarlet Archmage)'),
(1,9452,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scarlet Enchanter)'),
(1,10817,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Duggan Wildhammer)'),
(1,10818,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Death Knight Soulbearer)'),
(1,10819,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Baron Bloodbane)'),
(1,10820,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Duke Ragereaver)'),
(1,10821,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Hed''mush the Rotting)'),
(1,10822,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Warlord Thresh''jin)'),
(1,10823,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Zul''Brin Warpbranch)'),
(1,10824,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Death-Hunter Hawkspear)'),
(1,10825,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Gish the Unmoving)'),
(1,10826,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Lord Darkscythe)'),
(1,10827,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Deathspeaker Selendre)'),
(1,10828,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Lynnia Abbendis)'),
(1,11290,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Mossflayer Zombie)'),
(1,11291,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Unliving Mossflayer)'),
(1,11896,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Borelgore)'),
(1,11897,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Duskwing)'),
(1,12250,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Zaeldarr the Outcast)'),
(1,12261,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Infected Mossflayer)'),
(1,16117,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Plagued Swine)'),
(1,16184,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Nerubian Overseer)'),
(1,17878,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scourge Siege Engineer)'),
(1,45443,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Death''s Step Putrifier)'),
(1,45444,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Death''s Step Miscreation)'),
(1,45450,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (The Lone Hunter)'),
(1,45453,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Blighthound)'),
(1,45664,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Landlocked Grouper)'),
(1,45681,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Garginox)'),
(1,45691,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Skullmage)'),
(1,45692,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Noxious Assassin)'),
(1,45743,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Nerubian Sycophant)'),
(1,45744,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Ix''lar the Underlord)'),
(1,45851,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Overstuffed Golem)'),
(1,45867,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Omasum Blighthoof)'),
(1,45868,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Karthis Darkrune)'),
(1,45897,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Plaguewood Reanimator)'),
(1,46092,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Scarlet Commander Marjhan)'),
(1,46093,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Mataus the Wrathcaster)'),
(1,46094,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Huntsman Leopold)'),
(1,46095,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Rohan the Assassin)'),
(1,46096,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Crusader Lord Valdelmar)'),
(1,46167,61387,0,0,1,0,85612,0,0,0,0,0,'','Hidden Stash needs Fiona''s Lucky Charm (Stephen Browman)');
