-- #496 Darkmoon Faire Ring Toss: cast of 101695 failed SPELL_FAILED_REQUIRES_SPELL_FOCUS (105).
-- Spell 101695 -> SpellCastingRequirements 8851 -> RequiresSpellFocus 1729 ("Standing at the Ring Toss Booth").
-- gameobject_template 301114 (type 8, focus 1729, radius 20) exists but had no spawn on map 974, so the
-- throw could never be cast (same for the Whack-a-Gnoll/Tonk foci 1471/1731 which ARE spawned: 547300/547301).
-- Spawn it at Jessica Rogers' booth (creature 303665 -4287.87 6309.00 13.2001), 20 yd covers Dubenko's pond.
DELETE FROM `game_event_gameobject` WHERE `guid`=4044101;
DELETE FROM `gameobject` WHERE `guid`=4044101;
INSERT INTO `gameobject`
(`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`position_x`,`position_y`,`position_z`,`orientation`,`rotation0`,`rotation1`,`rotation2`,`rotation3`,`spawntimesecs`,`animprogress`,`state`,`ScriptName`,`VerifiedBuild`)
VALUES
(4044101,301114,974,0,0,1,1,0,0,-4287.87,6309.00,13.2001,1.48353,0,0,0.674,0.739,300,255,1,'',0);
INSERT INTO `game_event_gameobject` (`eventEntry`,`guid`) VALUES (75,4044101);
