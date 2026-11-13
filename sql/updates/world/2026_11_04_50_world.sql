-- Studies in Lethality 28324 (Ruins of Southshore): the anger meter. Lethality Analyzer 88375 (aura 346, UnitPowerBar 24
-- "Blight Sample", 0-250) is given with the quest and taken at its completion or abandonment; every Specimen Collected 88769
-- (the Blight Slime's answer to each Depleted, +1 on the bar and the kill credit) sets the Slime Anger level from the bar
-- (89783/89784 Unfriendly, 89785 Hostile toward faction 1179 "Slime", with the warnings 48491-48493), and Levels II and III
-- fall one level when they run out. Script: zone_hillsbrad_purgation.cpp.
UPDATE `quest_template_addon` SET `ScriptName`='quest_hillsbrad_studies_in_lethality' WHERE `ID`=28324;

DELETE FROM `spell_script_names` WHERE `spell_id` IN (88769,89784,89785);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(88769,'spell_hillsbrad_specimen_collected'),
(89784,'spell_hillsbrad_slime_anger'),
(89785,'spell_hillsbrad_slime_anger');
