-- Eastern Plaguelands, I'm Not Supposed to Tell You This (27392): The Lone Hunter lies still, gets up to fight, is back in 2 minutes.
--
-- Sourced: the wolf lies "appearing dead" east of Darrowshire and leaps up when you come close (comments OLD 2010-10-22 [+60],
-- 2010-11-24, 2012-11-25 "just lying down"); both TDBs spawn him static (wander 0, MovementType 0) with respawn 120 s; comments
-- 2011-07-27 and 2011-08-26 put the respawn at "about two minutes". Before: spawn 173508 wandered 3 yd while wearing Cosmetic - State
-- Dead 81238, so a corpse strolled about, and the pose never ended in a fight.
-- Ours: the SmartAI row that removes 81238 on aggro (81238 has no aura interrupt flags: Spell.dbc SpellInterruptsId 0); an evade or a
-- respawn reloads the template addon, so the pose comes back by itself. An evade reloads it at once, so two more rows (ours) take
-- the pose off when he evades and put it back when he is home: he does not glide home lying down.
UPDATE `creature` SET `wander_distance`=0, `MovementType`=0, `spawntimesecs`=120 WHERE `guid`=173508 AND `id`=45450;
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=45450;
DELETE FROM `smart_scripts` WHERE `entryorguid`=45450 AND `source_type`=0 AND `id` IN (3,4,5);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45450,0,3,0,4,0,100,0,0,0,0,0,28,81238,0,0,0,0,0,1,0,0,0,0,0,0,0,'The Lone Hunter - On Aggro - Remove Aura Cosmetic - State Dead (ours)'),
(45450,0,4,0,7,0,100,0,0,0,0,0,28,81238,0,0,0,0,0,1,0,0,0,0,0,0,0,'The Lone Hunter - On Evade - Remove Aura Cosmetic - State Dead (ours)'),
(45450,0,5,0,21,0,100,0,0,0,0,0,75,81238,0,0,0,0,0,1,0,0,0,0,0,0,0,'The Lone Hunter - On Reached Home - Add Aura Cosmetic - State Dead (ours)');
