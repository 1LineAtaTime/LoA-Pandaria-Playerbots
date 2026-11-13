-- LoA beta data batch; source pin 8a26cd41fe. See doc/loa-beta-integration-2026-11-07.md.

-- UPSTREAM: 2026_10_28_963_quest_objective_poi_flags.sql
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


-- UPSTREAM: 2026_10_29_986_03_ti_mohawk_camp_z.sql
-- ported from PR 15 2026_10_23_02_world.sql (#986)
-- Night Elf Mohawk camp in Tirisfal Glades: put the pre-Cataclysm spawns back on the lowered terrain
UPDATE `creature` SET `position_z` = 34.299 WHERE `guid` = 62020; -- Night Elf Mohawk
UPDATE `creature` SET `position_z` = 33.745 WHERE `guid` = 83023; -- Mighty Frostsaber
UPDATE `gameobject` SET `position_z` = 34.596 WHERE `guid` = 9035; -- Night Elf Stool
UPDATE `gameobject` SET `position_z` = 34.000 WHERE `guid` = 9792; -- Night Elf Tent 02 x.8, sunk into the slope so its downhill edge touches the ground


-- UPSTREAM: 2026_10_29_986_05_ti_speak_with_sevren.sql
-- ported from PR 15 2026_10_24_01_world.sql (#986)
-- Speak with Sevren (25005): Coleman Farthing offered it straight away, but it hands in the Agamand remains collected in Deaths in
-- the Family (25029), as the pre-Cataclysm 355 followed 354
UPDATE `quest_template_addon` SET `PrevQuestID` = 25029 WHERE `ID` = 25005;
-- #986 addition (not in the PR): Wowpedia lists "Speak with Sevren" as following both The Mills Overrun (25004) and Deaths in the
-- Family (25029); the PR only sets PrevQuestID 25029, so also require 25004 to be rewarded (SFDB 5.4.8 has no prerequisite at all)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` = 25005 AND `ConditionTypeOrReference` = 8 AND `ConditionValue1` = 25004;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 25005, 0, 0, 8, 0, 25004, 0, 0, 0, 0, 0, '', 'Speak with Sevren (25005) requires The Mills Overrun (25004) rewarded');


-- UPSTREAM: 2026_10_29_986_06_respawn_quest_targets.sql
-- ported from PR 15 2026_10_24_03_world.sql (#986)
-- Hostile quest targets and rares that respawned after 1-6 seconds
-- The world dump gives these spawns a respawn delay of 1-6 seconds (plus the 60 second corpse decay, halved once looted), so named
-- quest targets came back within 30-60 seconds and rares were up almost permanently. TDB 4.3.4 and AzerothCore use 300 seconds for
-- all of them (1800 for the rare Gug Fatcandle, 60 for Felendren the Banished); spawns without an older counterpart get the delay of
-- the other spawns of the same creature or of their neighbours. Scripted event, escort and vehicle creatures that rely on a short
-- delay, and the Dead Scar battle on the Isle of Quel'Danas, keep theirs.

-- Tirisfal Glades: Nissa Agamand (1655), Devlin Agamand (1657), Captain Perrine (1662), Scarlet Lieutenant Gebler (39002)
-- Deathknell: Karrel Grayves (1918)
UPDATE `creature` SET `spawntimesecs` = 300 WHERE `guid` IN (194142, 194144, 192609, 194288, 194116, 192562);
-- Elwynn Forest: Princess (330); Northshire: Kurtok the Slayer (42938); Durotar: Lieutenant Palliter (39269)
UPDATE `creature` SET `spawntimesecs` = 300 WHERE `guid` IN (180100, 178468, 252854);
-- Sunstrider Isle: Felendren the Banished (15367)
UPDATE `creature` SET `spawntimesecs` = 60 WHERE `guid` = 54969;
-- Blade's Edge Mountains: Consortium Engineer (19673), the only one of its spawns with 6 seconds
UPDATE `creature` SET `spawntimesecs` = 300 WHERE `guid` = 102294;

-- Rares: Gug Fatcandle (62), Tormented Spirit (1533), Fellicent's Shade (10358)
UPDATE `creature` SET `spawntimesecs` = 1800 WHERE `guid` IN (302867, 18298, 194297, 16737, 193453);

-- The Wandering Isle: Fe-Feng Wiseman (55601), like the other Fe-Feng hozen in the village
UPDATE `creature` SET `spawntimesecs` = 300 WHERE `guid` IN (562795, 562796, 562797, 562798, 562800, 562801, 562802, 562803, 562804, 562808,
562819, 562820, 562826, 562827, 562828, 562830, 562831, 562832, 562834, 562835, 562838, 562839, 562844, 562859, 562872, 562878, 562890);
-- The Wandering Isle: Amberleaf Scamp (54130) and Corsac Fox (53704, 57797), like their other spawns
UPDATE `creature` SET `spawntimesecs` = 600 WHERE `guid` IN (562056, 562057, 562058, 562061, 562062, 562065, 562066, 562067, 562069, 562074,
562076, 562078, 562079, 562082, 562083, 562084, 562085, 562086, 562087, 562090, 562095, 562098, 562099, 562100, 562101, 562102, 562105,
562106, 562107, 562108, 562110, 562115, 562116, 562117, 562118, 562119, 562120, 563539);
UPDATE `creature` SET `spawntimesecs` = 600 WHERE `guid` IN (562063, 562064, 562080, 562091, 563535, 562075, 562088);


-- UPSTREAM: 2026_10_29_986_09_target_dummies_stationary.sql
-- ported from PR 15 2026_10_25_46_world.sql (#986)
-- Practice Target (5674) beside Helena Atwood in the Undercity and the Archery Targets (5202) in Durotar were spawned with
-- random movement and wandered around. They are target dummies and stand still, like the other Archery Target spawns.
UPDATE `creature` SET `MovementType` = 0, `wander_distance` = 0 WHERE `guid` IN (192550, 255194, 255197, 255272, 255290);
