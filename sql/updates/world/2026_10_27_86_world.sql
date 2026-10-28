-- Deadmines (normal, levels 15-21): the creatures kept the Cataclysm heroic templates' Cataclysm health expansion (exp 3), and
-- creature_classlevelstats has no expansion 3 health below level 81 (basehp3 = 1), so trash had 1-10 HP and bosses 25-40 HP.
-- Retail (TDB 1210.26091, DifficultyID 1, else 0) scales them with classic health (HealthScalingExpansion 0): use exp 0 and its
-- normal-mode health modifiers. Rows whose retail modifier comes from a later expansion's scaling keep ours.
-- Their gold was also the level 85 heroic amount (20 silver to 2 gold per mob). Scale it to the level: gold * (average gold of
-- open-world normal mobs at that level) / (the same average at level 85: 2570 copper); the level 10/14/15/16 averages are
-- 11/21/25/29 copper.
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 598; -- Defias Miner
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 6 WHERE `entry` = 645; -- Cookie
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 3 WHERE `entry` = 657; -- Defias Pirate
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 3 WHERE `entry` = 1732; -- Defias Squallshaper
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 12 WHERE `entry` = 3586; -- [UNUSED 4.x ]Miner Johnson
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 3 WHERE `entry` = 3947; -- Goblin Shipbuilder
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 4075; -- Rat
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 35 WHERE `entry` = 43778; -- Foe Reaper 5000
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 3 WHERE `entry` = 46612; -- Lieutenant Horatio Laine
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 46613; -- Crime Scene Alarm-o-Bot
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 2 WHERE `entry` = 46614; -- Stormwind Investigator
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 3 WHERE `entry` = 46889; -- Kagtha
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 3 WHERE `entry` = 46890; -- Shattered Hand Assassin
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 3 WHERE `entry` = 46902; -- Miss Mayhem
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 4 WHERE `entry` = 46903; -- Mayhem Reaper Prototype
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 46906; -- Slinky Sharpshiv
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 35 WHERE `entry` = 47162; -- Glubtok
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 47282; -- Fire Blossom Bunny
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 47284; -- Frost Blossom Bunny
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 30 WHERE `entry` = 47297; -- Lumbering Oaf
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 10 WHERE `entry` = 47404; -- Defias Watcher
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 40 WHERE `entry` = 47626; -- Admiral Ripsnarl
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 47677; -- General Purpose Bunny JMF (Look 3)
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 35 WHERE `entry` = 47739; -- "Captain" Cookie
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48229; -- Kobold Digger
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 7 WHERE `entry` = 48230; -- Ogre Henchman
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 7 WHERE `entry` = 48262; -- Ogre Bodyguard
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48266; -- Defias Cannon
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48278; -- Mining Monkey
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 7 WHERE `entry` = 48279; -- Goblin Overseer
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 3 WHERE `entry` = 48280; -- Goblin Craftsman
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 0.01 WHERE `entry` = 48284; -- Mining Powder
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 48338; -- Mine Bunny
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 48340; -- Drink Tray
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 48341; -- Goblin Cocktail
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 48342; -- Goblin Cocktail
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 48343; -- Goblin Cocktail
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 48351; -- Mine Bunny
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 48417; -- Defias Blood Wizard
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 48418; -- Defias Envoker
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48419; -- Defias Miner
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48420; -- Defias Digger
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 48421; -- Defias Overseer
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 3 WHERE `entry` = 48439; -- Goblin Engineer
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48440; -- Mining Monkey
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48441; -- Mining Monkey
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48442; -- Mining Monkey
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 10 WHERE `entry` = 48445; -- Oaf Lackey
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 48446; -- Scorch Mark Bunny JMF
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48447; -- Monstrous Parrot
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48450; -- Sunwing Squawker
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48451; -- Ol' Beaky
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 6 WHERE `entry` = 48502; -- Defias Enforcer
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 48505; -- Defias Shadowguard
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 48521; -- Defias Squallshaper
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 48522; -- Defias Pirate
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 49457; -- Steam Valve
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 49552; -- Vanessa's Rope Anchor
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 50595; -- Stormwind Defender

UPDATE `creature_template` SET `mingold` = 65, `maxgold` = 65 WHERE `entry` = 48420; -- Defias Digger (was 7903)
UPDATE `creature_template` SET `mingold` = 65, `maxgold` = 65 WHERE `entry` = 48419; -- Defias Miner (was 7906)
UPDATE `creature_template` SET `mingold` = 226, `maxgold` = 226 WHERE `entry` = 43778; -- Foe Reaper 5000 (was 20008)
UPDATE `creature_template` SET `mingold` = 147, `maxgold` = 147 WHERE `entry` = 47626; -- Admiral Ripsnarl (was 13029)
UPDATE `creature_template` SET `mingold` = 69, `maxgold` = 69 WHERE `entry` = 48230; -- Ogre Henchman (was 7084)
UPDATE `creature_template` SET `mingold` = 63, `maxgold` = 63 WHERE `entry` = 48229; -- Kobold Digger (was 7684)
UPDATE `creature_template` SET `mingold` = 47, `maxgold` = 47 WHERE `entry` = 48262; -- Ogre Bodyguard (was 4819)
UPDATE `creature_template` SET `mingold` = 68, `maxgold` = 68 WHERE `entry` = 48279; -- Goblin Overseer (was 6975)
UPDATE `creature_template` SET `mingold` = 31, `maxgold` = 31 WHERE `entry` = 48445; -- Oaf Lackey (was 7141)
UPDATE `creature_template` SET `mingold` = 74, `maxgold` = 74 WHERE `entry` = 48502; -- Defias Enforcer (was 7576)
UPDATE `creature_template` SET `mingold` = 72, `maxgold` = 72 WHERE `entry` = 48505; -- Defias Shadowguard (was 7403)
UPDATE `creature_template` SET `mingold` = 83, `maxgold` = 83 WHERE `entry` = 48417; -- Defias Blood Wizard (was 8562)
UPDATE `creature_template` SET `mingold` = 58, `maxgold` = 58 WHERE `entry` = 48521; -- Defias Squallshaper (was 5920)
UPDATE `creature_template` SET `mingold` = 59, `maxgold` = 59 WHERE `entry` = 48522; -- Defias Pirate (was 6028)
UPDATE `creature_template` SET `mingold` = 83, `maxgold` = 83 WHERE `entry` = 48418; -- Defias Envoker (was 8541)
UPDATE `creature_template` SET `mingold` = 70, `maxgold` = 70 WHERE `entry` = 48421; -- Defias Overseer (was 7165)
UPDATE `creature_template` SET `mingold` = 17, `maxgold` = 17 WHERE `entry` = 48278; -- Mining Monkey (was 2137)
UPDATE `creature_template` SET `mingold` = 19, `maxgold` = 19 WHERE `entry` = 48441; -- Mining Monkey (was 2285)
UPDATE `creature_template` SET `mingold` = 148, `maxgold` = 148 WHERE `entry` = 47162; -- Glubtok (was 13158)
UPDATE `creature_template` SET `mingold` = 80, `maxgold` = 80 WHERE `entry` = 48338; -- Mine Bunny (was 8244)
UPDATE `creature_template` SET `mingold` = 141, `maxgold` = 141 WHERE `entry` = 47739; -- "Captain" Cookie (was 12490)
