-- ported from PR 15 2026_10_27_75_world.sql (#986)
-- Four quests completed by reaching a place had no area trigger linked to their area-trigger objective. They are handed in
-- from the log (auto-complete), and the core let such quests be handed in unfinished, which hid this; now they need the
-- objective done. The Cataclysm Classic world database gives each objective's area trigger. This core keys area-trigger
-- objectives by the objective's own id, so the link goes through areatrigger_involvedrelation.
--   14389 Wasn't It Obvious?   AreaTrigger 5610
--   25930 Ascending the Vale   AreaTrigger 5940
--   26258 Deathwing's Fall     AreaTrigger 6126
--   28228 Rejoining the Forest AreaTrigger 6505
DELETE FROM `areatrigger_involvedrelation` WHERE `id` IN (5610, 5940, 6126, 6505);
INSERT INTO `areatrigger_involvedrelation` (`id`, `quest`) VALUES
(5610, 14389),
(5940, 25930),
(6126, 26258),
(6505, 28228);
UPDATE `quest_objective` SET `objectId` = 5610 WHERE `questId` = 14389 AND `type` = 10;
UPDATE `quest_objective` SET `objectId` = 5940 WHERE `questId` = 25930 AND `type` = 10;
UPDATE `quest_objective` SET `objectId` = 6126 WHERE `questId` = 26258 AND `type` = 10;
UPDATE `quest_objective` SET `objectId` = 6505 WHERE `questId` = 28228 AND `type` = 10;
UPDATE `quest_template_addon` SET `SpecialFlags` = `SpecialFlags` | 2 WHERE `ID` IN (14389, 25930, 26258, 28228);
