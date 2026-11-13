-- The Princess Unleashed (26912 / 26049): Myzrael 2755 cannot be attacked during her speech, then attacks the player who summoned her (not the closest player) and stays 10 minutes;
-- her Prismatic Exiles 2887 cast Frost Nova and put Lightning Shield on themselves, not on their victim.
-- Sourced: TDB 1210/442 creature_template 2755 unit_flags 256 (IMMUNE_TO_PC) and action list 275500 (face the summoner, three lines, remove IMMUNE_TO_PC, attack the summoner);
-- event_scripts 420 despawn 600000 in both TDBs (mop 180000); Frost Nova 11831 (0-8 yd, every 15-25 s) from the TDB 2887 script and Wowhead's Abilities tab;
-- Lightning Shield 12550 (an aura on the caster) as the TDB 442/1210 2887 rows: on self out of combat after 0.5-1 s, every 10 min, and again whenever it is missing (15-30 s).
-- Ours: the speech keeps mop's TEXT_OVER chain and its 5 s gaps (retail waits 3, 3 and 5 s) instead of the action list; the old "store targetlist" row goes.
-- Not changed: the Exiles are summoned as guardians and take Myzrael's faction; this core's SummonGuardian makes them attack her victim when she is in combat, so they need no attack row.
UPDATE `creature_template` SET `unit_flags` = 256 WHERE `entry` = 2755;
UPDATE `event_scripts` SET `datalong2` = 600000 WHERE `id` = 420 AND `command` = 10 AND `datalong` = 2755;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 2755 AND `source_type` = 0 AND `id` BETWEEN 4 AND 9;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(2755,0,4,5,54,0,100,1,0,0,0,0,0,1,0,5000,0,0,0,0,23,0,0,0,0,0,0,0,0,'Myzrael - On Just Summoned - Say Line 0 (to the summoner)'),
(2755,0,5,0,61,0,100,1,0,0,0,0,0,66,0,0,0,0,0,0,23,0,0,0,0,0,0,0,0,'Myzrael - Linked - Face the summoner'),
(2755,0,6,0,52,0,100,1,0,2755,0,0,0,1,1,5000,0,0,0,0,1,0,0,0,0,0,0,0,0,'Myzrael - On Text 0 Over - Say Line 1'),
(2755,0,7,8,52,0,100,1,1,2755,0,0,0,1,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Myzrael - On Text 1 Over - Say Line 2'),
(2755,0,8,9,61,0,100,1,0,0,0,0,0,19,256,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Myzrael - Linked - Remove Unit Flag Immune To PC'),
(2755,0,9,0,61,0,100,1,0,0,0,0,0,49,0,0,0,0,0,0,23,0,0,0,0,0,0,0,0,'Myzrael - Linked - Attack the summoner');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 2887 AND `source_type` = 0 AND `id` = 2;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(2887,0,2,0,9,0,100,0,0,8,15000,25000,0,11,11831,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Prismatic Exile - Within 0-8 Range - Cast ''Frost Nova''');
DELETE FROM `smart_scripts` WHERE `entryorguid` = 2887 AND `source_type` = 0 AND `id` IN (1,3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(2887,0,1,0,1,0,100,0,500,1000,600000,600000,0,11,12550,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Prismatic Exile - Out of Combat - Cast ''Lightning Shield'' on self'),
(2887,0,3,0,16,0,100,0,12550,1,15000,30000,0,11,12550,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Prismatic Exile - Lightning Shield missing - Cast ''Lightning Shield'' on self');
