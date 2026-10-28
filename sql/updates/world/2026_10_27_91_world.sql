-- Undo the exp 0 / Health_mod changes of 2026_10_27_86 (Deadmines), _87 (Shadowfang Keep) and _90 (Scholomance). The same
-- templates run heroic from creature_template_difficulty at levels 85-87, where exp 0 has no health either (basehp0 is 1 above
-- level 85), so heroic trash lost most of its health and the level 87 bosses had 1 x health_mod HP (as Lumbering Oaf did
-- before 2026_10_14_50). Creature::SelectLevel now takes the column of the level when the template's column has no health,
-- which fixes normal mode without touching the templates. Health_mod is back too: in a dungeon both modes read health_mod
-- from creature_template_difficulty, so the template's only matters outside one.
-- Values: the original world dump (world_548_20240722), Lumbering Oaf's exp from 2026_10_14_50.
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 598; -- Defias Miner
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 6 WHERE `entry` = 645; -- Cookie
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 3 WHERE `entry` = 657; -- Defias Pirate
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 3 WHERE `entry` = 1732; -- Defias Squallshaper
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 3 WHERE `entry` = 3586; -- [UNUSED 4.x ]Miner Johnson
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 3 WHERE `entry` = 3947; -- Goblin Shipbuilder
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 0.2 WHERE `entry` = 4075; -- Rat
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 30 WHERE `entry` = 43778; -- Foe Reaper 5000
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 3 WHERE `entry` = 46612; -- Lieutenant Horatio Laine
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 46613; -- Crime Scene Alarm-o-Bot
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 2 WHERE `entry` = 46614; -- Stormwind Investigator
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 3 WHERE `entry` = 46889; -- Kagtha
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 3 WHERE `entry` = 46890; -- Shattered Hand Assassin
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 3 WHERE `entry` = 46902; -- Miss Mayhem
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 46903; -- Mayhem Reaper Prototype
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 46906; -- Slinky Sharpshiv
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 35 WHERE `entry` = 47162; -- Glubtok
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 47282; -- Fire Blossom Bunny
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 47284; -- Frost Blossom Bunny
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 20 WHERE `entry` = 47297; -- Lumbering Oaf
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 10 WHERE `entry` = 47404; -- Defias Watcher
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 25 WHERE `entry` = 47626; -- Admiral Ripsnarl
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 47677; -- General Purpose Bunny JMF (Look 3)
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 30 WHERE `entry` = 47739; -- "Captain" Cookie
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48229; -- Kobold Digger
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 48230; -- Ogre Henchman
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 6 WHERE `entry` = 48262; -- Ogre Bodyguard
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48266; -- Defias Cannon
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48278; -- Mining Monkey
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 5 WHERE `entry` = 48279; -- Goblin Overseer
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 3 WHERE `entry` = 48280; -- Goblin Craftsman
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 0.01 WHERE `entry` = 48284; -- Mining Powder
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 48338; -- Mine Bunny
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48340; -- Drink Tray
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48341; -- Goblin Cocktail
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48342; -- Goblin Cocktail
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48343; -- Goblin Cocktail
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 48351; -- Mine Bunny
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 48417; -- Defias Blood Wizard
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 48418; -- Defias Envoker
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48419; -- Defias Miner
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48420; -- Defias Digger
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 3 WHERE `entry` = 48421; -- Defias Overseer
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 3 WHERE `entry` = 48439; -- Goblin Engineer
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48440; -- Mining Monkey
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48441; -- Mining Monkey
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48442; -- Mining Monkey
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 6 WHERE `entry` = 48445; -- Oaf Lackey
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48446; -- Scorch Mark Bunny JMF
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48447; -- Monstrous Parrot
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48450; -- Sunwing Squawker
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48451; -- Ol' Beaky
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 48502; -- Defias Enforcer
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 48505; -- Defias Shadowguard
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 48521; -- Defias Squallshaper
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 48522; -- Defias Pirate
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 49457; -- Steam Valve
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 49552; -- Vanessa's Rope Anchor
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 50595; -- Stormwind Defender
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 0.01 WHERE `entry` = 2110; -- Black Rat
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 3864; -- Fel Steed
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 3865; -- Shadow Charger
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 3869; -- Lesser Gargoyle
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 3870; -- Stone Sleeper
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 6 WHERE `entry` = 3872; -- Deathsworn Captain
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 3873; -- Tormented Officer
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 3875; -- Haunted Servitor
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 3877; -- Wailing Guardsman
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 25 WHERE `entry` = 3887; -- Baron Silverlaine
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 25 WHERE `entry` = 4278; -- Commander Springvale
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 0.02 WHERE `entry` = 4958; -- Haunting Spirit
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 6486; -- Black Skeletal Horse
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 0.3 WHERE `entry` = 10441; -- Plagued Rat
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 0.3 WHERE `entry` = 10536; -- Plagued Maggot
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 11153; -- Red Skeletal Horse
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 11154; -- Blue Skeletal Horse
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 15108; -- Forsaken Horse
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 23033; -- Invisible Stalker (Floating)
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 34238; -- Black Skeletal Horse
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 36530; -- [DND] Valentine Boss - Vial Bunny
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 36710; -- [DND] Apothecary Table (Spell Effect)
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 36885; -- Crown Apothecary
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 43679; -- DEBUG Announcer
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 25 WHERE `entry` = 46962; -- Baron Ashbury
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 25 WHERE `entry` = 46963; -- Lord Walden
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 30 WHERE `entry` = 46964; -- Lord Godfrey
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 47030; -- Veteran Forsaken Trooper
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 47031; -- Forsaken Blightspreader
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 47131; -- Frantic Geist
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 47132; -- Dark Creeper
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 47134; -- Corpse Eater
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 47135; -- Fetid Ghoul
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 47136; -- Unstable Ravager
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 8 WHERE `entry` = 47137; -- Mindless Horror
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 8 WHERE `entry` = 47138; -- Pustulant Monstrosity
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 47140; -- Sorcerous Skeleton
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 47141; -- Dread Scryer
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 8 WHERE `entry` = 47143; -- Spitebone Skeleton
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 8 WHERE `entry` = 47145; -- Spitebone Guardian
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 8 WHERE `entry` = 47146; -- Spitebone Flayer
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 47231; -- Shadowy Attendant
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 4 WHERE `entry` = 47232; -- Ghostly Cook
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 12 WHERE `entry` = 47293; -- Deathstalker Commander Belmont
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 10 WHERE `entry` = 47862; -- Magistrate Henry Maleb
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 47865; -- Forsaken Trooper
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 48124; -- Rabid Mastiff
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 51400; -- Haunted Stable Hand
UPDATE `creature_template` SET `exp` = 2, `Health_mod` = 1 WHERE `entry` = 30298; -- Invisible Stalker (Float, Uninteractible, LargeAOI)
UPDATE `creature_template` SET `exp` = 3, `Health_mod` = 1 WHERE `entry` = 54020; -- General Purpose Bunny JMF (Look 2 - Flying, Huge AOI)
