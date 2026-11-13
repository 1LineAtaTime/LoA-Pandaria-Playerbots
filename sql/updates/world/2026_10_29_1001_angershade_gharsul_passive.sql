-- Refs #1001: Angershade (15656) / Gharsul (15958) are summoned by npc_apprentice_mirveda (zone_eversong_woods.cpp)
-- at 'Unexpected Results' accept, ~20yd from the player, faction 21 (hostile). They aggro the player before
-- 'Start WP' runs, so SmartAI::StartPath logs "wanted to start waypoint movement while in combat" and never walks to Mirveda.
-- The existing 'On Reached WP1 - Set react state aggressive' row implies they are meant to start passive.
-- Fix: insert 'Set react state passive' (action 8, param 0) into the On Just Summoned link chain before Start WP.
-- Sources: the reference DB/TDB rows identical to ours (no passive); AC has no such rows. Beyond-parity design fix.
-- Undo: DELETE FROM smart_scripts WHERE source_type=0 AND ((entryorguid=15656 AND id=7) OR (entryorguid=15958 AND id=5));
--       UPDATE smart_scripts SET link=3 WHERE source_type=0 AND entryorguid=15656 AND id=2;
--       UPDATE smart_scripts SET link=1 WHERE source_type=0 AND entryorguid=15958 AND id=0;
DELETE FROM `smart_scripts` WHERE `source_type`=0 AND ((`entryorguid`=15656 AND `id`=7) OR (`entryorguid`=15958 AND `id`=5));
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(15656,0,7,3,61,0,100,0,0,0,0,0,0,8,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Angershade - On Just Summoned - Set react state passive'),
(15958,0,5,1,61,0,100,0,0,0,0,0,0,8,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Gharsul the Remorseless - On Just Summoned - Set react state passive');
UPDATE `smart_scripts` SET `link`=7 WHERE `source_type`=0 AND `entryorguid`=15656 AND `id`=2;
UPDATE `smart_scripts` SET `link`=5 WHERE `source_type`=0 AND `entryorguid`=15958 AND `id`=0;
