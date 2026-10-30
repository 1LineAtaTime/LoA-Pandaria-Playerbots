-- ported from PR 15 2026_10_28_48_world.sql (#986)
-- Brill, Tirisfal Glades: lift an Earthroot node that was spawned below the ground
UPDATE `gameobject` SET `position_z`=29.03 WHERE `guid`=95716 AND `id`=1619;
