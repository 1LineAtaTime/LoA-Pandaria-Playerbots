-- Silverpine Forest: the quest-invisibility detections the retail and Cataclysm Classic world databases give (identical rows),
-- which ours lacked, so their Forsaken stood unseen:
--   83232 Detect: Quest Invis Zone 1  Forsaken High Command  The Warchief Cometh rewarded until Lordaeron is taken
--         (the Deathstalkers and High Warlord Cromush, Apply Quest Invis Zone 1)
--   83989 Detect: Quest Invis Zone 3  Forsaken High Command  from Warchief's Command (either copy) or The Warchief Cometh until
--         Rise, Forsaken is rewarded, and from No Escape rewarded until Lordaeron is taken (Agatha, Apply Quest Invis Zone 3)
--   84241 Detect: Quest Invis Zone 5  Forsaken Rear Guard    from The Warchief's Fleet taken until Lordaeron is taken
--   83979 Bond of the Val'kyr         Fenris Isle, Fenris Keep, the Shining Strand, Lordamere Lake and Forsaken High Command,
--         besides the zone-wide row we had
DELETE FROM `spell_area` WHERE `spell` = 83232 AND `area` = 5369;
DELETE FROM `spell_area` WHERE `spell` = 83989 AND `area` = 5369;
DELETE FROM `spell_area` WHERE `spell` = 84241 AND `area` = 5386;
DELETE FROM `spell_area` WHERE `spell` = 83979 AND `area` IN (172, 235, 927, 1338, 5369);
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(83232, 5369, 26965, 27098, 0, 0, 2, 1, 64, 1),
(83989, 5369, 26964, 27097, 0, 0, 2, 1, 66, 1),
(83989, 5369, 26965, 27097, 0, 0, 2, 1, 64, 1),
(83989, 5369, 27099, 27098, 0, 0, 2, 1, 64, 1),
(83989, 5369, 28568, 27097, 0, 0, 2, 1, 66, 1),
(84241, 5386, 27065, 27098, 0, 0, 2, 1, 74, 1),
(83979, 172, 27097, 27099, 0, 0, 2, 1, 74, 11),
(83979, 235, 27097, 27099, 0, 0, 2, 1, 74, 11),
(83979, 927, 27097, 27099, 0, 0, 2, 1, 74, 11),
(83979, 1338, 27097, 27099, 0, 0, 2, 1, 74, 11),
(83979, 5369, 27097, 27099, 0, 0, 2, 1, 74, 11);
