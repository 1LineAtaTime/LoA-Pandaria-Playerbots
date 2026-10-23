-- Missing capital-city bot data (issue #61).
--
-- Undercity (1497), Darnassus (1657), Ironforge (1537), Thunder Bluff (1638), The Exodar (3557)
-- and Shattrath (3703) had no `playerbot_farming_zone` row and/or no `playerbot_city` anchor, so
-- bots were never sent to them for camping/wandering and the city looked empty.
--
-- The city anchors are the densest creature spawn clusters inside each zone, read from the live
-- 5.4.8 world data (world.creature grouped by zone/map/x/y/z), which is the same source the rest of
-- the seeded city rows use. TeamsDisabled uses the existing convention: 0 = both factions,
-- 67 = Horde excluded, 469 = Alliance excluded.

DELETE FROM `playerbot_farming_zone` WHERE `ZoneId` IN (1497, 1657, 1537, 1638, 3557, 3703);
INSERT INTO `playerbot_farming_zone`
    (`ZoneId`, `ZoneTyp`, `MinLevel`, `MaxLevel`, `TeamsDisabled`, `MapId`, `MinPlayers`, `MaxPlayers`)
VALUES
    (1497, 1, 1, 90, 469,   0, 10, 20),  -- Undercity (Horde)
    (1657, 1, 1, 90,  67,   1, 10, 20),  -- Darnassus (Alliance)
    (1537, 1, 1, 90,  67,   0, 10, 20),  -- Ironforge (Alliance)
    (1638, 1, 1, 90, 469,   1, 10, 20),  -- Thunder Bluff (Horde)
    (3557, 1, 1, 90,  67, 530, 10, 20),  -- The Exodar (Alliance)
    (3703, 1, 1, 90,   0, 530, 10, 20);  -- Shattrath (neutral)

DELETE FROM `playerbot_city` WHERE `ZoneId` IN (1497, 1657, 1537, 1638, 3557, 3703);
INSERT INTO `playerbot_city`
    (`ZoneId`, `CityId`, `Minlevel`, `Maxlevel`, `TeamsDisabled`, `X`, `Y`, `Z`, `Radius`)
VALUES
    (1497, 1, 1, 90, 469, 1578,   307,  -62, 25),  -- Undercity: Trade Quarter
    (1497, 2, 1, 90, 469, 1627,   223,  -43, 20),  -- Undercity: War Quarter
    (1497, 3, 1, 90, 469, 1521,   242,  -41, 20),  -- Undercity: Rogues' Quarter
    (1657, 1, 1, 90,  67, 9922,  2500, 1318, 25),  -- Darnassus: Temple Gardens
    (1657, 2, 1, 90,  67, 9985,  2345, 1331, 20),  -- Darnassus: Craftmen's Terrace
    (1657, 3, 1, 90,  67, 9720,  2545, 1336, 20),  -- Darnassus: Warrior's Terrace
    (1537, 1, 1, 90,  67, -4844,  -862, 502, 25),  -- Ironforge: center
    (1537, 2, 1, 90,  67, -4915,  -954, 501, 20),  -- Ironforge: Military Ward
    (1537, 3, 1, 90,  67, -5031, -1269, 505, 20),  -- Ironforge: Hall of Explorers
    (1638, 1, 1, 90, 469, -1242,    82, 129, 25),  -- Thunder Bluff: center
    (1638, 2, 1, 90, 469, -1385,   -86, 160, 20),  -- Thunder Bluff: Spirit Rise
    (1638, 3, 1, 90, 469, -1092,   -16, 141, 20),  -- Thunder Bluff: Elder Rise
    (3557, 1, 1, 90,  67, -3909, -11615, -138, 25),-- Exodar: center
    (3557, 2, 1, 90,  67, -4159, -11452, -131, 20),-- Exodar: Crystal Hall
    (3557, 3, 1, 90,  67, -3845, -11420, -132, 20),-- Exodar: Traders' Tier
    (3703, 1, 1, 90,   0, -1788,   5159,  -39, 25);-- Shattrath: center
