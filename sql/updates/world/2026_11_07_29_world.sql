-- Hillsbrad: Zephyrus stays on his roof. Sourced: static in TDB 4.4.2 and 1210; mop had wander 3 on a raised vmap surface.
UPDATE `creature` SET `MovementType`=0,`wander_distance`=0 WHERE `guid`=177071 AND `id`=49263;
