-- Wrong teams and an anomalous value in the capital-city bot data (issue #96).
--
-- `playerbot_city` stored Stormwind (1519) as TeamsDisabled = 469, which is the *Horde* exclusion
-- value: it excluded Alliance bots from Stormwind and let Horde bots in (observed live). The matching
-- `playerbot_farming_zone` row already had the correct 67, so the two tables disagreed about the same
-- zone. `playerbot_farming_zone` zone 1 also carried 64, which matches neither faction and therefore
-- silently disabled the filter for that zone.
--
-- Convention (module code compares this against `Player::GetTeam()`, see `enum Team` in
-- src/server/game/Miscellaneous/SharedDefines.h):
--   0   = both factions allowed (neutral)
--   469 = Alliance excluded  (Horde city)
--   67  = Horde excluded     (Alliance city)
--
-- The C++ side (RandomPlayerbotMgr::IsEnemyCapitalZone) no longer trusts these rows for capital
-- cities, so this is data hygiene plus the film for any row the code does not cover.

-- The inverted row that let Horde bots into Stormwind.
UPDATE `playerbot_city` SET `TeamsDisabled` = 67 WHERE `ZoneId` = 1519;

-- Normalise every capital row to its owning faction (idempotent).
UPDATE `playerbot_city`         SET `TeamsDisabled` = 469 WHERE `ZoneId` IN (1497, 1637, 1638, 3487);
UPDATE `playerbot_city`         SET `TeamsDisabled` =  67 WHERE `ZoneId` IN (1519, 1537, 1657, 3557);
UPDATE `playerbot_farming_zone` SET `TeamsDisabled` = 469 WHERE `ZoneId` IN (1497, 1637, 1638, 3487);
UPDATE `playerbot_farming_zone` SET `TeamsDisabled` =  67 WHERE `ZoneId` IN (1519, 1537, 1657, 3557);

-- Anything that matches neither faction is a free pass for both; make it explicit (neutral).
UPDATE `playerbot_farming_zone` SET `TeamsDisabled` = 0 WHERE `TeamsDisabled` NOT IN (0, 67, 469);
UPDATE `playerbot_city`         SET `TeamsDisabled` = 0 WHERE `TeamsDisabled` NOT IN (0, 67, 469);
