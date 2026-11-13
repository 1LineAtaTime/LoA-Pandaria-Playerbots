-- Keep the existing Gilneas quest encounter available for repeat attempts.
-- Pair with 2026_11_11_04_world_gorerot_ground_spawn.sql.
-- Existing native encounter uses a persistent spawn, not a per-battle summon.
UPDATE creature SET spawntimesecs=60, spawntimesecs_max=60
WHERE id=38331 AND map=654;
