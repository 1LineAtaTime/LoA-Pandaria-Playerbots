-- Published 2026_08_17_world_04.sql, adapted to the active local schema.
-- Required by the paired native detection-range loader and live template reload.
ALTER TABLE creature_template ADD COLUMN IF NOT EXISTS detection_range FLOAT NOT NULL DEFAULT 20;
UPDATE creature_template SET detection_range=10 WHERE maxlevel<=6;
