-- #685 Scholomance: Jandice Barov's Gravity Flux carrier used NPC 67272 (Assault on Zanvess "Scenario Controller",
-- ScriptName npc_zanvess_scenario_controller, faction 7). MoP 5.4.8 has no dedicated Gravity Flux carrier either
-- (checked 2026-10-22: no creature_template spell/name for 114035). New neutral stalker cloned from 57860 "Frost Grasp Stalker"
-- (the other Scholomance spell-carrier stalker: faction 14, unit_flags NON_ATTACKABLE|..., flags_extra 128 trigger, no script).
-- scholomance.h NPC_GRAVITY_FLUX_STALKER and MopScholoShared.h NPC_GRAVITY_FLUX_STALKER = 300001 change in the same commit.
-- Undo: DELETE FROM creature_template WHERE entry=300001; DELETE FROM creature_template_model WHERE CreatureID=300001;
DELETE FROM `creature_template` WHERE `entry`=300001;
DELETE FROM `creature_template_model` WHERE `CreatureID`=300001;
CREATE TEMPORARY TABLE `tmp_gf` SELECT * FROM `creature_template` WHERE `entry`=57860;
UPDATE `tmp_gf` SET `entry`=300001, `name`='Gravity Flux Stalker', `minlevel`=90, `maxlevel`=90, `ScriptName`='', `AIName`='';
INSERT INTO `creature_template` SELECT * FROM `tmp_gf`;
DROP TEMPORARY TABLE `tmp_gf`;
INSERT INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,`VerifiedBuild`)
SELECT 300001,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,0 FROM `creature_template_model` WHERE `CreatureID`=57860;
