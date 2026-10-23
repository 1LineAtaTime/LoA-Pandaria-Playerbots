-- Refs #182 (pool data inconsistencies, part 3 of 4: move mixed-map pool_creature members to the per-map pool)
-- ERROR CLASS: "`pool_creature` has creature spawns on multiple different maps for creature guid G in pool id (P), skipped." (4 lines per boot,
--   PoolMgr.cpp:536). Same cause and same fix as _41, for creatures. Needs _40 first.
-- ROWS: 4 pool_creature rows moved to a per-map piece pool (map = creature.map). Idempotent (exact guids + old pool_entry).
-- VERIFY:  SELECT pc.pool_entry,COUNT(DISTINCT c.map) FROM pool_creature pc JOIN creature c ON c.guid=pc.guid GROUP BY pc.pool_entry HAVING COUNT(DISTINCT c.map)>1; -- 0 rows
-- EXPECTED DBErrors.log DROP: 4 lines.
-- NOTE pool 210 (max_limit 1; creature 34973 id 783 on map 0 + creature 53202 Swiftmane id 5831 on map 1, chance 12): after the split Swiftmane is
--   alone in its own pool, and an explicit-chance-only pool whose chances do not sum to 100 is refused by PoolMgr::CheckPool
--   ("Pool Id N has no equal chance pooled entites defined and explicit chance sum is not 100", PoolMgr.cpp:772) and never spawns.
--   So its chance is set to 100 (it spawns like any normal un-pooled rare, on its own respawn timer). The 12/88 split was a rare-vs-common
--   choice between two creatures that are 'maps' apart, which cannot be honoured.
-- UNDO:    UPDATE pool_creature SET pool_entry = FLOOR((pool_entry-1000000)/8) WHERE pool_entry >= 1000000;  then  UPDATE pool_creature SET chance=12 WHERE guid=53202;
UPDATE `pool_creature` SET `pool_entry`=1001681 WHERE `pool_entry`=210 AND `guid` IN (53202);
UPDATE `pool_creature` SET `pool_entry`=1240081 WHERE `pool_entry`=30010 AND `guid` IN (318795,371477);
UPDATE `pool_creature` SET `pool_entry`=1240082 WHERE `pool_entry`=30010 AND `guid` IN (371491);
