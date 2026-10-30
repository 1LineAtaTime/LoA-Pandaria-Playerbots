-- ported from PR 15 2026_10_27_82_world.sql (#986)
-- The Forsaken Front: the risen Godfrey, Walden and Ashbury of the camp after Rise, Godfrey carry Apply Quest Invis Zone 15
-- (85514, template auras, as on retail) and nothing gave Detect: Quest Invis Zone 15 (85513), so they were never seen once the
-- scene was over. Give it in Silverpine from Rise, Godfrey completed until Taking the Battlefront is rewarded, when they leave
-- the camp to go with the player (Battlefront Summon Master Aura, whose spells sit next to 85513). Those bounds are ours.
DELETE FROM `spell_area` WHERE `spell` = 85513 AND `area` = 130;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(85513, 130, 27472, 27542, 0, 0, 2, 1, 66, 43);
