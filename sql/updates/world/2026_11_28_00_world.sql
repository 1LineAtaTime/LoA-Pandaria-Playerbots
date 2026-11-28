-- Rivett Clutchpop (67608, Domination Point, Krasarang Wilds) is the start and end NPC of the Horde dailies
-- 32221 "Storming the Beach" and 32214 "Bilgewater Infiltrators", but he has no quest giver flag and no quest relations,
-- so neither quest is ever offered or can be turned in.
-- Before: npcflag 0, no creature_queststarter and no creature_questender rows for 67608.
-- After: quest giver flag set, he starts and ends both quests.
-- Source: Wowhead MoP Classic quests 32221 and 32214 (start and end: Rivett Clutchpop, Horde); the same relation rows
--         exist in other 5.4.x data sets, and creature_template npcflag 2 is the usual quest giver pattern.
-- Rollback: UPDATE `creature_template` SET `npcflag` = `npcflag` & ~2 WHERE `entry` = 67608;
--           DELETE FROM `creature_queststarter` WHERE `id` = 67608 AND `quest` IN (32214, 32221);
--           DELETE FROM `creature_questender` WHERE `id` = 67608 AND `quest` IN (32214, 32221);
UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = 67608;
DELETE FROM `creature_queststarter` WHERE `id` = 67608 AND `quest` IN (32214, 32221);
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES (67608, 32214), (67608, 32221);
DELETE FROM `creature_questender` WHERE `id` = 67608 AND `quest` IN (32214, 32221);
INSERT INTO `creature_questender` (`id`, `quest`) VALUES (67608, 32214), (67608, 32221);
