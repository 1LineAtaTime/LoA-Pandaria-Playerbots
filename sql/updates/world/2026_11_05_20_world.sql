-- Hillsbrad Foothills, The Road to Purgation (28375): the player rode Kasha 48467 without seeing Kingslayer Orkus in front.
-- Ride to Purgation 90122 (5.4.8 DBC: effect 0 control vehicle, effect 1 quest invisibility type 4 on the rider, effect 2
-- phase 348) made the rider invisible, and this core's WorldObject::CanDetectInvisibilityOf lets an invisible unit see only units
-- that detect its invisibility type. Kasha is the player's own summon and is always seen; Orkus, her summon, detects no type 4.
-- The scene cameras' ride auras had the same effect layout and the same problem: spell_hillsbrad_scene_camera_ride prevents the
-- invisibility and hides the rider server-side instead (as GM invisibility), and shows them again when the ride ends.
-- (The Heroes of the Horde! flight on Kasha 48504 seats the player directly and has no ride aura.)
DELETE FROM `spell_script_names` WHERE `spell_id` = 90122 AND `ScriptName` = 'spell_hillsbrad_scene_camera_ride';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90122, 'spell_hillsbrad_scene_camera_ride');
