-- #989 (Refs #989): Ragefire Chasm MoP layout
-- Source: DeltaTho playerbots-fork PR 15, file 2026_10_26_19 (commit as in deltatho/playerbots-fork), ported as-is unless noted below.
-- Design check 2026-10-29: Wowpedia Ragefire Chasm (MediaWiki API, 5.0.4 redesign): 4 bosses Adarogg, Dark Shaman Koranthal, Slagmaw, Lava Guard Gordoth; trash flame hounds / Corrupted Flamecaller/Houndmaster/Reaver / Dark Shaman Acolyte+Researcher; Horde NPCs Xorenth, Bagran, Kor'kron Elite/Scout; Alliance NPCs Dirken, Aryna, SI:7 Ranger; Horde/Alliance quest pairs (A New Enemy, The Dark Shaman, Animal Control, No Orc/Man Left Behind). Wowhead tooltip API confirms all quest-NPC names and zone 2437. Cata quest givers 44216/44217 removed (Wowpedia: quests removed in 5.0.4). Guids 4045000-4045129 (creature), 4045000-4045016 (gameobject): collision-checked against the test DB (0 rows in 4045000-4045999 before apply).
-- Idempotent: DELETE+INSERT / absolute UPDATEs.

-- Ragefire Chasm (389): replace the Cataclysm layout with the Mists of Pandaria one
--
-- Patch 5.0.4 rebuilt Ragefire Chasm and removed all of its Cataclysm quests, but mop_world still spawned the Cataclysm trash,
-- Oggleflint/Jergosh/Bazzalan and Kurjack's quest camp, with only the four MoP bosses and Commander Bagran added on top. This
-- file removes the Cataclysm creatures and ports the MoP layout from a retail sniff (TrinityCore TDB 1210.26091, which still has
-- the 5.0.4 instance): trash packs with their formations, idle paths, auras and emotes, the Kor'kron (Horde) and SI:7 (Alliance)
-- quest NPCs, the rescue events for No Orc Left Behind / No Man Left Behind, and the quest givers who come to the end of the
-- dungeon after Lava Guard Gordoth dies.
--
-- Guids: creatures 4045000-4045129 (new), gameobjects 4045000-4045016 (new). Existing MoP spawns kept:
--   564238 Lava Guard Gordoth - kept where it is: the sniff has him inside the Gordoth Cage (211792), which LoA spawns closed with
--          collision and has no script to open, so he would be unreachable;
--   564239 Slagmaw - kept: boss_slagmaw's first submerge point (SlagMawPoints[0]) is this spot;
--   564240 Koranthal, 564241 Adarogg, 564242 Commander Bagran - moved to their sniffed spots (Koranthal stands on his Ground Runes);
--   375361-375363 Summon Enabler Stalkers - identical to the sniff.
--
-- Factions: retail shows the Kor'kron to the Horde and the SI:7 to the Alliance through spawn groups, which LoA lacks. Here the
-- zone's phase_definitions give Alliance players phase 1+2 and Horde players phase 1+4; SI:7 spawns are in phase 2, Kor'kron in 4.
-- The end-of-dungeon groups wait in phase 8 (Alliance) / 16 (Horde) and move into 2 / 4 when Gordoth's encounter is DONE,
-- because LoA does not re-evaluate phase_definitions when a boss state changes.
--
-- Levels: Wowhead MoP Classic lists the trash, the scouts and the rangers at 14, the Dark Shaman Researchers at 15 and the
-- Kor'kron/SI:7 officers at 20 (the bosses are already 16). Retail uses classic health scaling for all of them
-- (HealthScalingExpansion 0), so their exp becomes 0: at exp 4 LoA's level 14-20 base health is 1 for every class but warriors.

-- 1. Cataclysm creatures: troggs, Searing Blade, Earthborers, Molten Elementals, the three old bosses, Bovaal Whitehorn,
--    Stone Guard Kurjack and the Orgrimmar Defenders (90 spawns). 10 of them had idle paths; nothing else refers to them.
DELETE FROM `waypoint_data` WHERE `id` IN (
319381, 319384, 319400, 319407, 319435, 319439, 319440, 319446, 319451, 319461);
DELETE FROM `creature_addon` WHERE `guid` IN (
319379, 319380, 319381, 319382, 319383, 319384, 319385, 319386, 319387, 319388, 319389, 319390, 319391, 319392, 319393, 319394,
319395, 319396, 319397, 319398, 319399, 319400, 319401, 319402, 319403, 319404, 319405, 319406, 319407, 319408, 319410, 319411,
319412, 319413, 319414, 319415, 319416, 319417, 319418, 319419, 319420, 319421, 319422, 319423, 319424, 319425, 319426, 319427,
319428, 319429, 319430, 319431, 319432, 319433, 319434, 319435, 319436, 319437, 319438, 319439, 319440, 319441, 319442, 319443,
319444, 319445, 319446, 319447, 319448, 319449, 319450, 319451, 319452, 319453, 319454, 319455, 319456, 319457, 319458, 319459,
319460, 319461, 319462, 319463, 319464, 319465, 319466, 319467, 319468, 319469);
DELETE FROM `creature` WHERE `map` = 389 AND `id` IN (11318, 11319, 11320, 11321, 11322, 11323, 11324, 11517, 11518, 11519, 44216, 44217, 44223) AND `guid` IN (
319379, 319380, 319381, 319382, 319383, 319384, 319385, 319386, 319387, 319388, 319389, 319390, 319391, 319392, 319393, 319394,
319395, 319396, 319397, 319398, 319399, 319400, 319401, 319402, 319403, 319404, 319405, 319406, 319407, 319408, 319410, 319411,
319412, 319413, 319414, 319415, 319416, 319417, 319418, 319419, 319420, 319421, 319422, 319423, 319424, 319425, 319426, 319427,
319428, 319429, 319430, 319431, 319432, 319433, 319434, 319435, 319436, 319437, 319438, 319439, 319440, 319441, 319442, 319443,
319444, 319445, 319446, 319447, 319448, 319449, 319450, 319451, 319452, 319453, 319454, 319455, 319456, 319457, 319458, 319459,
319460, 319461, 319462, 319463, 319464, 319465, 319466, 319467, 319468, 319469);

-- Blood Filled Orb summoned Zelemar the Wrathful for a pre-Cataclysm quest; the MoP instance has neither
DELETE FROM `gameobject` WHERE `guid` = 187683 AND `id` = 182024;

-- 2. MoP creatures (TDB guids 4000000-4000137 in order, minus the 8 reused spawns above)
SET @CGUID := 4045000;
DELETE FROM `creature_formations` WHERE `leaderGUID` BETWEEN @CGUID+0 AND @CGUID+129 OR `memberGUID` BETWEEN @CGUID+0 AND @CGUID+129;
DELETE FROM `creature_addon` WHERE `guid` BETWEEN @CGUID+0 AND @CGUID+129;
DELETE FROM `waypoint_data` WHERE `id` BETWEEN (@CGUID+0)*10 AND (@CGUID+129)*10;
DELETE FROM `creature` WHERE `guid` BETWEEN @CGUID+0 AND @CGUID+129;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`,
`equipment_id`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`) VALUES
(@CGUID+0, 61657, 389, 2437, 2437, 2, 1, -62.6195, -32.0749, -18.3342, 5.01637, 0, 7200, 0, 0, 0, 0, 0, 0), -- Adolescent Flame Hound
(@CGUID+1, 61657, 389, 2437, 2437, 2, 1, -59.5038, -35.413, -18.9467, 2.76572, 0, 7200, 0, 0, 0, 0, 0, 0), -- Adolescent Flame Hound
(@CGUID+2, 61658, 389, 2437, 2437, 2, 1, -78.4044, -34.6446, -19.7785, 0, 0, 7200, 0, 0, 0, 0, 2, 0), -- Mature Flame Hound
(@CGUID+3, 61560, 389, 2437, 2437, 2, 1, -98.9559, -50.4502, -33.7143, 1.64341, 0, 7200, 0, 0, 0, 0, 2, 0), -- Lava
(@CGUID+4, 61657, 389, 2437, 2437, 2, 1, -61.6939, -37.014, -18.4917, 1.68224, 0, 7200, 0, 0, 0, 0, 0, 0), -- Adolescent Flame Hound
(@CGUID+5, 61659, 389, 2437, 2437, 2, 1, -99.4745, -38.764, -29.2049, 3.3959, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+6, 61657, 389, 2437, 2437, 2, 1, -75.064, -37.5734, -18.7147, 6.04074, 0, 7200, 0, 0, 0, 0, 0, 0), -- Adolescent Flame Hound
(@CGUID+7, 61716, 389, 2437, 2437, 2, 4, -16.5848, -47.4531, -21.7804, 1.11719, 0, 7200, 0, 0, 0, 0, 0, 0), -- Invoker Xorenth
(@CGUID+8, 61657, 389, 2437, 2437, 2, 1, -78.229, -30.6828, -19.1404, 6.28124, 0, 7200, 0, 0, 0, 0, 0, 0), -- Adolescent Flame Hound
(@CGUID+9, 61658, 389, 2437, 2437, 2, 1, -127.022, -40.4365, -32.6678, 0, 0, 7200, 0, 0, 0, 0, 0, 0), -- Mature Flame Hound
(@CGUID+10, 61659, 389, 2437, 2437, 2, 1, -61.5377, -32.6011, -18.4476, 3.00772, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+11, 61404, 389, 2437, 2437, 2, 4, -26.0675, -57.8228, -21.2882, 2.62083, 0, 7200, 0, 0, 0, 0, 0, 0), -- Kor'kron Elite
(@CGUID+12, 61657, 389, 2437, 2437, 2, 1, -74.9759, -32.3497, -18.9412, 6.28027, 0, 7200, 0, 0, 0, 0, 0, 0), -- Adolescent Flame Hound
(@CGUID+13, 61680, 389, 2437, 2437, 2, 4, -15.9522, -44.834, -21.6942, 6.0768, 0, 7200, 0, 0, 0, 0, 0, 0), -- Kor'kron Scout
(@CGUID+14, 61658, 389, 2437, 2437, 2, 1, -127.89, -30.7097, -32.7365, 6.26038, 0, 7200, 0, 0, 0, 0, 0, 0), -- Mature Flame Hound
(@CGUID+15, 61404, 389, 2437, 2437, 2, 4, -22.3918, -42.5715, -21.6953, 2.92225, 0, 7200, 0, 0, 0, 0, 0, 0), -- Kor'kron Elite
(@CGUID+16, 61657, 389, 2437, 2437, 2, 1, -78.1841, -35.1548, -19.592, 6.12138, 0, 7200, 0, 0, 0, 0, 0, 0), -- Adolescent Flame Hound
(@CGUID+17, 61672, 389, 2437, 2437, 2, 1, -146.053, 12.4743, -22.08, 2.89901, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Acolyte
(@CGUID+18, 61672, 389, 2437, 2437, 2, 1, -105.082, 41.4172, -17.989, 1.79953, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Acolyte
(@CGUID+19, 61659, 389, 2437, 2437, 2, 1, -140.49, -17.8001, -54.6612, 4.04904, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+20, 61680, 389, 2437, 2437, 2, 4, -88.894, 68.7328, -18.5693, 3.61982, 0, 7200, 0, 0, 0, 0, 0, 0), -- Kor'kron Scout
(@CGUID+21, 61658, 389, 2437, 2437, 2, 1, -151.034, -29.7282, -35.6131, 4.1222, 0, 7200, 0, 0, 0, 0, 0, 0), -- Mature Flame Hound
(@CGUID+22, 61657, 389, 2437, 2437, 2, 1, -153.819, -33.3491, -36.7905, 0.495185, 0, 7200, 0, 0, 0, 0, 0, 0), -- Adolescent Flame Hound
(@CGUID+23, 61672, 389, 2437, 2437, 2, 1, -96.2438, 47.405, -17.4781, 2.66912, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Acolyte
(@CGUID+24, 61678, 389, 2437, 2437, 2, 1, -132.798, 11.9237, -20.6849, 3.00788, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Reaver
(@CGUID+25, 61790, 389, 2437, 2437, 2, 1, -88.9257, 68.8181, -17.734, 3.78904, 0, 7200, 0, 0, 0, 0, 0, 0), -- Inconspicuous Crate
(@CGUID+26, 61659, 389, 2437, 2437, 2, 1, -145.333, -61.9615, -58.935, 2.29627, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+27, 61678, 389, 2437, 2437, 2, 1, -99.6469, 42.6892, -17.4792, 2.14266, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Reaver
(@CGUID+28, 61672, 389, 2437, 2437, 2, 1, -134.24, 3.84759, -20.6104, 3.00788, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Acolyte
(@CGUID+29, 61657, 389, 2437, 2437, 2, 1, -149.702, -33.9839, -35.3104, 2.88195, 0, 7200, 0, 0, 0, 0, 0, 0), -- Adolescent Flame Hound
(@CGUID+30, 61678, 389, 2437, 2437, 2, 1, -167.943, 11.8834, -29.5828, 0, 0, 7200, 0, 0, 0, 0, 2, 0), -- Corrupted Reaver
(@CGUID+31, 61659, 389, 2437, 2437, 2, 1, -150.899, -32.5604, -35.6334, 1.90356, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+32, 61659, 389, 2437, 2437, 2, 1, -150.554, 0.980218, -39.5868, 2.29627, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+33, 61795, 389, 2437, 2437, 2, 1, -163.859, -6.7062, -63.4372, 4.83534, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dancing Flames
(@CGUID+34, 61659, 389, 2437, 2437, 2, 1, -149.863, 23.8116, -38.5054, 0.416137, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+35, 61666, 389, 2437, 2437, 2, 1, -275.043, -63.9595, -60.3566, 2.27193, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Houndmaster
(@CGUID+36, 61659, 389, 2437, 2437, 2, 1, -287.345, -77.7271, -60.5109, 1.58424, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+37, 61658, 389, 2437, 2437, 2, 1, -216.08, -33.6877, -53.6355, 3.30043, 0, 7200, 0, 0, 0, 0, 0, 0), -- Mature Flame Hound
(@CGUID+38, 61680, 389, 2437, 2437, 2, 4, -172.466, 1.87796, -33.0117, 1.64543, 0, 7200, 0, 0, 0, 0, 0, 16777216), -- Kor'kron Scout
(@CGUID+39, 61678, 389, 2437, 2437, 2, 1, -286.815, 9.52373, -46.223, 0, 0, 7200, 0, 0, 0, 0, 2, 0), -- Corrupted Reaver
(@CGUID+40, 61795, 389, 2437, 2437, 2, 1, -178.007, -54.5245, -63.4446, 4.77359, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dancing Flames
(@CGUID+41, 61678, 389, 2437, 2437, 2, 1, -191.725, 146.23, -25.1366, 2.76122, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Reaver
(@CGUID+42, 61705, 389, 2437, 2437, 2, 1, -265.408, 99.229, -24.9408, 1.38821, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Flamecaller
(@CGUID+43, 61659, 389, 2437, 2437, 2, 1, -146.191, 24.2084, -38.7544, 4.87271, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+44, 61659, 389, 2437, 2437, 2, 1, -286.829, -77.5806, -60.5104, 4.29094, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+45, 61795, 389, 2437, 2437, 2, 1, -172.789, -8.29691, -63.0922, 4.83534, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dancing Flames
(@CGUID+46, 61601, 389, 2437, 2437, 2, 1, -276.397, 11.5205, -45.2924, 0, 0, 7200, 0, 0, 0, 0, 2, 0), -- Lava
(@CGUID+47, 61666, 389, 2437, 2437, 2, 1, -273.028, -50.9456, -60.7311, 3.38415, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Houndmaster
(@CGUID+48, 61413, 389, 2437, 2437, 2, 1, -117.044, 71.0705, 1.37577, 0, 0, 7200, 0, 0, 0, 0, 0, 0), -- Flame Visual
(@CGUID+49, 61795, 389, 2437, 2437, 2, 1, -204.258, -58.2133, -63.7113, 4.77359, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dancing Flames
(@CGUID+50, 61630, 389, 2437, 2437, 2, 1, -117.044, 71.0705, -20.7203, 5.35135, 0, 7200, 0, 0, 0, 0, 0, 0), -- Ground Runes
(@CGUID+51, 61659, 389, 2437, 2437, 2, 1, -181.101, 4.53392, -55.7473, 3.2368, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+52, 61678, 389, 2437, 2437, 2, 1, -272.643, 101.703, -24.9312, 0.921121, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Reaver
(@CGUID+53, 61825, 389, 2437, 2437, 2, 2, -14.5404, -63.1119, -21.2901, 4.29036, 0, 7200, 0, 0, 0, 0, 0, 0), -- Stormwind Portal
(@CGUID+54, 61680, 389, 2437, 2437, 2, 4, -310.909, -24.3211, -58.851, 0, 0, 7200, 0, 0, 0, 0, 0, 0), -- Kor'kron Scout
(@CGUID+55, 61659, 389, 2437, 2437, 2, 1, -175.254, -65.8698, -57.7781, 4.20285, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+56, 61659, 389, 2437, 2437, 2, 1, -253.731, -62.3931, -61.9606, 2.80024, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+57, 61795, 389, 2437, 2437, 2, 1, -179.942, -9.48245, -63.1635, 4.83534, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dancing Flames
(@CGUID+58, 61678, 389, 2437, 2437, 2, 1, -190.944, 162.391, -25.1366, 3.50185, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Reaver
(@CGUID+59, 61705, 389, 2437, 2437, 2, 1, -189.287, 154.579, -25.1366, 3.17954, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Flamecaller
(@CGUID+60, 61672, 389, 2437, 2437, 2, 1, -206.744, 90.8302, -25.2989, 2.3596, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Acolyte
(@CGUID+61, 61795, 389, 2437, 2437, 2, 1, -198.762, -13.5985, -63.3259, 4.83534, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dancing Flames
(@CGUID+62, 61659, 389, 2437, 2437, 2, 1, -202.531, -0.913089, -61.8754, 2.03186, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+63, 61666, 389, 2437, 2437, 2, 1, -280.789, -40.198, -60.6165, 4.4242, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Houndmaster
(@CGUID+64, 61658, 389, 2437, 2437, 2, 1, -214.955, -40.6686, -53.3408, 3.30043, 0, 7200, 0, 0, 0, 0, 0, 0), -- Mature Flame Hound
(@CGUID+65, 61659, 389, 2437, 2437, 2, 1, -288.378, -79.4173, -60.5169, 5.15742, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+66, 61659, 389, 2437, 2437, 2, 1, -227.945, -64.4667, -60.9304, 2.29627, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+67, 61795, 389, 2437, 2437, 2, 1, -190.235, -11.5225, -63.562, 4.83534, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dancing Flames
(@CGUID+68, 61678, 389, 2437, 2437, 2, 1, -257.837, 97.8295, -24.9346, 1.63576, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Reaver
(@CGUID+69, 61672, 389, 2437, 2437, 2, 1, -223.386, 9.90856, -43.495, 3.19938, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Acolyte
(@CGUID+70, 61659, 389, 2437, 2437, 2, 1, -253.562, -6.75165, -62.9747, 2.3414, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+71, 61659, 389, 2437, 2437, 2, 1, -203.313, -71.12, -63.0215, 3.3959, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+72, 61680, 389, 2437, 2437, 2, 4, -257.035, -49.3293, -60.6823, 3.41253, 0, 7200, 0, 0, 0, 0, 0, 0), -- Kor'kron Scout
(@CGUID+73, 61672, 389, 2437, 2437, 2, 1, -202.755, 94.8417, -25.5049, 2.35953, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Acolyte
(@CGUID+74, 61669, 389, 2437, 2437, 2, 1, -276.682, -34.6181, -60.6085, 0, 0, 7200, 0, 0, 0, 0, 0, 0), -- Oggleflint
(@CGUID+75, 61666, 389, 2437, 2437, 2, 1, -241.534, -37.5544, -57.5347, 0.0478053, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Houndmaster
(@CGUID+76, 61795, 389, 2437, 2437, 2, 1, -168.996, -53.4878, -63.774, 4.77359, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dancing Flames
(@CGUID+77, 61658, 389, 2437, 2437, 2, 1, -243.829, -42.8094, -58.0734, 0.0948671, 0, 7200, 0, 0, 0, 0, 0, 0), -- Mature Flame Hound
(@CGUID+78, 61659, 389, 2437, 2437, 2, 1, -286.218, -79.0774, -60.5157, 3.00772, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+79, 61659, 389, 2437, 2437, 2, 1, -222.492, -10.768, -61.721, 3.66177, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+80, 61659, 389, 2437, 2437, 2, 1, -263.848, -93.3509, -61.7372, 4.83414, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+81, 61666, 389, 2437, 2437, 2, 1, -202.69, -35.1236, -49.1017, 0, 0, 7200, 0, 0, 0, 0, 2, 0), -- Corrupted Houndmaster
(@CGUID+82, 61795, 389, 2437, 2437, 2, 1, -195.62, -56.6674, -63.4984, 4.77359, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dancing Flames
(@CGUID+83, 61658, 389, 2437, 2437, 2, 1, -258.193, -49.7012, -60.6389, 0.425607, 0, 7200, 0, 0, 0, 0, 0, 0), -- Mature Flame Hound
(@CGUID+84, 61780, 389, 2437, 2437, 2, 1, -311.041, -24.0794, -59.3486, 0, 0, 7200, 0, 0, 0, 0, 0, 0), -- Suspicious Rock
(@CGUID+85, 61672, 389, 2437, 2437, 2, 1, -195.689, 6.81701, -34.5062, 0, 0, 7200, 0, 0, 0, 0, 2, 0), -- Dark Shaman Acolyte
(@CGUID+86, 61795, 389, 2437, 2437, 2, 1, -185.22, -55.2664, -63.8084, 4.77359, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dancing Flames
(@CGUID+87, 61658, 389, 2437, 2437, 2, 1, -244.689, -32.999, -58.1244, 0.0478053, 0, 7200, 0, 0, 0, 0, 0, 0), -- Mature Flame Hound
(@CGUID+88, 61659, 389, 2437, 2437, 2, 1, -161.739, -30.8162, -58.2288, 2.31221, 0, 7200, 0, 0, 0, 0, 0, 0), -- Slain Trogg
(@CGUID+89, 61678, 389, 2437, 2437, 2, 1, -183.871, 77.9451, -22.2752, 0, 0, 7200, 0, 0, 0, 0, 2, 0), -- Corrupted Reaver
(@CGUID+90, 61680, 389, 2437, 2437, 2, 4, -286.979, 134.491, -27.0197, 1.85857, 0, 7200, 0, 0, 0, 0, 0, 16777216), -- Kor'kron Scout
(@CGUID+91, 61678, 389, 2437, 2437, 2, 1, -222.389, 205.345, -24.9083, 4.25548, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Reaver
(@CGUID+92, 61678, 389, 2437, 2437, 2, 1, -294.764, 139.549, -25.0472, 0.311638, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Reaver
(@CGUID+93, 61705, 389, 2437, 2437, 2, 1, -231.043, 206.925, -24.7876, 4.39804, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Flamecaller
(@CGUID+94, 61705, 389, 2437, 2437, 2, 1, -296.531, 146.731, -24.942, 6.23717, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Flamecaller
(@CGUID+95, 61678, 389, 2437, 2437, 2, 1, -296.176, 154.142, -25.0448, 6.05655, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Reaver
(@CGUID+96, 61678, 389, 2437, 2437, 2, 1, -238.911, 207.359, -24.8119, 4.71167, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Reaver
(@CGUID+97, 61705, 389, 2437, 2437, 2, 1, -288.647, 205.474, -25.7272, 5.53463, 0, 7200, 0, 0, 0, 0, 0, 0), -- Corrupted Flamecaller
(@CGUID+98, 61672, 389, 2437, 2437, 2, 1, -284.827, 209.647, -25.3874, 5.53463, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Acolyte
(@CGUID+99, 61678, 389, 2437, 2437, 2, 1, -285.738, 206.633, -25.6597, 0, 0, 7200, 0, 0, 0, 0, 2, 0), -- Corrupted Reaver
(@CGUID+100, 61680, 389, 2437, 2437, 2, 4, -376, 181.558, -20.8818, 0.462226, 0, 7200, 0, 0, 0, 0, 0, 0), -- Kor'kron Scout
(@CGUID+101, 61680, 389, 2437, 2437, 2, 4, -377.136, 185.143, -20.9595, 0.462226, 0, 7200, 0, 0, 0, 0, 0, 0), -- Kor'kron Scout
(@CGUID+102, 61644, 389, 2437, 2437, 2, 1, -364.914, 191.712, -22.2708, 1.35503, 2, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Researcher
(@CGUID+103, 61644, 389, 2437, 2437, 2, 1, -351.529, 203.458, -22.1448, 3.03309, 2, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Researcher
(@CGUID+104, 61680, 389, 2437, 2437, 2, 4, -378.609, 188.419, -20.8019, 0.462226, 0, 7200, 0, 0, 0, 0, 0, 0), -- Kor'kron Scout
(@CGUID+105, 61644, 389, 2437, 2437, 2, 1, -381.413, 197.913, -21.7809, 3.03309, 2, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Researcher
(@CGUID+106, 61644, 389, 2437, 2437, 2, 1, -375.027, 203.089, -21.801, 0.0212829, 2, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Researcher
(@CGUID+107, 61644, 389, 2437, 2437, 2, 1, -362.571, 215.728, -21.5527, 4.72562, 2, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Researcher
(@CGUID+108, 61644, 389, 2437, 2437, 2, 1, -377.922, 209.601, -21.801, 2.8793, 2, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Researcher
(@CGUID+109, 61644, 389, 2437, 2437, 2, 1, -370.877, 217.592, -21.801, 1.45237, 0, 7200, 0, 0, 0, 0, 0, 0), -- Dark Shaman Researcher
(@CGUID+110, 61821, 389, 2437, 2437, 2, 2, -23.8017, -42.9664, -21.7354, 2.7036, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Commando
(@CGUID+111, 61822, 389, 2437, 2437, 2, 2, -23.4555, -52.0436, -21.5553, 2.71608, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Field Commander Dirken
(@CGUID+112, 61823, 389, 2437, 2437, 2, 2, -14.3154, -59.726, -21.2882, 4.61212, 0, 7200, 0, 0, 0, 0, 0, 0), -- High Sorceress Aryna
(@CGUID+113, 61788, 389, 2437, 2437, 2, 2, -88.894, 68.7328, -18.5693, 3.61982, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Ranger
(@CGUID+114, 61788, 389, 2437, 2437, 2, 2, -172.404, 1.68506, -33.0252, 1.64543, 0, 7200, 0, 0, 0, 0, 0, 16777216), -- SI:7 Ranger
(@CGUID+115, 61788, 389, 2437, 2437, 2, 2, -257.035, -49.3293, -60.6823, 3.41253, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Ranger
(@CGUID+116, 61821, 389, 2437, 2437, 2, 2, -27.833, -57.7237, -21.2882, 2.7036, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Commando
(@CGUID+117, 61788, 389, 2437, 2437, 2, 2, -310.909, -24.3211, -58.851, 0, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Ranger
(@CGUID+118, 61788, 389, 2437, 2437, 2, 2, -286.979, 134.491, -27.0197, 1.85857, 0, 7200, 0, 0, 0, 0, 0, 16777216), -- SI:7 Ranger
(@CGUID+119, 61788, 389, 2437, 2437, 2, 2, -376, 181.558, -20.8818, 0.462226, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Ranger
(@CGUID+120, 61788, 389, 2437, 2437, 2, 2, -378.609, 188.419, -20.8019, 0.462226, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Ranger
(@CGUID+121, 61788, 389, 2437, 2437, 2, 2, -377.136, 185.143, -20.9595, 0.462226, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Ranger
(@CGUID+122, 61724, 389, 2437, 2437, 2, 16, -299.521, 221.177, -25.1319, 3.08751, 0, 7200, 0, 0, 0, 0, 0, 0), -- Commander Bagran
(@CGUID+123, 61404, 389, 2437, 2437, 2, 16, -296.936, 223.106, -24.9686, 3.02628, 0, 7200, 0, 0, 0, 0, 0, 0), -- Kor'kron Elite
(@CGUID+124, 61716, 389, 2437, 2437, 2, 16, -299.499, 215.781, -25.4273, 2.84618, 0, 7200, 0, 0, 0, 0, 0, 0), -- Invoker Xorenth
(@CGUID+125, 61404, 389, 2437, 2437, 2, 16, -297.063, 218.29, -25.3583, 3.02628, 0, 7200, 0, 0, 0, 0, 0, 0), -- Kor'kron Elite
(@CGUID+126, 61821, 389, 2437, 2437, 2, 8, -296.399, 221.741, -25.1562, 2.83423, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Commando
(@CGUID+127, 61821, 389, 2437, 2437, 2, 8, -296.526, 216.925, -25.3941, 3.06792, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Commando
(@CGUID+128, 61822, 389, 2437, 2437, 2, 8, -299.025, 219.819, -25.3599, 2.97568, 0, 7200, 0, 0, 0, 0, 0, 0), -- SI:7 Field Commander Dirken
(@CGUID+129, 61823, 389, 2437, 2437, 2, 8, -299.022, 215.894, -25.431, 2.87139, 0, 7200, 0, 0, 0, 0, 0, 0); -- High Sorceress Aryna

-- Bosses and Bagran that already had MoP spawns (absolute values, so re-running is harmless)
UPDATE `creature` SET `position_x` = -20.5822, `position_y` = -51.1101, `position_z` = -21.7212, `orientation` = 2.74547, `phaseMask` = 4 WHERE `guid` = 564242 AND `id` = 61724; -- Commander Bagran
UPDATE `creature` SET `position_x` = -117.044, `position_y` = 71.0705, `position_z` = -20.7203, `orientation` = 5.34351, `phaseMask` = 65535 WHERE `guid` = 564240 AND `id` = 61412; -- Dark Shaman Koranthal
UPDATE `creature` SET `position_x` = -284.638, `position_y` = -53.8253, `position_z` = -60.7344, `orientation` = 0.321085, `phaseMask` = 65535 WHERE `guid` = 564241 AND `id` = 61408; -- Adarogg

-- Sniffed auras, emotes, stand states and anim kits; idle paths use waypoint_data id = guid * 10
INSERT INTO `creature_addon` (`guid`, `path_id`, `mount`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(@CGUID+2, (@CGUID+2)*10, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL), -- Mature Flame Hound
(@CGUID+3, (@CGUID+3)*10, 0, 0, 3, 0, 1, 0, 0, 0, 0, 0, 3, '119436'), -- Lava
(@CGUID+5, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '35356 119897'), -- Slain Trogg
(@CGUID+7, 0, 0, 8, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL), -- Invoker Xorenth
(@CGUID+11, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Kor'kron Elite
(@CGUID+13, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '114371'), -- Kor'kron Scout
(@CGUID+15, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Kor'kron Elite
(@CGUID+19, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '35356 119897'), -- Slain Trogg
(@CGUID+24, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+28, 0, 0, 0, 0, 0, 1, 0, 333, 0, 0, 0, 0, NULL), -- Dark Shaman Acolyte
(@CGUID+30, (@CGUID+30)*10, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+34, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '35356 119897'), -- Slain Trogg
(@CGUID+35, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Houndmaster
(@CGUID+38, 0, 0, 0, 3, 0, 1, 0, 0, 2286, 0, 0, 0, NULL), -- Kor'kron Scout
(@CGUID+39, (@CGUID+39)*10, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+41, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+46, (@CGUID+46)*10, 0, 0, 3, 0, 1, 0, 0, 0, 0, 0, 3, '119896'), -- Lava
(@CGUID+47, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Houndmaster
(@CGUID+48, 0, 0, 0, 3, 0, 1, 0, 0, 0, 0, 0, 3, NULL), -- Flame Visual
(@CGUID+51, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '35356 119897'), -- Slain Trogg
(@CGUID+52, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+58, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+63, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Houndmaster
(@CGUID+68, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+71, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '35356 119897'), -- Slain Trogg
(@CGUID+72, 0, 0, 0, 0, 0, 1, 0, 0, 2281, 0, 0, 0, NULL), -- Kor'kron Scout
(@CGUID+80, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '35356 119897'), -- Slain Trogg
(@CGUID+81, (@CGUID+81)*10, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL), -- Corrupted Houndmaster
(@CGUID+83, 0, 0, 0, 0, 0, 1, 0, 0, 1735, 0, 0, 0, NULL), -- Mature Flame Hound
(@CGUID+84, 0, 0, 0, 0, 0, 1, 0, 0, 1443, 0, 0, 0, NULL), -- Suspicious Rock
(@CGUID+85, (@CGUID+85)*10, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL), -- Dark Shaman Acolyte
(@CGUID+89, (@CGUID+89)*10, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+90, 0, 0, 0, 3, 0, 1, 0, 0, 2286, 0, 0, 0, NULL), -- Kor'kron Scout
(@CGUID+91, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+92, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+95, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+96, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+99, (@CGUID+99)*10, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL), -- Corrupted Reaver
(@CGUID+105, 0, 0, 0, 0, 0, 1, 0, 69, 0, 0, 0, 0, NULL), -- Dark Shaman Researcher
(@CGUID+108, 0, 0, 0, 0, 0, 1, 0, 69, 0, 0, 0, 0, NULL), -- Dark Shaman Researcher
(@CGUID+109, 0, 0, 0, 0, 0, 1, 0, 173, 0, 0, 0, 0, NULL), -- Dark Shaman Researcher
(@CGUID+110, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- SI:7 Commando
(@CGUID+114, 0, 0, 0, 3, 0, 1, 0, 0, 2297, 0, 0, 0, NULL), -- SI:7 Ranger
(@CGUID+115, 0, 0, 0, 0, 0, 1, 0, 0, 2281, 0, 0, 0, NULL), -- SI:7 Ranger
(@CGUID+116, 0, 0, 0, 0, 0, 1, 0, 375, 0, 0, 0, 0, NULL), -- SI:7 Commando
(@CGUID+118, 0, 0, 0, 3, 0, 1, 0, 0, 2297, 0, 0, 0, NULL), -- SI:7 Ranger
(@CGUID+33, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, NULL), -- Dancing Flames
(@CGUID+40, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, NULL), -- Dancing Flames
(@CGUID+45, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, NULL), -- Dancing Flames
(@CGUID+49, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, NULL), -- Dancing Flames
(@CGUID+50, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 3, '119627'), -- Ground Runes
(@CGUID+57, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, NULL), -- Dancing Flames
(@CGUID+61, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, NULL), -- Dancing Flames
(@CGUID+67, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, NULL), -- Dancing Flames
(@CGUID+76, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, NULL), -- Dancing Flames
(@CGUID+82, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, NULL), -- Dancing Flames
(@CGUID+86, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 4, NULL); -- Dancing Flames

-- Kept and moved bosses: retail's far visibility (TDB spawn addons of Slagmaw and Gordoth, Koranthal's template addon)
DELETE FROM `creature_addon` WHERE `guid` IN (564238, 564239, 564240);
INSERT INTO `creature_addon` (`guid`, `path_id`, `mount`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(564238, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 3, NULL), -- Lava Guard Gordoth (not his in-cage AIAnimKit 947, he stays outside the cage)
(564239, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 3, NULL), -- Slagmaw
(564240, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 3, NULL); -- Dark Shaman Koranthal

INSERT INTO `waypoint_data` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `move_flag`) VALUES
((@CGUID+2)*10, 1, -78.4044, -34.6446, -19.7785, 0, 11915, 0),
((@CGUID+2)*10, 2, -83.7406, -33.9416, -21.5926, 0, 0, 0),
((@CGUID+2)*10, 3, -95.0029, -34.2261, -27.8864, 0, 15513, 0),
((@CGUID+2)*10, 4, -78.4044, -34.6446, -19.7785, 0, 0, 0), -- Mature Flame Hound - Cosmetic Path
((@CGUID+3)*10, 1, -98.9559, -50.4502, -33.7143, 0, 6495, 0),
((@CGUID+3)*10, 2, -98.9559, -50.4502, -30.7143, 0, 8074, 0), -- Lava trigger - Path
((@CGUID+30)*10, 1, -167.943, 11.8834, -29.5828, 0, 0, 0),
((@CGUID+30)*10, 2, -176.288, 11.1999, -31.5366, 0, 0, 0),
((@CGUID+30)*10, 3, -167.943, 11.8834, -29.5828, 0, 0, 0),
((@CGUID+30)*10, 4, -156.962, 11.1934, -25.9206, 0, 0, 0),
((@CGUID+30)*10, 5, -146.374, 8.36332, -22.3629, 0, 0, 0),
((@CGUID+30)*10, 6, -140.192, 7.30182, -21.2047, 0, 2, 0),
((@CGUID+30)*10, 7, -146.374, 8.36332, -22.3629, 0, 0, 0),
((@CGUID+30)*10, 8, -156.962, 11.1934, -25.9206, 0, 0, 0), -- Corrupted Reaver - Formation Path
((@CGUID+39)*10, 1, -286.815, 9.52373, -46.223, 0, 0, 0),
((@CGUID+39)*10, 2, -295.871, 4.27555, -47.4308, 0, 0, 0),
((@CGUID+39)*10, 3, -301.825, 0.972978, -48.8705, 0, 0, 0),
((@CGUID+39)*10, 4, -304.764, -12.8444, -55.0277, 0, 0, 0),
((@CGUID+39)*10, 5, -301.825, 0.972978, -48.8705, 0, 0, 0),
((@CGUID+39)*10, 6, -295.871, 4.27555, -47.4308, 0, 0, 0),
((@CGUID+39)*10, 7, -290.538, 7.36829, -46.6274, 0, 0, 0), -- Corrupted Reaver - Cosmetic Path
((@CGUID+46)*10, 1, -276.397, 11.5205, -45.2924, 0, 7282, 0),
((@CGUID+46)*10, 2, -276.397, 11.5205, -49.2924, 0, 4481, 0), -- Lava - Cosmetic Path
((@CGUID+81)*10, 1, -202.69, -35.1236, -49.1017, 0, 0, 0),
((@CGUID+81)*10, 2, -189.081, -32.4486, -45.2422, 0, 0, 0),
((@CGUID+81)*10, 3, -179.287, -31.2225, -44.204, 0, 282, 0),
((@CGUID+81)*10, 4, -189.081, -32.4486, -45.2422, 0, 0, 0),
((@CGUID+81)*10, 5, -202.69, -35.1236, -49.1017, 0, 0, 0),
((@CGUID+81)*10, 6, -216.924, -37.4035, -53.9907, 0, 276, 0), -- Corrupted Houndmaster - Cosmetic Path
((@CGUID+85)*10, 1, -195.689, 6.81701, -34.5062, 0, 0, 0),
((@CGUID+85)*10, 2, -206.778, 5.8203, -38.5698, 0, 0, 0),
((@CGUID+85)*10, 3, -217.103, 6.2946, -41.9711, 0, 0, 0),
((@CGUID+85)*10, 4, -225.018, 5.80747, -43.713, 0, 0, 0),
((@CGUID+85)*10, 5, -217.103, 6.2946, -41.9711, 0, 0, 0),
((@CGUID+85)*10, 6, -206.808, 5.82168, -38.5987, 0, 0, 0), -- Dark Shaman Acolyte - Formation Path
((@CGUID+89)*10, 1, -183.871, 77.9451, -22.2752, 0, 0, 0),
((@CGUID+89)*10, 2, -174.485, 76.0839, -21.8262, 0, 0, 0),
((@CGUID+89)*10, 3, -183.871, 77.9451, -22.2752, 0, 0, 0),
((@CGUID+89)*10, 4, -197.31, 85.4361, -24.4099, 0, 0, 0),
((@CGUID+89)*10, 5, -209.026, 97.0867, -25.403, 0, 0, 0),
((@CGUID+89)*10, 6, -197.31, 85.4361, -24.4099, 0, 0, 0), -- Corrupted Reaver - Formation Path
((@CGUID+99)*10, 1, -285.738, 206.633, -25.6597, 0, 0, 0),
((@CGUID+99)*10, 2, -297.634, 217.683, -25.5011, 0, 0, 0),
((@CGUID+99)*10, 3, -305.792, 221.885, -24.6557, 0, 0, 0),
((@CGUID+99)*10, 4, -314.336, 223.61, -22.5346, 0, 0, 0),
((@CGUID+99)*10, 5, -323.748, 222.828, -21.0305, 0, 0, 0),
((@CGUID+99)*10, 6, -314.336, 223.61, -22.5346, 0, 0, 0),
((@CGUID+99)*10, 7, -305.792, 221.885, -24.6557, 0, 0, 0),
((@CGUID+99)*10, 8, -297.634, 217.683, -25.5011, 0, 0, 0); -- Corrupted Reaver - Formation Path

INSERT INTO `creature_formations` (`leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`, `point_1`, `point_2`) VALUES
(@CGUID+30, @CGUID+17, 6, 90, 515, 1, 5),
(@CGUID+30, @CGUID+30, 0, 0, 515, 1, 5),
(@CGUID+81, @CGUID+37, 4, 315, 515, 1, 0),
(@CGUID+81, @CGUID+64, 4, 45, 515, 1, 0),
(@CGUID+81, @CGUID+81, 0, 0, 515, 1, 0),
(@CGUID+85, @CGUID+69, 4, 90, 515, 3, 0),
(@CGUID+85, @CGUID+85, 0, 0, 515, 3, 0),
(@CGUID+89, @CGUID+60, 4, 50, 515, 1, 4),
(@CGUID+89, @CGUID+73, 4, 310, 515, 1, 4),
(@CGUID+89, @CGUID+89, 0, 0, 515, 1, 4),
(@CGUID+99, @CGUID+97, 4, 50, 515, 4, 0),
(@CGUID+99, @CGUID+98, 4, 310, 515, 4, 0),
(@CGUID+99, @CGUID+99, 0, 0, 515, 4, 0),
(@CGUID+122, @CGUID+122, 0, 0, 512, 0, 0),
(@CGUID+122, @CGUID+123, 4, 45, 512, 0, 0),
(@CGUID+122, @CGUID+125, 4, 315, 512, 0, 0),
(@CGUID+128, @CGUID+126, 4, 45, 512, 0, 0),
(@CGUID+128, @CGUID+127, 4, 315, 512, 0, 0),
(@CGUID+128, @CGUID+128, 0, 0, 512, 0, 0);

-- 3. Gameobjects: 43 of the sniff's 60 props were already spawned; add the other 17
SET @GGUID := 4045000;
DELETE FROM `gameobject` WHERE `guid` BETWEEN @GGUID+0 AND @GGUID+16;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`,
`rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`) VALUES
(@GGUID+0, 175309, 389, 2437, 2437, 2, 1, 56.7668, 101.443, -5.37528, 3.73501, 0, 0, -0.956305, 0.292372, 7200, 255, 1), -- Brazier
(@GGUID+1, 175295, 389, 2437, 2437, 2, 1, 39.5509, 173.859, 33.7116, 3.14159, 0, 0, -1, 0, 7200, 255, 1), -- Brazier
(@GGUID+2, 175296, 389, 2437, 2437, 2, 1, 10.5774, 161.329, 23.0549, 3.14159, 0, 0, -1, 0, 7200, 255, 1), -- Brazier
(@GGUID+3, 175311, 389, 2437, 2437, 2, 1, 63.9071, 88.004, -5.47027, 3.22886, 0, 0, -0.999048, 0.0436193, 7200, 255, 1), -- Ironwood Staves and Wands
(@GGUID+4, 175297, 389, 2437, 2437, 2, 1, 50.521, 143.669, 14.1311, 2.10312, 0, 0, 0.868198, 0.496217, 7200, 255, 1), -- Brazier
(@GGUID+5, 175298, 389, 2437, 2437, 2, 1, -4.42081, 120.719, 12.2467, 2.02458, 0, 0, 0.848047, 0.529921, 7200, 255, 1), -- Brazier
(@GGUID+6, 175299, 389, 2437, 2437, 2, 1, 64.3886, 114.015, 12.1089, 3.14159, 0, 0, -1, 0, 7200, 255, 1), -- Brazier
(@GGUID+7, 175300, 389, 2437, 2437, 2, 1, 24.8166, 132.476, 13.3321, 3.14159, 0, 0, -1, 0, 7200, 255, 1), -- Brazier
(@GGUID+8, 175301, 389, 2437, 2437, 2, 1, 52.0178, 113.448, 10.7392, 3.89209, 0, 0, -0.930417, 0.366502, 7200, 255, 1), -- Brazier
(@GGUID+9, 175302, 389, 2437, 2437, 2, 1, 17.9547, 117.038, 12.5586, 6.25701, 0, 0, -0.0130892, 0.999914, 7200, 255, 1), -- The Slow Blade
(@GGUID+10, 175303, 389, 2437, 2437, 2, 1, 69.4831, 115.518, 12.0885, 3.14159, 0, 0, -1, 0, 7200, 255, 1), -- Rekkul's Poisons
(@GGUID+11, 175304, 389, 2437, 2437, 2, 1, 36.3528, 133.038, 13.0252, 4.39823, 0, 0, -0.809016, 0.587786, 7200, 255, 1), -- Shadowswift Brotherhood
(@GGUID+12, 211874, 389, 2437, 2437, 2, 1, -267.805, -65.6172, -60.44, 5.39915, 0, 0, -0.427766, 0.90389, 7200, 255, 1), -- Fresh Carcass
(@GGUID+13, 211788, 389, 2437, 2437, 2, 1, -381.401, 215.404, -21.8843, 5.47497, 0, 0, -0.393197, 0.919454, 7200, 255, 1), -- Horde Crate
(@GGUID+14, 211788, 389, 2437, 2437, 2, 1, -377.785, 219.062, -21.8843, 5.47497, 0, 0, -0.393197, 0.919454, 7200, 255, 1), -- Horde Crate
(@GGUID+15, 211788, 389, 2437, 2437, 2, 1, -372.996, 222.741, -21.796, 4.85832, 0, 0, -0.653677, 0.756774, 7200, 255, 1), -- Horde Crate
(@GGUID+16, 211788, 389, 2437, 2437, 2, 1, -379.771, 217.413, -21.8843, 5.47497, 0, 0, -0.393197, 0.919454, 7200, 255, 1); -- Horde Crate

-- 4. Templates (only creatures that exist nowhere but Ragefire Chasm; Dancing Flames 61795 is shared with Siege of Orgrimmar and is
--    left alone). Levels go in creature_template and in the DUNGEON_NORMAL row of creature_template_difficulty, which overrides
--    the template inside the instance.
UPDATE `creature_template` SET `minlevel` = 14, `maxlevel` = 14, `exp` = 0 WHERE `entry` IN (61657, 61658, 61666, 61672, 61678, 61705, 61680, 61788);
UPDATE `creature_template` SET `minlevel` = 15, `maxlevel` = 15, `exp` = 0 WHERE `entry` = 61644;
UPDATE `creature_template` SET `minlevel` = 20, `maxlevel` = 20, `exp` = 0 WHERE `entry` IN (61404, 61716, 61724, 61821, 61822, 61823);
UPDATE `creature_template_difficulty` SET `level_min` = 14, `level_max` = 14 WHERE `difficulty` = 'DUNGEON_NORMAL' AND `id` IN (61657, 61658, 61666, 61672, 61678, 61705, 61680, 61788);
UPDATE `creature_template_difficulty` SET `level_min` = 15, `level_max` = 15 WHERE `difficulty` = 'DUNGEON_NORMAL' AND `id` = 61644;
UPDATE `creature_template_difficulty` SET `level_min` = 20, `level_max` = 20 WHERE `difficulty` = 'DUNGEON_NORMAL' AND `id` IN (61404, 61716, 61724, 61821, 61822, 61823);
UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (61404, 61644, 61657, 61658, 61666, 61672, 61678, 61680, 61705, 61716, 61724, 61780, 61788, 61790, 61821, 61822, 61823);
-- Suspicious Rock and Inconspicuous Crate have their gossip menus but not the gossip flag
UPDATE `creature_template` SET `npcflag` = 1 WHERE `entry` IN (61780, 61790);

-- Template addons as sniffed: corpses feign death, Koranthal's Ground Runes, the rock's idle kit. The Dark Shaman Researchers' addon
-- had Bloody Suicide (120022, a 250 ms periodic Suicide) and SI:7 Rangers hovered (AnimTier 3); retail has neither, and the
-- cliff-hanging rangers get their AnimTier from their spawn addon like the Kor'kron Scouts.
UPDATE `creature_template_addon` SET `auras` = '35356' WHERE `entry` IN (61659, 61669);
UPDATE `creature_template_addon` SET `auras` = '119627' WHERE `entry` = 61630;
UPDATE `creature_template_addon` SET `aiAnimKit` = 1443 WHERE `entry` = 61780;
UPDATE `creature_template_addon` SET `AnimTier` = 0 WHERE `entry` = 61788;
UPDATE `creature_template_addon` SET `auras` = NULL WHERE `entry` = 61644;
-- Only the entrance Invoker Xorenth kneels (his spawn addon); the template made the end-of-dungeon one walk in kneeling
UPDATE `creature_template_addon` SET `StandState` = 0 WHERE `entry` = 61716;

-- Weapons of the Kor'kron NPCs (the SI:7 ones already have theirs)
DELETE FROM `creature_equip_template` WHERE `CreatureID` IN (61404, 61716, 61724);
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `ItemID2`, `ItemID3`) VALUES
(61404, 1, 73894, 0, 0),
(61716, 1, 77404, 0, 0),
(61724, 1, 73894, 0, 0);
-- Six of the seven Dark Shaman Researchers are unarmed (equipment_id 2); equipment_id 0 would give them the default staff
DELETE FROM `creature_equip_template` WHERE `CreatureID` = 61644 AND `ID` = 2;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `ItemID2`, `ItemID3`) VALUES
(61644, 2, 0, 0, 0);

-- Scout cages: usable (retail flags 0; LoA had them not selectable) and scripted
UPDATE `gameobject_template_addon` SET `flags` = 0 WHERE `entry` IN (211883, 211884, 211885);
UPDATE `gameobject_template` SET `AIName` = 'SmartGameObjectAI' WHERE `entry` IN (211883, 211884, 211885);
-- A cage only credits while its prisoner is inside (after a restart the cages are closed again but rescued scouts are still
-- waiting to respawn). The cages share phases 1+2+4 so that check finds the Kor'kron and SI:7 prisoners.
UPDATE `gameobject` SET `phaseMask` = 7 WHERE `guid` IN (540749, 540750, 540751) AND `id` IN (211883, 211884, 211885);

-- Rescued Kor'kron Scouts and SI:7 Rangers say one of these
DELETE FROM `creature_text` WHERE `CreatureID` IN (61680, 61788);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(61680, 0, 1, 'RUN!!', 12, 0, 20, 0, 0, 0, 61290, 0, 'Kor\'kron Scout - Rescued'),
(61680, 0, 2, 'Thanks!', 12, 0, 20, 0, 0, 0, 61286, 0, 'Kor\'kron Scout - Rescued'),
(61680, 0, 3, 'I didn\'t think anyone was coming for us.', 12, 0, 20, 0, 0, 0, 61287, 0, 'Kor\'kron Scout - Rescued'),
(61680, 0, 4, 'I owe you an ale!', 12, 0, 20, 0, 0, 0, 61288, 0, 'Kor\'kron Scout - Rescued'),
(61680, 0, 5, 'I thought I was going to die down here!', 12, 0, 20, 0, 0, 0, 61285, 0, 'Kor\'kron Scout - Rescued'),
(61788, 0, 1, 'RUN!!', 12, 0, 20, 0, 0, 0, 61290, 0, 'SI:7 Ranger - Rescued'),
(61788, 0, 2, 'Thanks!', 12, 0, 20, 0, 0, 0, 61286, 0, 'SI:7 Ranger - Rescued'),
(61788, 0, 3, 'I didn\'t think anyone was coming for us.', 12, 0, 20, 0, 0, 0, 61287, 0, 'SI:7 Ranger - Rescued'),
(61788, 0, 4, 'I owe you an ale!', 12, 0, 20, 0, 0, 0, 61288, 0, 'SI:7 Ranger - Rescued'),
(61788, 0, 5, 'I thought I was going to die down here!', 12, 0, 20, 0, 0, 0, 61285, 0, 'SI:7 Ranger - Rescued');

-- Invoker Xorenth's greeting (menu 13813). Commander Bagran's (13816, text 61284) has no 5.4.8 broadcast text; the SI:7 menus already exist.
DELETE FROM `npc_text` WHERE `ID` = 19934;
INSERT INTO `npc_text` (`ID`, `text0_0`, `BroadcastTextID0`, `Probability0`, `text1_0`, `BroadcastTextID1`, `Probability1`) VALUES
(19934, 'We must stop the Dark Shaman now.', 61261, 1, 'Did Garrosh send you?', 61262, 1);
DELETE FROM `gossip_menu` WHERE `MenuID` = 13813;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`) VALUES (13813, 19934);
UPDATE `creature_template` SET `gossip_menu_id` = 13813, `npcflag` = 3 WHERE `entry` = 61716;
-- Dirken and Aryna had their greeting menus but only the quest giver flag, so the menus never showed (retail: gossip + quest giver)
UPDATE `creature_template` SET `npcflag` = 3 WHERE `entry` IN (61822, 61823);

DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (13814, 13815);
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcflag`) VALUES
(13814, 0, 0, 'It\'s safe to come out now.', 61264, 1, 1),
(13815, 0, 0, 'It\'s safe to come out now.', 61264, 1, 1);

-- The two scouts and two rangers hanging from ledges are rescued by clicking them (Save Cliffhanger 1 / 2)
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (61680, 61788);
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(61680, 120327, 3, 1),
(61680, 120328, 3, 1),
(61788, 120327, 3, 1),
(61788, 120328, 3, 1);

-- 5. SmartAI, from the sniffed TDB scripts. Changes for LoA: retail's credit spells 120026/120292 are not in the 5.4.8 client, so the
--    object that starts a rescue gives the kill credit (Call KilledMonster to the clicker's party, or to the hound's loot
--    recipients). TDB tells the Kor'kron and SI:7 copies apart by spawn groups and StringIds; here the rescuer sets data on both
--    copies by spawn guid (the one in the other faction's phase walks off unseen), and StringId conditions became guid conditions.
--    TC-master-only actions were mapped: self-cast 85 -> cast 11 on self, Set Uninteractible -> remove the spellclick flag,
--    SetAnimTier 0 -> remove UNIT_FIELD_BYTES_1 byte 3, jumps target self with the destination in target_x/y/z
--    (LoA's Jump To Pos gets no target from target type 8). Waypoint points are 1-based here (TDB nodes are 0-based).
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` IN (61644, 61657, 61658, 61666, 61672, 61678, 61680, 61705, 61716, 61724, 61780, 61788, 61790, 61822, 61823);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` BETWEEN -(@CGUID+129) AND -(@CGUID+0);
DELETE FROM `smart_scripts` WHERE `source_type` = 9 AND `entryorguid` IN (6168000, 6168001, 6168002, 6168003, 6168004, 6178000, 6179000);
DELETE FROM `smart_scripts` WHERE `source_type` = 1 AND `entryorguid` IN (211883, 211884, 211885);
-- Trash abilities (TDB timers)
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(61657, 0, 0, 0, 1, 0, 100, 0, 2000, 8000, 2000, 8000, 0, 11, 120064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Adolescent Flame Hound - OOC - Cast Eat Trog (only the five eaters, see conditions)'),
(61657, 0, 1, 0, 0, 0, 100, 0, 38200, 46700, 38200, 46700, 0, 11, 120069, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Adolescent Flame Hound - IC - Cast Rend Flesh'),
(61658, 0, 0, 0, 1, 0, 100, 0, 2000, 8000, 2000, 8000, 0, 11, 120064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mature Flame Hound - OOC - Cast Eat Trog'),
(61658, 0, 1, 0, 0, 0, 100, 0, 37000, 37000, 37000, 37000, 0, 11, 120076, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mature Flame Hound - IC - Cast Vicious Tear'),
(61658, 0, 2, 0, 0, 0, 100, 0, 7300, 7300, 12100, 12100, 0, 11, 119999, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mature Flame Hound - IC - Cast Ground Rupture'),
(61666, 0, 0, 0, 0, 0, 100, 0, 39400, 39400, 39400, 39400, 0, 11, 120096, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Houndmaster - IC - Cast Corrupted Strike'),
(61672, 0, 0, 0, 0, 0, 100, 0, 92200, 92200, 23000, 23000, 0, 11, 119902, 0, 0, 0, 0, 0, 26, 30, 0, 0, 0, 0, 0, 0, 0, 'Dark Shaman Acolyte - IC - Cast Bloodlust on Closest Ally'),
(61672, 0, 1, 0, 0, 0, 100, 0, 83800, 83800, 18100, 18100, 0, 11, 119903, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Shaman Acolyte - IC - Cast Corrupted Lava Burst'),
(61678, 0, 0, 0, 0, 0, 100, 0, 39700, 39700, 15700, 15700, 0, 11, 120166, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Reaver - IC - Cast Serrated Blade'),
(61705, 0, 0, 0, 0, 0, 100, 0, 37300, 37300, 3600, 3600, 0, 11, 120112, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Flamecaller - IC - Cast Fireball'),
(61823, 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 32783, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'High Sorceress Aryna - On Reset - Cast Arcane Channeling');

-- Flamecallers and Researchers that channel out of combat (a guid script replaces the entry script, so it keeps Fireball)
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(-(@CGUID+42), 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 120264, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Flamecaller - On Reset - Cast Fire Channeling'),
(-(@CGUID+42), 0, 1, 0, 0, 0, 100, 0, 37300, 37300, 3600, 3600, 0, 11, 120112, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Flamecaller - IC - Cast Fireball'),
(-(@CGUID+59), 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 120264, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Flamecaller - On Reset - Cast Fire Channeling'),
(-(@CGUID+59), 0, 1, 0, 0, 0, 100, 0, 37300, 37300, 3600, 3600, 0, 11, 120112, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Flamecaller - IC - Cast Fireball'),
(-(@CGUID+93), 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 120264, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Flamecaller - On Reset - Cast Fire Channeling'),
(-(@CGUID+93), 0, 1, 0, 0, 0, 100, 0, 37300, 37300, 3600, 3600, 0, 11, 120112, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Flamecaller - IC - Cast Fireball'),
(-(@CGUID+94), 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 120264, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Flamecaller - On Reset - Cast Fire Channeling'),
(-(@CGUID+94), 0, 1, 0, 0, 0, 100, 0, 37300, 37300, 3600, 3600, 0, 11, 120112, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Corrupted Flamecaller - IC - Cast Fireball'),
(-(@CGUID+102), 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 51835, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Shaman Researcher - On Reset - Cast Cosmetic - Channel Omni'),
(-(@CGUID+103), 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 51835, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Shaman Researcher - On Reset - Cast Cosmetic - Channel Omni'),
(-(@CGUID+106), 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 51835, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Shaman Researcher - On Reset - Cast Cosmetic - Channel Omni'),
(-(@CGUID+107), 0, 0, 0, 25, 0, 100, 0, 0, 0, 0, 0, 0, 11, 51835, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Dark Shaman Researcher - On Reset - Cast Cosmetic - Channel Omni');

-- Mature Flame Hound mauling a scout and a ranger near Adarogg: killing it rescues them
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(-(@CGUID+83), 0, 0, 0, 1, 0, 100, 0, 2000, 8000, 2000, 8000, 0, 11, 120064, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mature Flame Hound - OOC - Cast Eat Trog'),
(-(@CGUID+83), 0, 1, 0, 0, 0, 100, 0, 37000, 37000, 37000, 37000, 0, 11, 120076, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mature Flame Hound - IC - Cast Vicious Tear'),
(-(@CGUID+83), 0, 2, 0, 0, 0, 100, 0, 7300, 7300, 12100, 12100, 0, 11, 119999, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 'Mature Flame Hound - IC - Cast Ground Rupture'),
(-(@CGUID+83), 0, 3, 4, 6, 0, 100, 0, 0, 0, 0, 0, 0, 33, 61680, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mature Flame Hound - On Death - Kill Credit Kor\'kron Scout to Loot Recipients'),
(-(@CGUID+83), 0, 4, 5, 61, 0, 100, 0, 0, 0, 0, 0, 0, 33, 61788, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Mature Flame Hound - On Death - Kill Credit SI:7 Ranger to Loot Recipients'),
(-(@CGUID+83), 0, 5, 6, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 1, 0, 0, 0, 0, 10, @CGUID+72, 61680, 0, 0, 0, 0, 0, 0, 'Mature Flame Hound - On Death - Set Data 0 1 Kor\'kron Scout'),
(-(@CGUID+83), 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 1, 0, 0, 0, 0, 10, @CGUID+115, 61788, 0, 0, 0, 0, 0, 0, 'Mature Flame Hound - On Death - Set Data 0 1 SI:7 Ranger');

-- Suspicious Rock and Inconspicuous Crate: the agents hiding there come out when told it is safe
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(61780, 0, 0, 1, 62, 0, 100, 0, 13814, 0, 0, 0, 0, 33, 61680, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Suspicious Rock - On Gossip Select - Kill Credit Kor\'kron Scout to Invoker Party'),
(61780, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 33, 61788, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Suspicious Rock - On Gossip Select - Kill Credit SI:7 Ranger to Invoker Party'),
(61780, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 2, 0, 0, 0, 0, 10, @CGUID+54, 61680, 0, 0, 0, 0, 0, 0, 'Suspicious Rock - On Gossip Select - Set Data 0 2 Kor\'kron Scout'),
(61780, 0, 3, 4, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 2, 0, 0, 0, 0, 10, @CGUID+117, 61788, 0, 0, 0, 0, 0, 0, 'Suspicious Rock - On Gossip Select - Set Data 0 2 SI:7 Ranger'),
(61780, 0, 4, 5, 61, 0, 100, 0, 0, 0, 0, 0, 0, 83, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Suspicious Rock - On Gossip Select - Remove Gossip Flag'),
(61780, 0, 5, 6, 61, 0, 100, 0, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Suspicious Rock - On Gossip Select - Close Gossip'),
(61780, 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 80, 6178000, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Suspicious Rock - On Gossip Select - Run Action List'),
(61790, 0, 0, 1, 62, 0, 100, 0, 13815, 0, 0, 0, 0, 33, 61680, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Inconspicuous Crate - On Gossip Select - Kill Credit Kor\'kron Scout to Invoker Party'),
(61790, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 33, 61788, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Inconspicuous Crate - On Gossip Select - Kill Credit SI:7 Ranger to Invoker Party'),
(61790, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 4, 0, 0, 0, 0, 10, @CGUID+20, 61680, 0, 0, 0, 0, 0, 0, 'Inconspicuous Crate - On Gossip Select - Set Data 0 4 Kor\'kron Scout'),
(61790, 0, 3, 4, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 4, 0, 0, 0, 0, 10, @CGUID+113, 61788, 0, 0, 0, 0, 0, 0, 'Inconspicuous Crate - On Gossip Select - Set Data 0 4 SI:7 Ranger'),
(61790, 0, 4, 5, 61, 0, 100, 0, 0, 0, 0, 0, 0, 83, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Inconspicuous Crate - On Gossip Select - Remove Gossip Flag'),
(61790, 0, 5, 6, 61, 0, 100, 0, 0, 0, 0, 0, 0, 72, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 'Inconspicuous Crate - On Gossip Select - Close Gossip'),
(61790, 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 80, 6179000, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Inconspicuous Crate - On Gossip Select - Run Action List'),
(6178000, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 128, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Suspicious Rock - Action List - Clear AIAnimKit'),
(6178000, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Suspicious Rock - Action List - Enable Gravity'),
(6178000, 9, 2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 103, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Suspicious Rock - Action List - Remove Root'),
(6178000, 9, 3, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 97, 14, 8, 0, 0, 0, 0, 1, 0, 0, 0, 0, -308.103, -15.2036, -56.5575, 0, 'Suspicious Rock - Action List - Jump To Pos'),
(6178000, 9, 4, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 41, 2000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Suspicious Rock - Action List - Despawn In 2 s'),
(6179000, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 103, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Inconspicuous Crate - Action List - Remove Root'),
(6179000, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Inconspicuous Crate - Action List - Enable Gravity'),
(6179000, 9, 2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 97, 14, 5, 0, 0, 0, 0, 1, 0, 0, 0, 0, -87.7343, 57.2282, -18.1655, 0, 'Inconspicuous Crate - Action List - Jump To Pos'),
(6179000, 9, 3, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 41, 2000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Inconspicuous Crate - Action List - Despawn In 2 s');

-- Scout cages near Lava Guard Gordoth
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(211883, 1, 0, 1, 70, 0, 100, 0, 2, 0, 0, 0, 0, 33, 61680, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Kill Credit Kor\'kron Scout to Invoker Party'),
(211883, 1, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 33, 61788, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Kill Credit SI:7 Ranger to Invoker Party'),
(211883, 1, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 6, 0, 0, 0, 0, 10, @CGUID+100, 61680, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Set Data 0 6 Kor\'kron Scout'),
(211883, 1, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 6, 0, 0, 0, 0, 10, @CGUID+119, 61788, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Set Data 0 6 SI:7 Ranger'),
(211884, 1, 0, 1, 70, 0, 100, 0, 2, 0, 0, 0, 0, 33, 61680, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Kill Credit Kor\'kron Scout to Invoker Party'),
(211884, 1, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 33, 61788, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Kill Credit SI:7 Ranger to Invoker Party'),
(211884, 1, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 7, 0, 0, 0, 0, 10, @CGUID+101, 61680, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Set Data 0 7 Kor\'kron Scout'),
(211884, 1, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 7, 0, 0, 0, 0, 10, @CGUID+121, 61788, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Set Data 0 7 SI:7 Ranger'),
(211885, 1, 0, 1, 70, 0, 100, 0, 2, 0, 0, 0, 0, 33, 61680, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Kill Credit Kor\'kron Scout to Invoker Party'),
(211885, 1, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 33, 61788, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Kill Credit SI:7 Ranger to Invoker Party'),
(211885, 1, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 8, 0, 0, 0, 0, 10, @CGUID+104, 61680, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Set Data 0 8 Kor\'kron Scout'),
(211885, 1, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 45, 0, 8, 0, 0, 0, 0, 10, @CGUID+120, 61788, 0, 0, 0, 0, 0, 0, 'Scout Cage - On Activated - Set Data 0 8 SI:7 Ranger');

-- Kor'kron Scouts and SI:7 Rangers: one script for both entries. Data 1 hound, 2 rock, 4 crate, 6-8 cages; the two cliff-hangers
-- (spellclick) are told apart by guid conditions. The action lists are shared too (TDB had a copy per entry that only differed
-- in the credit spell). They walk to safety and despawn.
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(61680, 0, 0, 0, 38, 0, 100, 0, 0, 1, 0, 0, 0, 80, 6168000, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Data Set 0 1 - Run Action List (hound killed)'),
(61680, 0, 1, 0, 38, 0, 100, 0, 0, 2, 0, 0, 0, 80, 6168001, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Data Set 0 2 - Run Action List (rock)'),
(61680, 0, 2, 3, 73, 0, 100, 0, 0, 0, 0, 0, 0, 33, 61680, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Spellclick - Kill Credit to Invoker Party (first cliff-hanger)'),
(61680, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 80, 6168002, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Spellclick - Run Action List (first cliff-hanger)'),
(61680, 0, 4, 0, 38, 0, 100, 0, 0, 4, 0, 0, 0, 80, 6168003, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Data Set 0 4 - Run Action List (crate)'),
(61680, 0, 5, 6, 73, 0, 100, 0, 0, 0, 0, 0, 0, 33, 61680, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Spellclick - Kill Credit to Invoker Party (second cliff-hanger)'),
(61680, 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 80, 6168004, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Spellclick - Run Action List (second cliff-hanger)'),
(61680, 0, 7, 0, 38, 0, 100, 0, 0, 6, 0, 0, 0, 53, 1, 6178805, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Data Set 0 6 - Start Waypoint (cage 1)'),
(61680, 0, 8, 0, 38, 0, 100, 0, 0, 7, 0, 0, 0, 53, 1, 6178806, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Data Set 0 7 - Start Waypoint (cage 2)'),
(61680, 0, 9, 0, 38, 0, 100, 0, 0, 8, 0, 0, 0, 53, 1, 6178807, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Data Set 0 8 - Start Waypoint (cage 3)'),
(61680, 0, 10, 0, 40, 0, 100, 0, 1, 6178800, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Waypoint 1 Reached - Despawn'),
(61680, 0, 11, 0, 40, 0, 100, 0, 5, 6178801, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Waypoint 5 Reached - Despawn'),
(61680, 0, 12, 0, 40, 0, 100, 0, 1, 6178802, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Waypoint 1 Reached - Despawn'),
(61680, 0, 13, 0, 40, 0, 100, 0, 3, 6178803, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Waypoint 3 Reached - Despawn'),
(61680, 0, 14, 0, 40, 0, 100, 0, 3, 6178804, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Waypoint 3 Reached - Despawn'),
(61680, 0, 15, 0, 40, 0, 100, 0, 1, 6178805, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Waypoint 1 Reached - Say Line 0'),
(61680, 0, 16, 0, 40, 0, 100, 0, 2, 6178805, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Waypoint 2 Reached - Despawn'),
(61680, 0, 17, 0, 40, 0, 100, 0, 1, 6178806, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Waypoint 1 Reached - Say Line 0'),
(61680, 0, 18, 0, 40, 0, 100, 0, 2, 6178806, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Waypoint 2 Reached - Despawn'),
(61680, 0, 19, 0, 40, 0, 100, 0, 1, 6178807, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Waypoint 1 Reached - Say Line 0'),
(61680, 0, 20, 0, 40, 0, 100, 0, 2, 6178807, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Scout - On Waypoint 2 Reached - Despawn'),
(61788, 0, 0, 0, 38, 0, 100, 0, 0, 1, 0, 0, 0, 80, 6168000, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Data Set 0 1 - Run Action List (hound killed)'),
(61788, 0, 1, 0, 38, 0, 100, 0, 0, 2, 0, 0, 0, 80, 6168001, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Data Set 0 2 - Run Action List (rock)'),
(61788, 0, 2, 3, 73, 0, 100, 0, 0, 0, 0, 0, 0, 33, 61788, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Spellclick - Kill Credit to Invoker Party (first cliff-hanger)'),
(61788, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 80, 6168002, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Spellclick - Run Action List (first cliff-hanger)'),
(61788, 0, 4, 0, 38, 0, 100, 0, 0, 4, 0, 0, 0, 80, 6168003, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Data Set 0 4 - Run Action List (crate)'),
(61788, 0, 5, 6, 73, 0, 100, 0, 0, 0, 0, 0, 0, 33, 61788, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Spellclick - Kill Credit to Invoker Party (second cliff-hanger)'),
(61788, 0, 6, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 80, 6168004, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Spellclick - Run Action List (second cliff-hanger)'),
(61788, 0, 7, 0, 38, 0, 100, 0, 0, 6, 0, 0, 0, 53, 1, 6178805, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Data Set 0 6 - Start Waypoint (cage 1)'),
(61788, 0, 8, 0, 38, 0, 100, 0, 0, 7, 0, 0, 0, 53, 1, 6178806, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Data Set 0 7 - Start Waypoint (cage 2)'),
(61788, 0, 9, 0, 38, 0, 100, 0, 0, 8, 0, 0, 0, 53, 1, 6178807, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Data Set 0 8 - Start Waypoint (cage 3)'),
(61788, 0, 10, 0, 40, 0, 100, 0, 1, 6178800, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Waypoint 1 Reached - Despawn'),
(61788, 0, 11, 0, 40, 0, 100, 0, 5, 6178801, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Waypoint 5 Reached - Despawn'),
(61788, 0, 12, 0, 40, 0, 100, 0, 1, 6178802, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Waypoint 1 Reached - Despawn'),
(61788, 0, 13, 0, 40, 0, 100, 0, 3, 6178803, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Waypoint 3 Reached - Despawn'),
(61788, 0, 14, 0, 40, 0, 100, 0, 3, 6178804, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Waypoint 3 Reached - Despawn'),
(61788, 0, 15, 0, 40, 0, 100, 0, 1, 6178805, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Waypoint 1 Reached - Say Line 0'),
(61788, 0, 16, 0, 40, 0, 100, 0, 2, 6178805, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Waypoint 2 Reached - Despawn'),
(61788, 0, 17, 0, 40, 0, 100, 0, 1, 6178806, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Waypoint 1 Reached - Say Line 0'),
(61788, 0, 18, 0, 40, 0, 100, 0, 2, 6178806, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Waypoint 2 Reached - Despawn'),
(61788, 0, 19, 0, 40, 0, 100, 0, 1, 6178807, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Waypoint 1 Reached - Say Line 0'),
(61788, 0, 20, 0, 40, 0, 100, 0, 2, 6178807, 0, 0, 0, 41, 1000, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Ranger - On Waypoint 2 Reached - Despawn'),
(6168000, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Say Line 0'),
(6168000, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 128, 1439, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Play One-Shot AnimKit 1439'),
(6168000, 9, 2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 128, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Clear AIAnimKit'),
(6168000, 9, 3, 0, 0, 0, 100, 0, 3400, 3400, 0, 0, 0, 53, 1, 6178800, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Start Waypoint'),
(6168001, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 11, 120296, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Cast Dust Explosion'),
(6168001, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 97, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, -306.174, -23.1592, -59.0882, 0, 'Rescued Scout/Ranger - Action List - Jump To Pos'),
(6168001, 9, 2, 0, 0, 0, 100, 0, 2200, 2200, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Say Line 0'),
(6168001, 9, 3, 0, 0, 0, 100, 0, 2200, 2200, 0, 0, 0, 53, 1, 6178801, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Start Waypoint'),
(6168002, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 128, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Clear AIAnimKit'),
(6168002, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 91, 3, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Remove AnimTier (UNIT_FIELD_BYTES_1 byte 3)'),
(6168002, 9, 2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Enable Gravity'),
(6168002, 9, 3, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 97, 8, 10, 0, 0, 0, 0, 1, 0, 0, 0, 0, -176.771, 12.8039, -31.5192, 0, 'Rescued Scout/Ranger - Action List - Jump To Pos'),
(6168002, 9, 4, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 83, 16777216, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Remove Spellclick Flag'),
(6168002, 9, 5, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Say Line 0'),
(6168002, 9, 6, 0, 0, 0, 100, 0, 3500, 3500, 0, 0, 0, 53, 1, 6178802, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Start Waypoint'),
(6168003, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 11, 120296, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Cast Dust Explosion'),
(6168003, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 97, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, -92.523, 65.7691, -18.6579, 0, 'Rescued Scout/Ranger - Action List - Jump To Pos'),
(6168003, 9, 2, 0, 0, 0, 100, 0, 2200, 2200, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Say Line 0'),
(6168003, 9, 3, 0, 0, 0, 100, 0, 2200, 2200, 0, 0, 0, 53, 1, 6178803, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Start Waypoint'),
(6168004, 9, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 128, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Clear AIAnimKit'),
(6168004, 9, 1, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 91, 3, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Remove AnimTier (UNIT_FIELD_BYTES_1 byte 3)'),
(6168004, 9, 2, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 60, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Enable Gravity'),
(6168004, 9, 3, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 97, 8, 10, 0, 0, 0, 0, 1, 0, 0, 0, 0, -295.687, 143.649, -25.0341, 0, 'Rescued Scout/Ranger - Action List - Jump To Pos'),
(6168004, 9, 4, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 83, 16777216, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Remove Spellclick Flag'),
(6168004, 9, 5, 0, 0, 0, 100, 0, 2000, 2000, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Say Line 0'),
(6168004, 9, 6, 0, 0, 0, 100, 0, 3500, 3500, 0, 0, 0, 53, 1, 6178804, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Rescued Scout/Ranger - Action List - Start Waypoint');

-- Quest givers at the end of the dungeon (TDB spawn groups 71/72, spawned when Lava Guard Gordoth is DONE): they wait in phase 8/16
-- and step into the faction phase once the encounter is done, then walk in; Xorenth and Aryna kneel when they arrive.
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(-(@CGUID+122), 0, 0, 1, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 0, 44, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Commander Bagran - OOC (Lava Guard Gordoth done) - Set Phase Mask 4'),
(-(@CGUID+122), 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 53, 0, 6172400, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Commander Bagran - OOC (Lava Guard Gordoth done) - Start Waypoint'),
(-(@CGUID+123), 0, 0, 0, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 0, 44, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Elite - OOC (Lava Guard Gordoth done) - Set Phase Mask 4'),
(-(@CGUID+124), 0, 0, 1, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 0, 44, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Invoker Xorenth - OOC (Lava Guard Gordoth done) - Set Phase Mask 4'),
(-(@CGUID+124), 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 6171600, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Invoker Xorenth - OOC (Lava Guard Gordoth done) - Start Waypoint'),
(-(@CGUID+124), 0, 2, 0, 40, 0, 100, 0, 2, 6171600, 0, 0, 0, 90, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Invoker Xorenth - On Waypoint 2 Reached - Kneel'),
(-(@CGUID+125), 0, 0, 0, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 0, 44, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'Kor\'kron Elite - OOC (Lava Guard Gordoth done) - Set Phase Mask 4'),
(-(@CGUID+126), 0, 0, 0, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 0, 44, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Commando - OOC (Lava Guard Gordoth done) - Set Phase Mask 2'),
(-(@CGUID+127), 0, 0, 0, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 0, 44, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Commando - OOC (Lava Guard Gordoth done) - Set Phase Mask 2'),
(-(@CGUID+128), 0, 0, 1, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 0, 44, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Field Commander Dirken - OOC (Lava Guard Gordoth done) - Set Phase Mask 2'),
(-(@CGUID+128), 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 53, 0, 6182200, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'SI:7 Field Commander Dirken - OOC (Lava Guard Gordoth done) - Start Waypoint'),
(-(@CGUID+129), 0, 0, 1, 1, 0, 100, 1, 1000, 1000, 1000, 1000, 0, 44, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'High Sorceress Aryna - OOC (Lava Guard Gordoth done) - Set Phase Mask 2'),
(-(@CGUID+129), 0, 1, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 53, 1, 6182300, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'High Sorceress Aryna - OOC (Lava Guard Gordoth done) - Start Waypoint'),
(-(@CGUID+129), 0, 2, 0, 40, 0, 100, 0, 2, 6182300, 0, 0, 0, 90, 8, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'High Sorceress Aryna - On Waypoint 2 Reached - Kneel');

-- SmartAI paths (waypoints is 1-based)
DELETE FROM `waypoints` WHERE `entry` IN (6171600, 6172400, 6178800, 6178801, 6178802, 6178803, 6178804, 6178805, 6178806, 6178807, 6182200, 6182300);
INSERT INTO `waypoints` (`entry`, `pointid`, `position_x`, `position_y`, `position_z`, `orientation`, `delay`, `point_comment`) VALUES
(6171600, 1, -326.362, 217.675, -20.4287, NULL, 0, 'Invoker Xorenth - Lava Guard Gordoth encounter done Path'),
(6171600, 2, -356.822, 205.992, -22.218, NULL, 0, 'Invoker Xorenth - Lava Guard Gordoth encounter done Path'),
(6172400, 1, -319.359, 221.974, -21.7194, NULL, 0, 'Commander Bagran - Lava Guard Gordoth encounter done Path'),
(6172400, 2, -326.181, 221.136, -20.6263, NULL, 0, 'Commander Bagran - Lava Guard Gordoth encounter done Path'),
(6172400, 3, -336.67, 218.982, -20.6522, NULL, 0, 'Commander Bagran - Lava Guard Gordoth encounter done Path'),
(6178800, 1, -218.402, -36.7901, -54.4282, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 1'),
(6178801, 1, -297.986, -41.8826, -60.9167, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 2'),
(6178801, 2, -286.895, -48.5545, -60.9322, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 2'),
(6178801, 3, -276.608, -47.3176, -60.9322, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 2'),
(6178801, 4, -253.954, -39.2558, -60.4965, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 2'),
(6178801, 5, -225.005, -37.1203, -55.7888, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 2'),
(6178802, 1, -234.865, 10.5354, -44.4542, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued -  Path 3'),
(6178803, 1, -108.384, 23.4184, -18.4485, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 4'),
(6178803, 2, -129.357, 9.65967, -20.6465, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 4'),
(6178803, 3, -143.547, 8.58631, -21.6209, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 4'),
(6178804, 1, -304.654, 122.731, -24.959, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 5'),
(6178804, 2, -293.629, 106.313, -26.6325, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 5'),
(6178804, 3, -280.128, 93.785, -25.0219, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 5'),
(6178805, 1, -371.524, 183.787, -22.1401, NULL, 178, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 6'),
(6178805, 2, -325.283, 223.744, -20.7957, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 6'),
(6178806, 1, -372.661, 187.373, -22.0765, NULL, 176, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 7'),
(6178806, 2, -325.283, 223.744, -20.7957, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 7'),
(6178807, 1, -374.134, 190.649, -22.0253, NULL, 185, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 8'),
(6178807, 2, -325.283, 223.744, -20.7957, NULL, 0, 'SI:7 Ranger | Kor\'kron Scout rescued - Path 8'),
(6182200, 1, -319.359, 221.974, -21.7194, NULL, 0, 'SI:7 Field Commander Dirken - Lava Guard Gordoth encounter done Path'),
(6182200, 2, -326.181, 221.136, -20.6263, NULL, 0, 'SI:7 Field Commander Dirken - Lava Guard Gordoth encounter done Path'),
(6182200, 3, -336.67, 218.982, -20.6522, NULL, 0, 'SI:7 Field Commander Dirken - Lava Guard Gordoth encounter done Path'),
(6182300, 1, -326.362, 217.675, -20.4287, NULL, 0, 'High Sorceress Aryna - Lava Guard Gordoth encounter done Path'),
(6182300, 2, -356.822, 205.992, -22.218, NULL, 0, 'High Sorceress Aryna - Lava Guard Gordoth encounter done Path');

-- 6. Conditions
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 120064;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` IN (61680, 61788) AND `SourceEntry` IN (120327, 120328);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` IN (61657, 61680, 61788) AND `SourceId` = 0;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` IN (211883, 211884, 211885) AND `SourceId` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` BETWEEN -(@CGUID+129) AND -(@CGUID+0) AND `SourceId` = 0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 120064, 0, 0, 31, 0, 3, 61659, 0, 0, 0, 0, '', 'Eat Trog - Only hits Slain Trogg'),
(13, 1, 120064, 0, 1, 31, 0, 3, 61669, 0, 0, 0, 0, '', 'Eat Trog - Or Oggleflint'),
(22, 1, 61657, 0, 0, 31, 1, 3, 61657, @CGUID+0, 0, 0, 0, '', 'Adolescent Flame Hound - Eat Trog only for the sniffed eaters'),
(22, 1, 61657, 0, 1, 31, 1, 3, 61657, @CGUID+1, 0, 0, 0, '', 'Adolescent Flame Hound - Eat Trog only for the sniffed eaters'),
(22, 1, 61657, 0, 2, 31, 1, 3, 61657, @CGUID+4, 0, 0, 0, '', 'Adolescent Flame Hound - Eat Trog only for the sniffed eaters'),
(22, 1, 61657, 0, 3, 31, 1, 3, 61657, @CGUID+22, 0, 0, 0, '', 'Adolescent Flame Hound - Eat Trog only for the sniffed eaters'),
(22, 1, 61657, 0, 4, 31, 1, 3, 61657, @CGUID+29, 0, 0, 0, '', 'Adolescent Flame Hound - Eat Trog only for the sniffed eaters'),
(18, 61680, 120327, 0, 0, 31, 1, 3, 61680, @CGUID+38, 0, 0, 0, '', 'Save Cliffhanger 1 - Only the first cliff-hanger'),
(18, 61680, 120328, 0, 0, 31, 1, 3, 61680, @CGUID+90, 0, 0, 0, '', 'Save Cliffhanger 2 - Only the second cliff-hanger'),
(22, 3, 61680, 0, 0, 31, 1, 3, 61680, @CGUID+38, 0, 0, 0, '', 'Spellclick rescue - Only the first cliff-hanger'),
(22, 6, 61680, 0, 0, 31, 1, 3, 61680, @CGUID+90, 0, 0, 0, '', 'Spellclick rescue - Only the second cliff-hanger'),
(18, 61788, 120327, 0, 0, 31, 1, 3, 61788, @CGUID+114, 0, 0, 0, '', 'Save Cliffhanger 1 - Only the first cliff-hanger'),
(18, 61788, 120328, 0, 0, 31, 1, 3, 61788, @CGUID+118, 0, 0, 0, '', 'Save Cliffhanger 2 - Only the second cliff-hanger'),
(22, 3, 61788, 0, 0, 31, 1, 3, 61788, @CGUID+114, 0, 0, 0, '', 'Spellclick rescue - Only the first cliff-hanger'),
(22, 6, 61788, 0, 0, 31, 1, 3, 61788, @CGUID+118, 0, 0, 0, '', 'Spellclick rescue - Only the second cliff-hanger'),
(22, 1, 211883, 1, 0, 29, 1, 61680, 3, 0, 0, 0, 0, '', 'Scout Cage - Credit only while a scout is inside'),
(22, 1, 211883, 1, 1, 29, 1, 61788, 3, 0, 0, 0, 0, '', 'Scout Cage - Or an SI:7 Ranger'),
(22, 1, 211884, 1, 0, 29, 1, 61680, 3, 0, 0, 0, 0, '', 'Scout Cage - Credit only while a scout is inside'),
(22, 1, 211884, 1, 1, 29, 1, 61788, 3, 0, 0, 0, 0, '', 'Scout Cage - Or an SI:7 Ranger'),
(22, 1, 211885, 1, 0, 29, 1, 61680, 3, 0, 0, 0, 0, '', 'Scout Cage - Credit only while a scout is inside'),
(22, 1, 211885, 1, 1, 29, 1, 61788, 3, 0, 0, 0, 0, '', 'Scout Cage - Or an SI:7 Ranger'),
(22, 1, -(@CGUID+122), 0, 0, 13, 1, 3, 3, 2, 0, 0, 0, '', 'End of dungeon group - Only once Lava Guard Gordoth (boss 3) is DONE'),
(22, 1, -(@CGUID+123), 0, 0, 13, 1, 3, 3, 2, 0, 0, 0, '', 'End of dungeon group - Only once Lava Guard Gordoth (boss 3) is DONE'),
(22, 1, -(@CGUID+124), 0, 0, 13, 1, 3, 3, 2, 0, 0, 0, '', 'End of dungeon group - Only once Lava Guard Gordoth (boss 3) is DONE'),
(22, 1, -(@CGUID+125), 0, 0, 13, 1, 3, 3, 2, 0, 0, 0, '', 'End of dungeon group - Only once Lava Guard Gordoth (boss 3) is DONE'),
(22, 1, -(@CGUID+126), 0, 0, 13, 1, 3, 3, 2, 0, 0, 0, '', 'End of dungeon group - Only once Lava Guard Gordoth (boss 3) is DONE'),
(22, 1, -(@CGUID+127), 0, 0, 13, 1, 3, 3, 2, 0, 0, 0, '', 'End of dungeon group - Only once Lava Guard Gordoth (boss 3) is DONE'),
(22, 1, -(@CGUID+128), 0, 0, 13, 1, 3, 3, 2, 0, 0, 0, '', 'End of dungeon group - Only once Lava Guard Gordoth (boss 3) is DONE'),
(22, 1, -(@CGUID+129), 0, 0, 13, 1, 3, 3, 2, 0, 0, 0, '', 'End of dungeon group - Only once Lava Guard Gordoth (boss 3) is DONE');

-- 7. Faction phases inside Ragefire Chasm (zone 2437): Alliance 1+2, Horde 1+4
DELETE FROM `phase_definitions` WHERE `zoneId` = 2437;
INSERT INTO `phase_definitions` (`zoneId`, `entry`, `phasemask`, `phaseId`, `terrainswapmap`, `worldMapArea`, `flags`, `comment`) VALUES
(2437, 1, 3, 0, 0, 0, 0, 'Ragefire Chasm: [A] SI:7 expedition'),
(2437, 2, 5, 0, 0, 0, 0, 'Ragefire Chasm: [H] Kor\'kron expedition');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 25 AND `SourceGroup` = 2437;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(25, 2437, 1, 0, 0, 6, 0, 469, 0, 0, 0, 0, 0, '', 'Ragefire Chasm - Alliance phase'),
(25, 2437, 2, 0, 0, 6, 0, 67, 0, 0, 0, 0, 0, '', 'Ragefire Chasm - Horde phase');

-- 8. Corrupted Insignia (A New Enemy): the cultists' quest-item rows. Flamecallers list it as a quest item but had no row; the
--    Houndmaster and Reaver rows had impossible chances (-243, -102).
DELETE FROM `creature_loot_template` WHERE `item` = 82605 AND `entry` IN (61412, 61666, 61672, 61678, 61705);
INSERT INTO `creature_loot_template` (`entry`, `item`, `ChanceOrQuestChance`, `lootmode`, `groupid`, `mincountOrRef`, `maxcount`) VALUES
(61412, 82605, -4, '', 0, 1, 1),
(61666, 82605, -100, '', 0, 1, 1),
(61672, 82605, -16, '', 0, 1, 1),
(61678, 82605, -100, '', 0, 1, 1),
(61705, 82605, -16, '', 0, 1, 1);

-- 9. Cataclysm quests: Stone Guard Kurjack and Bovaal Whitehorn are gone, so they no longer start or end Repel the Invasion (26856),
--    Taragaman the Hungerer (26858) and Elemental Tampering (26862). Nothing else starts an old Ragefire Chasm quest.
DELETE FROM `creature_queststarter` WHERE `id` IN (44216, 44217);
DELETE FROM `creature_questender` WHERE `id` IN (44216, 44217);

