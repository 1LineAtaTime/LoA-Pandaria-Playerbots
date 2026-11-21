-- The Endless Flow (26922/27161): the Scourge Bone Animus 44360 raises the Stickbone Berserkers 44329 instead of 257 statics.
-- Sourced: neither TDB (4.4.2, 12.1.0) spawns a static Stickbone Berserker; the 5.4.8 DBC has 82875 Summon Stickbone Berserker:
-- Missile (random spot within 10 yd, triggers 82861) and 82861 Summon Stickbone Berserker (44329, SummonProperties 64, 40 s),
-- and 83109 The Endless Flow: Kill Credit (credit 44360 within 30 yd, instakill within 35 yd). Wowhead comments on 26922/27161:
-- "they respawn immediately after you kill them", "about 1 every second until there are 15 or 20 there", "Any skellies hitting
-- you when you do that die with it", "The respawn on the bone pile is about 15 seconds". The piles stand still (TDB MovementType 0).
-- Ours: the cap of 15, the 40 yd player range, the 40 s timer counting only out of combat, despawning idle berserkers with
-- their pile (npc_wpl_scourge_bone_animus), and the 83109 conditions (players for the credit, berserkers for the kill).
-- mop's 257 statics (phase 259, all within 40 yd of a pile, 69 on paths) were those summons caught wave after wave by a sniff.

UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_wpl_scourge_bone_animus' WHERE `entry`=44360;
DELETE FROM `smart_scripts` WHERE `entryorguid`=44360 AND `source_type`=0;
UPDATE `creature` SET `spawntimesecs`=15, `MovementType`=0, `wander_distance`=0 WHERE `guid` IN (213073,213137) AND `id`=44360;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=83109;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,83109,0,0,32,0,16,0,0,0,0,0,'','The Endless Flow: Kill Credit - credit only players'),
(13,2,83109,0,0,31,0,3,44329,0,0,0,0,'','The Endless Flow: Kill Credit - kill only Stickbone Berserkers');

-- The static berserkers, their addon rows and their 69 paths (used by no other creature)
DELETE `w` FROM `waypoint_data` `w` JOIN `creature_addon` `a` ON `a`.`path_id`=`w`.`id` JOIN `creature` `c` ON `c`.`guid`=`a`.`guid`
WHERE `c`.`id`=44329 AND `c`.`guid` BETWEEN 213090 AND 213743;
DELETE `a` FROM `creature_addon` `a` JOIN `creature` `c` ON `c`.`guid`=`a`.`guid` WHERE `c`.`id`=44329 AND `c`.`guid` BETWEEN 213090 AND 213743;
DELETE FROM `creature` WHERE `id`=44329 AND `guid` BETWEEN 213090 AND 213743;
