-- Quest 29422 "Huo, the Spirit of Fire" (Wandering Isle, epic #440): "Reignite the Spirit of Fire" (kill credit
-- 54787) had no source. The quest item 72583 "Huo's Offerings" casts 102522 "Fan the Flames", whose effect 0 is a
-- periodic dummy (aura 226) that nothing handled, so the credit spell 109107 "Fan the Flames: Credit" was never cast.
-- Intended: Wowhead spell=102522 "Channeled (12 sec cast)" / "Fan Huo, the Spirit of Fire"; Wowpedia "Huo, the
-- Spirit of Fire": "Use the provided quest item, Huo's Offerings ... Huo will get noticeably bigger and wake up."
-- Script: spell_fan_the_flames (src/server/scripts/Pandaria/WanderingIsland/zone_wandering_island_north.cpp) -
-- a full channel casts 109107; an interrupted one gives nothing. Needs the core build that contains the script.
--
-- Rollback: DELETE FROM `spell_script_names` WHERE `spell_id` = 102522 AND `ScriptName` = 'spell_fan_the_flames';

DELETE FROM `spell_script_names` WHERE `spell_id` = 102522 AND `ScriptName` = 'spell_fan_the_flames';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(102522, 'spell_fan_the_flames');
