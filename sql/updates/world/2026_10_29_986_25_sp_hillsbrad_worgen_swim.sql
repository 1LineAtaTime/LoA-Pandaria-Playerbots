-- ported from PR 15 2026_10_27_02_world.sql (#986)
-- Silverpine Forest, The Waters Run Red...: the Hillsbrad Worgen (45270) swim across Lordamere Lake in waves to the Horde Cannons
-- at the Decrepit Fields. Their 24 sniffed swim paths run from the far shore to ours, but each swimmer was spawned at the point
-- where the sniff saw it, part-way across, and looped its path back and forth. Each one now starts mid-lake, out of sight of the
-- cannons, swims across once and, on reaching our shore, disappears to start again about 15 seconds later; a killed one is back
-- as soon. Their corpse delay is 5 seconds: the respawn time is set at death (or a despawn) as respawn plus corpse delay, so the
-- 60-second default made every cycle 80 seconds long. They start where their
-- path crosses y = 815, not on the far shore, so they come into sight soon. Creatures only update near a player
-- (Map.UpdatableObjectList is off): the waves run while someone is at the cannons and pause when nobody is. The paths move from
-- waypoint_data to SmartAI waypoints, under the swimmer's guid. Retail summons these worgen; the sniffed spawns stand in.
-- They swim in straight lines: the navmesh puts water paths on the surface, where a creature counts as walking on the water, so
-- it lost its swim flag and dropped under the surface at every step.
UPDATE `creature_template` SET `AIName` = 'SmartAI', `flags_extra` = 536870912 WHERE `entry` = 45270; -- CREATURE_FLAG_EXTRA_IGNORE_PATHFINDING

UPDATE `creature` SET `position_x` = 673.0563, `position_y` = 815.0000, `position_z` = 32.1237, `orientation` = 1.7735, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307216 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 714.4180, `position_y` = 815.0000, `position_z` = 31.7415, `orientation` = 1.9167, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307217 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 781.4416, `position_y` = 815.0000, `position_z` = 32.5506, `orientation` = 1.6586, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307218 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 714.4180, `position_y` = 815.0000, `position_z` = 31.7415, `orientation` = 1.9167, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307220 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 781.4416, `position_y` = 815.0000, `position_z` = 32.5506, `orientation` = 1.6586, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307222 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 781.4701, `position_y` = 815.0000, `position_z` = 32.5508, `orientation` = 1.6588, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307223 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 781.3030, `position_y` = 815.0000, `position_z` = 32.5531, `orientation` = 1.6578, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307225 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 673.0563, `position_y` = 815.0000, `position_z` = 32.1237, `orientation` = 1.7735, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307226 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 699.7828, `position_y` = 815.0000, `position_z` = 30.2523, `orientation` = 1.7778, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307231 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 673.0563, `position_y` = 815.0000, `position_z` = 32.1237, `orientation` = 1.7735, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307232 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 714.4180, `position_y` = 815.0000, `position_z` = 31.7415, `orientation` = 1.9167, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307234 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 700.7681, `position_y` = 815.0000, `position_z` = 32.0569, `orientation` = 1.9769, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307235 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 781.4701, `position_y` = 815.0000, `position_z` = 32.5508, `orientation` = 1.6588, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307237 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 781.4416, `position_y` = 815.0000, `position_z` = 32.5506, `orientation` = 1.6586, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307239 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 781.4701, `position_y` = 815.0000, `position_z` = 32.5508, `orientation` = 1.6588, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307241 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 673.0563, `position_y` = 815.0000, `position_z` = 32.1237, `orientation` = 1.7735, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307243 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 673.0563, `position_y` = 815.0000, `position_z` = 32.1237, `orientation` = 1.7735, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307247 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 774.5820, `position_y` = 815.0000, `position_z` = 32.6233, `orientation` = 1.4344, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307249 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 781.4701, `position_y` = 815.0000, `position_z` = 32.5508, `orientation` = 1.6588, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307250 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 774.5820, `position_y` = 815.0000, `position_z` = 32.6233, `orientation` = 1.4344, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307253 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 774.5820, `position_y` = 815.0000, `position_z` = 32.6233, `orientation` = 1.4344, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307257 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 673.0563, `position_y` = 815.0000, `position_z` = 32.1237, `orientation` = 1.7735, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307260 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 673.0563, `position_y` = 815.0000, `position_z` = 32.1237, `orientation` = 1.7735, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307263 AND `id` = 45270;
UPDATE `creature` SET `position_x` = 673.0563, `position_y` = 815.0000, `position_z` = 32.1237, `orientation` = 1.7735, `MovementType` = 0, `spawntimesecs` = 10 WHERE `guid` = 307264 AND `id` = 45270;

UPDATE `creature_addon` SET `path_id` = 0 WHERE `guid` IN (307216, 307217, 307218, 307220, 307222, 307223, 307225, 307226, 307231, 307232, 307234, 307235, 307237, 307239, 307241, 307243, 307247, 307249, 307250, 307253, 307257, 307260, 307263, 307264);
DELETE FROM `waypoint_data` WHERE `id` IN (307216, 307217, 307218, 307220, 307222, 307223, 307225, 307226, 307231, 307232, 307234, 307235, 307237, 307239, 307241, 307243, 307247, 307249, 307250, 307253, 307257, 307260, 307263, 307264);

DELETE FROM `waypoints` WHERE `entry` IN (307216, 307217, 307218, 307220, 307222, 307223, 307225, 307226, 307231, 307232, 307234, 307235, 307237, 307239, 307241, 307243, 307247, 307249, 307250, 307253, 307257, 307260, 307263, 307264);
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `point_comment`) VALUES
(307216, 1, 673.0563, 815.0000, 32.1237, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307216, 2, 645.1440, 950.8960, 30.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307216, 3, 644.7420, 952.8560, 32.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307216, 4, 644.3400, 954.8150, 33.1091, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307216, 5, 643.9380, 956.7730, 33.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307216, 6, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307216, 7, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307217, 1, 714.4180, 815.0000, 31.7415, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307217, 2, 683.4680, 900.8940, 32.7460, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307217, 3, 661.7210, 961.0810, 31.4841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307217, 4, 661.0410, 962.9620, 32.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307217, 5, 660.3610, 964.8430, 32.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307217, 6, 659.5490, 967.2670, 33.5215, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307217, 7, 659.5490, 967.2670, 33.5215, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307218, 1, 781.4416, 815.0000, 32.5506, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307218, 2, 763.7060, 1016.0000, 31.9335, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307218, 3, 763.6180, 1017.0000, 30.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307218, 4, 763.5300, 1017.9900, 31.6117, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307218, 5, 763.4860, 1019.0200, 31.9013, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307218, 6, 763.4860, 1019.0200, 31.9013, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307220, 1, 714.4180, 815.0000, 31.7415, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307220, 2, 683.4680, 900.8940, 32.7460, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307220, 3, 661.7210, 961.0810, 31.4841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307220, 4, 661.0410, 962.9620, 32.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307220, 5, 660.3610, 964.8430, 32.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307220, 6, 659.5490, 967.2670, 33.5215, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307220, 7, 659.5490, 967.2670, 33.5215, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307222, 1, 781.4416, 815.0000, 32.5506, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307222, 2, 763.7060, 1016.0000, 31.9335, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307222, 3, 763.6180, 1017.0000, 30.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307222, 4, 763.5300, 1017.9900, 31.6117, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307222, 5, 763.4860, 1019.0200, 31.9013, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307222, 6, 763.4860, 1019.0200, 31.9013, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307223, 1, 781.4701, 815.0000, 32.5508, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307223, 2, 763.7380, 1015.9600, 31.9349, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307223, 3, 763.6500, 1016.9600, 30.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307223, 4, 763.4750, 1018.9500, 32.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307223, 5, 763.4750, 1018.9500, 32.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307225, 1, 781.3030, 815.0000, 32.5531, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307225, 2, 763.7770, 1015.8600, 31.9363, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307225, 3, 763.6900, 1016.8600, 30.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307225, 4, 763.5170, 1018.8500, 32.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307225, 5, 763.5170, 1018.8500, 32.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307226, 1, 673.0563, 815.0000, 32.1237, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307226, 2, 645.1440, 950.8960, 30.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307226, 3, 644.7420, 952.8560, 32.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307226, 4, 644.3400, 954.8150, 33.1091, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307226, 5, 643.9380, 956.7730, 33.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307226, 6, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307226, 7, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307231, 1, 699.7828, 815.0000, 30.2523, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307231, 2, 668.5160, 963.7320, 31.9602, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307231, 3, 667.9000, 966.6680, 31.7227, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307231, 4, 667.6270, 968.0830, 32.0098, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307231, 5, 667.6270, 968.0830, 32.0098, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307232, 1, 673.0563, 815.0000, 32.1237, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307232, 2, 645.1440, 950.8960, 30.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307232, 3, 644.7420, 952.8560, 32.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307232, 4, 644.3400, 954.8150, 33.1091, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307232, 5, 643.9380, 956.7730, 33.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307232, 6, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307232, 7, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307234, 1, 714.4180, 815.0000, 31.7415, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307234, 2, 683.4680, 900.8940, 32.7460, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307234, 3, 661.7210, 961.0810, 31.4841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307234, 4, 661.0410, 962.9620, 32.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307234, 5, 660.3610, 964.8430, 32.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307234, 6, 659.5490, 967.2670, 33.5215, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307234, 7, 659.5490, 967.2670, 33.5215, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307235, 1, 700.7681, 815.0000, 32.0569, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307235, 2, 688.4370, 843.6510, 32.7414, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307235, 3, 643.4450, 948.4110, 31.3591, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307235, 4, 642.6560, 950.2490, 32.6091, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307235, 5, 641.4730, 953.0060, 33.6091, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307235, 6, 640.6840, 954.8440, 34.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307235, 7, 640.2890, 955.7630, 35.1091, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307235, 8, 639.9480, 956.4320, 35.4373, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307235, 9, 639.9480, 956.4320, 35.4373, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307237, 1, 781.4701, 815.0000, 32.5508, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307237, 2, 763.7380, 1015.9600, 31.9349, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307237, 3, 763.6500, 1016.9600, 30.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307237, 4, 763.4750, 1018.9500, 32.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307237, 5, 763.4750, 1018.9500, 32.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307239, 1, 781.4416, 815.0000, 32.5506, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307239, 2, 763.7060, 1016.0000, 31.9335, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307239, 3, 763.6180, 1017.0000, 30.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307239, 4, 763.5300, 1017.9900, 31.6117, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307239, 5, 763.4860, 1019.0200, 31.9013, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307239, 6, 763.4860, 1019.0200, 31.9013, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307241, 1, 781.4701, 815.0000, 32.5508, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307241, 2, 763.7380, 1015.9600, 31.9349, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307241, 3, 763.6500, 1016.9600, 30.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307241, 4, 763.4750, 1018.9500, 32.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307241, 5, 763.4750, 1018.9500, 32.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307243, 1, 673.0563, 815.0000, 32.1237, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307243, 2, 645.1440, 950.8960, 30.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307243, 3, 644.7420, 952.8560, 32.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307243, 4, 644.3400, 954.8150, 33.1091, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307243, 5, 643.9380, 956.7730, 33.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307243, 6, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307243, 7, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307247, 1, 673.0563, 815.0000, 32.1237, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307247, 2, 645.1440, 950.8960, 30.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307247, 3, 644.7420, 952.8560, 32.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307247, 4, 644.3400, 954.8150, 33.1091, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307247, 5, 643.9380, 956.7730, 33.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307247, 6, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307247, 7, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307249, 1, 774.5820, 815.0000, 32.6233, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307249, 2, 803.5780, 1026.5300, 32.2218, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307249, 3, 803.7140, 1027.5200, 31.0908, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307249, 4, 804.0210, 1029.5100, 32.2092, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307249, 5, 804.0210, 1029.5100, 32.2092, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307250, 1, 781.4701, 815.0000, 32.5508, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307250, 2, 763.7380, 1015.9600, 31.9349, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307250, 3, 763.6500, 1016.9600, 30.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307250, 4, 763.4750, 1018.9500, 32.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307250, 5, 763.4750, 1018.9500, 32.3617, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307253, 1, 774.5820, 815.0000, 32.6233, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307253, 2, 803.5780, 1026.5300, 32.2218, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307253, 3, 803.7140, 1027.5200, 31.0908, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307253, 4, 804.0210, 1029.5100, 32.2092, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307253, 5, 804.0210, 1029.5100, 32.2092, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307257, 1, 774.5820, 815.0000, 32.6233, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307257, 2, 803.5780, 1026.5300, 32.2218, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307257, 3, 803.7140, 1027.5200, 31.0908, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307257, 4, 804.0210, 1029.5100, 32.2092, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307257, 5, 804.0210, 1029.5100, 32.2092, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307260, 1, 673.0563, 815.0000, 32.1237, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307260, 2, 645.1440, 950.8960, 30.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307260, 3, 644.7420, 952.8560, 32.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307260, 4, 644.3400, 954.8150, 33.1091, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307260, 5, 643.9380, 956.7730, 33.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307260, 6, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307260, 7, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307263, 1, 673.0563, 815.0000, 32.1237, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307263, 2, 645.1440, 950.8960, 30.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307263, 3, 644.7420, 952.8560, 32.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307263, 4, 644.3400, 954.8150, 33.1091, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307263, 5, 643.9380, 956.7730, 33.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307263, 6, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307263, 7, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307264, 1, 673.0563, 815.0000, 32.1237, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307264, 2, 645.1440, 950.8960, 30.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307264, 3, 644.7420, 952.8560, 32.2341, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307264, 4, 644.3400, 954.8150, 33.1091, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307264, 5, 643.9380, 956.7730, 33.9841, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307264, 6, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake'),
(307264, 7, 643.8300, 957.1980, 34.2948, 0, 0, 'Hillsbrad Worgen - Swim across Lordamere Lake');

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (-307216, -307217, -307218, -307220, -307222, -307223, -307225, -307226, -307231, -307232, -307234, -307235, -307237, -307239, -307241, -307243, -307247, -307249, -307250, -307253, -307257, -307260, -307263, -307264) AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(-307216, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307216, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307216, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307217, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307217, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307217, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307218, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307218, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307218, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307220, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307220, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307220, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307222, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307222, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307222, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307223, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307223, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307223, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307225, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307225, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307225, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307226, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307226, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307226, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307231, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307231, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307231, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307232, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307232, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307232, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307234, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307234, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307234, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307235, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307235, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307235, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307237, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307237, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307237, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307239, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307239, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307239, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307241, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307241, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307241, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307243, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307243, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307243, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307247, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307247, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307247, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307249, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307249, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307249, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307250, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307250, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307250, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307253, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307253, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307253, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307257, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307257, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307257, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307260, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307260, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307260, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307263, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307263, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307263, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds'),
(-307264, 0, 0, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 307264, 0, 0, 1000, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Swim across Lordamere Lake, then despawn'),
(-307264, 0, 1, 0, 11, 0, 100, 0, 0, 0, 0, 0, 0, 116, 5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Hillsbrad Worgen - On Respawn - Set Corpse Delay 5 seconds');
