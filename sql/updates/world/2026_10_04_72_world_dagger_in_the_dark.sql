-- Epic #441 (alexkulya audit rows SQ014 / SQ015 / WU009). Staged 2026-09-23,
-- re-checked 2026-10-04 against the LoA base world dump + sql/updates and LoA SmartAI enums.
-- Scenario "Dagger in the Dark" (map 1095): creature data needed by LoA's existing C++
-- (src/server/scripts/Pandaria/Scenarios/DaggerInTheDark/dagger_in_the_dark.cpp).
-- Source: alexkulya MoP commit 39ab764, sql/updates/world/2026_09_05_00_world_dagger_in_the_dark.sql
-- Key fix: Vol'jin (67414) had gossip_menu_id 0 in LoA, but npc_dagger_in_dark_voljin::OnGossipHello adds
-- option 0 of GetDefaultGossipMenuForSource() ("I'm ready. Let's go." = gossip_menu_option 15183/0, present
-- in LoA) - so the scenario intro could never be started. Allied Horde NPCs (Kor'kron Bodyguard, Rak'gor
-- Bloodrazor, Grizzle Gearslip, Goblin Cannon, The Spring Saurok-Slayer/Drifter, Broken Mogu Tablets) had
-- hostile faction 14 in LoA; alexkulya sets 250 plus unit_flags 512.
-- LoA adaptations:
--  * all ScriptName assignments dropped: LoA already has every one of them (identical strings).
--  * Vol'jin npcflag kept at LoA's 3 (alexkulya: 1) because LoA has creature_queststarter (67414, 32251);
--    only gossip_menu_id is set.
--  * creature_template.InhabitType does not exist in LoA (dropped in LoA's 2024_07_24_00_world.sql); converted
--    with LoA's own mapping to creature_template_movement: InhabitType 4 -> Ground 0/Swim 0/Flight 1,
--    InhabitType 6 -> Ground 0/Swim 1/Flight 1. None of these 4 entries had a movement row in LoA.

UPDATE `creature_template` SET `gossip_menu_id` = 15183 WHERE `entry` = 67414;
UPDATE `creature_template` SET `faction` = 250 WHERE `entry` IN (67681, 67266, 67782);
UPDATE `creature_template` SET `unit_flags` = 512, `faction` = 250 WHERE `entry` IN (67783, 67862, 67863, 67694, 67706);
UPDATE `creature_template` SET `unit_flags` = 262148 WHERE `entry` IN (67779, 67792);

DELETE FROM `creature_template_movement` WHERE `CreatureId` IN (67273, 67590, 67783, 67706);
INSERT INTO `creature_template_movement` (`CreatureId`, `Ground`, `Swim`, `Flight`, `Rooted`) VALUES
(67273, 0, 0, 1, 0), -- Saurok Swarmling Ceiling Crawling Vehicle (InhabitType 4)
(67590, 0, 0, 1, 0), -- Darkhatched Swarmling (InhabitType 4)
(67783, 0, 1, 1, 0), -- The Spring Drifter (InhabitType 6)
(67706, 0, 1, 1, 0); -- The Spring Saurok-Slayer (InhabitType 6)
