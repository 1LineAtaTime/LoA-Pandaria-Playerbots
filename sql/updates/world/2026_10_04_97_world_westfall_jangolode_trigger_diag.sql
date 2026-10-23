-- #485 (epic #441): bind the Livin' the Life General Trigger spells to the diagnostic spell script
-- spell_westfall_jangolode_general_trigger (src/server/scripts/EasternKingdoms/zone_westfall.cpp).
--
-- t441 r4 (build 6): Two-Shoed Lou's Old House 42500 cast all eight triggers on schedule (actionlist 4250000),
-- the player rode the house and Glubtok 42492 / Shadowy Figure 42515 were spawned, yet neither actor ever spoke
-- (no SMART_EVENT_SPELLHIT reaction), so quest 26228 never got its credit (79275). The script only logs, on the
-- "server" logger with a [J441] prefix: the area-target result of each trigger and, when it is empty, each actor's
-- distance / LOS / private owner / visibility from the house. It does not change behaviour.
-- Rollback: DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_westfall_jangolode_general_trigger';

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_westfall_jangolode_general_trigger';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(79279, 'spell_westfall_jangolode_general_trigger'), -- General Trigger 1: Figure
(79283, 'spell_westfall_jangolode_general_trigger'), -- General Trigger 2: Figure
(79284, 'spell_westfall_jangolode_general_trigger'), -- General Trigger 3: Figure
(79287, 'spell_westfall_jangolode_general_trigger'), -- General Trigger 4: Figure
(79290, 'spell_westfall_jangolode_general_trigger'), -- General Trigger 1: Glubtok
(79292, 'spell_westfall_jangolode_general_trigger'), -- General Trigger 2: Glubtok
(79294, 'spell_westfall_jangolode_general_trigger'), -- General Trigger 3: Glubtok
(79297, 'spell_westfall_jangolode_general_trigger'); -- General Trigger 4: Glubtok
