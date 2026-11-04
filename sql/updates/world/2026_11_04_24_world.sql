-- Hillsbrad Foothills, the Sludge Fields: A Blight Upon the Land (28237) could only be handed in to the caged Master Apothecary
-- Lydon 47900 at the farm, about 190 yd from Warden Stillwater's spot by the lab where the head is taken.
--
-- Retail: 47900 is the quest's only ender in TDB 442, TDB 1210 and Wowhead (Quick Facts: start 48020, end 47900), and no 5.4.8
-- spell summons 47900 (only 48020/48021 have summon spells). Wowhead (MoP Classic) has three map pins for 47900: the cage at
-- 40.2,59.4 and two at the lab, 36.4,61.2 and 36.6,61.2 (WorldMapArea 267: -502.6,80.0 and -502.6,70.3; Cataclysm Classic
-- has the cage and 36.6,61.2), and a comment on 47900: "This is the one that is in the cage. The other follows you around and
-- does the fighting. Then this one reappears again after Stillwater is killed." The quest's turn-in POI (objective -1) is at
-- -494,88 in mop (quest_poi idx 1) and -493,87 in retail TDB 1210, next to the head's (-502,79); 28209's is at the cage
-- (-441,-112). Neither TDB spawns 47900 anywhere (312121 comes from the original world dump), so the lab spawn and the phase
-- assignments below are inferred:
--  * a second 47900 at the MoP Classic pin on Stillwater's spot, in phase 333 "The Sludge Fields Reclaimed" (Phase.dbc, group
--    540 with 332 "A Blight Upon the Land"), seen once the head is taken (28237 complete or rewarded): he takes the head and
--    starts Trouble at Southshore (28251) there. The companion Lydon 48020 leaves on the controller's next check (C++, every
--    5 s), so for up to 5 s after the head is looted both Lydons can be seen;
--  * phase 332 (Stillwater 48080, Arnauld and his Sludge Guards) only while the head is not yet taken (was: in the log), so
--    the respawned, friendly Stillwater does not stand beside the Lydon who just sentenced him;
--  * the caged 312121 in phase 196 "Quest Zone-Specific 21" (generic; unused by any spawn, and nothing else in zone 267 uses
--    it; five 5.4.8 spells grant it in other zones: 72446, 78603, 80913, 79041, 81377; the assignment is ours) until Freedom
--    for Lydon is rewarded and Protocol (28230) is taken: he ends 28209 ("I'll let myself out, thanks") and is, with the
--    companion, a starter of Protocol in both TDBs and on Wowhead, and afterwards he goes with the player, not stays in the
--    cage.

-- The lab Lydon (orientation as Stillwater 48080's)
DELETE FROM `creature` WHERE `guid`=3110900;
INSERT INTO `creature` (`guid`,`id`,`map`,`zoneId`,`areaId`,`spawnMask`,`phaseMask`,`phaseId`,`phaseGroup`,`modelid`,`equipment_id`,`position_x`,`position_y`,`position_z`,`orientation`,`spawntimesecs`,`spawntimesecs_max`,`wander_distance`,`currentwaypoint`,`curhealth`,`curmana`,`MovementType`,`npcflag`,`npcflag2`,`unit_flags`,`unit_flags2`,`dynamicflags`,`ScriptName`,`walk_mode`,`VerifiedBuild`) VALUES
(3110900,47900,0,267,286,1,1,333,0,0,0,-502.65,80.05,55.66,4.71239,300,0,0,0,2988,8175,0,0,0,0,0,0,'',0,0);

UPDATE `creature` SET `phaseId`=196, `phaseGroup`=0 WHERE `guid`=312121 AND `id`=47900;

DELETE FROM `phase_area` WHERE `AreaId`=267 AND `PhaseId` IN (196,333);
INSERT INTO `phase_area` (`AreaId`,`PhaseId`,`Comment`) VALUES
(267,196,'Hillsbrad Foothills - Lydon 47900 in his cage, until he is freed and Protocol is taken'),
(267,333,'Hillsbrad Foothills - The Sludge Fields Reclaimed: Lydon 47900 at the lab once Stillwater''s head is taken');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceEntry`=267 AND `SourceGroup` IN (196,332,333);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(26,196,267,0,0,47,0,28209,43,0,0,0,0,'','Hillsbrad Foothills - phase 196 until Freedom for Lydon is rewarded'),
(26,196,267,0,1,47,0,28230,1,0,0,0,0,'','Hillsbrad Foothills - phase 196 while Protocol is not taken'),
(26,332,267,0,0,47,0,28237,8,0,0,0,0,'','Hillsbrad Foothills - phase 332 while A Blight Upon the Land is in progress'),
(26,333,267,0,0,47,0,28237,66,0,0,0,0,'','Hillsbrad Foothills - phase 333 once A Blight Upon the Land is complete or rewarded');
