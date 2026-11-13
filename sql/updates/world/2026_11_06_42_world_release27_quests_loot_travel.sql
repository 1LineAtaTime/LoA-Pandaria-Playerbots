-- SF27 quest text, progression, race eligibility, turn-ins, loot and LFG travel.

-- Source revision df19329b209fa1d1ae80444b43f6201d5dc124f0; SkyFire contributors, GPL-3.0.

-- Adapt to active split quest and LFG tables; preserve custom unrelated keys.

-- 2026_08_17_world_02.sql

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26391;

UPDATE `quest_template_addon` SET `PrevQuestID`=-26389 WHERE `ID`=26391;

-- 2026_08_18_world_00.sql

INSERT IGNORE INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25027;

UPDATE `quest_request_items` SET `CompletionText`='I can see why the Horde hates these little nuisances. Have you taken on Three-Tooth yet?' WHERE `ID`=25027;

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25027;

UPDATE `quest_offer_reward` SET `RewardText`='...Torn limb from limb!? Well, I\'d almost feel bad but the burning hatred gets in the way. She got what she deserved.$B$B<Corporal Teegan takes the Bramblestaff and snaps it over his knee.>$B$BProblem solved.' WHERE `ID`=25027;

INSERT IGNORE INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25000;

UPDATE `quest_request_items` SET `CompletionText`='What is it, $c? This here camp ain\'t fit for sight-seein\'.' WHERE `ID`=25000;

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25000;

UPDATE `quest_offer_reward` SET `RewardText`='Hey, you\'re tougher than you look. These terrortooth hides reflect arrows and turn axe-blades - they don\'t go down easy, but their hides are worth their weight in silver in these light-forsaken Barrens. Nice work.' WHERE `ID`=25000;

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25034;

UPDATE `quest_offer_reward` SET `RewardText`='Ah, $n! I\'ve been talking you up to the General here. Thank you again for saving my life.' WHERE `ID`=25034;

-- 2026_08_20_world_00.sql

INSERT INTO `lfg_dungeon_template` (`dungeonId`,`position_x`,`position_y`,`position_z`,`orientation`,`requiredItemLevel`) VALUES
(163,820.743,607.812,13.6389,0,0),
(164,1124.64,512.467,0.989549,1.5708,0),
(285,1124.64,512.467,0.989549,1.5708,0)
ON DUPLICATE KEY UPDATE `position_x`=VALUES(`position_x`),`position_y`=VALUES(`position_y`),`position_z`=VALUES(`position_z`),`orientation`=VALUES(`orientation`),`requiredItemLevel`=VALUES(`requiredItemLevel`);

-- 2026_08_20_world_02.sql

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25022;

UPDATE `quest_offer_reward` SET `RewardText`='Good work. Gaines passed through here not long ago on his way to Northwatch and a warm bath.$B$BThe mission wasn\'t a total loss, though. We\'ve learned a good deal about these quilboar and their leadership structure.$B$BNow we can handle things ... MY way.' WHERE `ID`=25022;

-- 2026_08_20_world_03.sql

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=13636;

UPDATE `quest_offer_reward` SET `RewardText`='Orders! Let\'s see what we have here... hah! He wants me to deal with the infestation of rat men. We\'re already in the middle of that, so no real deviation there.$B$BCare to lend a hand?' WHERE `ID`=13636;

-- 2026_08_20_world_04.sql

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28565;

UPDATE `quest_offer_reward` SET `RewardText`='We haven\'t gotten much in the way of reinforcements lately. We can use every pair of hands we can get.' WHERE `ID`=28565;

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25395;

UPDATE `quest_offer_reward` SET `RewardText`='Thank the Makers! It\'s downright pristine. Those orcs couldn\'t figure out the tap-lock, it seems...and couldn\'t hack through the mastercraft work of Ironforge\'s keg-crafters with their crude weapons!' WHERE `ID`=25395;

INSERT IGNORE INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25395;

UPDATE `quest_request_items` SET `CompletionText`='Do you have it? Is the poor thing intact?' WHERE `ID`=25395;

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25211;

UPDATE `quest_offer_reward` SET `RewardText`='That\'ll show \'em!  Nice work!' WHERE `ID`=25211;

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25770;

UPDATE `quest_offer_reward` SET `RewardText`='Hah! Nice to see those addlebrained mountaineers knew to hand the ale off to a real man to get the job done.$B$BWelcome to my survey. If you\'re of the mind to help Ironforge by takin\' over the intended jobs of some useless, lazy sods that call themselves dwarves, you\'re in the right place!' WHERE `ID`=25770;

INSERT IGNORE INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25770;

UPDATE `quest_request_items` SET `CompletionText`='Aye? What\'ve you got for me there?' WHERE `ID`=25770;

-- 2026_08_20_world_05.sql

UPDATE `creature_template` SET `lootid`=41145 WHERE `entry`=41145;

INSERT INTO `creature_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(41145,55234,-10,1,0,1,1),
(41145,62771,35,1,0,1,1),
(41145,3673,35,1,0,1,1),
(41145,3670,16,1,0,1,1),
(41145,3674,14,1,0,1,1)
ON DUPLICATE KEY UPDATE `ChanceOrQuestChance`=VALUES(`ChanceOrQuestChance`),`groupid`=VALUES(`groupid`),`mincountOrRef`=VALUES(`mincountOrRef`),`maxcount`=VALUES(`maxcount`);

UPDATE `creature_template` SET `lootid`=41147 WHERE `entry`=41147;

INSERT INTO `creature_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(41147,62771,42,1,0,1,1),
(41147,3673,32,1,0,1,1),
(41147,3670,16,1,0,1,1),
(41147,3674,9,1,0,1,1)
ON DUPLICATE KEY UPDATE `ChanceOrQuestChance`=VALUES(`ChanceOrQuestChance`),`groupid`=VALUES(`groupid`),`mincountOrRef`=VALUES(`mincountOrRef`),`maxcount`=VALUES(`maxcount`);

-- 2026_08_20_world_06.sql

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26980;

UPDATE `quest_offer_reward` SET `RewardText`='Stoutfist sent you, eh? He\'s a good captain, runs his men well. Just wish he\'d stop starin\' at my midriff.$B$BDon\'t let me catch you doin\' it either. We\'ve work to take care of here!' WHERE `ID`=26980;

-- 2026_08_22_world_01.sql

UPDATE `quest_template` SET `AllowableRaces`=2|16|32|128|256|512|33554432 WHERE `ID`=24807;

-- 2026_08_24_world_00.sql

UPDATE `quest_template` SET `AllowableRaces`=2|16|32|128|256|512|33554432 WHERE `ID`=24633;

UPDATE `quest_template` SET `AllowableRaces`=2|16|32|128|256|512|33554432 WHERE `ID`=24604;

-- 2026_08_24_world_01.sql

UPDATE `quest_template` SET `AllowableRaces`=1|4|8|64|1024|2097152|16777216 WHERE `ID`=25084;

-- 2026_08_26_world_02.sql

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=13527;

UPDATE `quest_offer_reward` SET `RewardText`='This definitely looks like the stuff they were eating. Let me take a closer peek...' WHERE `ID`=13527;

-- 2026_08_26_world_04.sql

INSERT INTO `item_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(57540,57541,100,1,0,1,1),
(57540,57542,100,1,0,1,1),
(57540,57543,100,1,0,1,1),
(57540,57544,100,1,0,1,1)
ON DUPLICATE KEY UPDATE `ChanceOrQuestChance`=VALUES(`ChanceOrQuestChance`),`groupid`=VALUES(`groupid`),`mincountOrRef`=VALUES(`mincountOrRef`),`maxcount`=VALUES(`maxcount`);

-- 2026_08_26_world_06.sql

UPDATE `creature_template` SET `unit_flags` = 0, `skinloot` = 2022 WHERE `entry` = 2022;

UPDATE `creature_template` SET `skinloot` = 2025 WHERE `entry` = 2025;

UPDATE `creature_template` SET `skinloot` = 2027 WHERE `entry` = 2027;

UPDATE `creature_template` SET `skinloot` = 2029 WHERE `entry` = 2029;

UPDATE `creature_template` SET `skinloot` = 2030 WHERE `entry` = 2030;

INSERT INTO `skinning_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(2022,765,2.1034,1,0,1,1),
(2022,785,25.8413,1,0,1,1),
(2022,2447,2.2636,1,0,1,1),
(2022,2449,2.0833,1,0,1,1),
(2022,23987,25.0601,1,0,1,1),
(2022,67357,64.1123,1,0,1,1),
(2025,765,3.6415,1,0,1,1),
(2025,785,25.2101,1,0,1,1),
(2025,2447,1.9608,1,0,1,1),
(2025,2449,3.3613,1,0,1,1),
(2025,23987,24.93,1,0,1,1),
(2025,67357,65.3185,1,0,1,1),
(2027,765,3.7262,1,0,1,1),
(2027,785,23.3969,1,0,1,1),
(2027,2447,2.9463,1,0,1,1),
(2027,2449,2.0797,1,0,1,1),
(2027,23987,27.383,1,0,1,1),
(2027,67357,60.0907,1,0,1,1),
(2029,765,2.1195,1,0,1,1),
(2029,785,32.7553,1,0,1,1),
(2029,2447,2.569,1,0,1,1),
(2029,2449,1.8626,1,0,1,1),
(2029,23987,24.6628,1,0,1,1),
(2029,67357,64.7761,1,0,1,1),
(2030,765,2.2876,1,0,1,1),
(2030,785,25.3268,1,0,1,1),
(2030,2447,1.5523,1,0,1,1),
(2030,2449,1.5523,1,0,1,1),
(2030,2450,12.2549,1,0,1,1),
(2030,2452,2.451,1,0,1,1),
(2030,23331,43.4641,1,0,1,1),
(2030,23987,13.8889,1,0,1,1),
(2030,54630,2.9423,1,0,1,1),
(2030,67357,31.1581,1,0,1,1)
ON DUPLICATE KEY UPDATE `ChanceOrQuestChance`=VALUES(`ChanceOrQuestChance`),`groupid`=VALUES(`groupid`),`mincountOrRef`=VALUES(`mincountOrRef`),`maxcount`=VALUES(`maxcount`);

-- 2026_08_31_world_00.sql

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28769;

UPDATE `quest_template_addon` SET `NextQuestID`=3104 WHERE `ID`=28769;

UPDATE `quest_template` SET `RewardNextQuest`=3104 WHERE `ID`=28769;

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28770;

UPDATE `quest_template_addon` SET `NextQuestID`=3101 WHERE `ID`=28770;

UPDATE `quest_template` SET `RewardNextQuest`=3101 WHERE `ID`=28770;

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28771;

UPDATE `quest_template_addon` SET `NextQuestID`=3103 WHERE `ID`=28771;

UPDATE `quest_template` SET `RewardNextQuest`=3103 WHERE `ID`=28771;

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28772;

UPDATE `quest_template_addon` SET `NextQuestID`=3102 WHERE `ID`=28772;

UPDATE `quest_template` SET `RewardNextQuest`=3102 WHERE `ID`=28772;

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28773;

UPDATE `quest_template_addon` SET `NextQuestID`=3105 WHERE `ID`=28773;

UPDATE `quest_template` SET `RewardNextQuest`=3105 WHERE `ID`=28773;

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28774;

UPDATE `quest_template_addon` SET `NextQuestID`=3100 WHERE `ID`=28774;

UPDATE `quest_template` SET `RewardNextQuest`=3100 WHERE `ID`=28774;

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28759;

UPDATE `quest_template_addon` SET `NextQuestID`=26910 WHERE `ID`=28759;

UPDATE `quest_template` SET `RewardNextQuest`=26910 WHERE `ID`=28759;

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31140;

UPDATE `quest_template_addon` SET `NextQuestID`=31141 WHERE `ID`=31140;

UPDATE `quest_template` SET `RewardNextQuest`=31141 WHERE `ID`=31140;

UPDATE `quest_template` SET `RewardNextQuest`=26913 WHERE `ID`=3100;

UPDATE `quest_template` SET `RewardNextQuest`=26918 WHERE `ID`=3101;

UPDATE `quest_template` SET `RewardNextQuest`=26915 WHERE `ID`=3102;

UPDATE `quest_template` SET `RewardNextQuest`=26919 WHERE `ID`=3103;

UPDATE `quest_template` SET `RewardNextQuest`=26916 WHERE `ID`=3104;

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=3105;

UPDATE `quest_template_addon` SET `NextQuestID`=26914 WHERE `ID`=3105;

UPDATE `quest_template` SET `RewardNextQuest`=26914 WHERE `ID`=3105;

UPDATE `quest_template` SET `RewardNextQuest`=26917 WHERE `ID`=26910;

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26910;

UPDATE `quest_template_addon` SET `AllowableClasses`=4 WHERE `ID`=26910;

UPDATE `quest_template` SET `RewardNextQuest`=31142 WHERE `ID`=31141;

UPDATE `quest_template` SET `RewardNextQuest`=28789 WHERE `ID`=26913;

UPDATE `quest_template` SET `RewardNextQuest`=28785 WHERE `ID`=26918;

UPDATE `quest_template` SET `RewardNextQuest`=28787 WHERE `ID`=26915;

UPDATE `quest_template` SET `RewardNextQuest`=28786 WHERE `ID`=26919;

UPDATE `quest_template` SET `RewardNextQuest`=28784 WHERE `ID`=26916;

UPDATE `quest_template` SET `RewardNextQuest`=28788 WHERE `ID`=26914;

UPDATE `quest_template` SET `RewardNextQuest`=28780 WHERE `ID`=26917;

UPDATE `quest_template` SET `RewardNextQuest`=31143 WHERE `ID`=31142;

-- 2026_09_02_world_01.sql

UPDATE `quest_template` SET `QuestDescription`='I am the caretaker for my kind, leading not through force, but through wisdom and swift action. Wisdom is what carries us into the deeper forest when danger beckons and swiftness is what leaves our enemies basking in our absence.$B$BIn this wisdom, I see through to your true intentions, and I know that you are deserving of my blessing, the blessing of my swiftness. It can be yours, but know that the other spirits will not welcome you once you proceed - consider this carefully before accepting my boon.' WHERE `ID`=13567;

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=13567;

UPDATE `quest_offer_reward` SET `RewardText`='I am the caretaker for my kind, leading not through force, but through wisdom and swift action. Wisdom is what carries us into the deeper forest when danger beckons and swiftness is what leaves our enemies basking in our absence.$B$BIn this wisdom, I see through to your true intentions, and I know that you are deserving of my blessing, the blessing of my swiftness. It can be yours, but know that the other spirits will not welcome you once you proceed - consider this carefully before accepting my boon.' WHERE `ID`=13567;

UPDATE `quest_template` SET `QuestDescription`='I share these forests willingly, accepting night elf kind into my home, and on occasion calling them friend. Though they have been tainted greatly in recent memory, I can see in you the same strength, honor, and spirit that first endeared them to me. If you place value in the ferocity necessary to overcome all odds, then I am willing to call you my champion. Know now that if you side with my kind, the other spirits will not accept you, so consider your choice carefully.' WHERE `ID`=13568;

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=13568;

UPDATE `quest_offer_reward` SET `RewardText`='I share these forests willingly, accepting night elf kind into my home, and on occasion calling them friend. Though they have been tainted greatly in recent memory, I can see in you the same strength, honor, and spirit that first endeared them to me. If you place value in the ferocity necessary to overcome all odds, then I am willing to call you my champion. Know now that if you side with my kind, the other spirits will not accept you, so consider your choice carefully.' WHERE `ID`=13568;

UPDATE `quest_template` SET `QuestDescription`='For countless ages I have walked these forests and lived in these mountains. Through snow and fire, I have watched civilizations crumble and nature press on. Now, the very earth turns against us, and it becomes very apparent that we must fight if our legacy is to continue. Your heart is true, and I would choose you as our champion. If you value the thick hide and resilience of my kind, then accept my boon. Know that the other spirits will not accept you once this choice is made, so choose wisely.' WHERE `ID`=13597;

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=13597;

UPDATE `quest_offer_reward` SET `RewardText`='For countless ages I have walked these forests and lived in these mountains. Through snow and fire, I have watched civilizations crumble and nature press on. Now, the very earth turns against us, and it becomes very apparent that we must fight if our legacy is to continue. Your heart is true, and I would choose you as our champion. If you value the thick hide and resilience of my kind, then accept my boon. Know that the other spirits will not accept you once this choice is made, so choose wisely.' WHERE `ID`=13597;

-- 2026_09_21_world_00.sql

UPDATE `creature_template` SET `unit_flags` = `unit_flags` & ~0x02000000
WHERE `entry` IN (17377, 17380, 17398);

-- 2026_09_21_world_10.sql

DELETE FROM `npc_trainer` WHERE (`entry`, `spell`) IN (


  (200001, 1160),

  (200001, 1464),

  (200001, 1680),

  (200001, 2565),

  (200001, 6572),

  (200001, 7384),

  (200001, 86346),



  (200002, 635),

  (200002, 879),

  (200002, 2812),

  (200002, 20164),

  (200002, 26573),

  (200002, 54428),

  (200002, 82326),

  (200002, 82327),

  (200002, 84963),



  (200003, 34026),

  (200003, 77767),



  (200004, 53),

  (200004, 32645),



  (200005, 527),

  (200005, 596),

  (200005, 2050),

  (200005, 2060),

  (200005, 2944),

  (200005, 8092),

  (200005, 14914),

  (200005, 64843),

  (200005, 73510),



  (200006, 49020),

  (200006, 56222),

  (200006, 56815),

  (200006, 85948),



  (200007, 331),

  (200007, 1535),

  (200007, 8232),

  (200007, 51505),

  (200007, 51730),

  (200007, 77472),



  (200008, 116),

  (200008, 2948),

  (200008, 5143),

  (200008, 30451),



  (200009, 348),

  (200009, 980),

  (200009, 1120),

  (200009, 1949),

  (200009, 5740),

  (200009, 6353),

  (200009, 27243),

  (200009, 29722),

  (200009, 74434),



  (200011, 1079),

  (200011, 2782),

  (200011, 2912),

  (200011, 5217),

  (200011, 5221),

  (200011, 5229),

  (200011, 6785),

  (200011, 8936),

  (200011, 16864),

  (200011, 33763),

  (200011, 50464),

  (200011, 52610),

  (200011, 88747),

  (200011, 88751),

  (200011, 106839)

) AND NOT (`entry`=200011 AND `spell`=106839);

-- 2026_09_23_world_00.sql

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26684;

UPDATE `quest_offer_reward` SET `RewardText`='Most superb! This will work perfectly. Many thanks!' WHERE `ID`=26684;

-- 2026_09_23_world_01.sql

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26785;

UPDATE `quest_template_addon` SET `PrevQuestID`=26674 WHERE `ID`=26785;

-- 2026_09_23_world_02.sql

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26408;

UPDATE `quest_template_addon` SET `PrevQuestID`=13897 WHERE `ID`=26408;

-- 2026_09_23_world_03.sql

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26683;

UPDATE `quest_offer_reward` SET `RewardText`='Why yes, I used to own a looking glass.' WHERE `ID`=26683;

-- 2026_09_23_world_04.sql

INSERT IGNORE INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26667;

UPDATE `quest_offer_reward` SET `RewardText`='By the Light... you actually went and got it?$B$BI''m shocked. I suppose I owe you thanks for returning it to the archives.' WHERE `ID`=26667;

-- 2026_09_23_world_05.sql

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=13896;

UPDATE `quest_template_addon` SET `PrevQuestID`=13948 WHERE `ID`=13896;

-- 2026_09_23_world_07.sql

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=13895;

UPDATE `quest_template_addon` SET `PrevQuestID`=13893 WHERE `ID`=13895;

-- 2026_09_23_world_08.sql

INSERT IGNORE INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=13898;

UPDATE `quest_template_addon` SET `PrevQuestID`=13953 WHERE `ID`=13898;

-- 2026_10_04_world_00.sql

UPDATE `creature_template` SET `lootid` = 2103 WHERE `entry` = 41080;

UPDATE `creature_template` SET `lootid` = 2102 WHERE `entry` IN (41072, 42107);

-- 2026_10_04_world_01.sql

INSERT IGNORE INTO `creature_questender` (`id`,`quest`) VALUES
(4772,14083),
(7954,14084);

-- 2026_10_04_world_03.sql

INSERT IGNORE INTO `creature_questender` (`id`,`quest`) VALUES
(35086,14146),
(37106,24502),
(44084,26810),
(44799,27040),
(46071,27768),
(18481,29674),
(18481,29675),
(55438,29745),
(56327,29924),
(58710,30478),
(29611,30987),
(39605,31012),
(16802,32329);

INSERT IGNORE INTO `gameobject_questender` (`id`,`quest`) VALUES
(206335,27693),
(206336,27709),
(206335,27885),
(206336,27886),
(209845,29888),
(213767,31314),
(215844,31779);

-- 2026_10_24_world_01.sql

UPDATE `quest_template` SET `AllowableRaces`=33555378 WHERE `ID`=26856;

UPDATE `quest_template` SET `AllowableRaces`=33555378 WHERE `ID`=26858;

UPDATE `quest_template` SET `AllowableRaces`=33555378 WHERE `ID`=26862;

UPDATE `quest_template` SET `AllowableRaces`=33555378 WHERE `ID`=30982;

UPDATE `quest_template` SET `AllowableRaces`=33555378 WHERE `ID`=30984;
