-- Refs #458
-- The Pride of Kezan (25066): "Shoot down 10 Gnomeregan Stealth Fighters" (Wowpedia The_Pride_of_Kezan, objective
-- "Gnomeregan Stealth Fighter shot down x10", read 2026-10-15). The fork's objective row asked for 1 kill: the release
-- DB carried the old "credit on accept" hack (SAI 38387 id0, removed by 2026_10_04_16), which only ever needed one
-- credit. MoP 5.4.8 (quest_objective questId 25066) has Amount 10 - the only Kezan/Lost Isles objective
-- row (116 quests, QuestSortID 4720/4737) that differs between the two DBs.
-- Rollback: UPDATE `quest_objective` SET `amount` = 1 WHERE `questId` = 25066 AND `objectId` = 39039 AND `type` = 0;

UPDATE `quest_objective` SET `amount` = 10 WHERE `questId` = 25066 AND `objectId` = 39039 AND `type` = 0;
