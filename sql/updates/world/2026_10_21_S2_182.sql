-- #182 pool cleanup, lane S2 (2026-10-21). Idempotent. World DB.
-- Classification of the 483 pool lines in the build-86 DBErrors.log (482 'is empty' + 1 'explicit chance sum'):
--   * 412 empty CHILD pools of live mother pools (pool_pool rows, none of them in AC; all 412 exist in the MoP 5.4.8 pool_template)
--       = the owner-deferred "_45" decision (removing them raises the live siblings' spawn odds). NOT applied here; the opt-in block at the
--         bottom is commented out.
--   * 70 pool_quest pools (69 in pool_quest + pool 609, the Klaxxi daily mother of 4 quest pools): false positive, PoolMgr::IsEmpty (PoolMgr.cpp:877) ignores quest groups.
--       Core fix, not SQL. Nothing to do here.
--   * 1 pool 1001681 ('[map 1, split of pool 210]'): the #182 split left one explicit-chance (12) creature alone, so the explicit sum is
--       12 != 100 and the pool never spawns ("Pool Id 1001681 has no equal chance pooled entites defined and explicit chance sum is not 100").
--       Fixed below: the sole member gets chance 0 (equal-chance), so it always spawns on its map.
-- UNDO: UPDATE `pool_creature` SET `chance` = 12 WHERE `guid` = 53202 AND `pool_entry` = 1001681;

UPDATE `pool_creature` SET `chance` = 0 WHERE `guid` = 53202 AND `pool_entry` = 1001681 AND `chance` <> 0;

-- ---------------------------------------------------------------------------------------------------------------------
-- OPT-IN (owner decision _45, NOT applied): drop the empty child-pool rows of live mothers (412 rows expected).
-- Back up first: CREATE TABLE pool_pool_bak_45 AS SELECT * FROM pool_pool;   UNDO: INSERT INTO pool_pool SELECT * FROM pool_pool_bak_45
--   WHERE pool_id NOT IN (SELECT pool_id FROM pool_pool);   (and the pool_template rows are left in place, so only pool_pool changes)
-- DELETE pp FROM `pool_pool` pp
--   WHERE NOT EXISTS (SELECT 1 FROM `pool_creature`   x WHERE x.`pool_entry` = pp.`pool_id`)
--     AND NOT EXISTS (SELECT 1 FROM `pool_gameobject` x WHERE x.`pool_entry` = pp.`pool_id`)
--     AND NOT EXISTS (SELECT 1 FROM `pool_quest`      x WHERE x.`pool_entry` = pp.`pool_id`)
--     AND pp.`pool_id` NOT IN (SELECT `mother_pool` FROM (SELECT `mother_pool` FROM `pool_pool`) m);
