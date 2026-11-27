-- Zino "The Shredder" Quickchop (67534, Krasarang Wilds, Landfall) is hostile, invisible in the Landfall hub phase and
-- cannot hand out or accept the quests he belongs to.
-- Before: faction 14 (he attacks players), npcflag 0, spawn 520234 in phase 1 while Boss-Lady Trixel (67535) and the hub
-- quests (32138 Work Order: Lumber -> 32139 Stacked!) live in phase 4, and no questender row for 32138/32139 and no
-- queststarter row for 32139 (32139 has PrevQuestID 32138).
-- After: friendly faction 83 and the quest giver flag like Trixel, spawn moved into hub phase 4, Zino ends 32138 and 32139 and starts 32139.
-- Source: Wowhead MoP Classic npc 67534 (starts Stacked!, ends 2 quests); creature_template 67534 faction 83, npcflag 2,
--         unit_flags 33536 (0x8000 + IMMUNE_TO_PC + IMMUNE_TO_NPC), the usual quest giver pattern (677 quest givers carry it).
-- Rollback: UPDATE `creature_template` SET `faction` = 14, `npcflag` = 0, `unit_flags` = 0 WHERE `entry` = 67534;
--           UPDATE `creature` SET `phaseMask` = 1 WHERE `guid` = 520234 AND `id` = 67534;
--           DELETE FROM `creature_queststarter` WHERE `id` = 67534 AND `quest` = 32139;
--           DELETE FROM `creature_questender` WHERE `id` = 67534 AND `quest` IN (32138, 32139);
UPDATE `creature_template` SET `faction` = 83, `npcflag` = `npcflag` | 2, `unit_flags` = 33536 WHERE `entry` = 67534;
UPDATE `creature` SET `phaseMask` = 4 WHERE `guid` = 520234 AND `id` = 67534;
DELETE FROM `creature_queststarter` WHERE `id` = 67534 AND `quest` = 32139;
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES (67534, 32139);
DELETE FROM `creature_questender` WHERE `id` = 67534 AND `quest` IN (32138, 32139);
INSERT INTO `creature_questender` (`id`, `quest`) VALUES (67534, 32138), (67534, 32139);
