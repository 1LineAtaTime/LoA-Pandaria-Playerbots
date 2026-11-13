-- ported from PR 15 2026_10_27_70_world.sql (#986)
-- Ruins of Gilneas: the fallen Forsaken at two lost outposts stood alive
-- The F.C.D.: "find our outpost on the coast. By now the soldiers there are likely all dead, but their F.C.D. might be
-- operational" - the five Forsaken Vanguards around the device (GO 205363).
-- Break in Communications: Dreadwatch Outpost: "Dreadwatch is lost?" - its abominations lie dead (Permanent Feign Death, as on
-- retail), but its nine Forsaken Vanguards stood alive.
-- Lay them all down with Permanent Feign Death, as the fallen Vanguards of the overrun Forward Command (45532) are.
DELETE FROM `creature_addon` WHERE `guid` IN (306612, 306613, 306614, 306615, 306616,
    306591, 306620, 306622, 306623, 306624, 306626, 306701, 306732, 306733);
INSERT INTO `creature_addon` (`guid`, `auras`) VALUES
(306612, '29266'), (306613, '29266'), (306614, '29266'), (306615, '29266'), (306616, '29266'),
(306591, '29266'), (306620, '29266'), (306622, '29266'), (306623, '29266'), (306624, '29266'),
(306626, '29266'), (306701, '29266'), (306732, '29266'), (306733, '29266');
