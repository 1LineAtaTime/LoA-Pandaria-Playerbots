-- #72: max-level bot density on Pandaria (870). Level-bracket rows for the Pandaria levelling zones, following AC's pattern.
--
-- With the new-rpg strategy the random-teleport pool and the GO_CAMP pool are the innkeeper hubs of the
-- zones whose bracket contains the bot level (AC TravelMgr::GetTeleportLocations / GetTravelHubs,
-- TravelMgr.cpp:4480-4495; hubs built from zone2LevelBracket, TravelMgr.cpp:4757-4771). AC's bracket
-- table (TravelMgr.cpp:4618-4629) runs each Northrend zone about three levels past its real range and up
-- to the cap: Dragonblight 71-77, Grizzly Hills 72-78, Borean 68-75, Zul'Drak 74-80, Sholazar 75-80,
-- Storm Peaks / Icecrown / Crystalsong 77-80. That gives a level-80 bot about 15 hubs across five zones.
-- The Pandaria rows here stopped one level past the real range, so a level-90 bot had 7 hubs in four
-- zones: 3 in Townlong, 2 in Dread Wastes, 1 in the Vale and 1 on the Timeless Isle. Measured at
-- 1000 bots: 926 on 870, with 33-40 bots in each 100-yd cell holding one of those 7 innkeepers.
-- AC at 80: busiest cell 16 bots.
-- Krasarang Wilds (6134) had no row at all, so it was never a destination.
--
-- MoP-only content extending AC's pattern (real range + 3, capped at 90). Real ranges from Wowpedia:
-- Jade Forest 85-86, Valley of the Four Winds 86-87, Krasarang 86-87, Kun-Lai 87-88, Townlong 88-89,
-- Dread Wastes 89-90.
UPDATE `playerbot_farming_zone` SET `MaxLevel` = 89 WHERE `ZoneId` = 5785;  -- The Jade Forest       85-87 -> 85-89
UPDATE `playerbot_farming_zone` SET `MaxLevel` = 90 WHERE `ZoneId` = 5805;  -- Valley of Four Winds  86-88 -> 86-90
UPDATE `playerbot_farming_zone` SET `MaxLevel` = 90 WHERE `ZoneId` = 5841;  -- Kun-Lai Summit        87-89 -> 87-90
INSERT IGNORE INTO `playerbot_farming_zone` (`ZoneId`, `ZoneTyp`, `MinLevel`, `MaxLevel`, `TeamsDisabled`, `MapId`, `MinPlayers`, `MaxPlayers`)
VALUES (6134, 0, 86, 90, 0, 870, 10, 20);                                   -- Krasarang Wilds       (missing) -> 86-90
