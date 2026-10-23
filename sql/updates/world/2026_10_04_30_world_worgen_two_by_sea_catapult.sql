-- Refs #439, #472 (epic: Worgen starting experience). Quest 14382 "Two By Sea" (Gilneas, map 654, Duskhaven area 5720).
-- Audit rows CR080 / WQ001 (loa_port=YES, unit U-GILNEAS-CATAPULT-DATA). DB-only, no C++.
--
-- Intended (Wowpedia "Two By Sea", MediaWiki API, read 2026-10-04): kill the machinists piloting the
-- Forsaken Catapults, hop in a catapult, aim for a ship deck, then slay Captain Anson (36397) and
-- Captain Morris (36399) below deck.
-- Fork today: catapult 36283 is faction 1735 with unit_flags 33554688 (NOT_SELECTABLE | IMMUNE_TO_PC), so its
-- spellclick 69434 can never be used; it has no vehicle accessory and its Launch spell 68659 has no script.
-- Both captains stand on the ships, so the quest cannot be done the intended way.
--
-- Source: MoP 5.4.8 world data:
--   * creature_template 36283: faction 35, unit_flags 16384;
--   * vehicle_template_accessory 36283 -> 36292 Forsaken Machinist (seat 0, minion 1, summontype 6, 30000);
--   * creature 36292: MoP 5.4.8 keeps only guid 221038. The 8 other fork spawns are sniffed seat passengers
--     standing beside each catapult; they duplicate the accessory and are removed (with their creature_addon rows).
--   * spell 68659 "Launch": MoP 5.4.8 binds spell_gen_launch, a class this fork lacks. The fork's
--     equivalent is spell_ioc_launch (src/server/scripts/Battlegrounds/isle_of_conquest.cpp:332). SpellEffect.dbc:
--     68659 and IoC 66218 have the same shape (EFFECT_0 DUMMY at TARGET_DEST_TRAJ, EFFECT_1 FORCE_CAST on
--     TARGET_UNIT_PASSENGER_0 -> "ride vehicle seat 1"), and vehicle 516 has seat 1.
-- Staged and dry-run verified 2026-09-23.
-- Not ported: alexkulya npc_forsaken_catapult / spell_catapult_boulder (abandoned WIP, incompatible with this data).
--
-- Rollback:
--   UPDATE `creature_template` SET `faction` = 1735, `unit_flags` = 33554688 WHERE `entry` = 36283;
--   DELETE FROM `vehicle_template_accessory` WHERE `entry` = 36283;
--   DELETE FROM `spell_script_names` WHERE `spell_id` = 68659 AND `ScriptName` = 'spell_ioc_launch';
--   The 8 machinist spawns: restore guids 221045 221189 221197 221290 221706 221723 221779 221788
--   (creature + creature_addon) from the world_548_20240722.sql dump.
-- Re-runnable: UPDATE + DELETE-then-INSERT; the spawn deletes are guarded by id = 36292.

UPDATE `creature_template` SET `faction` = 35, `unit_flags` = 16384 WHERE `entry` = 36283;

DELETE FROM `vehicle_template_accessory` WHERE `entry` = 36283;
INSERT INTO `vehicle_template_accessory` (`entry`, `accessory_entry`, `seat_id`, `minion`, `description`, `summontype`, `summontimer`) VALUES
(36283, 36292, 0, 1, 'Forsaken Catapult - Forsaken Machinist', 6, 30000);

DELETE FROM `spell_script_names` WHERE `spell_id` = 68659 AND `ScriptName` = 'spell_ioc_launch';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(68659, 'spell_ioc_launch');

DELETE `a` FROM `creature_addon` `a` JOIN `creature` `c` ON `c`.`guid` = `a`.`guid`
WHERE `c`.`id` = 36292 AND `a`.`guid` IN (221045, 221189, 221197, 221290, 221706, 221723, 221779, 221788);
DELETE FROM `creature` WHERE `id` = 36292 AND `guid` IN (221045, 221189, 221197, 221290, 221706, 221723, 221779, 221788);
