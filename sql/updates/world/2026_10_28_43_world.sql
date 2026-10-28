-- Hillsbrad Foothills: Tarren Mill, Ravenholdt, Durnholde and Brazie's farm spawns, A Fighting Chance, graveyard.

-- Tarren Mill has no guards: the 15 Dreadguards (47214) of the Cataclysm sniff (TDB 4.4.2, also TCPP) are missing.
-- Wowhead MoP Classic shows 10 Dreadguard pins in Tarren Mill. Guids 3120000-3120014 (hub C range).
-- areaId 272 is written for all; the core recomputes zone/area on load.
DELETE FROM `creature` WHERE `guid` BETWEEN 3120000 AND 3120014;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3120000,47214,0,267,272,1,1,0,0,0,0,2.17361,-1027.32,56.1809,0.0488509,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120001,47214,0,267,272,1,1,0,0,0,0,2.02712,-1024.32,57.0252,0.0487124,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120002,47214,0,267,272,1,1,0,0,0,0,19.4896,-949.172,56.0637,3.45575,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120003,47214,0,267,272,1,1,0,0,0,0,-46.7257,-950.429,54.364,1.16937,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120004,47214,0,267,272,1,1,0,0,0,0,-63.8003,-925.245,56.3965,2.05949,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120005,47214,0,267,272,1,1,0,0,0,0,-41.4861,-904.986,56.0453,0.15708,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120006,47214,0,267,272,1,1,0,0,0,0,-105.861,-831.22,57.4444,3.29266,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120007,47214,0,267,272,1,1,0,0,0,0,-106.244,-828.299,57.5589,3.2758,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120008,47214,0,267,272,1,1,0,0,0,0,-16.0139,-870.842,59.3509,5.13127,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120009,47214,0,267,272,1,1,0,0,0,0,-6.76389,-871.153,59.0241,4.11898,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120010,47214,0,267,272,1,1,0,0,0,0,10.6953,-835.906,58.514,0.0761733,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120011,47214,0,267,272,1,1,0,0,0,0,10.467,-832.915,58.0894,0.0765198,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120012,47214,0,267,272,1,1,0,0,0,0,10.3264,-909.17,54.3651,2.35619,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120013,47214,0,267,272,1,1,0,0,0,0,-47.3142,-828.734,56.8786,4.23464,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120014,47214,0,267,272,1,1,0,0,0,0,-49.1689,-826.376,56.7536,4.18456,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0);
-- sources: 4.4.2 guid 322901, 4.4.2 guid 322902, 4.4.2 guid 322913, 4.4.2 guid 322916, 4.4.2 guid 322931, 4.4.2 guid 322933, 4.4.2 guid 323074, 4.4.2 guid 323075, 4.4.2 guid 323077, 4.4.2 guid 323080, 4.4.2 guid 323082, 4.4.2 guid 323083, 4.4.2 guid 323084, 4.4.2 guid 323103, 4.4.2 guid 323104

-- Ravenholdt Manor still has the pre-Cataclysm Ravenholdt Guards (6766, 9 spawns). Neither TDB 4.4.2 nor TCPP has them there,
-- and Wowhead MoP Classic shows only 3 elsewhere (not at the manor). The sniff has Ravenholdt Sentries (55783) on the same
-- spots instead: mop has 2 of its 8 sentries and 2 of its 4 lookouts (55808). Remove the guards, add the missing six
-- sentries and two lookouts (guids 3120020-3120027). Quest 6701 Syndicate Emblems (removed in 4.0, no starter in either TDB)
-- loses its stale starter row; its ender row stays as in the TDBs.
DELETE FROM `creature_addon` WHERE `guid` IN (175873,175875,175878,176072,176073,176075,176076,176079,176179);
DELETE FROM `creature` WHERE `guid` IN (175873,175875,175878,176072,176073,176075,176076,176079,176179); -- all nine are Ravenholdt Guard 6766 at the manor
DELETE FROM `creature_queststarter` WHERE `id`=6766 AND `quest`=6701;
DELETE FROM `creature` WHERE `guid` BETWEEN 3120020 AND 3120027;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3120020,55783,0,267,3486,1,1,0,0,0,0,22.0955,-1613.42,195.503,0.593412,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120021,55783,0,267,3486,1,1,0,0,0,0,14.3194,-1604.86,195.503,0.890118,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120022,55783,0,267,3486,1,1,0,0,0,0,-31.7153,-1577.2,194.537,0.0698132,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120023,55783,0,267,3486,1,1,0,0,0,0,-29.1094,-1577.43,194.537,3.22886,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120024,55783,0,267,3486,1,1,0,0,0,0,28.9496,-1586.33,194.113,1.62316,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120025,55783,0,267,3486,1,1,0,0,0,0,19.1806,-1585.14,194.587,1.44862,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120026,55808,0,267,3486,1,1,0,0,0,0,12.0156,-1567.98,190.93,5.387,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0),
(3120027,55808,0,267,3486,1,1,0,0,0,0,61.6874,-1516.69,173.949,4.89556,120,0,0,0,0,0,0,0,0,0,0,0,'',0,0);
-- sources: 4.4.2 guid 322585, 4.4.2 guid 322586, 4.4.2 guid 322589, 4.4.2 guid 322594, 4.4.2 guid 322608, 4.4.2 guid 322609, 4.4.2 guid 322598, 4.4.2 guid 322613

-- The Durnholde Challenge: D-1000 (28647). mop has a second elite "D-1000" (51343, model 25113, no AI, no loot, no kill credit)
-- standing 8 yd from the real one (49268, guid 177183). Neither TDB 4.4.2, TCPP nor Wowhead MoP Classic (no map pins) has it:
-- it only pulls with the quest target and gives nothing. Remove it.
DELETE FROM `creature_addon` WHERE `guid`=177173;
DELETE FROM `creature` WHERE `guid`=177173 AND `id`=51343;

-- Brazie's Farmstead: a static Warden Stillwater (49819, the Lawn of the Dead minigame boss, hostile zombie faction 2102)
-- stands at the far end of Brazie's lanes (-37.6, 220.4). The boss is a personal summon of the minigame; no sniff
-- (TDB 4.4.2, TCPP, Wowhead MoP Classic) has a static spawn. Remove it (sniffed temp-summon artifact).
DELETE FROM `creature_addon` WHERE `guid`=372479;
DELETE FROM `creature` WHERE `guid`=372479 AND `id`=49819;

-- A Fighting Chance (28495) works: the wand (90679) gives the credit through the Hill Fawn's SmartAI. warcraft.wiki.gg: "Using
-- the wand on a fawn once 'saves' it but enrages it. Using it on the same fawn a second time makes it explode." mop blocked the
-- second use instead (condition 101: SmartAI phase). Phase 1 = unblessed (set on Reset, which also runs at spawn and after an
-- evade, after the core's SetPhase(0)), phase 2 = blessed. The phase-2 handler has the lower id, because events run in id order
-- and the phase-1 handler switches to phase 2 on the same hit. Only the first blessing gives credit.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceGroup`=1 AND `SourceEntry`=90679 AND `ConditionTypeOrReference`=101;
DELETE FROM `smart_scripts` WHERE `entryorguid`=48187 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(48187,0,0,0,11,0,100,0,0,0,0,0,0,2,1999,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - On Respawn - Set Faction 1999'),
(48187,0,2,0,25,0,100,0,0,0,0,0,0,22,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - On Reset - Set Phase 1 (unblessed)'),
(48187,0,3,4,8,2,100,0,90679,0,0,0,0,11,90686,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - On Spellhit Duskingdawn''s Blessing (Phase 2, blessed) - Cast Fawnplode'),
(48187,0,4,0,61,0,100,0,0,0,0,0,0,41,500,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Despawn after 0.5 sec'),
(48187,0,5,6,8,1,100,0,90679,0,0,0,0,22,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - On Spellhit Duskingdawn''s Blessing (Phase 1) - Set Phase 2'),
(48187,0,6,7,61,0,100,0,0,0,0,0,0,33,48684,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Quest Credit ''A Fighting Chance'' to Caster'),
(48187,0,7,8,61,0,100,0,0,0,0,0,0,11,90685,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Cast Duskingdawn''s Blessing'),
(48187,0,8,9,61,0,100,0,0,0,0,0,0,2,14,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Set Faction 14'),
(48187,0,9,10,61,0,100,0,0,0,0,0,0,8,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Set React State Defensive'),
(48187,0,10,0,61,0,100,0,0,0,0,0,0,41,20000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - Linked - Despawn after 20 sec'),
(48187,0,11,0,0,0,100,0,5000,8000,12000,15000,0,11,32019,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Hill Fawn - In Combat - Cast Gore'),
(48187,0,12,0,6,0,100,1,0,0,0,0,0,11,90686,3,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hill Fawn - On Death - Cast Fawnplode (No Repeat)');

-- Hillsbrad Foothills (267) lost its northern graveyard: "Alterac Mountains - Central GY" (WorldSafeLocs 1411, 801/-430) is
-- linked to zone 267 for both factions in TDB 4.4.2 and TCPP (faction 0), but not in mop. Without it, a death in Strahnbrad,
-- the Uplands or Ravenholdt sends Alliance players to Southshore/Hillsbrad Fields and Horde players to Tarren Mill.
DELETE FROM `game_graveyard_zone` WHERE `id`=1411 AND `ghost_zone`=267;
INSERT INTO `game_graveyard_zone` (`id`,`ghost_zone`,`faction`) VALUES (1411,267,0);
