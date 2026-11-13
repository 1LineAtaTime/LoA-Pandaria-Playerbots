-- Hillsbrad: the four credit bunnies of Breaking the Hand stand still (sourced: wander 0 / MovementType 0 in TDB 442 and 1210).
UPDATE `creature` SET `wander_distance`=0,`MovementType`=0 WHERE `guid` IN (175917,175918,175919,175920) AND `id` IN (48862,48864,48865,48867);
