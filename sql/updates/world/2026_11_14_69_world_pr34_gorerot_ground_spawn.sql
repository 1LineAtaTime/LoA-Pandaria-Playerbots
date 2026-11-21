-- Gorerot belongs in the prison courtyard, not on the roof.
-- Position from ArkCORE-NG a7304c3075bf8ee7eb5bdf45f31cda01c8626b52,
-- zone_gilneas_city2.cpp: SpawnPosGorerot (GPL-2.0-or-later).
-- https://github.com/Arkania/ArkCORE-NG/blob/a7304c3075bf8ee7eb5bdf45f31cda01c8626b52/src/server/scripts/EasternKingdoms/Gilneas/zone_gilneas_city2.cpp
-- Applies only to the existing Gilneas battle actor; no character progress changes.
UPDATE creature
SET position_x = -1703.10, position_y = 1419.87,
    position_z = 21.9268, orientation = 3.89208
WHERE id = 38331 AND map = 654;
