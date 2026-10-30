-- #944 audit (MoP-only data): AC's hard-coded zone level brackets (Travel/TravelMgr.cpp acBrackets) still carry the
-- WotLK levels for Eastern Kingdoms zones that Cataclysm re-levelled, so e.g. level 36-46 bots were sent to the level
-- 53 mobs of Swamp of Sorrows. playerbot_farming_zone rows (ZoneTyp 0) override those brackets, the same route the
-- Kalimdor zones already use. Levels = the Cataclysm zone ranges, checked against the average hostile mob minlevel of
-- each zone in the MoP world DB (2026-10-27): 8 53, 47 32, 28 36, 139 41, 45 27, 3 43, 46 51, 4 56, 267 22, 130 10.
DELETE FROM `playerbot_farming_zone` WHERE `ZoneId` IN (8, 47, 28, 139, 45, 3, 46, 4, 267, 130);
INSERT INTO `playerbot_farming_zone` (`ZoneId`, `ZoneTyp`, `MinLevel`, `MaxLevel`, `TeamsDisabled`, `MapId`, `MinPlayers`, `MaxPlayers`) VALUES
(8,   0, 52, 54, 0,   0, 10, 20), -- Swamp of Sorrows
(47,  0, 30, 35, 0,   0, 10, 20), -- The Hinterlands
(28,  0, 35, 40, 0,   0, 10, 20), -- Western Plaguelands
(139, 0, 40, 45, 0,   0, 10, 20), -- Eastern Plaguelands
(45,  0, 25, 30, 0,   0, 10, 20), -- Arathi Highlands
(3,   0, 44, 48, 0,   0, 10, 20), -- Badlands
(46,  0, 49, 52, 0,   0, 10, 20), -- Burning Steppes
(4,   0, 55, 60, 0,   0, 10, 20), -- Blasted Lands
(267, 0, 20, 25, 0,   0, 10, 20), -- Hillsbrad Foothills
(130, 0, 10, 20, 469, 0, 10, 20); -- Silverpine Forest (Horde)
