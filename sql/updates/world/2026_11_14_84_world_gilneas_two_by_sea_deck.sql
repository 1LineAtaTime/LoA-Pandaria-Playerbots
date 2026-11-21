-- Quest 14382 "Two By Sea" (Gilneas, map 654): thin out the Forsaken Footsoldiers (36810) on the two Forsaken ships.
--
-- The player is launched by catapult onto each ship and kills Captain Anson (36397) and Captain Morris (36399)
-- "astern below deck" (Wowpedia "Two By Sea"). Retail places only the captains and Forsaken Sailors (36396)
-- there (Wowhead NPC tooltips: 36396 at 24.0-27.8 / 74.8-81.0, 36397 at 24-24.6 / 75-76, 36399 at
-- 26.8-27.6 / 79.4-80.6); Wowhead lists no spawn locations for 36810 at all. The 24 static 36810 spawns came
-- from a later batch import: a block of soldiers at the same height and facing on the ships. A level 6-7
-- player (the chain XP matches the reference data) landed among 7 of them plus sailors and invaders at Anson
-- and died within seconds, every time.
-- Keep the 9 spawns a working reference database also keeps (303027, 303030, 303032, 303033, 303034, 303035,
-- 303051, 303052, 303059) and remove the other 15, including all of them on Captain Anson's ship.
-- None of the removed spawns has an addon, formation, pool, linked-respawn, game-event or SmartAI row.
-- Re-runnable.
DELETE FROM `creature` WHERE `id` = 36810 AND `map` = 654 AND `guid` IN
(303022, 303023, 303024, 303025, 303026, 303028, 303029, 303031, 303036, 303037, 303049, 303060, 303061, 303064, 303066);

-- The two captains stood 25-40 yd off their ships, in the shallows among the Footsoldiers and Invaders. Retail
-- places them astern on the ships (Wowhead: Anson 24-24.6 / 75-76, Morris 26.8-27.6 / 79.4-80.6, which is about
-- -2107..-2128, 2666..2685 and -2199..-2224, 2571..2596 in world coordinates). The Forsaken Sailors (36396) of
-- each ship already stand at these points. Move each captain there (same positions as the working reference
-- database); keep phase, respawn and movement.
UPDATE `creature` SET `position_x` = -2099.37, `position_y` = 2693.64, `position_z` = 1.31471, `orientation` = 3.95083
WHERE `guid` = 221721 AND `id` = 36397;
UPDATE `creature` SET `position_x` = -2198.52, `position_y` = 2593.89, `position_z` = 1.72233, `orientation` = 3.86993
WHERE `guid` = 221715 AND `id` = 36399;
