-- The Quel'Danil battle: Highvale Swiftblades and Vindicators fight Deathstalker Invaders around Quel'Danil Lodge, seen once
-- Quel'Danil Lodge 27725 is turned in
-- Sourced: warcraft.wiki.gg (Quel'Danil Lodge): "Completing the quest phases in melee fights between Highvale Vindicators and
-- Swiftblades and Deathstalker Invaders"; Phase.dbc 294 "Quel'Danil Battle"; the spots are Wowhead's map pins (MoP Classic) of
-- Highvale Swiftblade 43539 (30), Highvale Vindicator 43540 (15) and Deathstalker Invader 43541 (38); the Highvale SmartAI is
-- both TDBs' (Expose Armor 60842, Crusader Strike 14517). No TDB spawns any of the three (unsniffed).
-- Ours: phase 294 granted zone-wide in The Hinterlands while 27725 is rewarded (In Defense of Quel'Danil 27625 needs 27725, so
-- its Invaders are always there for it); one spawn per pin, pins of one entry in neighbouring 0.2 % cells merged (72 spawns:
-- 32 Invaders, 29 Swiftblades, 11 Vindicators), a Highvale and an Invader on the same pin set 2.4 yd apart, each facing the
-- nearest enemy; z from the map data (terrain, or the lodge's floor indoors and on the lake deck); 60 s respawn as mop's
-- Invaders; the factions stay mop's (1802 vs 1801: they fight each other). mop's 29 Invaders (LoA's own ring around the lodge,
-- not on the pins) are replaced; guids 373622-373650 are reused and 3125400-3125442 are new.
-- Also ours: the Invaders get flags_extra 0x400000 (CREATURE_FLAG_EXTRA_NO_PLAYER_DAMAGE_REQ, as this core's NPC battles Forest
-- Ettin 44367 and Worgen Renegade 44793), so an Invader the Highvale wounded still gives the player loot and kill credit.
DELETE FROM `creature` WHERE `id`=43541;
DELETE FROM `creature` WHERE `guid` BETWEEN 3125400 AND 3125442;
DELETE FROM `creature` WHERE `id` IN (43539,43540);
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(373622,43541,0,47,350,1,1,294,0,0,0,305.334,-2668.400,114.778,0.0000,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 28.4 45.2
(373623,43541,0,47,350,1,1,294,0,0,0,295.067,-2668.400,116.777,0.0000,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 28.4 45.6
(373624,43541,0,47,350,1,1,294,0,0,0,274.534,-2676.100,119.584,0.0000,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 28.6 46.4
(373625,43541,0,47,350,1,1,294,0,0,0,224.400,-2706.900,119.677,4.7124,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 29.4 48.4
(373626,43541,0,47,350,1,1,294,0,0,0,193.600,-2706.900,114.689,4.1244,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 29.4 49.6
(373627,43541,0,47,350,1,1,294,0,0,0,147.400,-2714.600,114.823,0.0000,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 29.6 51.4
(373628,43541,0,47,350,1,1,294,0,0,0,352.734,-2730.000,118.383,4.7124,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 30.0 43.4
(373629,43541,0,47,350,1,1,294,0,0,0,178.200,-2745.400,115.531,1.2490,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 30.4 50.2
(373630,43541,0,47,350,1,1,294,0,0,0,203.867,-2753.100,121.091,0.9828,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 30.6 49.2
(373631,43541,0,47,350,1,1,294,0,0,0,244.934,-2768.500,123.316,3.1416,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 31.0 47.6
(373632,43541,0,47,350,1,1,294,0,0,0,347.600,-2776.200,117.206,4.7124,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 31.2 43.6
(373633,43541,0,47,350,1,1,294,0,0,0,357.867,-2783.900,120.069,3.1416,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 31.4 43.2
(373634,43541,0,47,350,1,1,294,0,0,0,250.067,-2783.900,123.421,5.1846,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 31.4 47.4
(373635,43541,0,47,350,1,1,294,0,0,0,147.400,-2791.600,112.039,4.7124,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 31.6 51.4
(373636,43541,0,47,350,1,1,294,0,0,0,275.734,-2799.300,120.990,5.3004,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 31.8 46.4
(373637,43541,0,47,350,1,1,294,0,0,0,260.334,-2799.300,123.421,2.0431,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 31.8 47.0
(373638,43541,0,47,350,1,1,294,0,0,0,224.400,-2799.300,121.535,1.8925,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 31.8 48.4
(373639,43541,0,47,350,1,1,294,0,0,0,193.600,-2799.300,117.426,4.7124,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 31.8 49.6
(373640,43541,0,47,350,1,1,294,0,0,0,202.667,-2807.000,117.601,0.0000,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 32.0 49.2
(373641,43541,0,47,350,1,1,294,0,0,0,142.267,-2807.000,107.231,5.3004,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 32.0 51.6
(373642,43541,0,47,350,1,1,294,0,0,0,357.867,-2822.400,122.082,2.1588,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 32.4 43.2
(373643,43541,0,47,350,1,1,294,0,0,0,347.600,-2822.400,120.363,2.1588,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 32.4 43.6
(373644,43541,0,47,350,1,1,294,0,0,0,352.734,-2837.800,119.167,1.5708,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 32.8 43.4
(373645,43541,0,47,350,1,1,294,0,0,0,316.800,-2845.500,115.423,3.1416,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 33.0 44.8
(373646,43541,0,47,350,1,1,294,0,0,0,286.000,-2845.500,114.453,0.0000,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 33.0 46.0
(373647,43541,0,47,350,1,1,294,0,0,0,183.333,-2845.500,105.241,5.5791,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 33.0 50.0
(373648,43541,0,47,350,1,1,294,0,0,0,265.467,-2853.200,111.901,4.1244,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 33.2 46.8
(373649,43541,0,47,350,1,1,294,0,0,0,228.934,-2861.939,110.288,1.5708,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 33.4 48.2
(373650,43541,0,47,350,1,1,294,0,0,0,223.200,-2876.300,109.112,0.0000,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 33.8 48.4
(3125400,43541,0,47,350,1,1,294,0,0,0,188.467,-2876.300,103.144,4.7124,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 33.8 49.8
(3125401,43541,0,47,350,1,1,294,0,0,0,357.867,-2884.000,118.232,1.5708,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 34.0 43.2
(3125402,43541,0,47,350,1,1,294,0,0,0,315.600,-2884.000,114.533,0.0000,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Deathstalker Invader, pin 34.0 44.8
(3125403,43539,0,47,350,1,1,294,0,0,0,307.734,-2668.400,114.260,3.1416,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 28.4 45.2
(3125404,43539,0,47,350,1,1,294,0,0,0,276.934,-2676.100,119.417,3.1416,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 28.6 46.4
(3125405,43539,0,47,350,1,1,294,0,0,0,265.467,-2676.100,119.590,0.0000,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 28.6 46.8
(3125406,43539,0,47,350,1,1,294,0,0,0,203.867,-2691.500,119.553,4.1244,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 29.0 49.2
(3125407,43539,0,47,350,1,1,294,0,0,0,229.534,-2699.200,117.907,4.1243,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 29.2 48.2
(3125408,43539,0,47,350,1,1,294,0,0,0,224.400,-2714.600,120.938,1.5708,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 29.6 48.4
(3125409,43539,0,47,350,1,1,294,0,0,0,188.467,-2714.600,114.956,0.9828,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 29.6 49.8
(3125410,43539,0,47,350,1,1,294,0,0,0,352.734,-2737.700,118.470,1.5708,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 30.2 43.4
(3125411,43539,0,47,350,1,1,294,0,0,0,214.134,-2737.700,123.421,4.1244,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 30.2 48.8
(3125412,43539,0,47,350,1,1,294,0,0,0,255.200,-2745.400,122.602,4.2942,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 30.4 47.2
(3125413,43539,0,47,350,1,1,294,0,0,0,234.667,-2768.500,123.419,0.0000,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 31.0 48.0
(3125414,43539,0,47,350,1,1,294,0,0,0,347.600,-2783.900,118.737,1.5708,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 31.4 43.6
(3125415,43539,0,47,350,1,1,294,0,0,0,219.267,-2783.900,123.405,5.0341,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 31.4 48.6
(3125416,43539,0,47,350,1,1,294,0,0,0,256.400,-2791.600,123.421,5.1847,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 31.6 47.2
(3125417,43539,0,47,350,1,1,294,0,0,0,203.867,-2791.600,120.649,3.7851,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 31.6 49.2
(3125418,43539,0,47,350,1,1,294,0,0,0,280.867,-2807.000,121.302,2.1588,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 32.0 46.2
(3125419,43539,0,47,350,1,1,294,0,0,0,193.600,-2807.000,116.589,1.5708,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 32.0 49.6
(3125420,43539,0,47,350,1,1,294,0,0,0,352.734,-2814.700,120.708,5.3004,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 32.2 43.4
(3125421,43539,0,47,350,1,1,294,0,0,0,342.467,-2814.700,120.947,5.3004,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 32.2 43.8
(3125422,43539,0,47,350,1,1,294,0,0,0,147.400,-2814.700,106.760,2.1588,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 32.2 51.4
(3125423,43539,0,47,350,1,1,294,0,0,0,378.400,-2853.200,119.792,2.6012,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 33.2 42.4
(3125424,43539,0,47,350,1,1,294,0,0,0,286.000,-2853.200,113.841,1.5708,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 33.2 46.0
(3125425,43539,0,47,350,1,1,294,0,0,0,194.800,-2853.200,105.613,2.5503,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 33.2 49.6
(3125426,43539,0,47,350,1,1,294,0,0,0,260.334,-2860.900,112.427,0.9828,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 33.4 47.0
(3125427,43539,0,47,350,1,1,294,0,0,0,230.734,-2860.900,110.388,3.6651,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 33.4 48.2
(3125428,43539,0,47,350,1,1,294,0,0,0,147.400,-2860.900,113.768,0.4049,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 33.4 51.4
(3125429,43539,0,47,350,1,1,294,0,0,0,357.867,-2876.300,118.010,4.7124,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 33.8 43.2
(3125430,43539,0,47,350,1,1,294,0,0,0,225.600,-2876.300,109.365,3.1416,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 33.8 48.4
(3125431,43539,0,47,350,1,1,294,0,0,0,318.000,-2884.000,114.454,3.1416,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Swiftblade, pin 34.0 44.8
(3125432,43540,0,47,350,1,1,294,0,0,0,297.467,-2668.400,116.205,3.1416,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Vindicator, pin 28.4 45.6
(3125433,43540,0,47,350,1,1,294,0,0,0,224.400,-2699.200,117.494,4.7124,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Vindicator, pin 29.2 48.4
(3125434,43540,0,47,350,1,1,294,0,0,0,265.467,-2745.400,122.590,3.9858,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Vindicator, pin 30.4 46.8
(3125435,43540,0,47,350,1,1,294,0,0,0,224.400,-2745.400,123.421,3.5004,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Vindicator, pin 30.4 48.4
(3125436,43540,0,47,350,1,1,294,0,0,0,254.000,-2791.600,123.421,2.0430,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Vindicator, pin 31.6 47.2
(3125437,43540,0,47,350,1,1,294,0,0,0,205.067,-2807.000,117.784,3.1416,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Vindicator, pin 32.0 49.2
(3125438,43540,0,47,350,1,1,294,0,0,0,234.667,-2822.400,114.364,1.9890,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Vindicator, pin 32.4 48.0
(3125439,43540,0,47,350,1,1,294,0,0,0,291.134,-2845.500,115.204,3.1416,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Vindicator, pin 33.0 45.8
(3125440,43540,0,47,350,1,1,294,0,0,0,192.400,-2853.200,104.905,2.4375,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Vindicator, pin 33.2 49.6
(3125441,43540,0,47,350,1,1,294,0,0,0,228.934,-2859.861,110.307,4.7124,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0), -- Highvale Vindicator, pin 33.4 48.2
(3125442,43540,0,47,350,1,1,294,0,0,0,188.467,-2884.000,104.214,1.5708,60,0,0,0,1,0,0,0,0,0,0,0,'',0,0); -- Highvale Vindicator, pin 34.0 49.8

DELETE FROM `phase_area` WHERE `AreaId`=47 AND `PhaseId`=294;
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(47,294,'The Hinterlands - phase 294 (Quel''Danil Battle) once Quel''Danil Lodge is rewarded');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup`=294 AND `SourceEntry`=47;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,294,47,0,0,47,0,27725,64,0,0,0,0,'','The Hinterlands - phase 294 (Quel''Danil Battle) once Quel''Danil Lodge is rewarded');

UPDATE `creature_template` SET `flags_extra`=4194304 WHERE `entry`=43541;
UPDATE `creature_template` SET `AIName`='SmartAI', `ScriptName`='' WHERE `entry` IN (43539,43540);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (43539,43540) AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(43539,0,0,0,0,0,100,0,3000,4500,18000,20000,0,11,60842,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Highvale Swiftblade - In Combat - Cast ''Expose Armor'''),
(43540,0,0,0,0,0,100,0,2000,4000,10000,10500,0,11,14517,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Highvale Vindicator - In Combat - Cast ''Crusader Strike''');
