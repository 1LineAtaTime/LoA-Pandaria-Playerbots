-- Release-26 published quest and starter AI adaptations for the LoA runtime schema.
-- SkyFire contributors: GPL-3.0; pinned upstream links and original hashes in the paired ledger.
-- No account, character, mail, item catalog, BattlePay or navigation changes.

-- 2026_07_19_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_19_world_00.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30267 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=30267);
UPDATE `quest_request_items` SET `CompletionText`='I refuse to not fish. Fishing\'s who I am!' WHERE `ID`=30267;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30267 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=30267);
UPDATE `quest_offer_reward` SET `RewardText`='Finally, someone who understands me.\r\nFishing is what it\'s all about, right?\r\n\r\nMaybe I should leave New Cifera... just pack up my things and join the Anglers, down in the south. Of course, I couldn\'t leave my family behind...\r\n\r\nBut still... a pandaren can dream, right?' WHERE `ID`=30267;

-- 2026_07_19_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_19_world_01.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30028 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=30028);
UPDATE `quest_request_items` SET `CompletionText`='Between birds and sprites we are never going to meet our quota to the Stormstout Brewery.' WHERE `ID`=30028;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30028 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=30028);
UPDATE `quest_offer_reward` SET `RewardText`='Why thank you very much stranger. This has not been a good day, but you certainly have helped us make it better.\r\n\r\nPlease, take this, you\'ve earned it.' WHERE `ID`=30028;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30351 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=30351);
UPDATE `quest_request_items` SET `CompletionText`='I have never seen the mogu this numerous before. They have always been a nuisance but never this bad.' WHERE `ID`=30351;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30351 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=30351);
UPDATE `quest_offer_reward` SET `RewardText`='That\'s it! You pick, I punch!' WHERE `ID`=30351;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30130 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=30130);
UPDATE `quest_request_items` SET `CompletionText`='I\'ve been punched, stabbed, thrown off of a cliff, hit in the face by someone\'s mace, hit in the face by own mace, and fell off the elevator at Thunder Bluff on more than one occasion.\r\n\r\nNone of that pain sounds even half as bad as what Leza is going through. I hope this tea helps...' WHERE `ID`=30130;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30130 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=30130);
UPDATE `quest_offer_reward` SET `RewardText`='Thank you hero. Kor is a brave warrior but he lacks compassion at times. Life must come before death, or there will be none of us left when the fighting is done.' WHERE `ID`=30130;

-- 2026_07_19_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_19_world_02.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30628 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=30628);
UPDATE `quest_offer_reward` SET `RewardText`='Peering inside, you find...' WHERE `ID`=30628;

-- 2026_07_20_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_00.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=32035 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=32035);
UPDATE `quest_offer_reward` SET `RewardText`='Greetings, $c. Welcome to the Silken Fields.' WHERE `ID`=32035;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=29917 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=29917);
UPDATE `quest_offer_reward` SET `RewardText`='I know I should be the one fighting them off. Maybe I\'m not cut out for shepherding after all.' WHERE `ID`=29917;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=29916 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=29916);
UPDATE `quest_offer_reward` SET `RewardText`='They... they\'re not stopping, but at least we fought back. Right?' WHERE `ID`=29916;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=29873 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=29873);
UPDATE `quest_offer_reward` SET `RewardText`='This place... it... it so SAD!\r\n\r\nKen-Ken never seen a place so sad.' WHERE `ID`=29873;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30081 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=30081);
UPDATE `quest_request_items` SET `CompletionText`='Cant\'t believe Ken-Ken did not think of this sooner. It\'s the ONE thing that can ruin his day...' WHERE `ID`=30081;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30081 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=30081);
UPDATE `quest_offer_reward` SET `RewardText`='Pandaren got a case of the spooky-dooks!\r\n\r\nDooks get scared sometimes, don\'t wanna come out. Poor, fluffy mopey-dopes must be ready to explode by now!\r\n\r\nThis just the thing to bring \'em out. That make pandaren happy for sure!\r\n\r\nJust like ol\' gramma used to say: can\'t THROW if can\'t GO!' WHERE `ID`=30081;

-- 2026_07_20_world_04.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_04.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30137 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=30137);
UPDATE `quest_request_items` SET `CompletionText`='The saurok keep the stolen serpent eggs in their camp.' WHERE `ID`=30137;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30137 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=30137);
UPDATE `quest_offer_reward` SET `RewardText`='Oh, wonderful!\r\n\r\nThese eggs look...well...they look a little on the small side. Are you sure you couldn\'t find any better ones?\r\n\r\nOh well, I suppose with the current situation, we can\'t be so picky.' WHERE `ID`=30137;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30159 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=30159);
UPDATE `quest_offer_reward` SET `RewardText`='Will those nasty Slitherscale saurok never give up?' WHERE `ID`=30159;
UPDATE `quest_template` SET `AreaDescription`='Well done, $n. As soon as Suchi told me she sent you to recover the stolen eggs, I knew you would accomplish the task.\r\n\r\nOur instructors will make sure to return these eggs to a safer place.' WHERE `ID`=30159;

-- 2026_07_20_world_05.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_20_world_05.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26343 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26343);
UPDATE `quest_offer_reward` SET `RewardText`='These skins should do. Their quality isn\'t too important, given that they\'ll make up the inside layer of the boots, but it\'s still important that they\'re comfortable.\r\n\r\nIf they\'re a bit too rigid, I\'ll use some tiger blood to soften it up. Tricks o\' the trade, you know?' WHERE `ID`=26343;

-- 2026_07_21_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_21_world_00.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=29670 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=29670);
UPDATE `quest_offer_reward` SET `RewardText`='This is extremely unsettling. If mogu are banding together like this all over the continent, it could become a major threat to Pandaria.' WHERE `ID`=29670;

-- 2026_07_21_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_21_world_01.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30064 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=30064);
UPDATE `quest_request_items` SET `CompletionText`='How are the sutras? Are they damaged?' WHERE `ID`=30064;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30064 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=30064);
UPDATE `quest_offer_reward` SET `RewardText`='Thank you! What a relief it is to see these texts are safe.' WHERE `ID`=30064;

-- 2026_07_22_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_22_world_02.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26378 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26378);
UPDATE `quest_offer_reward` SET `RewardText`='Murder, rookie. That\'S what you\'re looking at on the ground in front of us.' WHERE `ID`=26378;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28562 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=28562);
UPDATE `quest_offer_reward` SET `RewardText`='Murder, rookie. That\'S what you\'re looking at on the ground in front of us.' WHERE `ID`=28562;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26504 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26504);
UPDATE `quest_offer_reward` SET `RewardText`='A job well done deserves a reward, wouldn\'t you say?\r\n\r\nDon\'t spend this all in one place, $c. Better to spread it around, if you catch my drift.' WHERE `ID`=26504;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28551 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=28551);
UPDATE `quest_offer_reward` SET `RewardText`='We\'re facing heavy resistance from the Horde, $c. Any reinforcements are welcome.' WHERE `ID`=28551;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24921 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24921);
UPDATE `quest_offer_reward` SET `RewardText`='Hm? What unit are you with, $c?\r\n\r\nNo matter, we need all hands. My fleet is in tatters and they\'ve broke into the hold.\r\nYou armed? That\'s good enough for me.' WHERE `ID`=24921;

-- 2026_07_23_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_23_world_01.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26819 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26819);
UPDATE `quest_offer_reward` SET `RewardText`='Hooray! It\'s been days and days since we last had Akiris reeds!' WHERE `ID`=26819;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26487 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26487);
UPDATE `quest_request_items` SET `CompletionText`='Let\'s see... we\'re going to need some shotguns for our nature hikes, rowboats for the oil righ day trip, and...\r\n\r\nOh, you\'re back already?' WHERE `ID`=26487;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26487 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26487);
UPDATE `quest_offer_reward` SET `RewardText`='Hey hey! Not bad. not bad at all! You may have a future in resource acquisition. Or hotel managment.' WHERE `ID`=26487;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26450 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26450);
UPDATE `quest_request_items` SET `CompletionText`='Let\'s see \'em!' WHERE `ID`=26450;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26450 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26450);
UPDATE `quest_offer_reward` SET `RewardText`='Nice work, $n. Now I\'ve just got to decide what to do with all of these...' WHERE `ID`=26450;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26820 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26820);
UPDATE `quest_offer_reward` SET `RewardText`='These statues are beautiful...\r\n\r\nWow, I had no idea the naga were so talented. Thanks. $n!' WHERE `ID`=26820;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26494 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26494);
UPDATE `quest_request_items` SET `CompletionText`='Hey, is that the stuff? From the magazine?' WHERE `ID`=26494;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26494 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26494);
UPDATE `quest_offer_reward` SET `RewardText`='I\'ll file these away for now. Maybe we can experiment later.\r\n\r\n<Jasper looks over at you.>\r\n\r\nYeah, you\'ll do.' WHERE `ID`=26494;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26533 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26533);
UPDATE `quest_request_items` SET `CompletionText`='Excellent... you\'re not a zombie yet.\r\nThat\'s good.\r\n\r\nOh, umm... nothing. It\'s just that... that was a potential risk. Did i forget to warn you?' WHERE `ID`=26533;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26533 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26533);
UPDATE `quest_offer_reward` SET `RewardText`='<Jasper carefully analyzes the strange compound.>\r\n\r\nThis mixture is... I don\'t understand it.\r\nPerhaps we need to run some clinical trials.' WHERE `ID`=26533;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26815 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26815);
UPDATE `quest_request_items` SET `CompletionText`='Excellent... you\'re not a zombie yet.\r\nThat\'s good.\r\n\r\nOh, umm... nothing.  It\'s just that... that was a potential risk.  Did I forget to warn you?' WHERE `ID`=26815;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26815 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26815);
UPDATE `quest_offer_reward` SET `RewardText`='<Dask carefully analyzes the strange compound.>\r\n\r\nThis mixture is... PERFECT! Let\'s see what it does to your potentially-titan artifact here!' WHERE `ID`=26815;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26433 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26433);
UPDATE `quest_request_items` SET `CompletionText`='<Flem\'s ears perk up as you approach.>\r\n\r\nThe holy Water of Clarity? Is it here!?' WHERE `ID`=26433;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26433 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26433);
UPDATE `quest_offer_reward` SET `RewardText`='<The goblin downs the water in one gulp.>\r\n\r\nAhh... do you know how hard it is to get fresh water in Stranglethorn? It\'s no wonder that the nagas guard their shrine so ferociously!\r\n\r\nOh, right... your artifact, and clarity.\r\nClarity...\r\n\r\nI\'ve got it!' WHERE `ID`=26433;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26592 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26592);
UPDATE `quest_request_items` SET `CompletionText`='<Flem\'s eyes sparkle.>\r\n\r\nAre those gems I see?' WHERE `ID`=26592;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26592 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26592);
UPDATE `quest_offer_reward` SET `RewardText`='<Flem weighs the gems in his hands and nods in approval.>\r\n\r\nYes, these will do. Now to perform the analysis...\r\n\r\n<The goblin clacks the gems together for a while, then looks up at you.>\r\n\r\nOh, you\'re still here? You don\'t think I\'m a fraud or something, do you?' WHERE `ID`=26592;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26435 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26435);
UPDATE `quest_request_items` SET `CompletionText`='Yes, $r? I\'m quite busy' WHERE `ID`=26435;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26435 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26435);
UPDATE `quest_offer_reward` SET `RewardText`='Fascinating!\r\n\r\nI\'ll need some time to study this. Why don\'t you have a look around Booty Bay and come back later?' WHERE `ID`=26435;

-- 2026_07_24_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_24_world_00.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26493 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26493);
UPDATE `quest_request_items` SET `CompletionText`='I hope you used the extraction procedure we discussed.' WHERE `ID`=26493;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26493 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26493);
UPDATE `quest_offer_reward` SET `RewardText`='Great! These look... disgusting. But probably pretty valuable. Let\'s get to splitting these up. Some for me, some for Jasper, some for Chabal...' WHERE `ID`=26493;

-- 2026_07_24_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_24_world_01.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26808 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26808);
UPDATE `quest_offer_reward` SET `RewardText`='<The troll looks into the package.>\r\n\r\nYou have my thanks. You may tell your young dwarven friend that I am not sad. I am waiting. For you.\r\n\r\nI have identified you as the most promising dwarf here in this strange town. I have a task for you, if you would be willing to take it.' WHERE `ID`=26808;

-- 2026_07_24_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_24_world_02.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26495 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26495);
UPDATE `quest_request_items` SET `CompletionText`='Care to explain yourself, $c?' WHERE `ID`=26495;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26495 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26495);
UPDATE `quest_offer_reward` SET `RewardText`='<The troll looks into the package.>\r\n\r\nYou have my thanks.\r\n\r\nI have been waiting for you to approach me, $r. I have identified you as the most promising $r here in this strange town. I have a task for you, if you would be willing to take it.' WHERE `ID`=26495;

-- 2026_07_24_world_04.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_24_world_04.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28554 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=28554);
UPDATE `quest_offer_reward` SET `RewardText`='If we can get things straightened out here, we have a chance to strike at the Alliance while most of their forces are busy in the Barrens.' WHERE `ID`=28554;

-- 2026_07_25_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_00.sql
UPDATE `quest_template` SET `RewardNextQuest`=27091 WHERE `ID`=8329;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Charge. Locate a Training Dummy outside the Sunspire and practice using Charge.' WHERE `ID`=27091;
UPDATE `quest_template` SET `QuestDescription`='Good. You are off to a bright start. This road will be long, and as you become more accomplished, you will begin to discover that you can do things you could never have imagined before.$B$BGo, get more experience, and learn a new technique. Once you have, demonstrate it on one of the training dummies.' WHERE `ID`=27091;
UPDATE `quest_template` SET `RewardNextQuest`=10069 WHERE `ID`=9676;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Seal of Command. Cast Seal of Command on yourself, then locate a Training Dummy outside the Sunspire and hit it.' WHERE `ID`=10069;
UPDATE `quest_template` SET `QuestDescription`='It is time for your first lesson in the ways of the Light, $n. As you fight more battles, it will bring you closer to the light, and you\'ll start to uncover new abilities. Go, get more experience, and learn a new ability. Return and demonstrate it for me, and we\'ll continue your training.' WHERE `ID`=10069;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=10069 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=10069);
UPDATE `quest_offer_reward` SET `RewardText`='' WHERE `ID`=10069;
UPDATE `quest_template` SET `RewardNextQuest`=10070 WHERE `ID`=9393;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Steady Shot. Locate a Training Dummy outside the Sunspire and practice using Steady Shot 5 times.' WHERE `ID`=10070;
UPDATE `quest_template` SET `QuestDescription`='And now, it is time for your first lesson.$B$BAs you become more skilled, you will begin to discover new techniques.$B$BGo out, get some more experience, and learn your first new ability. Once you do, go find a training dummy and practice your new skill.' WHERE `ID`=10070;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=10070 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=10070);
UPDATE `quest_offer_reward` SET `RewardText`='You have done well, $n. You should be proud of your progress.' WHERE `ID`=10070;
UPDATE `quest_template` SET `RewardNextQuest`=10071 WHERE `ID`=9392;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Eviscerate, then locate a Training Dummy outside the Sunspire and practice using Eviscerate 3 times.' WHERE `ID`=10071;
UPDATE `quest_template` SET `QuestDescription`='As you grow in power, you\'ll start to figure out the tricks of the trade.$B$BOne of the first things you\'ll probably figure out is how to Eviscerate your enemy after landing some initial attacks. Get some experience, learn how to do it, and then find one of the training dummies outside and give it a try.' WHERE `ID`=10071;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=10071 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=10071);
UPDATE `quest_offer_reward` SET `RewardText`='' WHERE `ID`=10071;
UPDATE `quest_template` SET `RewardNextQuest`=10072 WHERE `ID`=8564;
UPDATE `quest_template` SET `LogTitle`='Learning the Word' WHERE `ID`=10072;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Shadow Word: Pain. Practice casting it 5 times on nearby training dummies.' WHERE `ID`=10072;
UPDATE `quest_template` SET `QuestDescription`='As you grow in skill, so will the array of powers that you can call upon.$B$BI\'d like you to go, continue to become more experienced, and discover a new spell. Once you do, come back and demonstrate it for me.' WHERE `ID`=10072;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=10072 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=10072);
UPDATE `quest_offer_reward` SET `RewardText`='' WHERE `ID`=10072;
UPDATE `quest_template` SET `RewardNextQuest`=10068 WHERE `ID`=8328;
UPDATE `quest_template` SET `QuestDescription`='As you begin to grow in power, your connection to the arcane will develop, granting you access to new spells.$B$BGo, develop your power, and then demonstrate what you learn for me.' WHERE `ID`=10068;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=10068 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=10068);
UPDATE `quest_offer_reward` SET `RewardText`='' WHERE `ID`=10068;
UPDATE `quest_template` SET `RewardNextQuest`=10073 WHERE `ID`=8563;
UPDATE `quest_template` SET `LogTitle`='Corruption' WHERE `ID`=10073;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Corruption. Locate a Training Dummy outside the Sunspire and practice casting Corruption 5 times.' WHERE `ID`=10073;
UPDATE `quest_template` SET `QuestDescription`='By now, you\'ve heard the rumors about our kind, the warlocks. Pay them no mind; they are uttered by those jealous of our fearsome powers.$B$BAs you become more experienced, you will uncover mysteries of controlling fel energies. Go, continue your journey until you uncover a new technique. I want you to practice using this new spell on the training dummies outside the Sunspire.' WHERE `ID`=10073;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=10073 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=10073);
UPDATE `quest_offer_reward` SET `RewardText`='' WHERE `ID`=10073;
UPDATE `quest_template` SET `RewardNextQuest`=31171 WHERE `ID`=31170;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31170 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31170);
UPDATE `quest_offer_reward` SET `RewardText`='Ah, welcome. I have been teaching your people the ways of the $c, and they seem to be well suited for it. Perhaps you too will take to it as quickly as they.' WHERE `ID`=31170;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31171 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31171);
UPDATE `quest_offer_reward` SET `RewardText`='I think, you\'ve mastered the basics of the Tiger Palm technique, $n. Perhaps you will impress me, after all.' WHERE `ID`=31171;

-- 2026_07_25_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_01.sql
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26958 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=26958);
UPDATE `quest_template_addon` SET `PrevQuestID`=0 WHERE `ID`=26958;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Charge. Find a Training Dummy on the northwest side of the Crash Site and practice using Charge.' WHERE `ID`=26958;
UPDATE `quest_template` SET `QuestDescription`='Let\'s start your training right away. As you get more experience in combat, you\'ll start to develop new techniques.$B$BGo, get some more experience, learn something new, and demonstrate it on the dummies to the northwest.' WHERE `ID`=26958;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26966 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=26966);
UPDATE `quest_template_addon` SET `PrevQuestID`=0 WHERE `ID`=26966;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Seal of Command. Apply Seal of Command, find a Training Dummy on the northwest side of the Crash Site and hit it with an attack.' WHERE `ID`=26966;
UPDATE `quest_template` SET `QuestDescription`='As a paladin, you\'re no stranger to the Light that touches all of us. But you also have the unique power to call upon the Light to help you in battle.$B$BAs you gain more experience, you\'ll grow even closer to the Light and be able to call upon new powers in battle. Go, grow in experience until you have learned a new power, and then demonstrate it for me.' WHERE `ID`=26966;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26963 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=26963);
UPDATE `quest_template_addon` SET `PrevQuestID`=0 WHERE `ID`=26963;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Steady Shot. Find a Training Dummy on the northwest side of the Crash Site and practice using Steady Shot 5 times.' WHERE `ID`=26963;
UPDATE `quest_template` SET `QuestDescription`='Let\'s start your training right away. It\'s going to be a struggle to survive here, and you\'ll play an important role in making sure our people are safe and well-supplied.$B$BAs you become more experiened, you\'ll begin to discover new abilities. Go ahead and go out, learn something new, and try it out. Return to me when you think you\'ve mastered it.' WHERE `ID`=26963;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26970 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=26970);
UPDATE `quest_template_addon` SET `PrevQuestID`=0 WHERE `ID`=26970;
UPDATE `quest_template` SET `LogTitle`='Learning the Word' WHERE `ID`=26970;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Shadow Word: Pain, then use it 5 times on target dummies near the crash site.' WHERE `ID`=26970;
UPDATE `quest_template` SET `QuestDescription`='As a priest, one of your most important duties is to use the Light\'s power to aid the wounded. Another is to use your power offensively, to protect them. As you grow more experienced, you will begin to uncover new abilities that you can use in your charge. Go, gain more experience, and then demonstrate the abilities you have learned to me.' WHERE `ID`=26970;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26969 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=26969);
UPDATE `quest_template_addon` SET `PrevQuestID`=0 WHERE `ID`=26969;
UPDATE `quest_template` SET `LogDescription`='Learn Primal Strike by reaching Level 3, and then find a Training Dummy on the northwest side of the Crash Site and practice using Primal Strike 3 times.' WHERE `ID`=26969;
UPDATE `quest_template` SET `QuestDescription`='Your walk with the elemental powers will take you to many places. As you grow in power, you will begin to learn new abilities. Let us start your journey with a simple, but useful, lesson. Learn the secret of Primal Strike, and then practice your new skill using the training dummies to the northwest of the Crash Site. Return to me when you are finished.' WHERE `ID`=26969;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26969 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26969);
UPDATE `quest_offer_reward` SET `RewardText`='It is a pleasure to teach such a capable student. As long as you continue the traditions faithfully, the shamanic arts will never be lost to our people again.' WHERE `ID`=26969;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26968 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=26968);
UPDATE `quest_template_addon` SET `PrevQuestID`=0 WHERE `ID`=26968;
UPDATE `quest_template` SET `QuestDescription`='In your future as a mage, you\'ll want to master a wide variety of spells. To do so, you need to deepen your connection by the arcane by growing in power. Go, get more experience as a mage, and demonstrate what you learn for me.' WHERE `ID`=26968;

-- 2026_07_25_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_03.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31156 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=31156);
UPDATE `quest_request_items` SET `CompletionText`='Ahh, you have come.' WHERE `ID`=31156;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31156 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31156);
UPDATE `quest_offer_reward` SET `RewardText`='We will honor our respective peoples by learning from each other. You will become a great $c. If I have anything to say about it. And I will learn the ways of your people.' WHERE `ID`=31156;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31157 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31157);
UPDATE `quest_offer_reward` SET `RewardText`='Good, good. Your first challenge surpassed. We will learn much from each other. I can see.' WHERE `ID`=31157;

-- 2026_07_25_world_04.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_04.sql
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Charge. Locate a Training Dummy and practice using Charge.' WHERE `ID`=25147;
UPDATE `quest_template` SET `QuestDescription`='I\'ve been instructed to prepare you for battle. Though, from the look of you, it looks like you\'ve already spent a little time out in the field. Good.$B$BAs you continue your journey, you\'ll discover that you can begin to use new techniques$B$BGo, get some more experience, and teach yourself how to Charge, then show me what you\'ve learned.' WHERE `ID`=25147;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Steady Shot. Locate a Training Dummy and practice using Steady Shot 5 times.' WHERE `ID`=25139;
UPDATE `quest_template` SET `QuestDescription`='You already know how to fire an arcane shot. Good. But doing so depletes your focus. To gain your focus back you will need to switch to a second type of ranged attack. Go out and get some more experience. As you do, you\'ll learn some new abilities. The first of these will help you get your focus back.' WHERE `ID`=25139;
UPDATE `quest_template` SET `LogDescription`='Reach Level 3 to Learn Eviscerate, then locate a Training Dummy and practice using Eviscerate 3 times.' WHERE `ID`=25141;
UPDATE `quest_template` SET `QuestDescription`='Let\'s start with the basics, shall we? I\'m the only rogue trainer in town, so you\'re going to have to learn it from me.$B$BGo get some more experience, figure out how to eviscerate your foes, and then show me on the training dummies here. I\'ll evaluate your form.' WHERE `ID`=25141;
UPDATE `quest_template` SET `LogDescription`='Learn Primal Strike by reaching Level 3, then look for a training dummy near Shikrik and practice using Primal Strike 3 times.' WHERE `ID`=25143;
UPDATE `quest_template` SET `QuestDescription`='As you grow in power by gaining levels, you will discover the power to use new abilities. The first you will discover on your journey is Primal Strike. Use it to deliver a strike with your weapon, infused with the power of the elements.$B$BIt is time to learn and master your new skill. Practice using it on the training dummies outside the Den.' WHERE `ID`=25143;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25138 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=25138);
UPDATE `quest_offer_reward` SET `RewardText`='Ahh, where to start... introductions! I\'m Acrypha - the one who sent you that parchment - I\'ve spent the last several years in an Undercity library, my head in a book. I learned everything I could about the arcane, and now I\'ve returned home to share it with grunts like yourself.$B$BShall we begin?' WHERE `ID`=25138;
UPDATE `quest_template` SET `LogTitle`='Corruption' WHERE `ID`=25145;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Corruption. Locate a Training Dummy and practice casting Corruption 5 times.' WHERE `ID`=25145;
UPDATE `quest_template` SET `QuestDescription`='It is my duty to train you, and I intend to do just that.$B$BAs you become more powerful, you will begin to hear the fel voices revealing new spells to you. Go, get more experience, learn a new spell, and then practice your new spell against one of the training dummies around here.' WHERE `ID`=25145;

-- 2026_07_25_world_07.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_25_world_07.sql
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Charge, then practice using Charge on a Training Dummy south of the entrance to Northshire Abbey.' WHERE `ID`=26913;
UPDATE `quest_template` SET `QuestDescription`='Now that you\'ve had the chance to test your abilities in battle, it\'s time to explain one more reason why training is so important.$B$BAs you get more experience, you\'ll be able to do new techniques you never thought of. Go, get some experience, learn a new skill, and demonstrate it on one of the training dummies.' WHERE `ID`=26913;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26913 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26913);
UPDATE `quest_offer_reward` SET `RewardText`='You\'re a quick learner, $n.' WHERE `ID`=26913;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Seal of Command. Cast Seal of Command on yourself and then hit a Training Dummy south of the entrance to Northshire Abbey.' WHERE `ID`=26918;
UPDATE `quest_template` SET `QuestDescription`='As you continue in your journey as a $c, you\'ll become closer to the light, learning new abilities. Go, get more experience, and learn a new ability. Demonstrate it for me, and we will continue your training.' WHERE `ID`=26918;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26918 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26918);
UPDATE `quest_offer_reward` SET `RewardText`='You\'re a quick learner, $n. Return to me as you grow stronger and I\'ll teach you more about using the Light to vanquish your enemies and strengthen your allies.' WHERE `ID`=26918;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Steady Shot, then find a Training Dummy south of the entrance to Northshire Abbey and practice using Steady Shot 5 times.' WHERE `ID`=26917;
UPDATE `quest_template` SET `QuestDescription`='I\'ve heard about your help in clearing out some of the orcs. You\'ve shown that you can handle yourself in combat.$B$BAs you get more experience in combat, you\'ll start learning how to do new things. I want you to go, get some more experience, then try the new abilities you learn on one of the training dummies south of the entrance to the abbey.' WHERE `ID`=26917;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26917 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26917);
UPDATE `quest_offer_reward` SET `RewardText`='I knew you would master Steady Shot quickly. I\'m looking forward to showing you other techniques as you get more experienced in the ways of the Hunter.' WHERE `ID`=26917;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Eviscerate, then find a Training Dummy south of the entrance to Northshire Abbey and practice Eviscerate 3 times.' WHERE `ID`=26915;
UPDATE `quest_template` SET `QuestDescription`='By now, you know the basics of combat as $c, but I want to teach you a new skill. As rogues, we are renowned for our ability to execute chains of combat techniques, often saving the most powerful strikes for the end.$B$BAs you get more experience, you\'ll learn new things - and this is one of the ones you\'ll learn first. Once you have, I want you to go try it on one of the training dummies south of the entrance to the abbey.' WHERE `ID`=26915;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26915 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26915);
UPDATE `quest_offer_reward` SET `RewardText`='You\'ve caught on quickly, $n, and a quick mind will serve you well.' WHERE `ID`=26915;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26919 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26919);
UPDATE `quest_offer_reward` SET `RewardText`='You\'re a quick learner, $n.' WHERE `ID`=26919;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Frost Nova, then practice casting it 2 times on a Training Dummy south of the entrance to Northshire Abbey.' WHERE `ID`=26916;
UPDATE `quest_template` SET `QuestDescription`='As a mage, you will learn a variety of spells as your understanding grows. All you need to do is get some more experience, and the secrets of the arcane will reveal themselves to you. You\'ll see. Go develop your powers, and show me what you learn.' WHERE `ID`=26916;
UPDATE `quest_template` SET `LogTitle`='Corruption' WHERE `ID`=26914;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn the Corruption ability, then practice casting Corruption 5 times on a Training Dummy south of the entrance to Northshire Abbey.' WHERE `ID`=26914;
UPDATE `quest_template` SET `QuestDescription`='We must take care in how we speak around the abbey and its environs, but don\'t let that deter you from the path of power.$B$BAs you become more powerful, you will begin to hear the fel voices, granting you their power in the dark arts. Go, get more experience and learn a new spell. Once you have, practice it on the training dummies.' WHERE `ID`=26914;

-- 2026_07_26_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_01.sql
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Charge, then use Charge on one of the Training Dummies outside the entrance to Anvilmar.' WHERE `ID`=24531;
UPDATE `quest_template` SET `QuestDescription`='Lesson one. Aside from your sword and shield, what\'s the most important tool you have in your arsenal?$B$B<Thran awaits your response.>$B$BOh never mind, I\'ll answer it for you. It\'s experience. As you get more experience, you\'ll discover new techniques, which means you can be much more effective than you imagined. Go, get some experience, and learn something new. Then come back and demonstrate.' WHERE `ID`=24531;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Seal of Command. Use Seal of Command on yourself, and then hit one of the Training Dummies at the entrance to Anvilmar.' WHERE `ID`=24528;
UPDATE `quest_template` SET `QuestDescription`='Are ye ready to start? Wonderful! We\'re glad to have ye.$B$BAs ye get stronger, it brings ye closer to the light, and ye learn new techniques. Go ahead and get some more experience, and show me what ye learn.' WHERE `ID`=24528;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24528 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24528);
UPDATE `quest_offer_reward` SET `RewardText`='There, ya did it. The Light favors quick learners like you.' WHERE `ID`=24528;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Steady Shot, then use Steady Shot 5 times on the Training Dummies outside the entrance to Anvilmar.' WHERE `ID`=24530;
UPDATE `quest_template` SET `QuestDescription`='The life of a hunter is steeped in tradition, ye know. Trackin\' mighty beasts through rugged terrain. Drinkin\'. Travelin\' the land, yer gun at yer side. Drinkin\'. Survivin\' on wits and courage and cunning, followed by a good stiff drink.$B$BBut before we get to the good stuff, though, ye\'ll need to learn how to do the beginner stuff. Go and get some more experience, and ye\'ll figure out how to do more things. Then ye\' drink.' WHERE `ID`=24530;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24530 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24530);
UPDATE `quest_offer_reward` SET `RewardText`='Now that ye\'ve got a new $c trick under yer belt, go out there an\' track some beasts!' WHERE `ID`=24530;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Eviscerate, then use Eviscerate 3 times on the Training Dummies near the entrance to Anvilmar.' WHERE `ID`=24532;
UPDATE `quest_template` SET `QuestDescription`='When you get to be more experienced, you\'ll start to find you can take advantage of new abilities. One of the things that you\'ll learn is the ability to use combat maneuvers to weaken an opponent, and then hit them with a devastating technique. Go and get some experience, figure out how to do it, and then try it out on some of the dummies here.' WHERE `ID`=24532;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Shadow Word: Pain, then cast it 5 times on training dummies.' WHERE `ID`=24533;
UPDATE `quest_template` SET `QuestDescription`='Now, if ya DO want to become somethin\' special, ye\'ll need to talk to me. A priest is nothin\' without his spells. And how do you learn new spells? Ya go and get more experience. So go out, get stronger, and then come show me what ya learn.' WHERE `ID`=24533;
UPDATE `quest_template` SET `LogDescription`='Reach Level 3 to learn Primal Strike, and then use Primal Strike 2 times on a Training Dummy near the entrance of Anvilmar.' WHERE `ID`=24527;
UPDATE `quest_template` SET `QuestDescription`='Excellent! Let us begin.$B$BAs you grow along your journey, you will unlocked the ancient secrets of shamans. Primal Strike is the first such skill. The primal spirits, when called upon, will strengthen your arm to deliver a mighty blow.$B$BHelpful, isn\'t it?' WHERE `ID`=24527;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24527 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24527);
UPDATE `quest_offer_reward` SET `RewardText`='This has been but one of many footfalls on the long path of the $c. Come back to me after you\'ve had some experience in the field, and i will help guide you on your next step.' WHERE `ID`=24527;
UPDATE `quest_template` SET `LogTitle`='Filling Up the Spellbook' WHERE `ID`=24526;
UPDATE `quest_template` SET `QuestDescription`='Excellent! Let\'s get started.$B$BAs you grow in power, your connection to the arcane arts will deepen, and you\'ll gain access to new spells and abilities. Go, get some more experience, and you\'ll see what I mean. Come back when you learn something new and demonstrate it on the training dummies here.' WHERE `ID`=24526;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24526 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24526);
UPDATE `quest_offer_reward` SET `RewardText`='There we go! One spell down, many many more to go. Come back to me after you\'ve had some experience casting this spell, and I\'ll show you some more tricks.' WHERE `ID`=24526;
UPDATE `quest_template` SET `LogTitle`='Corruption' WHERE `ID`=26904;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Corruption, then cast it 5 times on a Training Dummy near the entrance to Anvilmar.' WHERE `ID`=26904;
UPDATE `quest_template` SET `QuestDescription`='Few people, especially among our allies, truly understand the nature of the power we wield. They shun us for pursuing knowledge that they are too weak to use properly.$B$BAs you become more powerful, you will begin to discover new spells, further magnifying your power. Go, get more experience, then return once you\'ve learned a new spell. We\'ll see if you have it in you to truly master the dark arts.' WHERE `ID`=26904;

-- 2026_07_26_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_02.sql
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Charge. Look for a Training Dummy and practice using Charge.' WHERE `ID`=26945;
UPDATE `quest_template` SET `QuestDescription`='As you overcome the challenges before you, you\'ll begin to learn new techniques.$B$BGo, get more experience, and learn a new skill. Then demonstrate it for me on one of the training dummies.' WHERE `ID`=26945;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26945 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26945);
UPDATE `quest_offer_reward` SET `RewardText`='You\'ve proven a capable student. When it is time to resume your training, return to me and I will show you other techniques.' WHERE `ID`=26945;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Steady Shot. Look for a Training Dummy southeast of Aldrassil, and practice using Steady Shot 5 times.' WHERE `ID`=26947;
UPDATE `quest_template` SET `QuestDescription`='When most people think of the kaldorei, they conjure visions of druids and sentinels, but the way of the hunter is just as ancient and just as venerable.$B$BI\'ve been observing you, and you\'ve had quite a bit of experience handling that bow. As you gain more, you\'ll start to learn some new abilities.$B$BI want you to go out, get some more experience, and figure out how to do something new. Then go use it on a training dummy near the southeastern edge of Aldrassil.' WHERE `ID`=26947;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26947 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26947);
UPDATE `quest_offer_reward` SET `RewardText`='You are a quick study in our ways.' WHERE `ID`=26947;
UPDATE `quest_template` SET `LogDescription`='Reach Level 3 and Learn Eviscerate. Look for a Training Dummy southeast of Aldrassil and practice using Eviscerate 3 times.' WHERE `ID`=26946;
UPDATE `quest_template` SET `QuestDescription`='I\'m certain that by now, you\'ve mastered using Sinister Strike against your enemies. But, as you learn more and advance as a rogue, you\'ll start to figure out new techniques. One you should figure out soon is called Eviscerate. It can be a strong way to end a combination of attacks.$B$BI want you to get some more experience, figure it out, then try it on a training dummy near the southeastern edge of Aldrassil.' WHERE `ID`=26946;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26946 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26946);
UPDATE `quest_offer_reward` SET `RewardText`='You\'ve learned quickly. If you are as sharp as you semm to be, you may prove to be one of my best students yet.' WHERE `ID`=26946;
UPDATE `quest_template` SET `LogTitle`='Learning the Word' WHERE `ID`=26949;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Shadow Word: Pain, then use it 5 times on training dummies.' WHERE `ID`=26949;
UPDATE `quest_template` SET `QuestDescription`='As you venture out into the world and develop your skills, you will begin to learn new abilities that you never knew you had within you. I\'d like you to go out, get some more experience, and then show me the first new ability you learn.' WHERE `ID`=26949;
UPDATE `quest_template` SET `LogTitle`='Frost Nova' WHERE `ID`=26940;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Frost Nova. Look for a Training Dummy near, and practice using it twice.' WHERE `ID`=26940;
UPDATE `quest_template` SET `QuestDescription`='We are the heirs to a legacy of arcane magic and your path will not be an easy one. Many still hold us responsible for the actions of our forebears, but our talents are needed.$B$BThough you must be cautious, it is important to develop your powers, as that will deepen your connection to the arcane and grant you access to new spells. Go - get more experience with your spells, and demonstrate what you learn for me.' WHERE `ID`=26940;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26940 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26940);
UPDATE `quest_offer_reward` SET `RewardText`='I knew you would master this lesson quickly. You are a credit to the highborn and to all our kin. Remember, as you grow in power, return to me and I will instruct you further.' WHERE `ID`=26940;
UPDATE `quest_template` SET `LogTitle`='Moonfire' WHERE `ID`=26948;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Moonfire. Use it on a training dummy in Aldrassil.' WHERE `ID`=26948;
UPDATE `quest_template` SET `QuestDescription`='Druids play an important role in our society, but all too often we are seen as passive dreamers in a world that is changing around us.$B$BThe truth is, though, we must actively practice and go out onto the world to develop our connection to nature. So go, get some more experience. You will begin to feel your connection deepen, learning new skills. Demonstrate the first you learn for me.' WHERE `ID`=26948;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26948 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26948);
UPDATE `quest_offer_reward` SET `RewardText`='You have mastered the basics of the druidic ways quickly, $n. I have no doubt that your skills will often come in hand. Return to me when you need further training.' WHERE `ID`=26948;

-- 2026_07_26_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_03.sql
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Charge. Locate a Training Dummy in Camp Narache and practice using Charge.' WHERE `ID`=27020;
UPDATE `quest_template` SET `QuestDescription`='You have done well. As you continue along your path, you will grow, learning new techniques that you couldn\'t have imagined before. Your first lesson is the power of experience. Go, learn a new ability, and demonstrate it for me.' WHERE `ID`=27020;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=27020 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=27020);
UPDATE `quest_offer_reward` SET `RewardText`='You have done well, $n. Return to me when you are in need of further training.' WHERE `ID`=27020;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Seal of Command. Locate a Training Dummy in Camp Narache, cast Seal of Command on yourself and hit the dummy.' WHERE `ID`=27023;
UPDATE `quest_template` SET `QuestDescription`='It is always a pleasure to meet a fellow Sunwalker. We are not as numerous as some of the more ancient orders, but our ways are no less powerful and our members no less respected.$B$BAs you continue on your journey, you will grow closer to the light and gain abilities you couldn\'t have imagined. Go, get more experience, and return when you\'ve learned a new ability. Demonstrate it to me.' WHERE `ID`=27023;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Steady Shot. Locate a Training Dummy in Camp Narache and practice using Steady Shot 5 times.' WHERE `ID`=27021;
UPDATE `quest_template` SET `QuestDescription`='It is time for your first lesson, $n. You have a long way to go to master the ways of the hunter, but as you complete more of your journey, you begin to take new steps on that path.$B$BGo, get more experience, and figure out a new ability. Then, use it on the training dummies here.' WHERE `ID`=27021;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=27021 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=27021);
UPDATE `quest_offer_reward` SET `RewardText`='You have done well, $n.' WHERE `ID`=27021;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=27014 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=27014);
UPDATE `quest_offer_reward` SET `RewardText`='You have arrived at last. During your training in Camp Narache, I will guide you in the ways of the $c. You have much to learn and I look forward to counting you among my students.' WHERE `ID`=27014;
UPDATE `quest_template` SET `LogTitle`='Learning the Word' WHERE `ID`=27066;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Shadow Word: Pain. Practice using it 5 times on Training Dummies in Camp Narache.' WHERE `ID`=27066;
UPDATE `quest_template` SET `QuestDescription`='As you become more skilled, your skills will expand, and you will gain access to many new powers. Go - get some experience out there, and come back to me when you\'ve learned something new. You can demonstrate on the training dummies nearby.' WHERE `ID`=27066;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Primal Strike, and then locate a Training Dummy in Camp Narache and practice using Primal Strike 3 times.' WHERE `ID`=27027;
UPDATE `quest_template` SET `QuestDescription`='I know you\'ve been busy helping the braves protect Camp Narache from the bristlebacks. We\'re going to need all the hammers and all the luck we can get. As your skills increase, so does your connection to the elements. This gives you access to new abilities. The first of these that we learn is Primal Strike. It should help you fell your enemies faster. Learn it, and then try it out.' WHERE `ID`=27027;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=27027 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=27027);
UPDATE `quest_offer_reward` SET `RewardText`='Well done, $n. I see you will have no trouble mastering our more advanced arts as you gain experience. Return to me when you need further training.' WHERE `ID`=27027;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31165 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31165);
UPDATE `quest_offer_reward` SET `RewardText`='I am honored that Chief Hawkwind would allow me to train you and grant you your rite of passage. While I am a master in the ways of the $n, I am but a child in the ways of your people.' WHERE `ID`=31165;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31165 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=31165);
UPDATE `quest_request_items` SET `CompletionText`='Ah, I am pleased to meet you.' WHERE `ID`=31165;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31166 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31166);
UPDATE `quest_offer_reward` SET `RewardText`='You have done well, $n.' WHERE `ID`=31166;
UPDATE `quest_template` SET `LogTitle`='Moonfire' WHERE `ID`=27067;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Moonfire. Practice using it on a Training Dummy in Camp Narache.' WHERE `ID`=27067;
UPDATE `quest_template` SET `QuestDescription`='As you develop your skills, druid, you will begin to feel your connection to the Earthmother deepening. You will begin to understand new secrets of nature, and learn to cast new spells. Go now, get more experience, and return to me when you\'ve learned a new spell. Demonstrate what you\'ve learned.' WHERE `ID`=27067;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=27067 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=27067);
UPDATE `quest_offer_reward` SET `RewardText`='You have a knack for the druidic arts, $n. Wherever you go, your skills will be in demand. Return to me when you are ready for more training.' WHERE `ID`=27067;

-- 2026_07_26_world_04.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_04.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26203 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26203);
UPDATE `quest_offer_reward` SET `RewardText`='For a $r who has spent so much time inside Gnomeregan, you look positively fit and healthy, $n. That\'s good. Before we can complete what Operation: Gnomeregan began, we\'re going to need more warriors like you.' WHERE `ID`=26203;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Charge. Locate a Training Dummy in New Tinkertown and use Charge on it.' WHERE `ID`=26204;
UPDATE `quest_template` SET `QuestDescription`='Learning is the best way to improve. As you learn more, you\'ll start to discover you can do things you never realized!$B$BGo, get some more experience, and learn a new technique. Then, come back and demonstrate it on a training dummy.' WHERE `ID`=26204;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26204 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26204);
UPDATE `quest_offer_reward` SET `RewardText`='Good work! Keep practicing your charge and you\'ll have an instant advantage at the start of battle.' WHERE `ID`=26204;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26206 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26206);
UPDATE `quest_offer_reward` SET `RewardText`='It\'s good to meet you, $n. Nevin tells me that you managed to survive the dangers in Gnomeregan by using your wits. That\'s exactly what we need up here. Clever minds and quick blades.' WHERE `ID`=26206;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Eviscerate, then find a training dummy in New Tinkertown and practice using Eviscerate 3 times.' WHERE `ID`=26207;
UPDATE `quest_template` SET `QuestDescription`='As you learn more and become a better rogue, you\'ll start to learn new abilities.$B$BOne of the most potent ones is to chain together moves in a combination, exploiting your enemy\'s weakness and opening up the opportunity to use stronger maneuvers. Go, get some more experience and figure out how to do it. Once you can, come back here and try it out.' WHERE `ID`=26207;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26207 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26207);
UPDATE `quest_offer_reward` SET `RewardText`='I\'m not surprised to see you\'re such a quick study. I look forward to showing you more of our tricks of the trade.' WHERE `ID`=26207;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26199 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26199);
UPDATE `quest_offer_reward` SET `RewardText`='It\'s good to meet you, $n. I\'m sure your time in Gnomeregan afforded you ample opportunity to practice your arts. Things are somewhat safer here, but your talents will never go to waste.' WHERE `ID`=26199;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Shadow Word: Pain. Use it 5 times on training dummies in New Tinkertown.' WHERE `ID`=26200;
UPDATE `quest_template` SET `QuestDescription`='You know, as you go out into this crazy world and get more powerful, you\'ll also start to discover new powers that you didn\'t know before.$B$BWhy don\'t you go out, get some more experience, and then come back and show me what you\'ve learned!' WHERE `ID`=26200;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26200 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26200);
UPDATE `quest_offer_reward` SET `RewardText`='Good work, $n. You have a natural talent for our arts.' WHERE `ID`=26200;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26197 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26197);
UPDATE `quest_offer_reward` SET `RewardText`='$n, isn\'t it? It\'s a pleasure to finally meet you. While you were getting cleaned up, Nevin\'s team told me about your escape from Gnomeregan.' WHERE `ID`=26197;
UPDATE `quest_template` SET `LogTitle`='The Arts of a Mage' WHERE `ID`=26198;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26198 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26198);
UPDATE `quest_offer_reward` SET `RewardText`='You picked that one up quickly. I can\'t wait to show you some of my other favorite tricks.' WHERE `ID`=26198;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Corruption, then cast it on a Training Dummy in New Tinkertown 5 times.' WHERE `ID`=26201;
UPDATE `quest_template` SET `QuestDescription`='I\'m not at all surprised that a warlock like you fought your way out of Gnomeregan. Few people, even among the gnomes, truly understand the nature of the power we wield. As you become more powerful, you\'re going to start hearing the fel voices revealing their secrets to you, teaching you new spells. I want you to go get some more experience. Once you learn something new, show me.' WHERE `ID`=26201;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26201 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26201);
UPDATE `quest_offer_reward` SET `RewardText`='As you continue to gain power, return to me and I will teach you new spells and techniques.' WHERE `ID`=26201;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31135 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31135);
UPDATE `quest_offer_reward` SET `RewardText`='Why, you look great! I\'ve heard bad things about what happens down there.$B$BWell, are you ready to start your training in the ways of the $c?' WHERE `ID`=31135;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31138 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31138);
UPDATE `quest_offer_reward` SET `RewardText`='You picked that one up quickly. I can\'t wait to show you some of my other favorite tricks.' WHERE `ID`=31138;

-- 2026_07_26_world_07.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_07.sql
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Charge. Practice using Charge against a Tiki Target.' WHERE `ID`=24640;
UPDATE `quest_template` SET `QuestDescription`='As ya get bettah\' at fightin\', you\'ll start to find new tings that ya nevah knew you could do.$B$BWanna see? Go, train up some, and learn someting new.$B$B\'Den come back and ya can show me.' WHERE `ID`=24640;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24640 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24640);
UPDATE `quest_offer_reward` SET `RewardText`='Good work, mon! Dis stuff come easy to ya.' WHERE `ID`=24640;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Steady Shot. Use Steady Shot 5 times on a Tiki Target.' WHERE `ID`=24778;
UPDATE `quest_template` SET `QuestDescription`='As ya\' start to get da hang of tings, ya gonna figure out how ta do new tricks.$B$BOne of da\' first ya gonna learn is to keep a steady hand between shots, or else ya gonna find yaself exhausted and not really managin\' to kill much.$B$BGo get some more experience \'till ya figure it out, \'den come back and try it out. Let me know when ya tink you have it down.' WHERE `ID`=24778;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Eviscerate, then use Eviscerate 3 times against a Tiki Target.' WHERE `ID`=24772;
UPDATE `quest_template` SET `QuestDescription`='As ya fight ya enemies, ya will learn new tricks you use ta be even bettah\'. There\'s one ya should be learnin\' soon to let ya see dere weaknesses and set up for devastating combo attacks. After ya\'ve struck a few blows to weaken ya foe, finish dem off with a vicious evisceration attack. When ya learn it, try it out. Let me know when ya tink you have it down.' WHERE `ID`=24772;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24772 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24772);
UPDATE `quest_offer_reward` SET `RewardText`='Good work, mon! Dis stuff come easy to ya.' WHERE `ID`=24772;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Shadow Word: Pain. Use it 5 times on 5 Tiki training dummies.' WHERE `ID`=24784;
UPDATE `quest_template` SET `QuestDescription`='As ya get more skill out there in tha\' world, you\'ll start to discovah\' new powahs.$B$BWhy dontcha go out an\' go get yaself stronger, and come back when ya learn sometin\' new. You can show me what ya learn.' WHERE `ID`=24784;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24784 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24784);
UPDATE `quest_offer_reward` SET `RewardText`='Good work, mon! Dis stuff come easy to ya.' WHERE `ID`=24784;
UPDATE `quest_template` SET `LogDescription`='Reach Level 3 to learn Primal Strike, and then use Primal Strike 3 times against a Tiki Target.' WHERE `ID`=24760;
UPDATE `quest_template` SET `QuestDescription`='As ya grow in powah, you begin ta discovah the voices of da ancestahs speakin\' to ya, teachin\' you dere secrets. Tha first of \'dese is to focus tha power of da elements into yer weapon. It allows ya to unleash a quick, vicious attack on your enemies. Dis should be easy enough for ya to make a lot of use of it.$B$BTry it out. Let me know when ya tink you have it down.' WHERE `ID`=24760;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24760 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24760);
UPDATE `quest_offer_reward` SET `RewardText`='Good work, mon! Dis stuff come easy to ya.' WHERE `ID`=24760;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31162 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31162);
UPDATE `quest_offer_reward` SET `RewardText`='Good work, mon! Dis stuff come easy to ya.' WHERE `ID`=31162;
UPDATE `quest_template` SET `LogDescription`='Reach level 3 to learn Moonfire. Practice using it on a Tiki target in the Darkspear Training Grounds.' WHERE `ID`=24766;
UPDATE `quest_template` SET `QuestDescription`='As ya grow in powah, ya ability to call on nature gets strongah, too. Ya wanna see? Go get some experience and show me what ya\' learn.' WHERE `ID`=24766;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24766 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24766);
UPDATE `quest_offer_reward` SET `RewardText`='Good work, mon! Dis stuff come easy to ya.' WHERE `ID`=24766;

-- 2026_07_26_world_09.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_26_world_09.sql
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24473 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=24473);
UPDATE `quest_template_addon` SET `PrevQuestID`=24470 WHERE `ID`=24473;
UPDATE `quest_template` SET `RewardNextQuest`=24474 WHERE `ID`=24473;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24487 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=24487);
UPDATE `quest_template_addon` SET `PrevQuestID`=24486 WHERE `ID`=24487;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=218 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=218);
UPDATE `quest_template_addon` SET `PrevQuestID`=24487 WHERE `ID`=218;

-- 2026_07_28_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_00.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30072 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=30072);
UPDATE `quest_request_items` SET `CompletionText`='Have you found all of the items? Did you pay close attention to our process?' WHERE `ID`=30072;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=30072 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=30072);
UPDATE `quest_offer_reward` SET `RewardText`='Superb specimens. I will see to it they find their way to the right place. Please enjoy this free sample.' WHERE `ID`=30072;

-- 2026_07_28_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_01.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26818 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26818);
UPDATE `quest_offer_reward` SET `RewardText`='<Linzi feels the hides and smiles to herself.>$B$BThese are so nice and cushy!  I think my customers are going to LOVE them!' WHERE `ID`=26818;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26434 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26434);
UPDATE `quest_request_items` SET `CompletionText`='Make sure the reagents are extra-cushy, $n.  A scratchy reagent simply won\'t do.' WHERE `ID`=26434;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26434 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26434);
UPDATE `quest_offer_reward` SET `RewardText`='<Flem feels the hides and smiles to himself.>$B$BExcellent work, $n!  These reagents will serve perfectly.' WHERE `ID`=26434;

-- 2026_07_28_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_02.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26816 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26816);
UPDATE `quest_offer_reward` SET `RewardText`='This is fascinating! Just think of all the applications!$B$BThanks again, $n. Remember, if you ever need anything mixed for you... and I mean ANYTHING... you can always ask The Flask.' WHERE `ID`=26816;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26535 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26535);
UPDATE `quest_request_items` SET `CompletionText`='Did he use shimmerweed? I\'m guessing he used shimmerweed$B$BNo wait... amberseeds!  It\'s gotta be amberseeds.' WHERE `ID`=26535;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26535 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26535);
UPDATE `quest_offer_reward` SET `RewardText`='This is fascinating! Just think of all the applications!$B$BJust so we\'re clear, I\'m not thinking about using this to make an army of slaves or anything like that.  No, no... I\'m much more interested in the ogre explosions!' WHERE `ID`=26535;

-- 2026_07_28_world_07.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_28_world_07.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=29581 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=29581);
UPDATE `quest_request_items` SET `CompletionText`='Yes, did you need something?' WHERE `ID`=29581;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=29581 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=29581);
UPDATE `quest_offer_reward` SET `RewardText`='Ah... dreamleaf! So you want to see the Golden Dream, do you?$B$BCome to think of it, I haven\'t seen the Dream in years - not since I was a cub spending long hours playing in the pools. Long before I took up the culinary arts.$B$BLooks like you brought enough dreamleaf for two cups of tea... so why not?$B$BHere\'s to better days!' WHERE `ID`=29581;

-- 2026_07_29_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_02.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31137 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31137);
UPDATE `quest_offer_reward` SET `RewardText`='It\'s good to meet you, $n. You\'ve probably heard others speaking of Operation: Gnomeregan. Let me tell you a bit about what happened and why we left the dwarven city of Ironforge.' WHERE `ID`=31137;

-- 2026_07_29_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_03.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26566 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26566);
UPDATE `quest_offer_reward` SET `RewardText`='So the high tinker himself has heard of my little project? Splendid! I\'ve almost finished my latest prototype, but maybe you can help me chase down those last few parts.' WHERE `ID`=26566;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26222 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26222);
UPDATE `quest_request_items` SET `CompletionText`='Did you get those parts?' WHERE `ID`=26222;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26222 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26222);
UPDATE `quest_offer_reward` SET `RewardText`='These are fantastic! Let\'s see what we can put together. Would you be interested in testing out the prototype once I\'m done?' WHERE `ID`=26222;

-- 2026_07_29_world_04.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_04.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26316 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26316);
UPDATE `quest_offer_reward` SET `RewardText`='It won\'t be tough to collapse th\' tunnel, but I\'m goin\' to need my team an\' my equipment \'afore I can get to th\' task.' WHERE `ID`=26316;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26285 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26285);
UPDATE `quest_request_items` SET `CompletionText`='Well, did you recover me missing supplies?' WHERE `ID`=26285;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26285 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26285);
UPDATE `quest_offer_reward` SET `RewardText`='Well, now, that\'s more like it. We may get around to blowin\' up somethin\' after all.' WHERE `ID`=26285;

-- 2026_07_29_world_05.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_05.sql
UPDATE `gameobject_template` SET `AIName`='SmartGameObjectAI' WHERE `entry`=204019;
DELETE FROM `smart_scripts` WHERE `entryorguid`=204019 AND `source_type`=1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(204019, 1, 0, 0, 70, 0, 100, 0, 2, 0, 0, 0, 0, 80, 20401900, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Makeshift Cage - Activated - Run Script');
DELETE FROM `smart_scripts` WHERE `entryorguid`=20401900 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(20401900, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 45, 1, 1, 0, 0, 0, 0, 11, 42645, 5, 0, 0, 0, 0, 0, 'Makeshift Cage - On Script - Set Data to Captured Demolitionist'),
(20401900, 9, 1, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 0, 33, 42645, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Makeshift Cage - On Script - Give Credit'),
(20401900, 9, 2, 0, 0, 0, 100, 0, 60000, 60000, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Makeshift Cage - On Script - Reset Gameobject');
DELETE FROM `creature_text` WHERE `entry`=42645;
INSERT INTO `creature_text` (`entry`, `CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(42645, 42645, 0, 0, 'I\'m free! I\'m really free!', 12, 0, 100, 0, 0, 0, 'Captured Demolitionist - SAY_0'),
(42645, 42645, 0, 1, 'Thank you for breaking me out of here!', 12, 0, 100, 0, 0, 0, 'Captured Demolitionist - SAY_0'),
(42645, 42645, 0, 2, 'Thanks. Now, let\'s blow up that cave!', 12, 0, 100, 0, 0, 0, 'Captured Demolitionist - SAY_0'),
(42645, 42645, 0, 3, 'I don\'t ever want to smell unwashed trogg again!', 12, 0, 100, 0, 0, 0, 'Captured Demolitionist - SAY_0'),
(42645, 42645, 0, 4, 'You have no idea how happy I am to see you!', 12, 0, 100, 0, 0, 0, 'Captured Demolitionist - SAY_0');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=42645;
DELETE FROM `smart_scripts` WHERE `entryorguid`=42645 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(42645, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 45, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Captured Demolitionist - On Spawn - Set Data 1 0'),
(42645, 0, 1, 0, 38, 0, 100, 0, 1, 1, 1000, 1000, 0, 80, 4264500, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Captured Demolitionist - Data Set - Say 1');
DELETE FROM `smart_scripts` WHERE `entryorguid`=4264500 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4264500, 9, 0, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Captured Demolitionist - On Script - SAY_0'),
(4264500, 9, 1, 0, 0, 0, 100, 0, 1000, 1000, 0, 0, 0, 114, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0, 'Captured Demolitionist - On Script - Move Forward'),
(4264500, 9, 2, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Captured Demolitionist - On Script - Despawn');
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26284 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26284);
UPDATE `quest_offer_reward` SET `RewardText`='You have me own thanks and th\' thanks of th\' men you rescued. I\'ve already had th\' boys start setting up the gear down inside the cave. It should be ready to go soon.' WHERE `ID`=26284;

-- 2026_07_29_world_06.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_06.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26329 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26329);
UPDATE `quest_request_items` SET `CompletionText`='Welcome back, $n.' WHERE `ID`=26329;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26329 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26329);
UPDATE `quest_offer_reward` SET `RewardText`='<The high tinker reads Jessup\'s report.>$B$BSplendid news! With the troggs taken care of, we should be able to turn our attention to Crushcog\'s troublemaking.' WHERE `ID`=26329;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26331 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26331);
UPDATE `quest_offer_reward` SET `RewardText`='That should set back Crushcog\'s plans a bit, but we can\'t rest until he is defeated and Chill Breeze Valley is secure.' WHERE `ID`=26331;

-- 2026_07_29_world_07.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_07.sql
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=42224;
DELETE FROM `smart_scripts` WHERE `entryorguid`=42224 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(42224, 0, 0, 0, 8, 0, 100, 0, 79751, 0, 0, 0, 0, 80, 4222400, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Repaired Mechano-Tank - On Spellhit - Run Script');
DELETE FROM `smart_scripts` WHERE `entryorguid`=4222400 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4222400, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 37, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Repaired Mechano-Tank - On Script - Die'),
(4222400, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 33, 42224, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Repaired Mechano-Tank - On Script - Give Credit');
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26333 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26333);
UPDATE `quest_offer_reward` SET `RewardText`='Mekkatorque will be pleased to hear that you\'ve ruined Crushcog\'s plans to use our old weapons for his followers. Without the mechano-tanks under his command, he\'ll be weak and vulnerable.' WHERE `ID`=26333;

-- 2026_07_29_world_08.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_29_world_08.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26339 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26339);
UPDATE `quest_offer_reward` SET `RewardText`='Ah, yes, Kelsey told me to expect you. There\'s still much to do before we can take on Crushcog.' WHERE `ID`=26339;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26373 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26373);
UPDATE `quest_offer_reward` SET `RewardText`='If Gnomeregan Covert Operations is recommending you, then you must have made quite an impression. Your help will be most welcome here, as we\'ve a wealth of enemies and few enough mountaineers to face them.' WHERE `ID`=26373;

-- 2026_07_30_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_30_world_00.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26208 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26208);
UPDATE `quest_offer_reward` SET `RewardText`='Even now, only the S.A.F.E. teams dare to venture far into the city to search for survivors. Meanwhile, Thermaplugg has sent one of his followers, Crushcog, to distract us and buy time for him to dig in.' WHERE `ID`=26208;

-- 2026_07_31_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_31_world_00.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=29580 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=29580);
UPDATE `quest_request_items` SET `CompletionText`='Were there any tools left?' WHERE `ID`=29580;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=29580 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=29580);
UPDATE `quest_offer_reward` SET `RewardText`='There are no hunters among us, but we\'ll fight to protect our loved ones.' WHERE `ID`=29580;

-- 2026_07_31_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_07_31_world_01.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=29585 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=29585);
UPDATE `quest_offer_reward` SET `RewardText`='A survivor came in moments ago and said the flames are beginning to die down.$B$BWe owe our very way of life to you, stranger. I cannot possibly reward you adequately, but please take this.' WHERE `ID`=29585;

-- 2026_08_01_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_01_world_00.sql
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25668 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=25668);
UPDATE `quest_template_addon` SET `PrevQuestID`=25724 WHERE `ID`=25668;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=313 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=313);
UPDATE `quest_template_addon` SET `PrevQuestID`=25724 WHERE `ID`=313;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25667 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=25667);
UPDATE `quest_template_addon` SET `PrevQuestID`=25724 WHERE `ID`=25667;
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=40940;
DELETE FROM `smart_scripts` WHERE `entryorguid`=40940 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(40940, 0, 0, 0, 0, 0, 100, 0, 8000, 10000, 10000, 12000, 0, 11, 3131, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Young Wendigo - In Combat - Cast Frost Breath');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=40941;
DELETE FROM `smart_scripts` WHERE `entryorguid`=40941 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(40941, 0, 0, 0, 0, 0, 100, 0, 8000, 10000, 10000, 12000, 0, 11, 3131, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Wendigo - In Combat - Cast Frost Breath');

-- 2026_08_01_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_01_world_01.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28563 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=28563);
UPDATE `quest_offer_reward` SET `RewardText`='Heard Westfall was in some deep doody.$B$B<Parker doesn\'t even turn to face you.>$B$BWell, we got our own set of problems here. See them gnolls out there? Looks like they\'re actin\' up again. I don\'t like it. Don\'t like it one bit.' WHERE `ID`=28563;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26503 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26503);
UPDATE `quest_request_items` SET `CompletionText`='Did you find the gnoll plans?' WHERE `ID`=26503;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26503 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26503);
UPDATE `quest_offer_reward` SET `RewardText`='I knew it! Looks like Yowler is behind this uprising - which is incredible, because we keep killing gnolls named Yowler. I don\'t know how many sons the original Yowler had, but it\'s got to be close to a hundred.$B$BWell, looks like we got ourselves another Yowler to kill.$B$BMagistrate Solomon must be notified.' WHERE `ID`=26503;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26505 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26505);
UPDATE `quest_request_items` SET `CompletionText`='Where did I put my monocle?! Conacher is that you?' WHERE `ID`=26505;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26505 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26505);
UPDATE `quest_offer_reward` SET `RewardText`='<Magistrate Solomon puts on his monocle.>$B$BOh, you\'re not Conacher. What have you got there?$B$B<Magistrate Solomon takes Parker\'s report from you.>$B$BOh for pity\'s sake. Yowler? AGAIN?' WHERE `ID`=26505;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26510 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26510);
UPDATE `quest_request_items` SET `CompletionText`='Have you recovered the gnomecorder?' WHERE `ID`=26510;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26510 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26510);
UPDATE `quest_offer_reward` SET `RewardText`='Excellent! Let me make a few adjustments here and we should be good to go.' WHERE `ID`=26510;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26508 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26508);
UPDATE `quest_request_items` SET `CompletionText`='Hi. I miss my necklace. My daddy got it for me. Daddy says that there are monsters in the lake. Did you beat up any monsters?' WHERE `ID`=26508;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26508 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26508);
UPDATE `quest_offer_reward` SET `RewardText`='Thank you for finding my necklace mister $c... you are very kind!  My kitty thanks you too - isn\'t that right Effsee?' WHERE `ID`=26508;

-- 2026_08_02_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_02.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26512 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26512);
UPDATE `quest_offer_reward` SET `RewardText`='<The gnomecorder crackles and pops.>$B$BCan you hear me, $n? Is this thing on? Ah, yes, I see you there now.$B$BLet\'s get to work!' WHERE `ID`=26512;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26514 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26514);
UPDATE `quest_request_items` SET `CompletionText`='Did you recover those collars?' WHERE `ID`=26514;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26514 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26514);
UPDATE `quest_offer_reward` SET `RewardText`='<The gnomecorder crackles and pops.>$B$BGood work, $n. I\'ve just been handed some important information. It would appear that our most hated enemy, the Blackrock orcs, have wised up...' WHERE `ID`=26514;

-- 2026_08_02_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_03.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26544 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26544);
UPDATE `quest_request_items` SET `CompletionText`='What have you found out?' WHERE `ID`=26544;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26544 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26544);
UPDATE `quest_offer_reward` SET `RewardText`='<The gnomecorder buzzes and whirs.>$B$BJust put the missive in the little compartment and I\'ll read it. I am fluent in orcish.$B$B<You hear a distraught yell in the background.>$B$BIt looks like these orcs are here under the direct command of Yowler. That means Yowler must have the orc invasion plan.' WHERE `ID`=26544;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26545 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26545);
UPDATE `quest_request_items` SET `CompletionText`='Do you have the Blackrock invasion plans?' WHERE `ID`=26545;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26545 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26545);
UPDATE `quest_offer_reward` SET `RewardText`='<Magistrate Solomon takes the plans from you and begins reading.>$B$BShadowhide army!? Damn those orcs to hell! These invasion plans speak of a massive build up of orcish and gnoll forces in the east. Looks like Gath\'Ilzogg, the Blackrock general, is preparing to march his armies across Lakeshire and attack Stormwind directly!' WHERE `ID`=26545;

-- 2026_08_02_world_04.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_04.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26506 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26506);
UPDATE `quest_request_items` SET `CompletionText`='Have you gotten my ingredients?' WHERE `ID`=26506;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26506 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26506);
UPDATE `quest_offer_reward` SET `RewardText`='PERFECT! I\'ll put these in the pot right away. Dinner should be ready in a few hours.$B$BThank you, darling!' WHERE `ID`=26506;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26511 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26511);
UPDATE `quest_offer_reward` SET `RewardText`='That ought to teach those murlocs a lesson. Hopefully the next time they decide to raid our town they\'ll think twice.$B$BWe both know that won\'t happen.' WHERE `ID`=26511;

-- 2026_08_02_world_05.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_05.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26513 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26513);
UPDATE `quest_request_items` SET `CompletionText`='Did you recover the supplies?' WHERE `ID`=26513;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26513 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26513);
UPDATE `quest_offer_reward` SET `RewardText`='Thank you, citizen. Without you, we would have starved or worse, been forced to go out and get food for ourselves.' WHERE `ID`=26513;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26570 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26570);
UPDATE `quest_offer_reward` SET `RewardText`='You\'ve probably bought us another day, maybe two. Nice work, $n.' WHERE `ID`=26570;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26728 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26728);
UPDATE `quest_offer_reward` SET `RewardText`='You are the help that we requested?$B$B<Althea sighs.>$B$BI suppose you will have to do.' WHERE `ID`=26728;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28564 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=28564);
UPDATE `quest_offer_reward` SET `RewardText`='You are the help that we requested?$B$B<Althea sighs.>$B$BI suppose you will have to do.' WHERE `ID`=28564;

-- 2026_08_02_world_06.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_02_world_06.sql
UPDATE `quest_template_addon` SET `PrevQuestID`=24489 WHERE `ID`=218;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24493 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=24493);
UPDATE `quest_template_addon` SET `PrevQuestID`=24491 WHERE `ID`=24493;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24532 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24532);
UPDATE `quest_offer_reward` SET `RewardText`='You\'ve got the hang of it.  Go get some practice out there... maybe take out a few of those troggs or trolls.  Or rabbits, I don\'t care.$B$BReturn after you\'ve gotten some practice, and I\'ll show you another trick of the trade.' WHERE `ID`=24532;

-- 2026_08_03_world_11.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_03_world_11.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26618 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26618);
UPDATE `quest_offer_reward` SET `RewardText`='Don\'t go thinking that was too easy. Far greater dangers lurk deeper in the woods.' WHERE `ID`=26618;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26645 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26645);
UPDATE `quest_offer_reward` SET `RewardText`='Splendid, $n. For your service to the people of Darkshire you shall be rewarded.' WHERE `ID`=26645;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26686 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26686);
UPDATE `quest_offer_reward` SET `RewardText`='The people of Darkshire thank you, $n. You have proven yourself to be a great ally of The Night Watch.' WHERE `ID`=26686;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26688 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26688);
UPDATE `quest_offer_reward` SET `RewardText`='Impressive, $n. It would seem that you are capable enough to handle yourself. Perhaps a more suitable challenge could be found for one of your abilities.' WHERE `ID`=26688;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26689 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26689);
UPDATE `quest_offer_reward` SET `RewardText`='You performed well against the Shadow Weavers, $n. But even now, years after their first arrival, there seem to be so many to replace the ones we kill.$B$BI will put my faith in Master Carevin. No doubt he will get to the bottom of the problem.' WHERE `ID`=26689;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26690 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26690);
UPDATE `quest_offer_reward` SET `RewardText`='$n, to be honest with you, I did not believe that you would get this far, but you are clearly a $c to be reckoned with. In fact, if you wish to formally join Master Carevin\'s struggle, I will gladly write for you a letter of recommendation.' WHERE `ID`=26690;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26691 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26691);
UPDATE `quest_offer_reward` SET `RewardText`='Excuse me for my cold reception, $n, but as I\'m sure you can tell, I am an extremely busy man. I see that you have impressed Calor--and I assure you, that is no small feat--and that he has given his recommendation.$B$BThere are unsavory types afoot, $n, and we can use the help of all who have proven themselves. We battle against demons, the undead, and those who would provide them aid. Be vigilant, be wary, and trust none who would not give aid to our cause.$B$BGlory under the Light.' WHERE `ID`=26691;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26620 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26620);
UPDATE `quest_offer_reward` SET `RewardText`='Mmm-mmm! The skirtsteak\'s the best part. It\'s not an efficient way to cook, though.$B$BHere, have this recipe. It\'s a little different, but flank meat\'s easier to come by when you really need a meal.' WHERE `ID`=26620;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26623 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26623);
UPDATE `quest_offer_reward` SET `RewardText`='Ah yes, a nice lump you have there! Let me just get this seasoned with my secret spices (no looking!) and get them to skittering on a skillet for a while...$B$BAnd, although Dusky Crab Cakes are my specialty and I won\'t give out the recipe, here\'s the recipe for a dish that\'s almost as good.' WHERE `ID`=26623;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26627 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26627);
UPDATE `quest_offer_reward` SET `RewardText`='Ah, welcome, stranger. I\'d offer you something, but there\'s not much here...$B$BCould I beg you to run some errands for a poor old hermit?' WHERE `ID`=26627;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26653 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26653);
UPDATE `quest_offer_reward` SET `RewardText`='Ah, ghost hair thread is what you need, is it? I\'m afraid I have none in stock, but I can make some for you...if you can supply the ghost hair.' WHERE `ID`=26653;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26652 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26652);
UPDATE `quest_offer_reward` SET `RewardText`='What is this? A comb? It\'s lovely! And it glides through my hair as if it weren\'t the stiff, stringy horror that it is.$B$BOh, if only I had a mirror...' WHERE `ID`=26652;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26654 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26654);
UPDATE `quest_offer_reward` SET `RewardText`='I can make a spool of ghost hair thread with this, and have a few strands to spare. Here are some coins for those extra strands.' WHERE `ID`=26654;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26655 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26655);
UPDATE `quest_offer_reward` SET `RewardText`='Delightful! This will do splendidly...$B$BHere, good $n, take this as payment for your honorable deed.' WHERE `ID`=26655;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26660 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26660);
UPDATE `quest_offer_reward` SET `RewardText`='You need some Zombie Juice, do you? Hmm...that\'s some strong stuff - I don\'t usually get requests for it.' WHERE `ID`=26660;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26661 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26661);
UPDATE `quest_offer_reward` SET `RewardText`='Good, you got the rot blossoms.$B$BI\'ll whip up the zombie juice... it\'s strong stuff, I warn you.' WHERE `ID`=26661;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26661 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26661);
UPDATE `quest_request_items` SET `CompletionText`='Hi, $n. Do you have those rot blossoms for me yet?' WHERE `ID`=26661;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26676 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26676);
UPDATE `quest_offer_reward` SET `RewardText`='A thousand thanks, $n. You warm an old man\'s heart with your foolish-...I mean...with your kindness!$B$BHere you are, friend. Take this as a token of my gratitude.' WHERE `ID`=26676;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26707 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26707);
UPDATE `quest_offer_reward` SET `RewardText`='I thank you, $c... and I\'m sure the worgen that may be returned to sanity with the potion this will make would thank you too.' WHERE `ID`=26707;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26717 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26717);
UPDATE `quest_offer_reward` SET `RewardText`='Sounds like a close call! But he ran off without harming you further...$B$BHe might be still fighting it. Master Harris must know!' WHERE `ID`=26717;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25235 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=25235);
UPDATE `quest_offer_reward` SET `RewardText`='I saw you drop a few of them from here. Nice work.$B$BHere\'s your reward, on behalf of the Night Watch.' WHERE `ID`=25235;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26719 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26719);
UPDATE `quest_offer_reward` SET `RewardText`='Welcome to our humble camp. I wouldn\'t call it pleasant, but it\'s at least somewhat private.$B$BBeg your pardon? A worgen at the farm?' WHERE `ID`=26719;

-- 2026_08_04_world_18.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_18.sql
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=384 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=384);
UPDATE `quest_template_addon` SET `RequiredSkillID`=0 WHERE `ID`=384;
UPDATE `quest_template_addon` SET `RequiredSkillPoints`=0 WHERE `ID`=384;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=384 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=384);
UPDATE `quest_request_items` SET `CompletionText`='I\'m going to need those crag boar ribs and a mug of Rhapsody Malt, $n.' WHERE `ID`=384;

-- 2026_08_04_world_19.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_04_world_19.sql
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=415 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=415);
UPDATE `quest_template_addon` SET `NextQuestID`=0 WHERE `ID`=415;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=315 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=315);
UPDATE `quest_template_addon` SET `PrevQuestID`=0 WHERE `ID`=315;

-- 2026_08_05_world_11.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_11.sql
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25792 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=25792);
UPDATE `quest_template_addon` SET `PrevQuestID`=313 WHERE `ID`=25792;

-- 2026_08_05_world_12.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_12.sql
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=412 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=412);
UPDATE `quest_template_addon` SET `PrevQuestID`=313 WHERE `ID`=412;

-- 2026_08_05_world_14.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_05_world_14.sql
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25838 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=25838);
UPDATE `quest_template_addon` SET `PrevQuestID`=25792 WHERE `ID`=25838;

-- 2026_08_06_world_13.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_06_world_13.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28868 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=28868);
UPDATE `quest_offer_reward` SET `RewardText`='Well done, $n. That\'ll teach them not to mess with us in our home!' WHERE `ID`=28868;

-- 2026_08_07_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_00.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25028 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=25028);
UPDATE `quest_request_items` SET `CompletionText`='I am very busy, $c. This uncontrolled growth threatens to engulf the continent. Do you have any news?' WHERE `ID`=25028;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25028 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=25028);
UPDATE `quest_offer_reward` SET `RewardText`='Ah, samples from different parts of the overgrowth? These will be immensely helpful, yes. Thank you, $c.' WHERE `ID`=25028;

-- 2026_08_07_world_09.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_09.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24938 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24938);
UPDATE `quest_offer_reward` SET `RewardText`='Aubrey sent you? Good! I take it you know your way around a cannon?' WHERE `ID`=24938;

-- 2026_08_07_world_10.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_10.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24941 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=24941);
UPDATE `quest_request_items` SET `CompletionText`='Paxton has already loaded a charge into the cannon. All it needs is the ammunition you improvised.' WHERE `ID`=24941;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24941 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24941);
UPDATE `quest_offer_reward` SET `RewardText`='The Horde is massing at the base of the hill. Breathlessly, you stuff the debris you collected into the muzzle of the cannon, step back, cover one ear, and tug on the igniter cord...' WHERE `ID`=24941;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24944 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=24944);
UPDATE `quest_template_addon` SET `PrevQuestID`=24941 WHERE `ID`=24944;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24944 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24944);
UPDATE `quest_offer_reward` SET `RewardText`='A steady stream of the walking wounded have come up the hill to see me, $n. I trust that was your work? Well done!' WHERE `ID`=24944;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24943 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=24943);
UPDATE `quest_template_addon` SET `PrevQuestID`=24941 WHERE `ID`=24943;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24943 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24943);
UPDATE `quest_offer_reward` SET `RewardText`='Great work! We\'ve knocked them on their heels, maybe long enough to shore up our defenses.$B$BProvided we find a way to take out their artillery...' WHERE `ID`=24943;

-- 2026_08_07_world_17.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_07_world_17.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31287 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31287);
UPDATE `quest_offer_reward` SET `RewardText`='The deed is done and a debt must be paid. Here is your reward. You have our gratitude.' WHERE `ID`=31287;

-- 2026_08_08_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_08_world_02.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31286 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=31286);
UPDATE `quest_request_items` SET `CompletionText`='Saurok are seldom this bold.' WHERE `ID`=31286;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=31286 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=31286);
UPDATE `quest_offer_reward` SET `RewardText`='Ah, our merchandise. Madam Goya will be pleased. You have the gratitude of her ladyship.' WHERE `ID`=31286;

-- 2026_08_09_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_09_world_00.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25197 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=25197);
UPDATE `quest_offer_reward` SET `RewardText`='You\'ve ventured all the way here from Honor\'s Stand? Excellent, we could use your experience here.$B$BThey may have breached the walls, but Northwatch isn\'t going down without a fight.' WHERE `ID`=25197;

-- 2026_08_09_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_09_world_01.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24934 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24934);
UPDATE `quest_offer_reward` SET `RewardText`='You make a hell of a marine, $n.$B$BWe\'ll mop up the others. But it looks like the Horde fleet is disengaging - maybe to land more troops ashore?$B$BAdmiral Aubrey will know what to do. Talk to me when you\'re ready to fly back!' WHERE `ID`=24934;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24938 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=24938);
UPDATE `quest_template_addon` SET `PrevQuestID`=24934 WHERE `ID`=24938;

-- 2026_08_10_world_00.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_10_world_00.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24948 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=24948);
UPDATE `quest_request_items` SET `CompletionText`='Burn it all, $c. Burn it to cinders!' WHERE `ID`=24948;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=24948 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=24948);
UPDATE `quest_offer_reward` SET `RewardText`='Good! Without heavy equipment, the Horde will have to pull back to regroup. We\'ve bought ourselves some time to re-secure the fortress. Well done!' WHERE `ID`=24948;

-- 2026_08_10_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_10_world_02.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25002 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=25002);
UPDATE `quest_request_items` SET `CompletionText`='What? My supply caravan was sacked by those cold-blooded scissor-paw things?' WHERE `ID`=25002;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25002 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=25002);
UPDATE `quest_offer_reward` SET `RewardText`='My supplies! Well, that showed some real initiative, $c. Those rap-saws or whatever they\'re called have torn lesser men to ribbons. Thank you!' WHERE `ID`=25002;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25036 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=25036);
UPDATE `quest_offer_reward` SET `RewardText`='So, Paxton says there\'s no jungle in the Barrens? What do you call THIS!?$B$BI\'ve got a pile of supplies to move to the interior and my camp has been overrun by a forest that - I swear - grows by the hour!' WHERE `ID`=25036;

-- 2026_08_10_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_10_world_03.sql
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25015 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=25015);
UPDATE `quest_template_addon` SET `PrevQuestID`=25036 WHERE `ID`=25015;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25015 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=25015);
UPDATE `quest_request_items` SET `CompletionText`='I should mention, you\'re probably going to need a machete to hack through this mess.$B$BThis jungle came out of nowhere!' WHERE `ID`=25015;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25015 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=25015);
UPDATE `quest_offer_reward` SET `RewardText`='<Corporal Teegan nods as you deliver your report, then grasps the Bramblestaff with both hands.>$B$BSo, this is what the quilboar are using against us? Do you think they used these to create this jungle?' WHERE `ID`=25015;

-- 2026_08_10_world_04.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_10_world_04.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25008 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=25008);
UPDATE `quest_offer_reward` SET `RewardText`='Ooooo-oooh aye! The best Hecklefangs are the dead ones.' WHERE `ID`=25008;

-- 2026_08_12_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_02.sql
DELETE FROM `creature_text` WHERE `entry`=808;
INSERT INTO `creature_text` (`entry`, `CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(808, 808, 0, 0, 'First da fire, and now da dwarf! You gonna die!', 12, 0, 100, 0, 0, 0, 'Grik\'nir the Cold - SAY_AGGRO');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=808;
DELETE FROM `smart_scripts` WHERE `entryorguid`=808 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(808, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Grik\'nir the Cold - On Aggro - Say 0'),
(808, 0, 1, 0, 0, 0, 100, 1, 4000, 5000, 9000, 10000, 0, 11, 6957, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Grik\'nir the Cold - In Combat - Cast Frostmane Strength'),
(808, 0, 2, 0, 0, 0, 100, 0, 2000, 4000, 7000, 9000, 0, 11, 79895, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Grik\'nir the Cold - In Combat - Cast Frost Strike');

-- 2026_08_12_world_05.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_05.sql
DELETE FROM `creature_text` WHERE `entry`=37112;
INSERT INTO `creature_text` (`entry`, `CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(37112, 37112, 0, 0, 'A gi kras tor shodru\'ga bromo!', 14, 12, 100, 0, 0, 0, 'Wayward Fire Elemental - SAY_0'),
(37112, 37112, 1, 0, 'T MA TORO NUK KRAST\'VEN FMERK', 12, 12, 100, 0, 0, 0, 'Wayward Fire Elemental - SAY_AGGRO');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=37112;
DELETE FROM `smart_scripts` WHERE `entryorguid`=37112 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(37112, 0, 0, 0, 4, 0, 100, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Wayward Fire Elemental - On Aggro - Say 1'),
(37112, 0, 1, 0, 1, 0, 100, 0, 60000, 60000, 60000, 60000, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Wayward Fire Elemental - Out of Combat - Say 0'),
(37112, 0, 2, 0, 0, 0, 100, 0, 4000, 6000, 9000, 11000, 0, 11, 9053, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Wayward Fire Elemental - In Combat - Cast Fireball');

-- 2026_08_12_world_06.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_06.sql
DELETE FROM `creature_text` WHERE `entry`=946;
INSERT INTO `creature_text` (`entry`, `CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(946, 946, 0, 0, 'Killing you be easy.', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO1'),
(946, 946, 0, 1, 'You be dead soon!', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO2'),
(946, 946, 0, 2, 'My weapon be thirsty!', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO3'),
(946, 946, 0, 3, 'I gonna make you into mojo!', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO4');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=946;
DELETE FROM `smart_scripts` WHERE `entryorguid`=946 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(946, 0, 0, 0, 1, 0, 100, 1, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Novice - Out Of Combat - Allow Combat Movement'),
(946, 0, 1, 2, 4, 0, 50, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Novice - On Aggro - Say 0'),
(946, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 11, 6949, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Novice - On Link - Cast Weak Frostbolt'),
(946, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Novice - On Link - Increment Phase'),
(946, 0, 4, 0, 9, 0, 100, 0, 0, 30, 4000, 5000, 0, 11, 6949, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Novice - At 0 - 30 Range - Cast Weak Frostbolt'),
(946, 0, 5, 6, 3, 0, 100, 0, 0, 15, 0, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Novice - At 15% Mana - Allow Combat Movement'),
(946, 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 23, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Novice - On Link - Increment Phase'),
(946, 0, 7, 0, 9, 0, 100, 1, 25, 80, 0, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Novice - At 25 - 80 Range - Allow Combat Movement'),
(946, 0, 8, 0, 9, 0, 100, 1, 5, 15, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Novice - At 5 - 15 Range - Allow Combat Movement'),
(946, 0, 9, 0, 9, 0, 100, 1, 0, 5, 0, 0, 0, 21, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Novice - At 5 - 15 Range - Allow Combat Movement'),
(946, 0, 10, 0, 3, 0, 100, 0, 30, 100, 100, 100, 0, 23, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Novice - At 100% Mana - Increment Phase');

-- 2026_08_12_world_07.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_07.sql
DELETE FROM `creature_text` WHERE `entry`=37507;
INSERT INTO `creature_text` (`entry`, `CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(37507, 37507, 0, 0, 'Killing you be easy.', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO1'),
(37507, 37507, 0, 1, 'You be dead soon!', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO2'),
(37507, 37507, 0, 2, 'My weapon be thirsty!', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO3'),
(37507, 37507, 0, 3, 'I gonna make you into mojo!', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO4');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=37507;
DELETE FROM `smart_scripts` WHERE `entryorguid`=37507 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(37507, 0, 0, 0, 4, 0, 50, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Blade - On Aggro - Say 0'),
(37507, 0, 1, 0, 9, 0, 100, 0, 0, 5, 15000, 20000, 0, 11, 70133, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Blade - Range 5 yards - Cast Poisoned Blade');

-- 2026_08_12_world_08.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_08.sql
DELETE FROM `creature_text` WHERE `entry`=706;
INSERT INTO `creature_text` (`entry`, `CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(706, 706, 0, 0, 'Killing you be easy.', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO1'),
(706, 706, 0, 1, 'You be dead soon!', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO2'),
(706, 706, 0, 2, 'My weapon be thirsty!', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO3'),
(706, 706, 0, 3, 'I gonna make you into mojo!', 12, 0, 80, 0, 0, 0, 'SAY_AGGRO4');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=706;
DELETE FROM `smart_scripts` WHERE `entryorguid`=706 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(706, 0, 0, 0, 4, 0, 50, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frostmane Troll Whelp - On Aggro - Say 0');

-- 2026_08_12_world_09.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_12_world_09.sql
DELETE FROM `creature_text` WHERE `entry`=41671;
INSERT INTO `creature_text` (`entry`, `CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(41671, 41671, 0, 0, 'You\'re the best thing I\'ve seen since the troggs showed up!', 12, 0, 100, 0, 0, 0, 'SAY_0_0'),
(41671, 41671, 0, 1, 'I thought I would die down here.', 12, 0, 100, 0, 0, 0, 'SAY_0_1'),
(41671, 41671, 0, 2, 'I thought they were going to tear me apart!', 12, 0, 100, 0, 0, 0, 'SAY_0_2'),
(41671, 41671, 0, 3, 'I can\'t wait to get home to my family. Thank you, stranger.', 12, 0, 100, 0, 0, 0, 'SAY_0_3');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=41671;
DELETE FROM `smart_scripts` WHERE `entryorguid`=41671 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(41671, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 17, 431, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Trapped Miner - On Respawn - Set Emote State'),
(41671, 0, 1, 0, 8, 0, 100, 0, 77819, 0, 0, 0, 0, 80, 4167100, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Trapped Miner - On Spellhit - Run Script');
DELETE FROM `smart_scripts` WHERE `entryorguid`=4167100 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4167100, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 66, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Trapped Miner - On Script - Set Orientation'),
(4167100, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Trapped Miner - On Script - Say 0'),
(4167100, 9, 2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 17, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Trapped Miner - On Script - Set Emote State'),
(4167100, 9, 3, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 0, 17, 26, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Trapped Miner - On Script - Set Emote State'),
(4167100, 9, 4, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 33, 41671, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Trapped Miner - On Script - Give Credit'),
(4167100, 9, 5, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 41, 5000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Trapped Miner - On Script - Despawn');
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=433 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=433);
UPDATE `quest_offer_reward` SET `RewardText`='What a relief! I\'m glad to see that you were able to save so many miners, $n. I will make sure that their families are notified immediately.$B$BThe Senate will know of your role in winning back the mines.' WHERE `ID`=433;

-- 2026_08_13_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_02.sql
DELETE FROM `creature_text` WHERE `entry`=41763;
INSERT INTO `creature_text` (`entry`, `CreatureID`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(41763, 41763, 0, 0, '%s gasps for air and shivers in the cold.', 16, 0, 100, 20, 0, 0, 'Frozen Mountaineer');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=41763;
DELETE FROM `smart_scripts` WHERE `entryorguid`=41763 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(41763, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 77906, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frozen Mountaineer - On Respawn - Summon'),
(41763, 0, 1, 0, 38, 0, 100, 0, 1, 1, 0, 0, 0, 80, 4176300, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frozen Mountaineer - On Data Set - Run Script');
DELETE FROM `smart_scripts` WHERE `entryorguid`=4176300 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4176300, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 28, 77910, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frozen Mountaineer - On Script - Remove Aura'),
(4176300, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frozen Mountaineer - On Script - Say 0'),
(4176300, 9, 2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 41, 5000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Frozen Mountaineer - On Script - Despawn');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=41768;
DELETE FROM `smart_scripts` WHERE `entryorguid`=41768 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(41768, 0, 0, 0, 6, 0, 100, 0, 0, 0, 0, 0, 0, 45, 1, 1, 0, 0, 0, 0, 11, 41763, 5, 0, 0, 0, 0, 0, 'Icy Tomb - On Death - Set Data'),
(41768, 0, 1, 0, 54, 0, 100, 0, 0, 0, 0, 0, 0, 80, 4176800, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Icy Tomb - On Summoned - Run Script');
DELETE FROM `smart_scripts` WHERE `entryorguid`=4176800 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4176800, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 45, 1, 0, 0, 0, 0, 0, 11, 41763, 5, 0, 0, 0, 0, 0, 'Icy Tomb - On Script - Set Data'),
(4176800, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 103, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Icy Tomb - On Script - Set Rooted'),
(4176800, 9, 2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Icy Tomb - On Script - Set React State');
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25978 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=25978);
UPDATE `quest_offer_reward` SET `RewardText`='You\'ve done it! The mountaineers are returning to camp and our medics will have their hands full. We are all in your debt, $n.' WHERE `ID`=25978;

-- 2026_08_13_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_03.sql
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=42046;
UPDATE `creature_template` SET `modelid1`=11686 WHERE `entry`=42046;
UPDATE `creature_template` SET `modelid2`=0 WHERE `entry`=42046;
DELETE FROM `smart_scripts` WHERE `entryorguid`=42046 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(42046, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 11, 78357, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Burning Bomber Credit - On Respawn - Cast Spell'),
(42046, 0, 1, 0, 8, 0, 100, 0, 78369, 0, 0, 0, 0, 80, 4204600, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Burning Bomber Credit - On Spellhit - Run Script');
DELETE FROM `smart_scripts` WHERE `entryorguid`=4204600 AND `source_type`=9;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(4204600, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 33, 42046, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Burning Bomber Credit - On Script - Give Credit'),
(4204600, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 'Burning Bomber Credit - On Script - Despawn');
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26078 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26078);
UPDATE `quest_offer_reward` SET `RewardText`='Well done! My men are working hard on getting our flying machines and steam tanks back into the fight.' WHERE `ID`=26078;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26078 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26078);
UPDATE `quest_request_items` SET `CompletionText`='We need to hurry and get those bombers airborne!' WHERE `ID`=26078;

-- 2026_08_13_world_04.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_13_world_04.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26112 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26112);
UPDATE `quest_request_items` SET `CompletionText`='What is the meaning of this intrusion?' WHERE `ID`=26112;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26112 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26112);
UPDATE `quest_offer_reward` SET `RewardText`='And just what is this?$B$B<Moira takes the report and the color drains from her face as she reads.>$B$BThis is impossible!' WHERE `ID`=26112;

-- 2026_08_14_world_01.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_01.sql
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25028 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=25028);
UPDATE `quest_template_addon` SET `PrevQuestID`=25015 WHERE `ID`=25028;
INSERT INTO `quest_template_addon` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=25034 AND NOT EXISTS (SELECT 1 FROM `quest_template_addon` WHERE `ID`=25034);
UPDATE `quest_template_addon` SET `PrevQuestID`=25027 WHERE `ID`=25034;

-- 2026_08_14_world_02.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_02.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26131 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26131);
UPDATE `quest_offer_reward` SET `RewardText`='You don\'t know how glad I am to see fresh faces from Ironforge. Perhaps sending Mountaineer Barleybrew wasn\'t a waste after all.' WHERE `ID`=26131;

-- 2026_08_14_world_03.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_03.sql
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=28567 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=28567);
UPDATE `quest_offer_reward` SET `RewardText`='Have our leaders finally heard my pleas? We\'re down to just a handful of pilots and mountaineers to defend Loch Modan.' WHERE `ID`=28567;

-- 2026_08_14_world_04.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_04.sql
UPDATE `quest_template` SET `QuestDescription`='My friend and fellow Siege Engine pilot, Mori Hildelve, is lost in the hills. We were looking for a rare ore that\'s needed for a high-grade blastpowder, and during our search he drove his engine up a steep hill and busted it!$B$BStill convinced that ore was in these mountains, Hildelve charged me to guard our engines while he continued on foot.$B$BIt\'s been days, and I\'ve heard some beastly growls north of Helm\'s Bed Lake, west of the tunnel. Mori\'s tough as nails, but I\'m worried.$B$BPlease, $n. Find him.' WHERE `ID`=26854;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26854 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26854);
UPDATE `quest_offer_reward` SET `RewardText`='Here is the corpse of a dwarf -- mauled, frozen, and picked clean by mountain scavengers. Clutched in its hand is a book containing the roughly scribbled notes of Mori Hildelve. And around its frame, still in superior condition, is a Brigadier\'s Vest.$B$BThis grisly scene must have been where the Siege Engine pilot met his end.' WHERE `ID`=26854;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26855 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26855);
UPDATE `quest_request_items` SET `CompletionText`='Hello, $n. Do you have word of my friend Hildelve?' WHERE `ID`=26855;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26855 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26855);
UPDATE `quest_offer_reward` SET `RewardText`='This is grim news, for Hildelve was a good friend. And I wish I could have been there with you when you found the beast, Mangeclaw!$B$BThank you, $n. Your deed will be remembered by the pilots of the Ironforge Siege Brigade.' WHERE `ID`=26855;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=13635 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=13635);
UPDATE `quest_offer_reward` SET `RewardText`='The South Gate status report? Excellent. I\'ve been awaiting news from them for a while.$B$BWith the increasing number of troggs in the hills on all sides, I fear the outposts will be the first to feel any aggressive action.' WHERE `ID`=13635;

-- 2026_08_14_world_05.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_05.sql
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=41848;
DELETE FROM `smart_scripts` WHERE `entryorguid`=41848 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(41848, 0, 0, 0, 73, 0, 100, 0, 0, 0, 0, 0, 0, 52, 2201, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Mathel\'s Flying Machine - On Spellclick - Activate Taxi');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=42175;
DELETE FROM `smart_scripts` WHERE `entryorguid`=42175 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(42175, 0, 0, 0, 73, 0, 100, 0, 0, 0, 0, 0, 0, 52, 2207, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 'Rixa\'s Flying Machine - On Spellclick - Activate Taxi');

-- 2026_08_14_world_06.sql | https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/sql/old/5.4.8/world/SFDB_release_25.000_to_26.001/2026_08_14_world_06.sql
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26145 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26145);
UPDATE `quest_request_items` SET `CompletionText`='Do you have eight trogg stone teeth to show me? If not, there is still work to be done, $n.' WHERE `ID`=26145;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26145 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26145);
UPDATE `quest_offer_reward` SET `RewardText`='I am quite impressed, $n. You have done the Dwarven race a great favor by defending our lands. King Magni Bronzebeard himself would be proud!' WHERE `ID`=26145;
UPDATE `quest_template` SET `QuestDescription`='We need to protect the Loch, $n! With so many of the King\'s soldiers fighting valiantly on remote battlefields, we\'ve become overwhelmed on the home front. Troggs are tunneling up from every crevice! The trogg infestation poses the largest threat to Ironforge.$B$BWe need you, brave adventurer, to venture forth and lay waste to the trogg threat. Slay twelve Stonesplinter troggs and venture back.' WHERE `ID`=26146;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26146 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26146);
UPDATE `quest_request_items` SET `CompletionText`='Loch Modan is under siege, $n! We need every able-bodied member of the Alliance to aid our cause. Have you killed those troggs yet?' WHERE `ID`=26146;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26146 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26146);
UPDATE `quest_offer_reward` SET `RewardText`='Well done, $n! You demonstrated uncanny courage on the battlefield. Our war against the Troggs will be won with efforts such as yours. You have served King Magni well.' WHERE `ID`=26146;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26148 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26148);
UPDATE `quest_request_items` SET `CompletionText`='Your mission is not complete until Grawmug has been brought to death. Khaz Modan is counting on you, $n.' WHERE `ID`=26148;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26148 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26148);
UPDATE `quest_offer_reward` SET `RewardText`='Most excellent, $n! You have brought hope to Loch Modan. With Grawmug dead, we stand a better chance of seeing the Troggs eliminated from the land.' WHERE `ID`=26148;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26147 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26147);
UPDATE `quest_offer_reward` SET `RewardText`='I knew the second I laid eyes on you, $n, that you would make Ironforge proud. You have served our people well. I salute you, $n.' WHERE `ID`=26147;
INSERT INTO `quest_request_items` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26176 AND NOT EXISTS (SELECT 1 FROM `quest_request_items` WHERE `ID`=26176);
UPDATE `quest_request_items` SET `CompletionText`='What can I do for you?' WHERE `ID`=26176;
INSERT INTO `quest_offer_reward` (`ID`) SELECT `ID` FROM `quest_template` WHERE `ID`=26176 AND NOT EXISTS (SELECT 1 FROM `quest_offer_reward` WHERE `ID`=26176);
UPDATE `quest_offer_reward` SET `RewardText`='Glad to hear you\'ve been clearing out the troggs. Let\'s have a look at this report, and...$B$B...Is that a beer stain?' WHERE `ID`=26176;
