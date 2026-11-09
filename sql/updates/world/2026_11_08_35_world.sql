-- Arathi Basin: offer one bracket of The Battle for Arathi Basin! and Cut Arathor Supply Lines at a time
-- Deathmaster Dwire (15021) and Deathstalker Mortis (15022) start one copy of each quest per level bracket, each with a
-- MinLevel and no MaxLevel (the same in both TDBs), so a level 32 player was offered the 20 and the 30 bracket together.
-- Sourced: AzerothCore caps The Battle for Arathi Basin! at 29, 39 and 49 (2025_07_03_01, brackets 20-29, 30-39, 40-49).
-- Ours: the same brackets for Cut Arathor Supply Lines (8162 20-29, 8161 30-39, 8160 40-49, 8123 50-59; 8299 starts at 60).
-- The More Resource Crates follow-ups need their own bracket's quest rewarded, so they follow.
UPDATE `quest_template_addon` SET `MaxLevel`=29 WHERE `ID` IN (8171,8162);
UPDATE `quest_template_addon` SET `MaxLevel`=39 WHERE `ID` IN (8170,8161);
UPDATE `quest_template_addon` SET `MaxLevel`=49 WHERE `ID` IN (8169,8160);
UPDATE `quest_template_addon` SET `MaxLevel`=59 WHERE `ID`=8123;
