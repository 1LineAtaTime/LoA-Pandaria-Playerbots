-- Kotonga 42624 sends Horde players up to the Totem of Akil'darah 204002 in Jintha'Alor, and the totem sends them back
-- Sourced: the option texts (broadcast 42500 "Send me to Akil'darah at the middle terrace." and 42498 "Akil'darah, return me to
-- my allies."), Kotonga's menu 11649 and the totem's menu 11648 (both TDBs, no options there), spells 79486/79496/83482 "Wings
-- of Akil'darah" (teleport units to a DB destination, no spell_target_position in mop, TDB or TCPP); Wowhead comments: Kotonga
-- "will port you to the top, to the Totem of Akil'Darah @ 58, 67", "the statue you arrive in front of", "The Totem will port
-- you back to Kotonga", "back to the quest NPC's at the bottom", and "When you accept [It's Ours Now 26308], Kotonga ... will
-- offer to port you up" (Kotonga's line 44805: "We've retaken the Idol of Akil'darah thanks to you"); the comments on Summit of
-- Fate 26363, Ongo'longo's Revenge 26367, Venomous Secrets 26368 and Hunt the Keeper 26369 send players up with the port too.
-- Ours: the arrival spots (4 yd in front of the totem, facing the way it faces, out of its room; 4 yd in front of Kotonga,
-- facing him; both on the ground, checked against the map data), which spell goes which way (79486 up, 79496 down; 83482
-- stays unused, all three read "Returns you to camp."), the gate (any of 26308, 26363, 26367, 26368, 26369 taken, complete or
-- rewarded) on both options and on the SmartAI gossip-select events (source 22, so a crafted packet cannot skip it), and closing
-- the window.
DELETE FROM `spell_target_position` WHERE `id` IN (79486,79496);
INSERT INTO `spell_target_position` (`id`,`effIndex`,`target_map`,`target_position_x`,`target_position_y`,`target_position_z`,`target_orientation`) VALUES
(79486,0,0,-261.94,-3827.28,238.45,3.0892),
(79496,0,0,-236.77,-4181.46,121.71,5.2884);

DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (11648,11649) AND `OptionID`=0;
INSERT INTO `gossip_menu_option` (`MenuID`,`OptionID`,`OptionIcon`,`OptionText`,`OptionBroadcastTextID`,`OptionType`,`OptionNpcflag`,`ActionMenuID`,`ActionPoiID`,`BoxCoded`,`BoxMoney`,`BoxText`,`BoxBroadcastTextID`,`VerifiedBuild`) VALUES
(11649,0,0,'Send me to Akil''darah at the middle terrace.',42500,1,1,0,0,0,0,NULL,0,0),
(11648,0,0,'Akil''darah, return me to my allies.',42498,1,1,0,0,0,0,NULL,0,0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup` IN (11648,11649) AND `SourceEntry`=0;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceGroup`=1 AND ((`SourceEntry`=42624 AND `SourceId`=0) OR (`SourceEntry`=204002 AND `SourceId`=1));
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(15,11649,0,0,0,47,0,26308,74,0,0,0,0,'','Kotonga option: teleport once It''s Ours Now is taken, complete or rewarded'),
(15,11649,0,0,1,47,0,26363,74,0,0,0,0,'','Kotonga option: teleport once Summit of Fate is taken, complete or rewarded'),
(15,11649,0,0,2,47,0,26367,74,0,0,0,0,'','Kotonga option: teleport once Ongo''longo''s Revenge is taken, complete or rewarded'),
(15,11649,0,0,3,47,0,26368,74,0,0,0,0,'','Kotonga option: teleport once Venomous Secrets is taken, complete or rewarded'),
(15,11649,0,0,4,47,0,26369,74,0,0,0,0,'','Kotonga option: teleport once Hunt the Keeper is taken, complete or rewarded'),
(15,11648,0,0,0,47,0,26308,74,0,0,0,0,'','Totem of Akil''darah option: teleport once It''s Ours Now is taken, complete or rewarded'),
(15,11648,0,0,1,47,0,26363,74,0,0,0,0,'','Totem of Akil''darah option: teleport once Summit of Fate is taken, complete or rewarded'),
(15,11648,0,0,2,47,0,26367,74,0,0,0,0,'','Totem of Akil''darah option: teleport once Ongo''longo''s Revenge is taken, complete or rewarded'),
(15,11648,0,0,3,47,0,26368,74,0,0,0,0,'','Totem of Akil''darah option: teleport once Venomous Secrets is taken, complete or rewarded'),
(15,11648,0,0,4,47,0,26369,74,0,0,0,0,'','Totem of Akil''darah option: teleport once Hunt the Keeper is taken, complete or rewarded'),
(22,1,42624,0,0,47,0,26308,74,0,0,0,0,'','Kotonga SmartAI gossip select: teleport once It''s Ours Now is taken, complete or rewarded'),
(22,1,42624,0,1,47,0,26363,74,0,0,0,0,'','Kotonga SmartAI gossip select: teleport once Summit of Fate is taken, complete or rewarded'),
(22,1,42624,0,2,47,0,26367,74,0,0,0,0,'','Kotonga SmartAI gossip select: teleport once Ongo''longo''s Revenge is taken, complete or rewarded'),
(22,1,42624,0,3,47,0,26368,74,0,0,0,0,'','Kotonga SmartAI gossip select: teleport once Venomous Secrets is taken, complete or rewarded'),
(22,1,42624,0,4,47,0,26369,74,0,0,0,0,'','Kotonga SmartAI gossip select: teleport once Hunt the Keeper is taken, complete or rewarded'),
(22,1,204002,1,0,47,0,26308,74,0,0,0,0,'','Totem of Akil''darah SmartAI gossip select: teleport once It''s Ours Now is taken, complete or rewarded'),
(22,1,204002,1,1,47,0,26363,74,0,0,0,0,'','Totem of Akil''darah SmartAI gossip select: teleport once Summit of Fate is taken, complete or rewarded'),
(22,1,204002,1,2,47,0,26367,74,0,0,0,0,'','Totem of Akil''darah SmartAI gossip select: teleport once Ongo''longo''s Revenge is taken, complete or rewarded'),
(22,1,204002,1,3,47,0,26368,74,0,0,0,0,'','Totem of Akil''darah SmartAI gossip select: teleport once Venomous Secrets is taken, complete or rewarded'),
(22,1,204002,1,4,47,0,26369,74,0,0,0,0,'','Totem of Akil''darah SmartAI gossip select: teleport once Hunt the Keeper is taken, complete or rewarded');

UPDATE `creature_template` SET `AIName`='SmartAI', `ScriptName`='' WHERE `entry`=42624;
UPDATE `gameobject_template` SET `AIName`='SmartGameObjectAI', `ScriptName`='' WHERE `entry`=204002;

DELETE FROM `smart_scripts` WHERE `entryorguid`=42624 AND `source_type`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=204002 AND `source_type`=1;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(42624,0,0,1,62,0,100,0,11649,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Kotonga - On Gossip Option 0 Selected - Close Gossip'),
(42624,0,1,0,61,0,100,0,0,0,0,0,0,85,79486,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Kotonga - Linked - Invoker Cast ''Wings of Akil''darah'' (to the totem)'),
(204002,1,0,1,62,0,100,0,11648,0,0,0,0,72,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Totem of Akil''darah - On Gossip Option 0 Selected - Close Gossip'),
(204002,1,1,0,61,0,100,0,0,0,0,0,0,85,79496,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Totem of Akil''darah - Linked - Invoker Cast ''Wings of Akil''darah'' (back to Kotonga)');
