-- Refs #182 (pool data inconsistencies, dangling references)
-- ERROR CLASS: "`pool_pool` included pool_id (N) is not in `pool_template`, skipped." (PoolMgr.cpp:689, 2 lines: 6576, 6590) and
--   "`game_event_creature` contains creature (GUID: N) not found in `creature` table." (2 lines: 77232, 136675).
-- WHY INVALID: pool 6576/6590 have no pool_template row in LoA, and the MoP 5.4.8 template rows (description 'GO 1732,[1733],map=1') have no
--   pool_gameobject / pool_creature members either, so there is nothing to restore; the pool_pool row is already skipped at load, deleting it changes nothing.
--   game_event_creature 77232 / 136675 (event 31, creature 35587 on map 571) point to creature guids absent from LoA's `creature` table
--   (the MoP 5.4.8 data still has them); the rows are skipped at load, so removing them changes nothing at runtime.
-- ROWS: 2 pool_pool + 2 game_event_creature.
-- VERIFY: SELECT COUNT(*) FROM pool_pool WHERE pool_id IN (6576,6590);  SELECT COUNT(*) FROM game_event_creature WHERE guid IN (77232,136675);  -- both 0
-- EXPECTED DBErrors.log DROP: 4 lines.
-- UNDO: INSERT INTO pool_pool (pool_id,mother_pool,chance) VALUES (6576,9234,0),(6590,9238,0); INSERT INTO game_event_creature (eventEntry,guid) VALUES (31,77232),(31,136675);
DELETE FROM `pool_pool` WHERE `pool_id` IN (6576,6590) AND `mother_pool` IN (9234,9238);
DELETE FROM `game_event_creature` WHERE `eventEntry`=31 AND `guid` IN (77232,136675);
