-- Arathi Highlands: creatures and gathering nodes that stood twice (or more) on one spot.
-- Sourced: the Cataclysm Classic TDB 4.4.2 spawns of each entry (taken by box; the retail TDB 1210 has the same rows here). A mop spawn goes when
-- it and another of the same entry map onto one TDB spawn, closer to each other than twice their wander distance (5 yd for standing NPCs, at most
-- 8 yd), and mop has more of the entry than the TDB within 20 yd of that spot. Pairs 2 yd apart or less go unless the TDB has two there too.
-- The copy that stays is the one with a waypoint path or pool if only one has it, otherwise the one closer to the TDB spawn.
-- Rares: Singer, Nimar the Slayer, Zalas Witherbark, Molok the Crusher and Ruul Onestone keep the copy on their TDB spot. Ruul's old pooled copy at
-- Boulderfist Hall, 969 yd away, goes with its pool row, and so does that pool's placeholder Boulderfist Mauler 11128: it matches no TDB
-- spawn (the TDB Mauler there is 160892) and would otherwise stand alone in pool 189, always up. Pool 189 goes once it is empty.
-- Deckhand Moishe and the Forsaken Courier keep the copy on the TDB spot. Hammerfall Guardian keeps 160117 (TDB spot, emote 173 and the template
-- aura) over 672, whose one-node path only replayed emote 173; its path and waypoint script 5 go with it.
-- Ours: Foulbelly and Darbel Montrose are in no TDB. Their rare pools 134 and 119 with placeholders stay, and the extra always-up copies go.
-- Ours: Prince Nazjak is in no TDB; the old whole-number copy 10970 goes and 313152 stays.
-- Ours: Midsummer (event 1) and Pilgrim's Bounty (event 41) spawns imported twice on the exact same spot, and Radulf Leder (event 20) twice on his
-- TDB spot. Critters, which neither TDB has here: only same-spot pairs (1.5 yd or less) without a path.
-- Ours: gathering nodes. An always-up copy within 2 yd of a pooled node of the same entry goes and the pooled node stays.
-- Lesser Bloodstone Deposit 7611 stood 1.3 yd from 7601 (no pool, no TDB row).
-- Dependent rows (addons, event and pool rows) are deleted only once their creature is gone, so the file is safe to run again.
-- Witherbark Axe Thrower
DELETE FROM `creature` WHERE `id`=2554 AND `guid` IN (161439);
-- Witherbark Witch Doctor
DELETE FROM `creature` WHERE `id`=2555 AND `guid` IN (160667,218734);
-- Highland Strider
DELETE FROM `creature` WHERE `id`=2559 AND `guid` IN (160654,160874,160950,161083,161459,217397);
-- Highland Thrasher
DELETE FROM `creature` WHERE `id`=2560 AND `guid` IN (218642);
-- Highland Fleshstalker
DELETE FROM `creature` WHERE `id`=2561 AND `guid` IN (217225);
-- Boulderfist Ogre
DELETE FROM `creature` WHERE `id`=2562 AND `guid` IN (160407,160634,160636,160650,160652,160712,161020,161025,217766,218757);
-- Plains Creeper
DELETE FROM `creature` WHERE `id`=2563 AND `guid` IN (160160,160176,160648,218629,218634,218744);
-- Boulderfist Enforcer
DELETE FROM `creature` WHERE `id`=2564 AND `guid` IN (160715);
-- Giant Plains Creeper
DELETE FROM `creature` WHERE `id`=2565 AND `guid` IN (191532);
-- Drywhisker Kobold
DELETE FROM `creature` WHERE `id`=2572 AND `guid` IN (161438,161440,161443,161454,161461,161467,161476,161477,161479,161480);
-- Drywhisker Surveyor
DELETE FROM `creature` WHERE `id`=2573 AND `guid` IN (161452,161515);
-- Drywhisker Digger
DELETE FROM `creature` WHERE `id`=2574 AND `guid` IN (161453,161483,161506,161509,161518,161522);
-- Young Mesa Buzzard
DELETE FROM `creature` WHERE `id`=2578 AND `guid` IN (218630);
-- Dabyrie Laborer
DELETE FROM `creature` WHERE `id`=2582 AND `guid` IN (161169);
-- Syndicate Prowler
DELETE FROM `creature` WHERE `id`=2588 AND `guid` IN (160911,161164);
-- Syndicate Mercenary
DELETE FROM `creature` WHERE `id`=2589 AND `guid` IN (161051);
-- Syndicate Conjuror
DELETE FROM `creature` WHERE `id`=2590 AND `guid` IN (161272);
-- Syndicate Magus
DELETE FROM `creature` WHERE `id`=2591 AND `guid` IN (161170);
-- Daggerspine Raider
DELETE FROM `creature` WHERE `id`=2595 AND `guid` IN (160922,217736);
-- Darbel Montrose
DELETE FROM `creature` WHERE `id`=2598 AND `guid` IN (161075);
-- Singer
DELETE FROM `creature` WHERE `id`=2600 AND `guid` IN (12205);
-- Foulbelly
DELETE FROM `creature` WHERE `id`=2601 AND `guid` IN (312677);
-- Ruul Onestone
DELETE FROM `creature` WHERE `id`=2602 AND `guid` IN (11134);
DELETE FROM `creature` WHERE `id`=2569 AND `guid` IN (11128);
-- Molok the Crusher
DELETE FROM `creature` WHERE `id`=2604 AND `guid` IN (53075);
-- Zalas Witherbark
DELETE FROM `creature` WHERE `id`=2605 AND `guid` IN (53076);
-- Nimar the Slayer
DELETE FROM `creature` WHERE `id`=2606 AND `guid` IN (11206);
-- Hammerfall Peon
DELETE FROM `creature` WHERE `id`=2618 AND `guid` IN (161070,161320,161354,161460,161472,161525,161526);
-- Hammerfall Grunt
DELETE FROM `creature` WHERE `id`=2619 AND `guid` IN (161086);
-- Prairie Dog
DELETE FROM `creature` WHERE `id`=2620 AND `guid` IN (160214,161316,161359,161365,161366,161496);
-- Hammerfall Guardian
DELETE FROM `creature` WHERE `id`=2621 AND `guid` IN (672);
-- Forsaken Courier
DELETE FROM `creature` WHERE `id`=2714 AND `guid` IN (161298);
-- Thundering Exile
DELETE FROM `creature` WHERE `id`=2762 AND `guid` IN (11461,11470,11471,11489,11535,11586,160361,160381,160567,160726,160776);
-- Deckhand Moishe
DELETE FROM `creature` WHERE `id`=2778 AND `guid` IN (161304);
-- Prince Nazjak
DELETE FROM `creature` WHERE `id`=2779 AND `guid` IN (10970);
-- Rat
DELETE FROM `creature` WHERE `id`=4075 AND `guid` IN (217321,217326);
-- Refuge Pointe Defender
DELETE FROM `creature` WHERE `id`=10696 AND `guid` IN (160010,160387,160408,160486,217273,217299);
-- Small Frog
DELETE FROM `creature` WHERE `id`=13321 AND `guid` IN (217304,217322);
-- Midsummer Bonfire
DELETE FROM `creature` WHERE `id`=16592 AND `guid` IN (369970,370041);
-- Midsummer Celebrant
DELETE FROM `creature` WHERE `id`=16781 AND `guid` IN (369967,369968,370039,370040,370058,370332,370333,370350);
-- Syndicate Thief
DELETE FROM `creature` WHERE `id`=24477 AND `guid` IN (161017,161151,161153,161158);
-- Fire Eater
DELETE FROM `creature` WHERE `id`=25962 AND `guid` IN (370036);
-- Flame Eater
DELETE FROM `creature` WHERE `id`=25994 AND `guid` IN (369980);
-- [DND] Midsummer Bonfire Faction Bunny - H
DELETE FROM `creature` WHERE `id`=26355 AND `guid` IN (370042);
-- Radulf Leder
DELETE FROM `creature` WHERE `id`=30231 AND `guid` IN (160072);
-- Wild Turkey
DELETE FROM `creature` WHERE `id`=32820 AND `guid` IN (135850);
-- Ocean Crawler
DELETE FROM `creature` WHERE `id`=41295 AND `guid` IN (216967);

DELETE a FROM `creature_addon` a LEFT JOIN `creature` c ON c.`guid`=a.`guid` WHERE a.`guid` IN (672,11128,10970,11134,11206,11461,11470,11471,11489,11535,11586,12205,53075,53076,135850,160010,160072,160160,160176,160214,160361,160381,160387,160407,160408,160486,160567,160634,160636,160648,160650,160652,160654,160667,160712,160715,160726,160776,160874,160911,160922,160950,161017,161020,161025,161051,161070,161075,161083,161086,161151,161153,161158,161164,161169,161170,161272,161298,161304,161316,161320,161354,161359,161365,161366,161438,161439,161440,161443,161452,161453,161454,161459,161460,161461,161467,161472,161476,161477,161479,161480,161483,161496,161506,161509,161515,161518,161522,161525,161526,191532,216967,217225,217273,217299,217304,217321,217322,217326,217397,217736,217766,218629,218630,218634,218642,218734,218744,218757,312677,369967,369968,369970,369980,370036,370039,370040,370041,370042,370058,370332,370333,370350) AND c.`guid` IS NULL;
DELETE g FROM `game_event_creature` g LEFT JOIN `creature` c ON c.`guid`=g.`guid` WHERE g.`guid` IN (672,11128,10970,11134,11206,11461,11470,11471,11489,11535,11586,12205,53075,53076,135850,160010,160072,160160,160176,160214,160361,160381,160387,160407,160408,160486,160567,160634,160636,160648,160650,160652,160654,160667,160712,160715,160726,160776,160874,160911,160922,160950,161017,161020,161025,161051,161070,161075,161083,161086,161151,161153,161158,161164,161169,161170,161272,161298,161304,161316,161320,161354,161359,161365,161366,161438,161439,161440,161443,161452,161453,161454,161459,161460,161461,161467,161472,161476,161477,161479,161480,161483,161496,161506,161509,161515,161518,161522,161525,161526,191532,216967,217225,217273,217299,217304,217321,217322,217326,217397,217736,217766,218629,218630,218634,218642,218734,218744,218757,312677,369967,369968,369970,369980,370036,370039,370040,370041,370042,370058,370332,370333,370350) AND c.`guid` IS NULL;
DELETE p FROM `pool_creature` p LEFT JOIN `creature` c ON c.`guid`=p.`guid` WHERE p.`guid` IN (672,11128,10970,11134,11206,11461,11470,11471,11489,11535,11586,12205,53075,53076,135850,160010,160072,160160,160176,160214,160361,160381,160387,160407,160408,160486,160567,160634,160636,160648,160650,160652,160654,160667,160712,160715,160726,160776,160874,160911,160922,160950,161017,161020,161025,161051,161070,161075,161083,161086,161151,161153,161158,161164,161169,161170,161272,161298,161304,161316,161320,161354,161359,161365,161366,161438,161439,161440,161443,161452,161453,161454,161459,161460,161461,161467,161472,161476,161477,161479,161480,161483,161496,161506,161509,161515,161518,161522,161525,161526,191532,216967,217225,217273,217299,217304,217321,217322,217326,217397,217736,217766,218629,218630,218634,218642,218734,218744,218757,312677,369967,369968,369970,369980,370036,370039,370040,370041,370042,370058,370332,370333,370350) AND c.`guid` IS NULL;
DELETE FROM `waypoint_data` WHERE `id`=672 AND NOT EXISTS (SELECT 1 FROM `creature_addon` WHERE `path_id`=672);
DELETE FROM `waypoint_scripts` WHERE `id`=5 AND NOT EXISTS (SELECT 1 FROM `waypoint_data` WHERE `action`=5);

-- Iron Deposit
DELETE FROM `gameobject` WHERE `id`=1735 AND `guid` IN (155125,155136,155152,184861,184879,184898);
-- Tin Vein
DELETE FROM `gameobject` WHERE `id`=1732 AND `guid` IN (184845);
-- Mithril Deposit
DELETE FROM `gameobject` WHERE `id`=2040 AND `guid` IN (155151,184887);
-- Gold Vein
DELETE FROM `gameobject` WHERE `id`=1734 AND `guid` IN (184854);
-- Goldthorn
DELETE FROM `gameobject` WHERE `id`=2046 AND `guid` IN (155093);
-- Khadgar's Whisker
DELETE FROM `gameobject` WHERE `id`=2043 AND `guid` IN (184851);
-- Kingsblood
DELETE FROM `gameobject` WHERE `id`=1624 AND `guid` IN (155115,184873);
-- Lesser Bloodstone Deposit
DELETE FROM `gameobject` WHERE `id`=2653 AND `guid` IN (7611);
DELETE FROM `pool_template` WHERE `entry`=189 AND NOT EXISTS (SELECT 1 FROM `pool_creature` WHERE `pool_entry`=189) AND NOT EXISTS (SELECT 1 FROM `pool_gameobject` WHERE `pool_entry`=189) AND NOT EXISTS (SELECT 1 FROM `pool_pool` WHERE `pool_id`=189);
