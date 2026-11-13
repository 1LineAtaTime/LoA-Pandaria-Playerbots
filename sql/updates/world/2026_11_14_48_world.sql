-- Eastern Plaguelands: spawns doubled on one spot by a second import batch.
-- - Cannibal Ghoul 214273, 0.6 yd from 175359.
-- - The Infectis Scar's quest pickups (Plague Puffer 45650, Infectis Incher 45655, Infectis Scuttler 45657) and a Spider
--   14881: the second guid of each pair within 0.5 yd (lights-shield's list from layers.txt). Pairs 0.6-1.8 yd apart stay:
--   clickable pickups, denser than the sniff, which only makes their quest quicker. 175053's 2-yd waypoint loop goes with it.
-- - Plagued Swine 16117 in Terrordale (A Fate Worse Than Butchery, 27533): nine of the later batch (2126xx, 2142xx, one
--   175420) stand 0.4-4.9 yd from another pig, 27 against the TDBs' 17; 18 stay.
-- Dependents checked: no pool, game event, formation, linked respawn or per-guid SmartAI on any of them; addon rows only.
DELETE FROM `creature_addon` WHERE `guid` IN (214273,213948,174844,173766,173684,173780,173721,175049,174949,175053,212682,212683,212693,214231,214264,214267,214269,214272,175420);
DELETE FROM `waypoint_data` WHERE `id`=175053;
DELETE FROM `creature` WHERE `guid`=214273 AND `id`=8530;
DELETE FROM `creature` WHERE `guid` IN (213948,174844,173766,173684,173780,173721,175049,174949,175053) AND `id` IN (14881,45650,45655,45657);
DELETE FROM `creature` WHERE `guid` IN (212682,212683,212693,214231,214264,214267,214269,214272,175420) AND `id`=16117;
