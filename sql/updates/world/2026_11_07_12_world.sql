-- Stacked yetis (Cave Yeti 2248, Ferocious Yeti 48628) in and around the yeti cave of Hillsbrad Foothills.
-- mop has 66 against 34 in the TDB 4.4.2 / TDB 1210 sniff (18 Cave, 16 Ferocious). 20 clusters of spawns chained within 3.3 yd of each
-- other (about the 3 yd wander) are one mob two to four times, some under both entries, and they make single pulls double or triple.
-- Ours: the selection. Each cluster keeps one copy, the one whose entry matches the nearest sniffed spawn. mop's other spawns are
-- left alone, leaving 40 (19 Cave, 21 Ferocious). None of these guids has an addon, SmartAI, event, pool, formation or linked respawn row.
DELETE FROM `creature` WHERE `id` IN (2248,48628) AND `guid` IN
(176648,176697,176710,176726,176897,176921,176922,176923,176924,176925,176926,176927,176928,176930,
 176931,176933,176937,176938,176940,176942,176953,176965,176966,176967,176968,176969);
