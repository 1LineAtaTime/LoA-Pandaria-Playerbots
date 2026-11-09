-- Faldir's Cove: 18 Blackwater Deckhands stood on two spots, within 1.2 yd each and inside their 3 yard wander, and 2 on a third: one mob each, many times. No TDB spawns them.
-- Sourced: AzerothCore has one each at (-2101, -2047.5), (-2088.7, -2061.8) and (-2070.5, -1997.3); the copy nearest to those stays. The three other Deckhands below deck are not doubles and stay.
-- Leaves 6. No guid here is in a pool, event, formation, addon, linked respawn or per-spawn script.
DELETE FROM `creature` WHERE `id` = 2636 AND `guid` IN (161255,161264,161303,161332,161337,161339,161253,161257,161268,161280,161285,161287,161291,161340,161344,161346,161261);
