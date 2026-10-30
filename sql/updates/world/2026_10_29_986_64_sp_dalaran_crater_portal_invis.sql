-- ported from PR 15 2026_10_27_83_world.sql (#986)
-- Dalaran Crater: the Portal to Dalaran Crater by Dalar Dawnweaver at the Sepulcher and the Portal to the Sepulcher in the crater
-- carry Apply Quest Invis Zone 13 (85319) and nothing gave Detect: Quest Invis Zone 13 (85320, next to the portals' spells
-- 85317/85318), so neither could be seen or used for Relios the Relic Keeper, Ley Energies and Practical Vengeance. Give it at
-- Silverpine and in the crater once Dalar Dawnweaver is completed, or while Practical Vengeance is in the log.
-- Those bounds are ours.
DELETE FROM `spell_area` WHERE `spell` = 85320 AND `area` IN (130, 279);
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(85320, 130, 27476, 0, 0, 0, 2, 1, 66, 0),
(85320, 279, 27476, 0, 0, 0, 2, 1, 66, 0),
(85320, 130, 27483, 0, 0, 0, 2, 1, 10, 0),
(85320, 279, 27483, 0, 0, 0, 2, 1, 10, 0);
