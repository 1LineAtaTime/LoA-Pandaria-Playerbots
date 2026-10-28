-- Scholomance (normal): these creatures kept a later expansion's health scaling (exp > 0), and creature_classlevelstats has no
-- health for that expansion at their level (basehp = 1), so they had 1-40 HP. Retail (TDB 1210.26091, DifficultyID 1,
-- else 0) scales them with classic health: use exp 0 and its normal-mode health modifier. Rows whose retail modifier
-- comes from a later expansion's scaling keep ours.
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 30298; -- Invisible Stalker (Float, Uninteractible, LargeAOI)
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 54020; -- General Purpose Bunny JMF (Look 2 - Flying, Huge AOI)

-- Scholomance gold was the level 90 heroic amount. Scale it to each creature's level: gold * (average gold of open-world
-- normal mobs at that level) / (the same average at level 90: 3865 copper); at levels 41/42/43 those averages are 224/236/260 copper.
UPDATE `creature_template` SET `mingold` = 27554, `maxgold` = 27554 WHERE `entry` = 58664; -- Instructor Chillheart's Phylactery (was 409121)
UPDATE `creature_template` SET `mingold` = 281, `maxgold` = 281 WHERE `entry` = 58757; -- Scholomance Acolyte (was 4830)
UPDATE `creature_template` SET `mingold` = 283, `maxgold` = 283 WHERE `entry` = 58823; -- Scholomance Neophyte (was 4878)
UPDATE `creature_template` SET `mingold` = 23489, `maxgold` = 23489 WHERE `entry` = 59080; -- Darkmaster Gandling (was 348770)
UPDATE `creature_template` SET `mingold` = 20334, `maxgold` = 20334 WHERE `entry` = 59153; -- Rattlegore (was 301916)
UPDATE `creature_template` SET `mingold` = 20301, `maxgold` = 20301 WHERE `entry` = 59184; -- Jandice Barov (was 301436)
UPDATE `creature_template` SET `mingold` = 348, `maxgold` = 348 WHERE `entry` = 59193; -- Boneweaver (was 5697)
UPDATE `creature_template` SET `mingold` = 356, `maxgold` = 356 WHERE `entry` = 59368; -- Krastinovian Carver (was 6124)
UPDATE `creature_template` SET `mingold` = 354, `maxgold` = 354 WHERE `entry` = 59467; -- Candlestick Mage (was 6094)
UPDATE `creature_template` SET `mingold` = 513, `maxgold` = 513 WHERE `entry` = 59613; -- Professor Slate (was 8396)
UPDATE `creature_template` SET `mingold` = 392, `maxgold` = 392 WHERE `entry` = 59614; -- Bored Student (was 6749)
