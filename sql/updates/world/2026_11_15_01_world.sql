-- Eastern Plaguelands: the Wretched lying dead-posed at Quel'Lithien Lodge stand up when they fight.
--
-- Reported in game: at Quel'Lithien Lodge the Wretched shoot and run away lying down. 13 spawns of Wretched Woodsman 8563,
-- Wretched Ranger 8564 and Wretched Pathstrider 8565 have creature_addon StandState 7 (dead pose; 10 of them aura 96007 too).
-- Nothing in this core clears stand state 7 on combat: Unit::IsStandState counts 7 as standing, so Unit::CombatStart never
-- stands them up, and a creature that starts the fight itself has no stand-up at all.
--
-- Ours: the way this DB's other lying mobs get up (Infected Kodo Beast 25596 id 3, action 91 param1 7 param2 0: clear bits 7
--   of byte 0 of UNIT_FIELD_BYTES_1), on aggro, and again on evade, because SmartAI::EnterEvadeMode reloads the addon (stand
--   state 7) before it runs the evade events. They run home upright; HomeMovementGenerator reloads the addon on arrival, so
--   they lie down again where they spawned. On the entries' standing spawns both rows do nothing.
--   The Zzz aura 96007 on 10 of the 13 breaks on damage, movement or a melee swing only (AuraInterruptFlags 0x100B), so a
--   Ranger that shoots from where it lay would keep it: removed on aggro; the addon reload gives it back at home.
-- Kept: creature_addon as it is; the existing rows (8563 ids 0-1, 8564 ids 0-3, 8565 id 0).

UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry` IN (8563,8564,8565);

DELETE FROM `smart_scripts` WHERE `source_type`=0 AND ((`entryorguid`=8563 AND `id` IN (2,3,4)) OR (`entryorguid`=8564 AND `id` IN (4,5,6)) OR (`entryorguid`=8565 AND `id` IN (1,2,3)));
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(8563,0,2,0,4,0,100,0,0,0,0,0,0,91,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Wretched Woodsman - On Aggro - Remove Standstate Dead'),
(8563,0,3,0,7,0,100,0,0,0,0,0,0,91,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Wretched Woodsman - On Evade - Remove Standstate Dead'),
(8564,0,4,0,4,0,100,0,0,0,0,0,0,91,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Wretched Ranger - On Aggro - Remove Standstate Dead'),
(8564,0,5,0,7,0,100,0,0,0,0,0,0,91,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Wretched Ranger - On Evade - Remove Standstate Dead'),
(8565,0,1,0,4,0,100,0,0,0,0,0,0,91,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Wretched Pathstrider - On Aggro - Remove Standstate Dead'),
(8565,0,2,0,7,0,100,0,0,0,0,0,0,91,7,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Wretched Pathstrider - On Evade - Remove Standstate Dead'),
(8563,0,4,0,4,0,100,0,0,0,0,0,0,28,96007,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Wretched Woodsman - On Aggro - Remove Aura ''Cosmetic - Sleep Zzz Breakable'''),
(8564,0,6,0,4,0,100,0,0,0,0,0,0,28,96007,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Wretched Ranger - On Aggro - Remove Aura ''Cosmetic - Sleep Zzz Breakable'''),
(8565,0,3,0,4,0,100,0,0,0,0,0,0,28,96007,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Wretched Pathstrider - On Aggro - Remove Aura ''Cosmetic - Sleep Zzz Breakable''');
