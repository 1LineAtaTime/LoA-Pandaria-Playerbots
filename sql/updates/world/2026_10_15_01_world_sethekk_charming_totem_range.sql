-- #633 Charming Totem 20343: the charm row shipped with target_param1 0 (SMART_TARGET_PLAYER_RANGE needs a max distance), so it
-- was skipped at boot ('has maxDist 0 as target_param1, skipped'). AC acore_world has 40.
UPDATE `smart_scripts` SET `target_param1` = 40 WHERE `entryorguid` = 20343 AND `source_type` = 0 AND `id` = 0 AND `target_type` = 21;
