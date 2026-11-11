-- The Hinterlands: Vilebranch Kidnappers only come from Lard's Picnic Basket.
-- Sourced: neither the TDB 4.4.2 nor the TDB 1210 spawns Vilebranch Kidnapper 14748; in both, Lard's Picnic Basket 179910 (goober,
-- event 8605) summons three of them by the basket for Lard Lost His Lunch 26212, and mop_world has the same template and event
-- rows. Wowhead MoP Classic pins the entry only at four spots within 20 yd of the basket on the island.
-- Ours: the two static kidnappers (one pathing on the island beside the basket, one in the shallows off it) and their
-- paths go (no pool, event, formation, SmartAI or linked respawn uses them).
-- They let a player loot Lard's Lunch without clicking the basket.
DELETE FROM `creature` WHERE `id`=14748 AND `guid` IN (192432,192433);
DELETE FROM `creature_addon` WHERE `guid` IN (192432,192433);
DELETE FROM `waypoint_data` WHERE `id` IN (192432,192433);
