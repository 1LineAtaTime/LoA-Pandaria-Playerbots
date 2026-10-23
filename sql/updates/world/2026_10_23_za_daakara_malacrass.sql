-- #678 Zul'Aman (map 568): DB bindings for the Daakara eagle form and the Cataclysm Hex Lord Malacrass kit (boss_daakara.cpp,
-- boss_hexlord.cpp). DRAFT - not applied. The PM applies it to the test `world` DB with a backup; needs a worldserver restart
-- (spell_script_names, creature_template and spell_areatrigger_template are read at startup) and the matching build (the script
-- names below exist only from that build on).
--
-- 42577 Zap Inform (Energy Storm 43983 proc, TARGET_UNIT_NEARBY_ENTRY on the Feather Vortex 24136 - its `conditions` row 13/1/42577
--   already exists in world): spell_daakara_zap makes the hit vortex cast Zap 43137 on the player who cast the
--   spell (port of AC boss_zuljin.cpp:438-459 spell_zuljin_zap; AC binds it the same way: acore_world spell_script_names 42577).
-- 52827 Blood Worm (Malacrass Death Knight kit): npc_hexlord_blood_worm walks to Malacrass and heals him 3% (97630) on arrival.
-- 407 Hex Sphere area trigger (Malacrass Monk kit, spell 127737 effect 179 misc 407): sat_hexlord_hex_sphere deals 127741 to the
--   first player that touches it. Radius 1.5 yd (Wowpedia: "enemies that touch it"); Flags/CollisionType as the other one-shot
--   sphere/pool triggers in this table (204 sat_scholomance_gravity_flux, 1 = sphere).
-- Before (world, checked 2026-10-23): no spell_script_names row for 42577; creature_template 52827 ScriptName '';
--   no spell_areatrigger_template row 407. No `disables` row for any of these spells.
-- Undo:
--   DELETE FROM `spell_script_names` WHERE `spell_id` = 42577 AND `ScriptName` = 'spell_daakara_zap';
--   UPDATE `creature_template` SET `ScriptName` = '' WHERE `entry` = 52827;
--   DELETE FROM `spell_areatrigger_template` WHERE `Entry` = 407;
DELETE FROM `spell_script_names` WHERE `spell_id` = 42577 AND `ScriptName` = 'spell_daakara_zap';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(42577, 'spell_daakara_zap');

UPDATE `creature_template` SET `ScriptName` = 'npc_hexlord_blood_worm' WHERE `entry` = 52827;

DELETE FROM `spell_areatrigger_template` WHERE `Entry` = 407;
INSERT INTO `spell_areatrigger_template` (`Entry`, `Flags`, `CollisionType`, `Radius`, `ScaleX`, `ScaleY`, `ScriptName`) VALUES
(407, 0, 1, 1.5, 0, 0, 'sat_hexlord_hex_sphere');
