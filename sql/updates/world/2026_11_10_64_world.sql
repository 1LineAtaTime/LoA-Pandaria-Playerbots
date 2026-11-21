-- Western Plaguelands: the Brownfeather Hawks fly their routes, and two doubled hawks go.
-- Brownfeather Hawk 44475 patrols up to 106 yd above the ground (18 paths, most at z 133-176) with no `creature_template_movement` row, so the client drops it to the
-- ground under its route. Two pairs also ran the same path: 192586 and 212176 share an 11-node route at z 141.5, and so do 193050 and
-- 212448 at z 133.4.
-- Sourced: both TDBs (4.4.2 and 1210) give the hawk AnimTier 3 (flying; mop has it too) and have 17 hawks against mop's 20, with one hawk
-- on each of those two routes (327736 at z 141.5, 327717 at z 133.4). The older guids 192586 and 193050 go with their paths and addons;
-- nothing else refers to them.
-- Ours: the movement row. Neither TDB has one for the hawk; it copies Diseased Hawk 44481's (Ground 1, Swim 0, Flight 2), the zone's other
-- flying hawk: it flies when in the air and still walks where a hawk is placed on the ground (194652, 212654).
DELETE FROM `waypoint_data` WHERE `id` IN (192586,193050);
DELETE FROM `creature_addon` WHERE `guid` IN (192586,193050);
DELETE FROM `creature` WHERE (`guid`,`id`) IN ((192586,44475),(193050,44475));
DELETE FROM `creature_template_movement` WHERE `CreatureId`=44475;
INSERT INTO `creature_template_movement` (`CreatureId`,`Ground`,`Swim`,`Flight`,`Rooted`) VALUES
(44475,1,0,2,0);
