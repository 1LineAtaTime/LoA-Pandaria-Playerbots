-- ported from PR 15 2026_10_28_65_world.sql (#986)
-- Welcome to the Machine: Summon Skeletal Steed seats the player with the generic ride instead of Ride Skeletal Steed (88543),
-- whose phase 317 hid every creature from the rider (spell_hillsbrad_machine_summon_skeletal_steed). The 88543 script of
-- 2026_10_28_63 could not stop that and goes.
DELETE FROM `spell_script_names` WHERE `spell_id`=88543 AND `ScriptName`='spell_hillsbrad_machine_ride_skeletal_steed';
DELETE FROM `spell_script_names` WHERE `spell_id`=88542 AND `ScriptName`='spell_hillsbrad_machine_summon_skeletal_steed';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(88542,'spell_hillsbrad_machine_summon_skeletal_steed');
