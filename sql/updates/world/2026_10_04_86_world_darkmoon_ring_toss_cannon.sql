-- Epic #441 (alexkulya audit rows SQ023); staged 2026-09-23,
-- re-checked 2026-10-04 against the LoA base world dump + sql/updates and LoA SmartAI enums.
-- 2026-10-04: cannon kill-credit spawn guid changed from the reference 8200300 to 4044100 (free: LoA base dump max
-- creature guid 582904, sql/updates use @CGUID 4000000+).
-- Darkmoon Faire (map 974): Ring Toss + Humanoid Cannonball mini-games (audit row SQ023).
--
-- In LoA the Ring Toss (Jessica Rogers 54485, quest 29455 "Target: Turtle") and the
-- Humanoid Cannonball (Maxima Blastenheimer 15303, quest 29436) cannot be played:
-- both NPCs only have "say text" SmartAI rows, no gossip-select handler, no spell
-- scripts for 101695/102058/102112, and the cannon credit NPC 54224 has no spawn.
-- (Whack-a-Gnoll, Shooting Gallery and Tonk Challenge are NOT touched: LoA already
-- implements them with SmartAI + conditions on the retail spells - see D-notes.md.)
--
-- Source: local patch
--   0014_skyfire_darkmoon_minigames.sql
--   (Ring Toss + Humanoid Cannonball sections only)
-- Companion C++ (must be added in the same commit):
--   src/server/scripts/Events/darkmoonfaire/darkmoon_faire.h
--   src/server/scripts/Events/darkmoonfaire/darkmoon_ring_toss.cpp
--   src/server/scripts/Events/darkmoonfaire/darkmoon_cannon.cpp
-- Adaptations for LoA:
--   * creature.movement_type -> MovementType; creature_addon bytes1/bytes2 -> LoA columns.
--   * Dubenko guid 303666 is the SAME Dubenko spawn in LoA (54490, map 974, event 75);
--     it keeps its guid and only gets the reference's 6-point swim loop (LoA's
--     current path 303666 moves < 1 yd) plus the Ring Toss aura 101696 in its
--     per-guid creature_addon (the per-guid row overrides the template addon).
--   * NOT ported from the reference: the two extra Dubenko spawns (8202000/8202001;
--     retail has a single Dubenko), quest 29455 RewardSpell change (LoA keeps 105326
--     like its sibling quests 29438/29463), gameobject_template 210173 data0/data1
--     (LoA already has non-zero values; game_event_gameobject row already exists),
--     Teleportologist Fozlebub C++ (LoA SmartAI already teleports back with 109244).
--   * Cannon kill-credit NPC 54224 spawned with the reference guid 4044100
--     (verified free in LoA world; LoA max creature guid is 4000077) and linked to
--     game event 75 "Darkmoon Faire" like all other LoA Darkmoon Island spawns.
-- Re-runnable: UPDATE / DELETE-then-INSERT only.

-- ---------------------------------------------------------------- Ring Toss
DELETE FROM `smart_scripts`
WHERE (`source_type` = 0 AND `entryorguid` IN (54485,54490))
   OR (`source_type` = 9 AND `entryorguid` = 5448500);

UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_darkmoon_jessica_rogers' WHERE `entry` = 54485;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = '' WHERE `entry` = 54490;

DELETE FROM `spell_script_names`
WHERE `spell_id` IN (101695,102058)
   OR `ScriptName` IN ('spell_darkmoon_ring_toss','spell_darkmoon_ring_toss_throw','spell_darkmoon_ring_toss_action');
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(101695,'spell_darkmoon_ring_toss_throw'),
(102058,'spell_darkmoon_ring_toss_action');

-- Dubenko (LoA guid 303666): swim loop in the Ring Toss pond.
UPDATE `creature`
SET `position_x`=-4302.40,`position_y`=6299.20,`position_z`=13.118,
    `orientation`=5.10,`wander_distance`=0,`currentwaypoint`=0,`MovementType`=2
WHERE `guid`=303666 AND `id`=54490 AND `map`=974;

DELETE FROM `creature_addon` WHERE `guid`=303666;
INSERT INTO `creature_addon`
(`guid`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`)
VALUES (303666,303666,0,0,0,0,0,0,0,0,0,0,0,0,'101696');

DELETE FROM `waypoint_data` WHERE `id`=303666;
INSERT INTO `waypoint_data`
(`id`,`point`,`position_x`,`position_y`,`position_z`,`orientation`,`delay`,`move_flag`,`action`,`action_chance`,`wpguid`) VALUES
(303666,1,-4302.40,6299.20,13.118,0,0,0,0,100,0),
(303666,2,-4305.80,6294.00,13.118,0,0,0,0,100,0),
(303666,3,-4303.00,6286.20,13.118,0,0,0,0,100,0),
(303666,4,-4295.00,6284.00,13.118,0,0,0,0,100,0),
(303666,5,-4290.50,6292.00,13.118,0,0,0,0,100,0),
(303666,6,-4293.60,6301.20,13.118,0,0,0,0,100,0);

-- -------------------------------------------------------- Humanoid Cannonball
DELETE FROM `game_event_creature` WHERE `guid`=4044100;
DELETE FROM `creature_addon` WHERE `guid`=4044100;
DELETE FROM `creature` WHERE `guid`=4044100;
INSERT INTO `creature`
(`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,
 `spawntimesecs`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`unit_flags`,`dynamicflags`)
VALUES
(4044100,54224,974,0,0,1,1,0,0,-4440.18,6211.33,0.224782,0,300,0,0,1,0,0,0,0,0);
INSERT INTO `game_event_creature` (`eventEntry`,`guid`) VALUES (75,4044100);

DELETE FROM `smart_scripts` WHERE `source_type`=0 AND `entryorguid`=15303;
UPDATE `creature_template`
SET `AIName`='',`ScriptName`='npc_darkmoon_maxima_blastenheimer',
    `gossip_menu_id`=6575,`npcflag`=`npcflag` | 3
WHERE `entry`=15303;
UPDATE `creature_template`
SET `AIName`='',`ScriptName`='npc_darkmoon_cannon_target'
WHERE `entry`=54224;

DELETE FROM `spell_script_names`
WHERE `ScriptName`='spell_darkmoon_cannon_preparation'
   OR (`spell_id`=102112 AND `ScriptName`<>'spell_darkmoon_cannon_preparation');
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (102112,'spell_darkmoon_cannon_preparation');
