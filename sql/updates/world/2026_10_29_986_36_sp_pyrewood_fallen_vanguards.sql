-- ported from PR 15 2026_10_27_50_world.sql (#986)
-- Pyrewood Village: the fallen Forsaken Vanguards (45882, Permanent Feign Death) were set to wander, so the corpses slid
-- around the village
UPDATE `creature` SET `MovementType`=0, `wander_distance`=0 WHERE `id`=45882;
