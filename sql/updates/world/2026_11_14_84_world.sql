-- Eastern Plaguelands, Victory From Within (27612): Argent Centurions come through each Argent Portal when it is opened.
-- Sourced: "Opening a portal releases Argent Centurions", "about 8 troops from both portals", they stand in the room "for a
-- brief time" and do not help (warcraft.wiki.gg notes; Wowhead comments 2011-05-01, 2012-07-05); three of 46138's Wowhead pins
-- lie on the two portals. Nothing summoned them: the portals' spells 85891 / 85892 are empty server-side rows.
-- Ours: four per portal, 20 s, one set at a time per portal (a 20 s lock, their lifetime), only for a player who has the quest in the log; the spots
-- (Hall of Arms: 5 yd around the dais on the hall floor; Library Wing: on the floor around the table the portal stands on;
-- heights from the building model). The kill credit of row 0 stays as it was, for every click.
-- What the player sees: on the click four centurions stand around the portal, facing outwards, for 20 seconds, then vanish.
--
-- Mechanism: an object's summons get no phase (Map::SummonCreature copies phases only from a unit), and a phased object
-- cannot find unphased creatures afterwards (AllWorldObjectsInRange checks the phase). So the two credit bunnies of the quest,
-- 46102 "Hall of Arms Credit" and 46103 "Library Wing Credit" (invisible model 11686, spawned nowhere, in neither TDB),
-- stand at their portals in the battle phase 287, unselectable, and do the summoning: their summons are born in 287 and are
-- never seen by players outside the battle. The portal's row 1 carries the quest condition and tells its bunny (SET_DATA);
-- the bunny's data-set row has the 20 s cooldown that a gossip-hello event cannot have. Spawning the bunnies is ours.
--
-- The centurions must stand: the template addon of 46138 carried 29266 Permanent Feign Death for the fallen scout at the
-- flare (guid 173886, the only spawn either TDB has). The aura moves to his own spawn; he also stops walking about dead
-- (random movement 3 yd). The living spawns have their own addon rows.

UPDATE `creature_template_addon` SET `auras` = NULL WHERE `entry` = 46138;
DELETE FROM `creature_addon` WHERE `guid` = 173886;
INSERT INTO `creature_addon` (`guid`, `path_id`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(173886, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, '29266');
UPDATE `creature` SET `MovementType` = 0, `wander_distance` = 0 WHERE `guid` = 173886 AND `id` = 46138;

DELETE FROM `creature` WHERE `guid` IN (3129012, 3129013);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `phaseId`, `phaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawntimesecs_max`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `npcflag2`, `unit_flags`, `unit_flags2`, `dynamicflags`, `ScriptName`, `walk_mode`, `VerifiedBuild`) VALUES
(3129012, 46102, 0, 139, 2266, 1, 1, 287, 0, 11686, 0, 1745.2, -5455.5, 83.4, 0, 300, 0, 0, 0, 42, 0, 0, 0, 0, 33555200, 0, 0, '', 0, 0), -- Hall of Arms portal; not selectable, immune
(3129013, 46103, 0, 139, 2266, 1, 1, 287, 0, 11686, 0, 1795.8, -5426, 90.8, 0, 300, 0, 0, 0, 42, 0, 0, 0, 0, 33555200, 0, 0, '', 0, 0); -- Library Wing portal

UPDATE `gameobject_template` SET `AIName` = 'SmartGameObjectAI' WHERE `entry` IN (205876, 205877);
DELETE FROM `smart_scripts` WHERE `source_type` = 1 AND `entryorguid` IN (205876, 205877) AND `id` = 1;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(205876, 1, 1, 0, 64, 0, 100, 0, 0, 0, 0, 0, 0, 45, 1, 1, 0, 0, 0, 0, 19, 46102, 10, 0, 0, 0, 0, 0, 0, 'Argent Portal (Hall of Arms) - On Gossip Hello (quest in log) - Set Data 1 1 on Hall of Arms Credit'),
(205877, 1, 1, 0, 64, 0, 100, 0, 0, 0, 0, 0, 0, 45, 1, 1, 0, 0, 0, 0, 19, 46103, 10, 0, 0, 0, 0, 0, 0, 'Argent Portal (Library Wing) - On Gossip Hello (quest in log) - Set Data 1 1 on Library Wing Credit');

UPDATE `creature_template` SET `AIName` = 'SmartAI' WHERE `entry` IN (46102, 46103);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` IN (46102, 46103);
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(46102, 0, 0, 1, 38, 0, 100, 0, 1, 1, 20000, 20000, 0, 12, 46138, 3, 20000, 0, 0, 0, 8, 0, 0, 0, 0, 1750.2, -5455.5, 82.6, 0, 'Hall of Arms Credit - On Data Set 1 1 (20 s cooldown) - Summon Argent Centurion'),
(46102, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 12, 46138, 3, 20000, 0, 0, 0, 8, 0, 0, 0, 0, 1740.2, -5455.5, 82.4, 3.14159, 'Hall of Arms Credit - Link - Summon Argent Centurion'),
(46102, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 12, 46138, 3, 20000, 0, 0, 0, 8, 0, 0, 0, 0, 1745.2, -5450.5, 82.4, 1.5708, 'Hall of Arms Credit - Link - Summon Argent Centurion'),
(46102, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 12, 46138, 3, 20000, 0, 0, 0, 8, 0, 0, 0, 0, 1745.2, -5460.5, 82.4, 4.71239, 'Hall of Arms Credit - Link - Summon Argent Centurion'),
(46103, 0, 0, 1, 38, 0, 100, 0, 1, 1, 20000, 20000, 0, 12, 46138, 3, 20000, 0, 0, 0, 8, 0, 0, 0, 0, 1801.3, -5426, 89.8, 0, 'Library Wing Credit - On Data Set 1 1 (20 s cooldown) - Summon Argent Centurion'),
(46103, 0, 1, 2, 61, 0, 100, 0, 0, 0, 0, 0, 0, 12, 46138, 3, 20000, 0, 0, 0, 8, 0, 0, 0, 0, 1790.3, -5426, 89.8, 3.14159, 'Library Wing Credit - Link - Summon Argent Centurion'),
(46103, 0, 2, 3, 61, 0, 100, 0, 0, 0, 0, 0, 0, 12, 46138, 3, 20000, 0, 0, 0, 8, 0, 0, 0, 0, 1795.8, -5419.5, 89.8, 1.5708, 'Library Wing Credit - Link - Summon Argent Centurion'),
(46103, 0, 3, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 12, 46138, 3, 20000, 0, 0, 0, 8, 0, 0, 0, 0, 1795.8, -5432, 89.8, 4.71239, 'Library Wing Credit - Link - Summon Argent Centurion');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 2 AND `SourceEntry` IN (205876, 205877) AND `SourceId` = 1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(22, 2, 205876, 1, 0, 47, 0, 27612, 10, 0, 0, 0, 0, '', 'Argent Portal (Hall of Arms) - centurions only for a player with Victory From Within in the log'),
(22, 2, 205877, 1, 0, 47, 0, 27612, 10, 0, 0, 0, 0, '', 'Argent Portal (Library Wing) - centurions only for a player with Victory From Within in the log');
