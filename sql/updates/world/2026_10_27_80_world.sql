-- The Forsaken Front: Godfrey's body and Arthura carry Apply Quest Invis Zone 12 (85089) and Daschla after Rise, Godfrey carries
-- Apply Quest Invis Zone 14 (85368), as on retail (template auras), but nothing gave the detections, so they were never seen:
-- above all Daschla, who gives and takes Unyielding Servitors, needed with Breaking the Barrier for Dalar Dawnweaver.
-- Detect: Quest Invis Zone 12 (85090) sits among The Great Escape's spells (the flight that brings Godfrey's body and Arthura,
-- 85073/85074, and its credit 85091): give it in Silverpine once The Great Escape is completed. Give Detect: Quest Invis Zone 14
-- (85369) once Rise, Godfrey is completed. No source records what gave them; those starting points are ours.
DELETE FROM `spell_area` WHERE `spell` IN (85090, 85369) AND `area` = 130;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`, `quest_start_status`, `quest_end_status`) VALUES
(85090, 130, 27438, 0, 0, 0, 2, 1, 66, 0),
(85369, 130, 27472, 0, 0, 0, 2, 1, 66, 0);
