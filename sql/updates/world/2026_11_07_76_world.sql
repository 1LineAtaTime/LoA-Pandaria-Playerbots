-- Goggle Boggle waits for the escort instead of completing on accept. npc_professor_phizzlethorpe already calls GroupEventHappens at its end.
-- Ours: the flag (the TDBs have no flag on it either). Sourced: the wiki's objective "Escort Professor Phizzlethorpe to the cave and back".
UPDATE `quest_template_addon` SET `SpecialFlags` = 2 WHERE `ID` = 26050;
