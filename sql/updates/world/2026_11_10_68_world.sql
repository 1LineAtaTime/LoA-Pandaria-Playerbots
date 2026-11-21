-- Western Plaguelands, Gahrron's Withering: Cauldron Lord Soulwrath fights with his abilities and his aggro yell.
-- Soulwrath 11078 (Gahrron's Withering Cauldron 27053 drops its key from him) had no AI in mop.
-- Sourced: TDB 4.4.2's SmartAI for 11078: Unbound Darkness 80132 on the victim (in combat, first after 5-8 s, then every 12-15 s),
-- Shadowflame 85800 on the victim (10-12 s, then 10-25 s), and on aggro text group 0 to the attacker (broadcast 6497 "$C - I will consume
-- your light!", yell), with its creature_text row. Both spells are in the 5.4.8 Spell.dbc; broadcast 6497 is in mop. No spawn has its own
-- script. TDB columns this schema lacks are dropped (SoundPlayType).
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=11078;
DELETE FROM `smart_scripts` WHERE `entryorguid`=11078 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(11078,0,0,0,0,0,100,0,5000,8000,12000,15000,0,11,80132,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Cauldron Lord Soulwrath - In Combat - Cast ''Unbound Darkness'''),
(11078,0,1,0,0,0,100,0,10000,12000,10000,25000,0,11,85800,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Cauldron Lord Soulwrath - In Combat - Cast ''Shadowflame'''),
(11078,0,2,0,4,0,100,0,0,0,0,0,0,1,0,0,1,0,0,0,7,0,0,0,0,0,0,0,0,'Cauldron Lord Soulwrath - On Aggro - Say Line 0');
DELETE FROM `creature_text` WHERE `CreatureID`=11078;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(11078,0,0,'$C - I will consume your light!',14,0,100,0,0,0,0,6497,0,'Cauldron Lord Soulwrath');
