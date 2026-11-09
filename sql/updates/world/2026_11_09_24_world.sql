-- The Hinterlands: the next quest is offered at turn-in again (quest_template.RewardNextQuest, 0 on every quest of the zone in mop).
-- Sourced: TDB 4.4.2 and 1210 (identical here); 28574 -> 26238 and 26542 -> 26547 are TDB 1210 only.
-- Ours: both TDBs point the Alliance Vilebranch Scum 26497 at the Horde 26307 and the Alliance Dark Vessels 26524 at the Horde 26368;
-- here they point at their own faction's twins, 26498 and 26525. The OOX quests 485/836 are left out.
-- The core only offers a quest its ender also starts and the player can take, so an offer whose prerequisites are not met just fails.
UPDATE `quest_template` SET `RewardNextQuest`=26224 WHERE `ID`=26223;
UPDATE `quest_template` SET `RewardNextQuest`=26432 WHERE `ID` IN (26267,26268);
UPDATE `quest_template` SET `RewardNextQuest`=26306 WHERE `ID`=26432;
UPDATE `quest_template` SET `RewardNextQuest`=26307 WHERE `ID`=26306;
UPDATE `quest_template` SET `RewardNextQuest`=26308 WHERE `ID`=26307;
UPDATE `quest_template` SET `RewardNextQuest`=26309 WHERE `ID`=26310;
UPDATE `quest_template` SET `RewardNextQuest`=26369 WHERE `ID`=26363;
UPDATE `quest_template` SET `RewardNextQuest`=26363 WHERE `ID`=26367;
UPDATE `quest_template` SET `RewardNextQuest`=26406 WHERE `ID`=26381;
UPDATE `quest_template` SET `RewardNextQuest`=26387 WHERE `ID`=26382;
UPDATE `quest_template` SET `RewardNextQuest`=26418 WHERE `ID`=26406;
UPDATE `quest_template` SET `RewardNextQuest`=26558 WHERE `ID`=26418;
UPDATE `quest_template` SET `RewardNextQuest`=26419 WHERE `ID`=26558;
UPDATE `quest_template` SET `RewardNextQuest`=26496 WHERE `ID` IN (26491,26492);
UPDATE `quest_template` SET `RewardNextQuest`=26497 WHERE `ID`=26496;
UPDATE `quest_template` SET `RewardNextQuest`=26498 WHERE `ID`=26497;
UPDATE `quest_template` SET `RewardNextQuest`=26516 WHERE `ID`=26498;
UPDATE `quest_template` SET `RewardNextQuest`=26515 WHERE `ID`=26518;
UPDATE `quest_template` SET `RewardNextQuest`=26524 WHERE `ID`=26523;
UPDATE `quest_template` SET `RewardNextQuest`=26525 WHERE `ID`=26524;
UPDATE `quest_template` SET `RewardNextQuest`=26529 WHERE `ID`=26528;
UPDATE `quest_template` SET `RewardNextQuest`=26530 WHERE `ID`=26529;
UPDATE `quest_template` SET `RewardNextQuest`=26531 WHERE `ID`=26530;
UPDATE `quest_template` SET `RewardNextQuest`=26532 WHERE `ID`=26531;
UPDATE `quest_template` SET `RewardNextQuest`=26547 WHERE `ID`=26542;
UPDATE `quest_template` SET `RewardNextQuest`=26548 WHERE `ID` IN (26546,26547);
UPDATE `quest_template` SET `RewardNextQuest`=26643 WHERE `ID`=26641;
UPDATE `quest_template` SET `RewardNextQuest`=26238 WHERE `ID`=28574;
