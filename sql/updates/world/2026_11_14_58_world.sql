-- Eastern Plaguelands, Argent Call: The Trial of the Crypt 27464: the trial, built (zone_eastern_plaguelands_chapel.cpp).
--  * The sniff copies go: Lord Raymond George 45707 (guid 307921, respawn 60 s) and five Argent Wardens 45698 (307922-307926)
--    stood in the crypt's last chamber as hostile statics that attacked anyone who walked in, quest or not, and George was the
--    only credit. Neither TDB spawns either entry; the wardens are the summons of 85263-85266 and George presides over the
--    trial. The three waypoint paths of the static wardens (307922, 307923, 307925; 19 rows) go with them: the script walks the
--    summoned wardens along the same points.
--  * George's template unit_flags 768 (immune to players and NPCs until the warm-up ends), as both TDBs (mop had 512).
--  * ScriptNames: George runs the trial (npc_epl_lord_raymond_george), the wardens walk in and fight the reader's group
--    (npc_epl_argent_warden); the scroll's spell 85308 starts it (spell_epl_trial_of_the_crypt).
--  * Texts (sourced, broadcast_text and the wiki): George 45805 at the start, 45806 when the wardens are down, 45812 on his
--    death; the Argent Sentry at the crypt door (guid 174031, 2343.4 -5391.8 96.0) says "Good luck." (45813) to a player
--    with the quest when he comes within 8 yd (wiki: "Entering the crypt an Argent Sentry says Good luck."). Ours: the
--    sentry's 60 s cooldown. The sentry's own rows 0-4 are the entry's five combat rows (2026_11_06_01), which a guid script
--    replaces (SmartScript::GetScript).
DELETE FROM `creature_addon` WHERE `guid` IN (307921,307922,307923,307924,307925,307926);
DELETE FROM `waypoint_data` WHERE `id` IN (307922,307923,307925);
DELETE FROM `creature` WHERE `guid` IN (307921,307922,307923,307924,307925,307926) AND `id` IN (45707,45698);

UPDATE `creature_template` SET `unit_flags`=768, `AIName`='', `ScriptName`='npc_epl_lord_raymond_george' WHERE `entry`=45707;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_epl_argent_warden' WHERE `entry`=45698;

DELETE FROM `spell_script_names` WHERE `spell_id`=85308;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (85308,'spell_epl_trial_of_the_crypt');

DELETE FROM `creature_text` WHERE `CreatureID`=45707;
DELETE FROM `creature_text` WHERE `CreatureID`=16378 AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(45707,0,0,'Let the trial begin.  Rise, champions!  Rise, and meet these three in combat!',12,0,100,0,0,0,0,45805,0,'Lord Raymond George - the trial begins'),
(45707,1,0,'Your warm-up is complete.  Now, the real trial begins!',12,0,100,0,0,0,0,45806,0,'Lord Raymond George - the wardens are down, he fights'),
(45707,2,0,'Well-done, heroes.  You have passed my test of combat.',12,0,100,0,0,0,0,45812,0,'Lord Raymond George - defeated'),
(16378,0,0,'Good luck.',12,0,100,0,0,0,0,45813,0,'Argent Sentry at the crypt door - a player on The Trial of the Crypt comes near');

UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=16378 AND `ScriptName`='';
DELETE FROM `smart_scripts` WHERE `entryorguid`=-174031 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(-174031,0,0,0,0,0,100,0,0,0,2300,3900,0,11,6660,64,0,0,0,0,2,0,0,0,0,0,0,0,0,'Argent Sentry (crypt door) - Cast Shoot'),
(-174031,0,1,0,13,0,100,0,20000,30000,0,0,0,11,15618,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Argent Sentry (crypt door) - Cast Snap Kick on Player Spell Cast'),
(-174031,0,2,0,9,0,100,0,0,5,15000,25000,0,11,19643,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Argent Sentry (crypt door) - Cast Mortal Strike on Close'),
(-174031,0,3,0,4,0,100,1,0,0,0,0,0,11,25821,0,0,0,0,0,2,0,0,0,0,0,0,0,0,'Argent Sentry (crypt door) - On Aggro - Cast ''Charge'''),
(-174031,0,4,0,0,0,100,0,1000,3000,9000,12000,0,11,18328,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Argent Sentry (crypt door) - In Combat - Cast ''Incapacitating Shout'''),
(-174031,0,5,0,10,0,100,0,1,8,60000,60000,1,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Argent Sentry (crypt door) - OOC LoS player within 8 yd - Say Good luck (The Trial of the Crypt incomplete)');
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceEntry`=-174031;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(22,6,-174031,0,0,9,0,27464,0,0,0,0,0,'','Argent Sentry at the crypt door: Good luck only to a player with The Trial of the Crypt incomplete');
