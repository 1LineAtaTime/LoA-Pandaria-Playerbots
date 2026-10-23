-- #627: random bots created before the RandomPlayerbotFactory fix could get a name that another character already had
-- (the fork cached all of playerbots_names without the CHAR_SEL_CHECK_NAME filter AC applies; a later boot that
-- topped up bot accounts reused taken names). For every duplicated name on a random-bot account (auth.account
-- username RNDBOT%), keep the lowest guid and give each other copy an unused playerbots_names name of the same
-- race+gender code (AC RandomPlayerbotFactory::CombineRaceAndGender: Generic 0/1, Gnome 2/3, Dwarf 4/5, NightElf 6/7,
-- Draenei 8/9, Orc 10/11, Troll 12/13, Tauren 14/15, BloodElf 16/17; goblin/worgen/pandaren/human/undead = Generic).
-- No at_login rename flag: the new name is final, and a bot cannot answer a client rename prompt.
-- Run with the worldserver STOPPED (CharacterCache loads names at boot). Re-runnable: a second run matches nothing.
-- Needs read access to the auth DB (named `auth` on the test DB). Every rename is recorded in dedupe_627_names.
-- ROLLBACK:
--   UPDATE characters c JOIN dedupe_627_names b ON b.guid = c.guid SET c.name = b.old_name;
--   DROP TABLE dedupe_627_names;

CREATE TABLE IF NOT EXISTS dedupe_627_names (
  guid INT UNSIGNED NOT NULL PRIMARY KEY,
  old_name VARCHAR(12) NOT NULL,
  new_name VARCHAR(12) NOT NULL
) ENGINE=InnoDB COMMENT='#627 bot name dedupe - rollback map';

DROP TEMPORARY TABLE IF EXISTS tmp_dupe_627;
CREATE TEMPORARY TABLE tmp_dupe_627 AS
SELECT d.guid, d.name, d.code,
       ROW_NUMBER() OVER (PARTITION BY d.code ORDER BY d.guid) AS rn
FROM (
  SELECT c.guid, c.name,
         (CASE c.race WHEN 7 THEN 2 WHEN 3 THEN 4 WHEN 4 THEN 6 WHEN 11 THEN 8 WHEN 2 THEN 10
                      WHEN 8 THEN 12 WHEN 6 THEN 14 WHEN 10 THEN 16 ELSE 0 END)
         + (CASE WHEN c.gender >= 2 THEN 0 ELSE c.gender END) AS code
  FROM characters c
  WHERE c.account IN (SELECT a.id FROM auth.account a WHERE a.username LIKE 'RNDBOT%')
    AND EXISTS (SELECT 1 FROM characters k WHERE k.name = c.name AND k.guid < c.guid)
) d;

DROP TEMPORARY TABLE IF EXISTS tmp_pool_627;
CREATE TEMPORARY TABLE tmp_pool_627 AS
SELECT n.name, n.gender AS code,
       ROW_NUMBER() OVER (PARTITION BY n.gender ORDER BY n.name_id) AS rn
FROM playerbots_names n
WHERE CHAR_LENGTH(n.name) BETWEEN 2 AND 12
  AND NOT EXISTS (SELECT 1 FROM characters c WHERE c.name = n.name)
  AND NOT EXISTS (SELECT 1 FROM reserved_name r WHERE r.name = n.name)
  AND n.gender IN (SELECT DISTINCT code FROM tmp_dupe_627);

INSERT INTO dedupe_627_names (guid, old_name, new_name)
SELECT d.guid, d.name, p.name
FROM tmp_dupe_627 d JOIN tmp_pool_627 p ON p.code = d.code AND p.rn = d.rn;

UPDATE characters c
JOIN tmp_dupe_627 d ON d.guid = c.guid
JOIN tmp_pool_627 p ON p.code = d.code AND p.rn = d.rn
SET c.name = p.name;

DROP TEMPORARY TABLE tmp_dupe_627;
DROP TEMPORARY TABLE tmp_pool_627;

-- Check: SELECT name, COUNT(*) FROM characters GROUP BY name HAVING COUNT(*) > 1;  -- expect 0 rows
