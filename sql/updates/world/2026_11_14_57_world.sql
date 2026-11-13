-- Naxxramas: remove the second Master Craftsman Omarion 16365 (guid 91803, map 533, 10- and 25-player), a quest giver of
-- Buried Blades 27467 inside the raid.
--  * Sourced: both TDBs (4.4.2, 12.1.0) spawn 16365 only once, in Light's Hope Chapel (guid 320410, the same spot as mop's
--    173812); neither has him in Naxxramas. The classic Omarion sat in a cage of the 40-player Naxxramas over the Eastern
--    Plaguelands; comments on his page from 2010 and 2023 say he is back in the chapel, "rescued from Naxx", and that he is
--    not in the Northrend Naxxramas. This spawn is the classic one carried into the Northrend raid (spawnMask 24), where the
--    same entry offered and took in Buried Blades a continent away from its troopers.
--  * Nothing else refers to the guid (addon, pool, game event, formation, SmartAI, linked respawn: none).
DELETE FROM `creature` WHERE `guid`=91803 AND `id`=16365;
DELETE FROM `creature_addon` WHERE `guid`=91803;
