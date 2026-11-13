-- Ongo'longo 42815: his two shackles ride him as vehicle passengers, and he throws them off when he breaks free
-- Sourced: Ongo'longo's VehicleId 924 (both TDBs and mop), whose two seats sit on his elbows (VehicleSeat 8134 attachment 4,
-- left elbow; 8135 attachment 3, right elbow; 5.4.8 DBC); TCPP's sniffed template addons give both shackles 46598 Ride Vehicle
-- Hardcoded, and no TDB spawns the shackles (passengers are not sniffed as spawns); mop's static shackle spawns (192304/192305)
-- stand exactly on his right and left elbow; Wowhead: "the right/left shackle holding Ongo'Longo ... It cannot be targeted".
-- Ours: which shackle takes which seat (the right shackle stands on his right in the old spawns), the accessory rows (minion,
-- summoned until removed), the spellclick that seats them limited to the two shackles (the Twinkles pattern), and throwing
-- them off when he breaks free (they go 2 s later with him).
DELETE FROM `creature` WHERE `guid` IN (192304,192305) AND `id` IN (42816,42817);

DELETE FROM `vehicle_template_accessory` WHERE `entry`=42815;
INSERT INTO `vehicle_template_accessory` (`entry`,`accessory_entry`,`seat_id`,`minion`,`description`,`summontype`,`summontimer`) VALUES
(42815,42817,0,1,'Ongo''longo - Ongolongo''s Left Shackle (left elbow)',8,0),
(42815,42816,1,1,'Ongo''longo - Ongolongo''s Right Shackle (right elbow)',8,0);

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=42815;
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`cast_flags`,`user_type`) VALUES
(42815,46598,1,0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=42815;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(18,42815,46598,0,0,31,0,3,42816,0,0,0,0,'','Ongo''longo: only his right shackle may ride him'),
(18,42815,46598,0,1,31,0,3,42817,0,0,0,0,'','Ongo''longo: only his left shackle may ride him');

-- His SmartAI as 2026_11_09_61, plus id 12: on the break the shackles' ride auras go, so they drop off him
DELETE FROM `smart_scripts` WHERE `entryorguid`=42815 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42815,0,0,1,25,0,100,0,0,0,0,0,0,42,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - On Reset - Set Invincibility 1 HP'),
(42815,0,1,2,61,0,100,0,0,0,0,0,0,8,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Set React Aggressive'),
(42815,0,2,0,61,0,100,0,0,0,0,0,0,19,256,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Remove Flag Immune To Players'),
(42815,0,3,0,4,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - On Aggro - Say Line 0'),
(42815,0,4,5,2,0,100,1,0,1,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Between 0-1% Health - Say Line 1 (No Repeat)'),
(42815,0,5,6,61,0,100,0,0,0,0,0,0,11,79875,2,0,0,0,0,2,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Cast ''Ongolongo Smash!'' on Victim'),
(42815,0,6,7,61,0,100,0,0,0,0,0,0,33,42815,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Quest Credit ''Ongo''longo Enraged'' (loot recipient and group)'),
(42815,0,7,8,61,0,100,0,0,0,0,0,0,33,42815,0,0,0,0,0,24,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Quest Credit ''Ongo''longo Enraged'' (threat list)'),
(42815,0,8,9,61,0,100,0,0,0,0,0,0,8,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Set React Passive'),
(42815,0,9,10,61,0,100,0,0,0,0,0,0,203,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Combat Stop'),
(42815,0,10,11,61,0,100,0,0,0,0,0,0,18,256,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Set Flag Immune To Players'),
(42815,0,11,12,61,0,100,0,0,0,0,0,0,41,2000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Despawn In 2 s'),
(42815,0,12,0,61,0,100,0,0,0,0,0,0,28,46598,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Ongo''longo - Linked - Remove ''Ride Vehicle Hardcoded'' (his shackles drop off)');
