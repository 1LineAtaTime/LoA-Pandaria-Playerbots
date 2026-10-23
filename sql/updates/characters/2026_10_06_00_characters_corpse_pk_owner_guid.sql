-- LoA characters DB (`characters`) - runtime DBErrors: corpse INSERT fails with
--   [1062] Duplicate entry 'N' for key 'corpse.PRIMARY'   (23 per owner session, 7,733 in the accumulated log)
--
-- Cause: corpse low guids come from Map::GenerateLowGuid<HighGuid::Corpse>() (Player.cpp:5257, Map.cpp:4206), a
-- PER-MAP counter that restarts at 1 every boot (the global seed in ObjectMgr.cpp:6882 is commented out), but
-- the table's PRIMARY KEY is `corpseGuid`. Corpse::SaveToDB (Corpse.cpp:98-127) deletes by OWNER (`guid`) and then
-- inserts, so a second corpse with the same per-map counter value (other map / earlier boot) collides and the
-- whole save transaction is lost: that corpse is not persisted across a restart.
-- `corpseGuid` is never read back as a key: Map::LoadCorpsesFromDB regenerates the guid and only prints field 15
-- in an error message.
--
-- AC spec: acore_characters.corpse has PRIMARY KEY (`guid`) (the owner) - one corpse per character.
-- This aligns LoA with that. Existing data: 52 rows, 52 distinct owners (checked 2026-10-06), so the new key holds.
--
-- BACKUP FIRST (verify with certutil -hashfile <file> SHA256 after copying):
--   mysqldump -h<host> -P<port> -u<user> -p<password> characters corpse > corpse-before-pk-20261006.sql
-- PRE-CHECK (must return 0 rows, else stop):
SELECT guid, COUNT(*) FROM corpse GROUP BY guid HAVING COUNT(*) > 1;

ALTER TABLE `corpse`
    DROP PRIMARY KEY,
    ADD PRIMARY KEY (`guid`),
    DROP INDEX `idx_player`,
    ADD INDEX `idx_corpseGuid` (`corpseGuid`);

-- ROLLBACK (only valid while corpseGuid is still unique; otherwise restore the dump above):
-- ALTER TABLE `corpse` DROP PRIMARY KEY, ADD PRIMARY KEY (`corpseGuid`), DROP INDEX `idx_corpseGuid`, ADD INDEX `idx_player` (`guid`);
-- If it fails on duplicates: DROP TABLE corpse; then source corpse-before-pk-20261006.sql
