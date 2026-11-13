-- Eastern Plaguelands, Heroes of Darrowshire (27388): two Blighthounds jump whoever takes Davil's Libram.
--
-- Sourced: taking the libram makes two Blighthounds appear and attack; they give XP, loot and skins, and it repeats each time the book
-- is taken (comments 2010-12-03, 2011-02-11 [+73], 2011-02-22, 2012-10-26, 2012-01-11, 2012-01-17). The libram 176206's looted event
-- (Data6) is 26321 in mop and both TDBs, and Spell::EffectOpenLock starts it on every opening of the chest, but no event_scripts row
-- 26321 exists in mop or either TDB, so nothing came.
-- Ours: the two rows, the positions (upper floor of the Marris Stead house, 4-7 yd from the libram; floor height 133.3 read from the
-- vmap), the 2 minute despawn of a hound nobody fights (TIMED_OR_DEAD_DESPAWN), and a SmartAI row that sends a summoned hound at
-- its summoner (the looter: event_scripts summons from the script's source), since a standing creature only checks for targets
-- when something moves. The eight static Blighthounds are not summoned and do not run the row.
DELETE FROM `event_scripts` WHERE `id`=26321;
INSERT INTO `event_scripts` (`id`,`delay`,`command`,`datalong`,`datalong2`,`dataint`,`x`,`y`,`z`,`o`) VALUES
(26321,0,10,45453,120000,0,1869.9,-3184.0,133.4,3.6),
(26321,0,10,45453,120000,0,1871.9,-3181.0,133.4,3.9);
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=45453;
DELETE FROM `smart_scripts` WHERE `entryorguid`=45453 AND `source_type`=0 AND `id`=2;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45453,0,2,0,54,0,100,0,0,0,0,0,49,0,0,0,0,0,0,7,0,0,0,0,0,0,0,'Blighthound - On Just Summoned - Attack Summoner (Davil''s Libram, ours)');
