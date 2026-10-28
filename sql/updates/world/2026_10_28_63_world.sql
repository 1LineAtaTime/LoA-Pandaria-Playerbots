-- Welcome to the Machine: Ride Skeletal Steed seats the player without its phase 317, which hid the scene's own actors
-- (spell_hillsbrad_machine_ride_skeletal_steed).
DELETE FROM `spell_script_names` WHERE `spell_id`=88543 AND `ScriptName`='spell_hillsbrad_machine_ride_skeletal_steed';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(88543,'spell_hillsbrad_machine_ride_skeletal_steed');
