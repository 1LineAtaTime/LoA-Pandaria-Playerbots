-- Echo Isles, quest 24814 "An Ancient Enemy": the event never reset when the player died or left during the fight.
--
-- Why: the player who picks Vol'jin's (38225) gossip "I am ready" is stored by Vol'jin and sent to Zar'jira (38306)
-- as target list 1, and only that player gets Zar'jira's kill credit. Zar'jira fights Vol'jin, Zuni (38423) and
-- Vanira (38437), so she stays in combat with them whether the player is there or not, and nothing in the SmartAI
-- watches the player. When he died or walked away, Zar'jira, Vol'jin and his allies kept fighting, Vol'jin had lost
-- his quest giver flag (action list 3822503), and the event could not be started again until a restart.
--
-- Fix: a presence check and a reset, SmartAI only.
-- * Every 10 s in combat Zar'jira casts "100 yd Leash" (39396, an invisible single-target dummy with no other user)
--   on the stored player, triggered. The spell conditions below let it land only while that player is alive, within
--   80 yd of her, and has the quest in progress or completed. A hit sets data 7 0 on Vol'jin (watchdog counter 2 back
--   to 0), the linked tick sets data 7 1 (counter + 1). CROSS_CAST is used instead of CAST because SmartScript delays
--   a CAST event while the caster is casting, and Zar'jira channels Frozen Torrent (72045) through the fire phases.
-- * Three ticks without a hit (30-40 s) bring Vol'jin's counter 2 to 4, and Zar'jira evading sends data 9 1. Both
--   start action list 3822504 (only in Vol'jin's fight phase 2): phase 3, data 9 9 to Zar'jira, Zuni, Vanira, the
--   Manifestations (38302) and the Fires of the Seas (38542), data 9 8 to one Fire, and Vol'jin despawns.
-- * On data 9 9 Zar'jira, Zuni, Vanira and the Manifestations despawn; the Fires hide again and stop their beam.
--   The Fire that got data 9 8 respawns Vol'jin 10 s later, and Vol'jin's existing "On Respawn" list (3822502)
--   respawns Zuni, Vanira and Zar'jira. A respawn restores Vol'jin's gossip and quest giver flags from the template,
--   his position and his phase, so the next "I am ready" starts a fresh event.
-- Counter 2 lives on Vol'jin, not on Zar'jira: every counter change re-checks all COUNTER_SET events of the creature,
-- and Zar'jira's "counter 1 = 3" event (fires stamped out) would run again on each tick.

-- Zar'jira
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38306 AND `source_type` = 0 AND `id` BETWEEN 35 AND 39;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38306,0,35,36,0,0,100,0,10000,10000,10000,10000,86,39396,2,1,0,0,0,12,1,0,0,0,0,0,0,'Zar''jira - In Combat - Cross Cast ''100 yd Leash'' on the stored player (presence check)'),
(38306,0,36,0,61,0,100,0,0,0,0,0,45,7,1,0,0,0,0,10,252717,38225,0,0,0,0,0,'Zar''jira - In Combat (Link) - Set Data 7 1 (Vol''jin, presence counter + 1)'),
(38306,0,37,0,31,0,100,0,39396,0,0,0,45,7,0,0,0,0,0,10,252717,38225,0,0,0,0,0,'Zar''jira - On Spell Hit Target ''100 yd Leash'' - Set Data 7 0 (Vol''jin, presence counter reset)'),
(38306,0,38,0,7,0,100,0,0,0,0,0,45,9,1,0,0,0,0,10,252717,38225,0,0,0,0,0,'Zar''jira - On Evade - Set Data 9 1 (Vol''jin, reset the event)'),
(38306,0,39,0,38,0,100,0,9,9,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zar''jira - On Data Set 9 9 - Despawn (event reset)');

-- Vol'jin
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38225 AND `source_type` = 0 AND `id` BETWEEN 18 AND 21;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38225,0,18,0,38,0,100,0,7,1,0,0,63,2,1,0,0,0,0,1,0,0,0,0,0,0,0,'Vol''jin - On Data Set 7 1 - Add 1 to Counter 2 (presence check tick)'),
(38225,0,19,0,38,0,100,0,7,0,0,0,63,2,0,1,0,0,0,1,0,0,0,0,0,0,0,'Vol''jin - On Data Set 7 0 - Reset Counter 2 (player present)'),
(38225,0,20,0,77,2,100,0,2,4,0,0,80,3822504,2,0,0,0,0,1,0,0,0,0,0,0,0,'Vol''jin - On Counter 2 = 4 - Start Script (event reset) (Phase 2)'),
(38225,0,21,0,38,2,100,0,9,1,0,0,80,3822504,2,0,0,0,0,1,0,0,0,0,0,0,0,'Vol''jin - On Data Set 9 1 - Start Script (event reset) (Phase 2)');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 3822504 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(3822504,9,0,0,0,0,100,0,0,0,0,0,22,3,0,0,0,0,0,1,0,0,0,0,0,0,0,'Vol''jin - On Script - Set Phase 3 (reset in progress)'),
(3822504,9,1,0,0,0,100,0,0,0,0,0,45,9,9,0,0,0,0,11,38306,100,0,0,0,0,0,'Vol''jin - On Script - Set Data 9 9 (Zar''jira)'),
(3822504,9,2,0,0,0,100,0,0,0,0,0,45,9,9,0,0,0,0,11,38423,100,0,0,0,0,0,'Vol''jin - On Script - Set Data 9 9 (Zuni)'),
(3822504,9,3,0,0,0,100,0,0,0,0,0,45,9,9,0,0,0,0,11,38437,100,0,0,0,0,0,'Vol''jin - On Script - Set Data 9 9 (Vanira)'),
(3822504,9,4,0,0,0,100,0,0,0,0,0,45,9,9,0,0,0,0,11,38302,100,0,0,0,0,0,'Vol''jin - On Script - Set Data 9 9 (Manifestations of the Sea Witch)'),
(3822504,9,5,0,0,0,100,0,0,0,0,0,45,9,9,0,0,0,0,11,38542,100,0,0,0,0,0,'Vol''jin - On Script - Set Data 9 9 (Fires of the Seas)'),
(3822504,9,6,0,0,0,100,0,0,0,0,0,45,9,8,0,0,0,0,10,302893,38542,0,0,0,0,0,'Vol''jin - On Script - Set Data 9 8 (one Fire of the Seas: respawn Vol''jin)'),
(3822504,9,7,0,0,0,100,0,0,0,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Vol''jin - On Script - Despawn');

-- Zuni, Vanira, Manifestation of the Sea Witch
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38423 AND `source_type` = 0 AND `id` = 21;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38437 AND `source_type` = 0 AND `id` = 20;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38302 AND `source_type` = 0 AND `id` = 4;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38423,0,21,0,38,0,100,0,9,9,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Zuni - On Data Set 9 9 - Despawn (event reset)'),
(38437,0,20,0,38,0,100,0,9,9,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Vanira - On Data Set 9 9 - Despawn (event reset)'),
(38302,0,4,0,38,0,100,0,9,9,0,0,41,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Manifestation of the Sea Witch - On Data Set 9 9 - Despawn (event reset)');

-- Fire of the Seas
DELETE FROM `smart_scripts` WHERE `entryorguid` = 38542 AND `source_type` = 0 AND `id` BETWEEN 6 AND 8;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(38542,0,6,7,38,0,100,0,9,9,0,0,47,0,0,0,0,0,0,1,0,0,0,0,0,0,0,'Fire of the Seas - On Data Set 9 9 - Set Visibility Off (event reset)'),
(38542,0,7,0,61,0,100,0,0,0,0,0,92,0,73294,1,0,0,0,1,0,0,0,0,0,0,0,'Fire of the Seas - On Data Set 9 9 (Link) - Interrupt Spell Cast (Fire of the Seas Energy Beam)'),
(38542,0,8,0,38,0,100,0,9,8,0,0,80,3854200,2,0,0,0,0,1,0,0,0,0,0,0,0,'Fire of the Seas - On Data Set 9 8 - Start Script (respawn Vol''jin)');

DELETE FROM `smart_scripts` WHERE `entryorguid` = 3854200 AND `source_type` = 9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(3854200,9,0,0,0,0,100,0,10000,10000,0,0,70,0,0,0,0,0,0,10,252717,38225,0,0,0,0,0,'Fire of the Seas - On Script - Respawn Vol''jin (event reset)');

-- 39396 "100 yd Leash" lands only on a living player with the quest in progress or complete, within 80 yd of the caster.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 17 AND `SourceGroup` = 0 AND `SourceEntry` = 39396;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(17,0,39396,0,0,36,1,0,0,0,0,0,0,'','100 yd Leash: target must be alive'),
(17,0,39396,0,0,47,1,24814,10,0,0,0,0,'','100 yd Leash: target has An Ancient Enemy in progress or complete'),
(17,0,39396,0,0,35,1,0,80,4,0,0,0,'','100 yd Leash: target within 80 yd of the caster');

-- Undo:
-- DELETE FROM `smart_scripts` WHERE `entryorguid` = 38306 AND `source_type` = 0 AND `id` BETWEEN 35 AND 39;
-- DELETE FROM `smart_scripts` WHERE `entryorguid` = 38225 AND `source_type` = 0 AND `id` BETWEEN 18 AND 21;
-- DELETE FROM `smart_scripts` WHERE `entryorguid` IN (3822504, 3854200) AND `source_type` = 9;
-- DELETE FROM `smart_scripts` WHERE `entryorguid` = 38423 AND `source_type` = 0 AND `id` = 21;
-- DELETE FROM `smart_scripts` WHERE `entryorguid` = 38437 AND `source_type` = 0 AND `id` = 20;
-- DELETE FROM `smart_scripts` WHERE `entryorguid` = 38302 AND `source_type` = 0 AND `id` = 4;
-- DELETE FROM `smart_scripts` WHERE `entryorguid` = 38542 AND `source_type` = 0 AND `id` BETWEEN 6 AND 8;
-- DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 17 AND `SourceGroup` = 0 AND `SourceEntry` = 39396;
