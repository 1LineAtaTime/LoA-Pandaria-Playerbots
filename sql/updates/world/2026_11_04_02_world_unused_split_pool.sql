-- Companion to 2026_10_15_40..43. The current world has no guid 53202,
-- so its pre-generated split template would otherwise introduce an empty pool.
-- Retain it if another installation actually has any member or association.
DELETE FROM `pool_template`
WHERE `entry`=1001681 AND `description`='[map 1, split of pool 210]'
AND NOT EXISTS (SELECT 1 FROM `pool_creature` WHERE `pool_entry`=1001681)
AND NOT EXISTS (SELECT 1 FROM `pool_gameobject` WHERE `pool_entry`=1001681)
AND NOT EXISTS (SELECT 1 FROM `pool_quest` WHERE `pool_entry`=1001681)
AND NOT EXISTS (SELECT 1 FROM `pool_pool` WHERE `pool_id`=1001681 OR `mother_pool`=1001681)
AND NOT EXISTS (SELECT 1 FROM `game_event_pool` WHERE `pool_entry`=1001681);
