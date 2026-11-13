-- #950 / #947: bots that log in at the Pandaria faction shrines or in Dalaran were thrown out by the #62 login
-- re-place (RandomPlayerbotMgr::OnBotLoginInternal), because none of those zones had a data row.
-- Capital-style ZoneTyp = 1 rows (as Orgrimmar/Stormwind): TravelMgr::PrepareZone2LevelBracket skips
-- zone_type != 0 (TravelMgr.cpp:436), so these rows do not become level brackets / hub destinations.
-- TeamsDisabled follows the existing convention: Horde city 469, Alliance city 67.
DELETE FROM `playerbot_farming_zone` WHERE `ZoneId` IN (6141, 6553, 4395);
INSERT INTO `playerbot_farming_zone`
    (`ZoneId`, `ZoneTyp`, `MinLevel`, `MaxLevel`, `TeamsDisabled`, `MapId`, `MinPlayers`, `MaxPlayers`)
VALUES
    (6141, 1, 85, 90, 469, 870, 10, 20),  -- Shrine of Two Moons (Horde)
    (6553, 1, 85, 90,  67, 870, 10, 20),  -- Shrine of Seven Stars (Alliance)
    (4395, 1, 71, 80,   0, 571, 10, 20);  -- Dalaran (neutral)

-- Vaultkeeper positions (world.creature 63964 / 63967).
DELETE FROM `playerbot_city` WHERE `ZoneId` IN (6141, 6553);
INSERT INTO `playerbot_city`
    (`ZoneId`, `CityId`, `Minlevel`, `Maxlevel`, `TeamsDisabled`, `X`, `Y`, `Z`, `Radius`)
VALUES
    (6141, 1, 85, 90, 469, 1714.05, 1005.79, 489.088, 25),  -- Shrine of Two Moons: vaultkeeper
    (6553, 1, 85, 90,  67,  763.298, 259.402, 519.874, 25); -- Shrine of Seven Stars: vaultkeeper
