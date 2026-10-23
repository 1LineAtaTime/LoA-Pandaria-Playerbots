-- #621 #622 #623: re-randomize random bots that have no main-hand weapon (hunters without a gun/crossbow, rogues
-- 12-69, DKs left at the level-55 start kit). NOT RUN - apply to the `playerbots` + `characters` DBs only after the
-- build that carries the RandomItemManager/BotFactory fixes, with the world stopped.
--
-- Mechanism: RandomPlayerbotMgr::ProcessBot(Player*) (Manager/RandomPlayerbotMgr.cpp:498-505) calls Randomize() for
-- an idle logged-in bot whenever GetEventValue(bot, "randomize") is 0, i.e. the row is absent or expired. Randomize
-- (:728-767) does a full RandomizeFirst for a DK below 56 or any bot below RandombotStartingLevel, and a level-90
-- bot gets Randomize(true), which now runs InitEquipment(true) (BotFactory.cpp, #623). Deleting the row is enough.
--
-- BACKUP FIRST (this is the rollback - the deleted rows are just timers, so a restore of the table undoes it):
--   CREATE TABLE playerbots.playerbots_random_bots_bak_20261005 AS SELECT * FROM playerbots.playerbots_random_bots;
-- ROLLBACK:
--   INSERT INTO playerbots.playerbots_random_bots (owner, bot, `time`, validIn, event, value, data)
--   SELECT owner, bot, `time`, validIn, event, value, data FROM playerbots.playerbots_random_bots_bak_20261005
--   WHERE event = 'randomize' AND bot IN (SELECT bot FROM playerbots.tmp_weaponless_bots);   -- list saved below

DROP TABLE IF EXISTS playerbots.tmp_weaponless_bots;
CREATE TABLE playerbots.tmp_weaponless_bots AS
SELECT DISTINCT c.guid AS bot
FROM characters.characters c
JOIN playerbots.playerbots_random_bots r ON r.bot = c.guid AND r.event = 'add'
LEFT JOIN characters.character_inventory mh ON mh.guid = c.guid AND mh.bag = 0 AND mh.slot = 15
WHERE mh.guid IS NULL
  AND c.level >= 5;                 -- level 1-4 bots use the start outfit (AC InitEquipment level < 5)

DELETE FROM playerbots.playerbots_random_bots
WHERE event = 'randomize' AND bot IN (SELECT bot FROM playerbots.tmp_weaponless_bots);

-- Bots with no randomize row get one scheduled by ProcessBot(uint32) on login (:344-351). Verify afterwards:
--   SELECT c.class, COUNT(*), SUM(mh.guid IS NULL) FROM characters.characters c
--   LEFT JOIN characters.character_inventory mh ON mh.guid = c.guid AND mh.bag = 0 AND mh.slot = 15 GROUP BY c.class;
