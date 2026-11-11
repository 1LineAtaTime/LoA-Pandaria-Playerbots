-- Shadra the Venom Queen (26419 Horde, 26532 Alliance): Shadra comes out of the lake when the quest is accepted, with helpers
-- (npc_hinterlands_shadra, npc_hinterlands_shadra_lookout, quest_hinterlands_shadra_the_venom_queen in zone_hinterlands.cpp)
-- Sourced: neither TDB spawns Shadra 43007, Darkcleric Marnal 43018 or the Deathstalker Bodychoppers 43016; Wowhead comments have
-- them come when the quest is accepted. Faction, unit flags and SmartAI of the Wildhammer Spirithunter 43213 and Gryphon Master
-- Talonaxe 43215 (the Alliance helpers), PvP flags and sheath state of all four helpers: TDB 442 and 1210. Spell 82121 Shadra Taunt
-- Aura (mop's template aura of Marnal) pulses 82122 Taunt Pulse at the nearby entry the conditions name: Shadra (5.4.8 DBC).
-- Ours: the scripts (see the C++), moving Marnal's aura into the script, and the deletion of the static copies below.

-- The static event: Shadra on a looping walk out of the lake and back, Marnal and five Bodychoppers by the Deathstalker Lookout.
-- The emergence walk (path 312702) is now in the script.
DELETE FROM `creature` WHERE `guid` BETWEEN 312696 AND 312702 AND `id` IN (43007,43016,43018);
DELETE FROM `creature_addon` WHERE `guid` BETWEEN 312696 AND 312702;
DELETE FROM `waypoint_data` WHERE `id`=312702;

UPDATE `creature_template` SET `ScriptName`='npc_hinterlands_shadra' WHERE `entry`=43007;
UPDATE `creature_template` SET `ScriptName`='npc_hinterlands_shadra_lookout' WHERE `entry` IN (43298,43299);
UPDATE `quest_template_addon` SET `ScriptName`='quest_hinterlands_shadra_the_venom_queen' WHERE `ID` IN (26419,26532);

-- Alliance helpers, as the TDBs
UPDATE `creature_template` SET `faction`=1054, `unit_flags`=256, `AIName`='SmartAI', `ScriptName`='' WHERE `entry`=43213;
UPDATE `creature_template` SET `faction`=2301, `unit_flags`=256 WHERE `entry`=43215;

DELETE FROM `smart_scripts` WHERE `entryorguid`=43213 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(43213,0,0,0,0,0,100,0,2000,4500,12000,12500,0,11,11977,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Wildhammer Spirithunter - In Combat - Cast ''Rend'''),
(43213,0,1,0,0,0,100,0,8000,9000,120000,125000,0,11,32064,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Wildhammer Spirithunter - In Combat - Cast ''Battle Shout''');

UPDATE `creature_template_addon` SET `SheathState`=1, `PvPFlags`=1 WHERE `entry` IN (43016,43018);
-- Marnal's Shadra Taunt Aura comes from the script once the helpers charge (as a template aura it pulled Shadra off the lake bed)
UPDATE `creature_template_addon` SET `auras`=NULL WHERE `entry`=43018;
DELETE FROM `creature_template_addon` WHERE `entry` IN (43213,43215);
INSERT INTO `creature_template_addon` (`entry`,`path_id`,`mount`,`MountCreatureID`,`StandState`,`AnimTier`,`VisFlags`,`SheathState`,`PvPFlags`,`emote`,`aiAnimKit`,`movementAnimKit`,`meleeAnimKit`,`visibilityDistanceType`,`auras`) VALUES
(43213,0,0,0,0,0,0,1,1,0,0,0,0,0,NULL),
(43215,0,0,0,0,0,0,1,1,0,0,0,0,0,NULL);

-- Taunt Pulse hits Shadra only
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceGroup`=1 AND `SourceEntry`=82122;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(13,1,82122,0,0,31,0,3,43007,0,0,0,0,'','Taunt Pulse - target Shadra');
