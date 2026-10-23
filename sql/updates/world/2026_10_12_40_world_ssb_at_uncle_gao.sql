-- Refs #728. Stormstout Brewery: bind AreaTrigger 8366 to the at_uncle_gao script (registration re-enabled in
-- instance_stormstout_brewery.cpp, needs the build).
--
-- AreaTrigger.dbc 8366: map 961, box 80 x 20 x 10 at (-697.2, 1154.8, 166.1), yaw 0.26 -
-- the Tasting Room floor where Uncle Gao 59074 (guid 540148, -682.3 1191.7 166.7) brews Yan-Zhu 59479 (yanzhuPos
-- -703.4 1162.4). The row was missing in the test world DB (only 7998 at_stormstout_intro).
-- Entering the box sends Gao ACTION_START_BOSS (4); npc_uncle_gao ignores it once its encounter has started.
DELETE FROM `areatrigger_scripts` WHERE `entry` = 8366;
INSERT INTO `areatrigger_scripts` (`entry`, `ScriptName`) VALUES (8366, 'at_uncle_gao');
