-- The ride auras of the four Hillsbrad scene cameras: their quest invisibility hid every creature from the rider
-- (spell_hillsbrad_scene_camera_ride hides the rider server side instead).
DELETE FROM `spell_script_names` WHERE `spell_id` IN (89149,91354,90714,91609) AND `ScriptName`='spell_hillsbrad_scene_camera_ride';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(89149,'spell_hillsbrad_scene_camera_ride'),
(91354,'spell_hillsbrad_scene_camera_ride'),
(90714,'spell_hillsbrad_scene_camera_ride'),
(91609,'spell_hillsbrad_scene_camera_ride');
