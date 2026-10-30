-- ported from PR 15 2026_10_27_63_world.sql (#986)
-- Ruins of Gilneas: show the quest-invisible Forsaken where the story puts them
-- Belmont, Arthura and the Deathstalkers carry "Apply Quest Invis Zone 7/9/10/11" (template or spawn auras, as on retail) and
-- nothing gave the matching "Detect: Quest Invis Zone N", so they were invisible: the Deathstalkers and Arthura at Forward
-- Command, Belmont and Arthura at Emberstone Village (where Fall Back! sends the player) and at Tempest's Reach. Retail does
-- not record what gave the detections; give them in the zone at the stage each group belongs to (ours):
--   Zone 7  (84685)  Deathstalkers at Forward Command      until Fall Back! is taken
--   Zone 9  (84711)  Arthura at Forward Command             from To Forsaken Forward Command taken until Fall Back! is taken
--   Zone 10 (84937)  Belmont and Arthura at Emberstone      from Fall Back! taken until A Man Named Godfrey is completed
--   Zone 11 (85039)  Belmont and Arthura at Tempest's Reach from A Man Named Godfrey completed until The Great Escape is done
-- (Belmont at Forward Command has his invisibility overridden by his spawn, as in the sniff.)
DELETE FROM `spell_area` WHERE `spell` IN (84685, 84711, 84937, 85039) AND `area` = 4706;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(84685, 4706, 0, 27405, 0, 0, 2, 1, 0, 33),
(84711, 4706, 27290, 27405, 0, 0, 2, 1, 74, 33),
(84937, 4706, 27405, 27406, 0, 0, 2, 1, 74, 41),
(85039, 4706, 27406, 27438, 0, 0, 2, 1, 66, 41);
