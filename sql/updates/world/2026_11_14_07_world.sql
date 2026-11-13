-- Eastern Plaguelands: Hed'mush the Rotting 10821 no longer spawns on the caravan's stops at Crown Guard, Eastwall and
-- Northpass Tower; he keeps his one spot in the Noxious Glade.
--
--  * Before: pool 1309 (max 1, under master pool 1045065 with Death-Hunter Hawkspear's 1310) held three of his spawns,
--    16335 inside Crown Guard Tower (11 yd from the Crown Guard Tarenar), 49406 at Eastwall (28 yd from the Eastwall Tarenar,
--    34 yd from the wagon) and 131469 at Northpass (5 yd from Tarenar, 6 yd from the wagon, 9 yd from Rimblat); his fourth
--    spawn 175220 in the Noxious Glade stood outside the pool, so two could be up at once. He is a level-42 hostile rare
--    (faction 16) who casts Disease Cloud 16577 on spawn (SmartAI): up at a tower, he stood with his cloud on the hand-in
--    spot of the caravan's quests.
--  * Sourced: 175220 (2652.89,-5479.03) is the only Hed'mush spawn in both TDBs; the three tower points are in neither and
--    date from when the towers were Scourge ground [inferred].
--  * Ours: 175220 takes the three points' place in pool 1309, so the master pool still alternates him with Hawkspear as
--    before (spawns S2-3). Nothing else refers to the three guids (no addon, formation, linked respawn, event or SmartAI row).

DELETE FROM `pool_creature` WHERE `guid` IN (16335,49406,131469,175220);
DELETE FROM `creature_addon` WHERE `guid` IN (16335,49406,131469);
DELETE FROM `creature` WHERE `guid` IN (16335,49406,131469) AND `id`=10821;
INSERT INTO `pool_creature` (`guid`,`pool_entry`,`chance`,`description`) VALUES
(175220,1309,0,'Hed''mush the Rotting, Noxious Glade');
