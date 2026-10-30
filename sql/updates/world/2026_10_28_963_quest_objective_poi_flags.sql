-- Issue #963: quest data load errors.
-- (a) "Quest N objective M has non existing AreaTrigger.db2 id 0" (92 lines): quest_objective rows type 10
--     (AREATRIGGER) with objectId 0. The loader accepts -1 ("no DB2 trigger"); the live trigger credit is keyed by
--     the objective ID (ObjectMgr _questAreaTriggerStore), never by objectId, so -1 is a pure validation fix.
-- (b) "areatrigger_involvedrelation ... quest does not have flag QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT" (13),
--     "SmartAIMgr ... SpecialFlags for Quest entry ... does not include FLAGS_EXPLORATION_OR_EVENT" (4, quests 14482, 25924)
--     and "SPELL_EFFECT_QUEST_COMPLETE ... quest not have flag" (3, quests 30470, 32640, 32641):
--     quest_template_addon.SpecialFlags |= 2 for the 20 quests (the reference DB's quest_template carries 0 here too; its
--     binary does not check). INSERT .. ON DUPLICATE for quests with no addon row yet (29536, 29539).
-- (c) "quest_poi references unknown quest points" (16): quest_poi rows with no quest_poi_points row of the same
--     (QuestID, Idx1) (loader joins on Idx1). the reference DB has no points for them either; rows deleted.
-- EXPECTED AFTER: 0 for all four message types (92 + 13 + 4 + 3 + 16 = 128 lines).
UPDATE `quest_objective` SET `objectId` = -1 WHERE `type` = 10 AND `objectId` = 0;

INSERT INTO `quest_template_addon` (`ID`, `SpecialFlags`) VALUES
(29536, 2), (29539, 2)
ON DUPLICATE KEY UPDATE `SpecialFlags` = `SpecialFlags` | 2;
UPDATE `quest_template_addon` SET `SpecialFlags` = `SpecialFlags` | 2
 WHERE `ID` IN (869, 13564, 14066, 14482, 25621, 25924, 26512, 26930, 27007, 27152, 27610, 29392, 29415, 30470, 32640, 32641);

DELETE p FROM `quest_poi` p
 WHERE (p.`QuestID`, p.`Idx1`) IN ((3379,1),(6922,0),(6922,1),(8306,1),(10216,1),(11078,6),(13892,0),(24591,2),(27228,2),(27316,0),(28170,1),(29151,0),(29178,0),(29763,1),(29861,0),(32944,1))
   AND NOT EXISTS (SELECT 1 FROM `quest_poi_points` x WHERE x.`QuestID` = p.`QuestID` AND x.`Idx1` = p.`Idx1`);
