-- Eastern Plaguelands, Tyr's Hand in four layers (it never changed: all 144 creatures of area 2266 were in phase 0).
--
--  layer              phase  a player in zone 139 has it when                      holds
--  before the flare   209    Soft Landing 27460 not rewarded (and Like Rats not)   Korfax, Angela, Eligor at the gate; eleven Scarlets around the forward spots
--  Scarlet garrison   282    Like Rats 27620 not rewarded                          the other 114 Scarlets, Lynnia Abbendis x2 (pool 149 whole)
--  battle             287    Soft Landing rewarded, Like Rats not rewarded         the three commanders forward, the five named Scarlets, both Argent Portals
--  Argent control     288    Like Rats rewarded                                    the three commanders forward, eight living Argent Centurions
--  always (phase 0)          -                                                     Crusader's Flare, its bunny, the fallen scout, chests, footlockers,
--                                                                                  Suffering Victims, benches, anvils, nodes, rats
--
-- Sourced: the commanders stand at the gate (both TDBs' only spawns) and move in after the flare, each to its quests' turn-in
-- point (retail quest_poi_points, same in mop_world: Eligor 1740,-5380 "outside the abbey", Korfax 1657,-5325 "in front of the
-- Barracks", Angela 1678,-5355 "central courtyard"; one extra Wowhead pin each); the named Scarlets are seen only on the chain
-- (Wowhead comments on Mataus 2011-03-10, 2012-10-21; neither TDB spawns 46092-46096); after the chain the Scarlets are gone for
-- that character and friendly NPCs stand there (comments on 9451, 2011-06-23 and 2013-10-20); Phase.dbc names 287 "Battle for
-- Tyr's Hand 1" and 288 "Battle for Tyr's Hand 2"; the centurions' spots are the Wowhead pins of 46138 (+-8 yd).
-- Ours: which spawns each of 287 / 288 held (nothing records it); 209 and 282 for the two layers Phase.dbc has no name for
-- (its "test!!!" and "[UNUSED] Redridge 4.x - Bridge": ids no spawn, phase_area row, condition or spell uses); that the
-- commanders stay after Like Rats and that eight centurions are all that stands there; z from terrain; facings; which Scarlets
-- give way at the flare: every garrison spawn that could pull a level-39 player standing at a forward spot, i.e. whose home
-- is nearer than Creature::GetAttackDistance (20 yd + its top level - 39) + its wander distance + 1.5 yd, 27.5 yd for all of
-- them: 174547, 174546, Enchanters 174276 / 174280 and Clerics 174644 / 174643 (a pair, the second at 27.8 yd) at Eligor's,
-- 174875, 174876, 175003 and Cleric 174133 at Korfax's, 175006 at Angela's. Pool 149's Warder 15832 stood 17.6 yd from
-- Korfax's spot and cannot change layer without splitting the pool: its spot moves 14 yd up the street to 31 yd from him.
-- Nearest left in the garrison: 28.5 yd (Korfax), 32.8 yd and a patrol passing at 28.8 yd with no wander (Eligor), 29.7 yd
-- (Angela).
-- Ours as well: once Like Rats is rewarded none of the chain's other quests can be accepted (conditions on quest accept). With
-- these files that state has nothing left to offer; under the old links a character could finish Like Rats with other quests
-- open, and their targets are gone for it.
--
-- The grants are zone-wide (AreaId 139, like Andorhal's rows on 28) so the town does not pop in at the area border. Nothing
-- else in zone 139 (Acherus 4281 included) or anywhere in mop_world has a spawn, a phase_area row or a condition on any of the
-- four ids, no Phase.dbc group contains them and no spell grants them. That matters beyond leaks: WorldObject::
-- UpdateAreaAndZonePhase first strips every phase listed for an area the player is not in, so an id shared with another
-- area's row (the generic 195 and 200 of the first design are Mount Hyjal's and Hillsbrad's) is removed and re-added on every
-- pass, in both places. Every row is keyed on "rewarded" only, so taking or abandoning a quest moves nobody.
-- 209 also ends with Like Rats: before this fix the chain could be finished without Soft Landing, and such a character would
-- otherwise see the gate and the forward commanders at once.
-- Each spawn is in exactly one phase; phased creatures neither see nor assist across layers (IsWithinDistInMap checks the phase).

DELETE FROM `phase_area` WHERE `AreaId` = 139 AND `PhaseId` IN (209, 282, 287, 288);
INSERT INTO `phase_area` (`AreaId`, `PhaseId`, `Comment`) VALUES
(139, 209, 'Eastern Plaguelands - Tyr''s Hand before the flare: the Brotherhood at the gate, until Soft Landing'),
(139, 282, 'Eastern Plaguelands - Tyr''s Hand: the Scarlet garrison, until Like Rats'),
(139, 287, 'Eastern Plaguelands - Battle for Tyr''s Hand 1: commanders forward, named Scarlets, portals; Soft Landing to Like Rats'),
(139, 288, 'Eastern Plaguelands - Battle for Tyr''s Hand 2: Argent control, from Like Rats');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 26 AND `SourceGroup` IN (209, 282, 287, 288) AND `SourceEntry` = 139;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 209, 139, 0, 0, 47, 0, 27460, 64, 0, 1, 0, 0, '', 'Tyr''s Hand before the flare - Soft Landing not rewarded'),
(26, 209, 139, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand before the flare - Like Rats not rewarded'),
(26, 282, 139, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand Scarlet garrison - Like Rats not rewarded'),
(26, 287, 139, 0, 0, 47, 0, 27460, 64, 0, 0, 0, 0, '', 'Battle for Tyr''s Hand 1 - Soft Landing rewarded'),
(26, 287, 139, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Battle for Tyr''s Hand 1 - Like Rats not rewarded'),
(26, 288, 139, 0, 0, 47, 0, 27620, 64, 0, 0, 0, 0, '', 'Battle for Tyr''s Hand 2 - Like Rats rewarded');

-- The garrison: every spawn of the six Scarlet entries and of Lynnia Abbendis (127 rows, all of them in area 2266; the three
-- members of pool 149, Lynnia 15598 / 17514 and Warder 15832, with them).
UPDATE `creature` SET `phaseId` = 282, `phaseGroup` = 0 WHERE `map` = 0 AND `areaId` = 2266 AND `id` IN (9447, 9448, 9449, 9450, 9451, 9452, 10828);
-- Before the flare only: the gate trio (sniffed spot) and the eleven Scarlets around the forward spots.
UPDATE `creature` SET `phaseId` = 209, `phaseGroup` = 0 WHERE `guid` IN (173882, 173883, 173884) AND `id` IN (16112, 16115, 16116);
UPDATE `creature` SET `phaseId` = 209, `phaseGroup` = 0 WHERE `guid` IN (174546, 174547, 174276, 174280, 174643, 174644, 174875, 174876, 175003, 174133, 175006) AND `id` IN (9447, 9449, 9452);
-- Pool 149's Warder out of reach of Korfax's forward spot (was 1667.7, -5311, 74.2).
UPDATE `creature` SET `position_x` = 1676, `position_y` = -5300, `position_z` = 74.4 WHERE `guid` = 15832 AND `id` = 9447;
-- During the battle only: the named Scarlets and the portals of Victory From Within (the portal rows also get their zone and area).
UPDATE `creature` SET `phaseId` = 287, `phaseGroup` = 0 WHERE `guid` IN (372312, 372313, 372314, 372315, 373004) AND `id` IN (46092, 46093, 46094, 46095, 46096);
UPDATE `gameobject` SET `phaseId` = 287, `phaseGroup` = 0, `zoneId` = 139, `areaId` = 2266 WHERE `guid` IN (220100, 220101) AND `id` IN (205876, 205877);
-- Afterwards only: the two living centurions before the cathedral (LoA's base rows, in neither TDB).
UPDATE `creature` SET `phaseId` = 288, `phaseGroup` = 0 WHERE `guid` IN (173950, 173951) AND `id` = 46138;

-- The commanders forward, once for the battle and once for afterwards, and six more centurions on the street pins (copies of
-- the gate rows / of centurion 173950; phase, position and facing replaced, no movement; the centurions take a random one of
-- the template's four models).
DELETE FROM `creature` WHERE `guid` BETWEEN 3129000 AND 3129011;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `phaseId`, `phaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawntimesecs_max`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `ScriptName`, `walk_mode`, `VerifiedBuild`)
SELECT n.`guid`, c.`id`, c.`map`, c.`zoneId`, c.`areaId`, c.`spawnMask`, c.`phaseMask`, n.`phase`, 0, IF(c.`id` = 46138, 0, c.`modelid`), c.`equipment_id`, n.`x`, n.`y`, n.`z`, n.`o`, c.`spawntimesecs`, c.`spawntimesecs_max`, 0, 0, c.`curhealth`, c.`curmana`, 0, c.`npcflag`, c.`npcflag2`, c.`unit_flags`, c.`unit_flags2`, c.`dynamicflags`, c.`ScriptName`, c.`walk_mode`, 0
FROM `creature` c JOIN (
  SELECT 3129000 `guid`, 173884 `src`, 287 `phase`, 1740.0 `x`, -5380.0 `y`, 80.9 `z`, 2.6 `o` UNION ALL -- Eligor outside the abbey (battle)
  SELECT 3129001, 173882, 287, 1657.0, -5325.0, 74.4, 5.5 UNION ALL                                      -- Korfax before the barracks (battle)
  SELECT 3129002, 173883, 287, 1678.0, -5355.0, 73.7, 1.0 UNION ALL                                      -- Angela in the courtyard (battle)
  SELECT 3129003, 173884, 288, 1740.0, -5380.0, 80.9, 2.6 UNION ALL                                      -- Eligor (afterwards)
  SELECT 3129004, 173882, 288, 1657.0, -5325.0, 74.4, 5.5 UNION ALL                                      -- Korfax (afterwards)
  SELECT 3129005, 173883, 288, 1678.0, -5355.0, 73.7, 1.0 UNION ALL                                      -- Angela (afterwards)
  SELECT 3129006, 173950, 288, 1710.0, -5286.8, 73.9, 3.1 UNION ALL                                      -- centurions, gate to cathedral
  SELECT 3129007, 173950, 288, 1672.4, -5302.9, 74.7, 0.5 UNION ALL
  SELECT 3129008, 173950, 288, 1683.2, -5319.0, 73.6, 0.5 UNION ALL
  SELECT 3129009, 173950, 288, 1710.0, -5359.3, 73.8, 3.1 UNION ALL
  SELECT 3129010, 173950, 288, 1645.5, -5367.4, 73.8, 0.5 UNION ALL
  SELECT 3129011, 173950, 288, 1634.8, -5472.2, 99.7, 0.5
) n ON n.`src` = c.`guid`;
-- The new centurions stand (the template addon still carries the fallen scout's feign death at this point).
DELETE FROM `creature_addon` WHERE `guid` BETWEEN 3129000 AND 3129011;
INSERT INTO `creature_addon` (`guid`, `path_id`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(3129006, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(3129007, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(3129008, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(3129009, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(3129010, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL),
(3129011, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, NULL);

-- Nothing of the chain is offered once Like Rats is rewarded.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceGroup` = 0 AND `SourceEntry` IN (27459, 27460, 27461, 27462, 27612, 27613, 27614, 27615, 27616, 27617, 27618, 27619) AND `ConditionTypeOrReference` = 47 AND `ConditionValue1` = 27620;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 27459, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded'),
(19, 0, 27460, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded'),
(19, 0, 27461, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded'),
(19, 0, 27462, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded'),
(19, 0, 27612, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded'),
(19, 0, 27613, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded'),
(19, 0, 27614, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded'),
(19, 0, 27615, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded'),
(19, 0, 27616, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded'),
(19, 0, 27617, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded'),
(19, 0, 27618, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded'),
(19, 0, 27619, 0, 0, 47, 0, 27620, 64, 0, 1, 0, 0, '', 'Tyr''s Hand - not offered once Like Rats is rewarded');
