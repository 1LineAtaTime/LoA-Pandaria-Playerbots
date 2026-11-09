-- Western Plaguelands, the Charred Outpost: Lindsay Ravensun waits there, and the burning Scarlet corpses lie still.
-- Strange New Faces 26979 sends players to "Lindsay Ravensun at the Charred Outpost", and it, When Death is Not Enough 26937 and Combat
-- Training 26938 (which she also starts) have their turn-in areas there (1851-1876, -1323..-1335), but mop's only Lindsay 44466 stood in
-- Andorhal, 450 yd away. And the
-- outpost's 14 Scarlet corpses (template auras 73304 and 29266 Permanent Feign Death) wandered 3 yd while playing dead.
-- Sourced:
--  * Lindsay: Wowhead MoP Classic's map pins for npc 44466 include two at 40.4/40.6, 52.0 in Western Plaguelands, which is (1876,
--    -1320..-1329) in world coordinates (WorldMapArea 22), among the corpses at the outpost; the other pins are her Andorhal spots.
--    Comments on 26979: "Turn in NPC is at Charred Outpost ... For me it is marked incorrect on map in Andorhal" (2025). Neither TDB spawns
--    her there.
--  * The corpses: both TDBs have the same 14 standing still (MovementType 0, wander 0); mop had MovementType 1, wander 3.
-- Ours: Lindsay's spot within the pins (1875.5, -1321.0, z from the terrain, facing the corpses), the new guid 3126000, 300 s and her health
-- and model from the Andorhal spawn. She is a second spawn of the same entry: the Andorhal Lindsay stays for War Machines and Lindsay
-- Ravensun, Revealed. Both are in phase 0 here; the Andorhal phase work decides who sees which.
DELETE FROM `creature` WHERE `guid`=3126000;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3126000,44466,0,28,5392,1,1,0,0,33917,0,1875.5,-1321.0,59.92,4.467,300,0,0,0,3048,0,0,0,0,0,0,0,'',0,0);
UPDATE `creature` SET `MovementType`=0, `wander_distance`=0 WHERE `guid` IN (212393,212398,212400,212406,212419,212424,212425,212428,212431,212433,212443,212444,212446,212447);
