-- Western Plaguelands: Zen'Kiki (26953-26955) and the Enthralled Val'kyr (26937, 26938, 27089) can follow the player together
-- (spell_wpl_escorting_zenkiki_summon on Escorting Zen'Kiki's summon effect).
--
--  * Why: both summons used the quest summon slot 6 (SummonProperties 2882 for Zen'Kiki, 3030 / 3032 for the val'kyr), a new summon
--    unsummons the slot's old one, and Bond of the Val'kyr re-summons a missing val'kyr every 5 s: with a val'kyr quest in the log
--    Zen'Kiki was gone within 5 s of every summon, and 26954 / 26955 could not be done.
--  * Ours: both companions at once (a Wowhead comment on 26953, 2011-05-04, says retail kept only one of them); Zen'Kiki is summoned
--    by the core's own summon effect as before, the slot is left as it was.

DELETE FROM `spell_script_names` WHERE `spell_id` IN (83819, 83456) AND `ScriptName` = 'spell_wpl_escorting_zenkiki_summon';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(83819, 'spell_wpl_escorting_zenkiki_summon'),
(83456, 'spell_wpl_escorting_zenkiki_summon');
