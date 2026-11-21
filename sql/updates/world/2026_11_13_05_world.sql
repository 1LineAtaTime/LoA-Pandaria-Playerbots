-- Western Plaguelands, Andorhal second battle (phase 261): fewer Provincial Minutemen in the Horde camp, held in combat.
-- Reported in game (Horde, Ace in the Hole 27089 in the log): "The quantity of Provincial minutemen making their way into the
-- horde side of town by Koltira Darkweaver and focusing the player seems excessive."
--
-- Provincial Minuteman 44944 had 48 spawns in 261; neither TDB (4.4.2, 12.1.0) spawns any. Wowhead shows 12 pins (mop-classic) and
-- 13 (cata). 4 of them (both flavours) are at the Alliance camp (1374,-1295, beside Thassarian), outside the battle; the other 21
-- merge (9 yd) into 15 battle spots. Ours stacked on them: 8 within 20 yd of the pin beside Koltira (1492,-1647; 5 within 5 yd of
-- each other at his feet), 9 around the pin at 1475,-1656 (4 within 1 yd), and 17 in a lane east of the camp (y -1700..-1726)
-- with one pin, 9 of them walking sniffed paths into the camp edge at 1464,-1662.
-- Each spot with a spawn within 20 yd keeps its nearest one (9: 312040, 312042, 312050, 312053, 312071, 312077, 312078, 312080,
-- 312092). Pins undercount here (45101 has 22/18 pins for 32 spawns), so 2 wanderers move onto 2 unheld battle spots, 82 and
-- 102 yd from Koltira (z: terrain from the extracted maps + 0.1, no model above): 312069 (1451.7,-1569.9) and 312058
-- (1411.6,-1690.3). 11 in all; the other 37 go with their path addons and paths. The 2 unheld spots in the north of the Horde camp
-- (1543.5,-1595.7 / 1549.2,-1604.3 and 1537.7,-1578.5) stay empty: a minuteman there stands 10-14 yd from the vendor Wanda
-- Henderson 47860 (faction 68, hostile to 2202), nearer than any behemoth, and would fight her.
--
-- Their opponents in the camp are the Opengut Behemoths 44946 (Wowhead 27084: "a field where seemingly infinite numbers of
-- [Provincial Minuteman] are attacking the Opengut Behemoths"). Neither side had a sparring limit, so a fight ended with a dead
-- behemoth (5 minutes) and free minutemen in the camp, or a dead minuteman. Both now take no NPC damage below 85 %, as the
-- Deathguards, Defenders and first-battle behemoths already do: the fights at the camp go on, and an engaged minuteman neither
-- turns to a passing player (SmartAI::MoveInLineOfSight needs no victim) nor answers a call for help (Creature::CanAssistTo
-- skips creatures in combat). Players' damage is not limited; the Ebon Gargoyle's kill (Unit::Kill) is not either.
--
-- Behemoth 312084 stood 10.6 yd from the cata pin that 311971 holds and on no pin of its own; it moves to the behemoth pin
-- 1509.1,-1621.5 (cata and mop-classic), beside minuteman 312042, which had no behemoth within 19.9 yd. z = terrain 64.5 + 0.1.
-- Frightened Provincial Minuteman 45101 (268, the 27089 kill target) is unchanged: 32 spawns for 28 pin spots, 2 within 45 yd of
-- Koltira, both on pins.

-- 37 minutemen
DELETE FROM `creature` WHERE `id`=44944 AND `guid` IN (312043,312044,312046,312047,312048,312049,312051,312052,312054,312055,
312056,312057,312059,312060,312061,312062,312063,312064,312065,312066,312067,312068,312070,312072,312073,312074,312075,
312076,312079,312081,312082,312085,312086,312088,312089,312091,312093);
DELETE FROM `creature_addon` WHERE `guid` IN (312043,312044,312046,312047,312048,312049,312051,312052,312054,312055,312056,312057,
312059,312060,312061,312062,312063,312064,312065,312066,312067,312068,312070,312072,312073,312074,312075,312076,312079,
312081,312082,312085,312086,312088,312089,312091,312093) AND `guid` NOT IN (SELECT `guid` FROM `creature`);
DELETE FROM `waypoint_data` WHERE `id` IN (312049,312062,312063,312073,312075,312076,312079,312081,312082,312091)
AND NOT EXISTS (SELECT 1 FROM `creature_addon` ca WHERE ca.`path_id`=`waypoint_data`.`id`);

-- 2 minutemen onto unheld battle spots
UPDATE `creature` SET `position_x`=1451.7, `position_y`=-1569.9, `position_z`=64.9, `orientation`=3.93 WHERE `guid`=312069 AND `id`=44944;
UPDATE `creature` SET `position_x`=1411.6, `position_y`=-1690.3, `position_z`=67.3, `orientation`=6.17 WHERE `guid`=312058 AND `id`=44944;

-- Behemoth onto its pin
UPDATE `creature` SET `position_x`=1509.1, `position_y`=-1621.5, `position_z`=64.6, `orientation`=0.06 WHERE `guid`=312084 AND `id`=44946;

-- Endless fights between the minutemen and the behemoths
DELETE FROM `creature_sparring_template` WHERE `creature_id` IN (44944,44946);
INSERT INTO `creature_sparring_template` (`creature_id`,`health_limit_pct`,`comment`) VALUES
(44944,85,'Provincial Minuteman - second battle for Andorhal'),
(44946,85,'Opengut Behemoth - second battle for Andorhal');
