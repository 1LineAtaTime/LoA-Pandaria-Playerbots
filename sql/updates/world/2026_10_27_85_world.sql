-- Silverpine Forest: two quest-invisible groups whose detection neither the retail nor the Cataclysm Classic world database
-- gives; bounded by the story here (ours):
--   84184 Detect: Quest Invis Zone 4  the Sepulcher  High Warlord Cromush (Apply Quest Invis Zone 4) stands there once the ride
--         of Lordaeron (whose spells 84166 and on sit next to 84183/84184) has brought the player in, until The Great Escape is
--         rewarded, when he is at the Forsaken Front.
--   84450 Detect: Quest Invis Zone 6  Deep Elem Mine Master Forteski's Veteran Forsaken Troopers (Apply Quest Invis Zone 6)
--         stand with him from Cornered and Crushed! taken until Nowhere to Run is rewarded.
DELETE FROM `spell_area` WHERE `spell` = 84184 AND `area` = 228;
DELETE FROM `spell_area` WHERE `spell` = 84450 AND `area` = 213;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(84184, 228, 27098, 27438, 0, 0, 2, 1, 66, 43),
(84450, 213, 27194, 27195, 0, 0, 2, 1, 74, 43);
