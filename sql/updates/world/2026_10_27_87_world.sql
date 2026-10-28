-- Shadowfang Keep (normal): these creatures kept a later expansion's health scaling (exp > 0), and creature_classlevelstats has no
-- health for that expansion at their level (basehp = 1), so they had 1-40 HP. Retail (TDB 1210.26091, DifficultyID 1,
-- else 0) scales them with classic health: use exp 0 and its normal-mode health modifier. Rows whose retail modifier
-- comes from a later expansion's scaling keep ours.
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 2110; -- Black Rat
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 3864; -- Fel Steed
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 3865; -- Shadow Charger
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 3869; -- Lesser Gargoyle
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 3870; -- Stone Sleeper
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 4.5 WHERE `entry` = 3872; -- Deathsworn Captain
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 3873; -- Tormented Officer
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 3875; -- Haunted Servitor
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 3877; -- Wailing Guardsman
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 35 WHERE `entry` = 3887; -- Baron Silverlaine
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 28 WHERE `entry` = 4278; -- Commander Springvale
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 0.02 WHERE `entry` = 4958; -- Haunting Spirit
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 6486; -- Black Skeletal Horse
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 0.3 WHERE `entry` = 10441; -- Plagued Rat
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 0.3 WHERE `entry` = 10536; -- Plagued Maggot
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 11153; -- Red Skeletal Horse
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 11154; -- Blue Skeletal Horse
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 15108; -- Forsaken Horse
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 23033; -- Invisible Stalker (Floating)
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 34238; -- Black Skeletal Horse
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 36530; -- [DND] Valentine Boss - Vial Bunny
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 36710; -- [DND] Apothecary Table (Spell Effect)
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 36885; -- Crown Apothecary
UPDATE `creature_template` SET `exp` = 0 WHERE `entry` = 43679; -- DEBUG Announcer
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 35 WHERE `entry` = 46962; -- Baron Ashbury
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 35 WHERE `entry` = 46963; -- Lord Walden
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 39 WHERE `entry` = 46964; -- Lord Godfrey
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 4 WHERE `entry` = 47030; -- Veteran Forsaken Trooper
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 4 WHERE `entry` = 47031; -- Forsaken Blightspreader
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 47131; -- Frantic Geist
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 47132; -- Dark Creeper
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 47134; -- Corpse Eater
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 47135; -- Fetid Ghoul
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 47136; -- Unstable Ravager
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 8 WHERE `entry` = 47137; -- Mindless Horror
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 8 WHERE `entry` = 47138; -- Pustulant Monstrosity
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 47140; -- Sorcerous Skeleton
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 4 WHERE `entry` = 47141; -- Dread Scryer
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 8 WHERE `entry` = 47143; -- Spitebone Skeleton
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 8 WHERE `entry` = 47145; -- Spitebone Guardian
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 8 WHERE `entry` = 47146; -- Spitebone Flayer
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 47231; -- Shadowy Attendant
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 5 WHERE `entry` = 47232; -- Ghostly Cook
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 12 WHERE `entry` = 47293; -- Deathstalker Commander Belmont
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 10 WHERE `entry` = 47862; -- Magistrate Henry Maleb
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 47865; -- Forsaken Trooper
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 48124; -- Rabid Mastiff
UPDATE `creature_template` SET `exp` = 0, `Health_mod` = 1 WHERE `entry` = 51400; -- Haunted Stable Hand

-- Shadowfang Keep gold was the level 85 heroic amount. Scale it to each creature's level: gold * (average gold of open-world
-- normal mobs at that level) / (the same average at level 85: 2979 copper); at levels 18/19/20/21 those averages are 38/41/46/52 copper.
UPDATE `creature_template` SET `mingold` = 195, `maxgold` = 195 WHERE `entry` = 3864; -- Fel Steed (was 14077)
UPDATE `creature_template` SET `mingold` = 177, `maxgold` = 177 WHERE `entry` = 3865; -- Shadow Charger (was 13801)
UPDATE `creature_template` SET `mingold` = 189, `maxgold` = 189 WHERE `entry` = 3869; -- Lesser Gargoyle (was 12163)
UPDATE `creature_template` SET `mingold` = 189, `maxgold` = 189 WHERE `entry` = 3870; -- Stone Sleeper (was 12190)
UPDATE `creature_template` SET `mingold` = 156, `maxgold` = 156 WHERE `entry` = 3873; -- Tormented Officer (was 10030)
UPDATE `creature_template` SET `mingold` = 103, `maxgold` = 103 WHERE `entry` = 3875; -- Haunted Servitor (was 7431)
UPDATE `creature_template` SET `mingold` = 94, `maxgold` = 94 WHERE `entry` = 3877; -- Wailing Guardsman (was 6770)
UPDATE `creature_template` SET `mingold` = 214, `maxgold` = 214 WHERE `entry` = 3887; -- Baron Silverlaine (was 12181)
UPDATE `creature_template` SET `mingold` = 175, `maxgold` = 175 WHERE `entry` = 4278; -- Commander Springvale (was 9945)
UPDATE `creature_template` SET `mingold` = 223, `maxgold` = 223 WHERE `entry` = 46962; -- Baron Ashbury (was 12683)
UPDATE `creature_template` SET `mingold` = 226, `maxgold` = 226 WHERE `entry` = 46963; -- Lord Walden (was 12829)
UPDATE `creature_template` SET `mingold` = 231, `maxgold` = 231 WHERE `entry` = 46964; -- Lord Godfrey (was 13127)
UPDATE `creature_template` SET `mingold` = 118, `maxgold` = 118 WHERE `entry` = 47131; -- Frantic Geist (was 8490)
UPDATE `creature_template` SET `mingold` = 139, `maxgold` = 139 WHERE `entry` = 47132; -- Dark Creeper (was 8930)
UPDATE `creature_template` SET `mingold` = 115, `maxgold` = 115 WHERE `entry` = 47134; -- Corpse Eater (was 8286)
UPDATE `creature_template` SET `mingold` = 84, `maxgold` = 84 WHERE `entry` = 47135; -- Fetid Ghoul (was 6075)
UPDATE `creature_template` SET `mingold` = 135, `maxgold` = 135 WHERE `entry` = 47136; -- Unstable Ravager (was 8687)
UPDATE `creature_template` SET `mingold` = 110, `maxgold` = 110 WHERE `entry` = 47137; -- Mindless Horror (was 7080)
UPDATE `creature_template` SET `mingold` = 106, `maxgold` = 106 WHERE `entry` = 47138; -- Pustulant Monstrosity (was 6824)
UPDATE `creature_template` SET `mingold` = 104, `maxgold` = 104 WHERE `entry` = 47140; -- Sorcerous Skeleton (was 7528)
UPDATE `creature_template` SET `mingold` = 135, `maxgold` = 135 WHERE `entry` = 47141; -- Dread Scryer (was 8675)
UPDATE `creature_template` SET `mingold` = 94, `maxgold` = 94 WHERE `entry` = 47143; -- Spitebone Skeleton (was 6799)
UPDATE `creature_template` SET `mingold` = 92, `maxgold` = 92 WHERE `entry` = 47145; -- Spitebone Guardian (was 6662)
UPDATE `creature_template` SET `mingold` = 135, `maxgold` = 135 WHERE `entry` = 47146; -- Spitebone Flayer (was 8679)
UPDATE `creature_template` SET `mingold` = 103, `maxgold` = 103 WHERE `entry` = 47231; -- Shadowy Attendant (was 7463)
UPDATE `creature_template` SET `mingold` = 101, `maxgold` = 101 WHERE `entry` = 47232; -- Ghostly Cook (was 7302)
