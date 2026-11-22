-- Darkmoon Faire: Whack-a-Gnoll Hogger points and Baby Needs Two Pair of Shoes (29508) credit.
-- Problem 1: the bonus gnoll (54549, Darkmoon Faire Gnoll - Bonus) gives the kill credit 101835 once; Mola's rules say
--   "Regular gnolls grant one point, while Hoggers count for three."
-- Problem 2: the Horseshoe (71967) casts 101847 Shoe Baby (dummy on Baby 54510), but Baby has no AI or script, so the
--   objective (creature 54510, 1) stays 0/1.
-- Change 1: the bonus gnoll casts the credit three times (event 1 -> link 3 -> link 4 -> link 2 despawn); the two extra
--   casts are triggered so they do not fail on a cast in progress.
-- Change 2: SmartAI on Baby: on spell hit 101847 the caster gets the kill credit for 54510.
-- Source: broadcast text 53319 (5.4.8 client data); 5.4.8 Spell.dbc 101847; quest objective of 29508.
-- Rollback:
--   UPDATE `smart_scripts` SET `link`=2 WHERE `entryorguid`=54549 AND `source_type`=0 AND `id`=1;
--   DELETE FROM `smart_scripts` WHERE `entryorguid`=54549 AND `source_type`=0 AND `id` IN (3,4);
--   DELETE FROM `smart_scripts` WHERE `entryorguid`=54510 AND `source_type`=0;
--   UPDATE `creature_template` SET `AIName`='' WHERE `entry`=54510;
UPDATE `smart_scripts` SET `link`=3 WHERE `entryorguid`=54549 AND `source_type`=0 AND `id`=1;
DELETE FROM `smart_scripts` WHERE `entryorguid`=54549 AND `source_type`=0 AND `id` IN (3,4);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(54549,0,3,4,61,0,100,0,0,0,0,0,0,11,101835,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Darkmoon Faire Gnoll - Bonus - Linked - Give Kill Credit (second point)'),
(54549,0,4,2,61,0,100,0,0,0,0,0,0,11,101835,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Darkmoon Faire Gnoll - Bonus - Linked - Give Kill Credit (third point)');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=54510 AND `ScriptName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=54510 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(54510,0,0,0,8,0,100,0,101847,0,0,0,0,33,54510,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Baby - On Spellhit Shoe Baby - Quest Credit Put New Horseshoes on Baby');
