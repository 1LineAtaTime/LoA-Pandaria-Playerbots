-- ported from PR 15 2026_10_27_45_world.sql (#986)
-- Ambermill: the Dimensional Barrier (QUEST INVIS 20) only shows during Transdimensional Warfare: Chapter III. Retail gives
-- its detection, Detect: Quest Invis Zone 20 (Ambermill Shield, invisibility type 28), in Silverpine while the quest is in
-- progress; mop had neither the spell_area row nor the barrier's invisibility, so everyone always saw it.
DELETE FROM `spell_area` WHERE `spell`=85406 AND `area`=130;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(85406,130,0,27518,0,0,2,1,0,8);

DELETE FROM `gameobject_addon` WHERE `guid`=160563;
INSERT INTO `gameobject_addon` (`guid`,`invisibilityType`,`invisibilityValue`) VALUES
(160563,28,1);
