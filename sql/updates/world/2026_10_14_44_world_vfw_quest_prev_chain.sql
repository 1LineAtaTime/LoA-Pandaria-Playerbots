-- Refs #784 (found by #783). Valley of the Four Winds (QuestSortID 5805): the later steps of the Tillers' vote chains had
-- no quest_template_addon.PrevQuestID, so a player or bot could take Vote III before Vote II (w34: 137 of 2314 level-90
-- accepts). Test world DB verified 2026-10-14: 30525 / 30527 / 30528 / 30529 / 31946 / 31949 have no quest_template_addon
-- row; 30517 has a row with PrevQuestID 0. The MoP 5.4.8 data has the same gaps (PrevQuestId 0 for all seven), so nothing to
-- port from there. The chain order below comes from the Wowpedia "previous" field of each quest page (MediaWiki API,
-- fetched 2026-10-14); the Wowhead tooltip API (nether.wowhead.com/tooltip/quest/<id>, 2026-10-14) confirms the id <->
-- title pairs but carries no prerequisite data.
--   Haohan's Vote   I 30521 -> II 30522 (already linked) -> III 30525 -> IV 30527 -> V 30528   (Wowpedia: each page's previous)
--   Mung-Mung's     I 30258 -> II 30259 (already linked) -> III 31946                          (Wowpedia 31946 previous = II)
--   Nana's          I 30519 -> II 31948 (already linked) -> III 31949                          (Wowpedia 31949 previous = II)
--   Farmer Fung's Vote I 30517 follows Mung-Mung's Vote III 31946                              (Wowpedia 30517 previous)
--   Growing the Farm III: The Mossy Boulder 30529 follows Haohan's Vote V 30528                (Wowpedia 30529 previous)
-- MinLevel is 90 on every row, so nothing else changes. Idempotent: re-running leaves the same rows.
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(30525, 30522),
(30527, 30525),
(30528, 30527),
(30529, 30528),
(31946, 30259),
(31949, 31948),
(30517, 31946)
ON DUPLICATE KEY UPDATE `PrevQuestID` = VALUES(`PrevQuestID`);

-- NOT applied, for owner review (each is a fan-out or has two sources that disagree):
--  * A Second Hand 30534, "The Earth-Slasher Master Plow" 31938: Wowpedia previous = 30529 (item/object-started follow-ups).
--  * "The Jinyu Princess Irrigation System" 31936: Wowpedia previous = 30516. "Inherit the Earth" 32682 -> the eight
--    Work Order I quests and the ~35 Learn and Grow V -> daily unlocks are reputation / completion unlocks, not chains.
--  * Cycles in PrevQuestID (neither quest can ever be taken): 30031 Taste Test <-> 30048 Li Li and the Grain,
--    30072 Where Silk Comes From <-> 32035 Got Silk?, 30182 Fox Mastery <-> 30183 Stalker Mastery. the MoP 5.4.8 data has
--    30048.PrevQuestId = 30053 (Wowpedia: 30046) and 30183.PrevQuestId = 0, and is itself cyclic for 30072 / 32035.
--    Seven more Pandaria cycles exist (zones 5785, 5841, 5840, 6138) plus three outside Pandaria.
