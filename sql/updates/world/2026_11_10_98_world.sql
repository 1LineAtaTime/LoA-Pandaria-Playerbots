-- The first battle for Andorhal (phase 259): the Opengut Behemoths 44562 join the fight and one-shot the soldiers, never players.
-- Sourced: a retail video the user found shows the Behemoths one-shotting the faction soldiers, besides their Scourge Hook pull
-- and Cleave. Wowhead comment on The Battle for Andorhal 27159: "the abominations will, after a few seconds, use a one-shot
-- ability on horde/alliance NPCs - but never on players"; War Machines 26923: "lumbering brutes that are throwing my deathguards
-- around like ragdolls". unit_flags 32832 (0x8040) is TCPP's 4.3.4 sniff (TDB 4.4.2/12.1.0: 64); mop had UNIT_FLAG_UNK_29
-- (the feign-death flag) and UNIT_FLAG_RENAME, which no source has. Cleave 40504 and Scourge Hook 50335 stay as the TDBs have them.
-- Inference: the one-shot is Opengut Cleave 83136 ("Inflicts physical damage, knocking the target back", 900-1100 damage),
-- which no source casts; its id follows Summon Andorhal Deathguard/Defender 83121/83135 (44331/44332, the soldiers the sniff
-- shows standing around the Behemoths).
-- Ours: faction 1988 (the template 826aa723 gave the Desiccated Scourge: same Scourge faction, group, friends and hostility
-- to players as 974, plus hostility to Alliance and Horde creatures), the C++ script npc_wpl_opengut_behemoth (it starts fights
-- on sight only with what 974 is hostile to; Opengut Cleave on an NPC victim 3-5 s into each fight, then every 6-9 s), the
-- spell script spell_wpl_opengut_cleave (an NPC target dies on the hit, so the soldiers' 85% sparring limit can't stop it), the
-- out-of-combat attack on the nearest soldier within 5 yd (as the Scourge's rows), and the 85% sparring limit for the Behemoth
-- itself (one reading of "they and the Scourge are set not to deal damage to them beyond ~80%"), so NPCs never take the War
-- Machines kills away from players.
UPDATE `creature_template` SET `faction`=1988, `unit_flags`=32832, `ScriptName`='npc_wpl_opengut_behemoth' WHERE `entry`=44562;

DELETE FROM `spell_script_names` WHERE `spell_id`=83136;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(83136,'spell_wpl_opengut_cleave');

DELETE FROM `creature_sparring_template` WHERE `creature_id`=44562;
INSERT INTO `creature_sparring_template` (`creature_id`,`health_limit_pct`,`comment`) VALUES
(44562,85,'Opengut Behemoth - Battle for Andorhal');

DELETE FROM `smart_scripts` WHERE `entryorguid`=44562 AND `source_type`=0 AND `id` BETWEEN 2 AND 5;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(44562,0,2,0,1,0,100,0,1000,3000,5000,8000,0,49,0,0,0,0,0,0,19,44326,5,0,0,0,0,0,0,'Opengut Behemoth - Out of Combat - Attack the Andorhal Deathguard within 5 yd'),
(44562,0,3,0,1,0,100,0,1000,3000,5000,8000,0,49,0,0,0,0,0,0,19,44328,5,0,0,0,0,0,0,'Opengut Behemoth - Out of Combat - Attack the Andorhal Defender within 5 yd'),
(44562,0,4,0,1,0,100,0,1000,3000,5000,8000,0,49,0,0,0,0,0,0,19,44331,5,0,0,0,0,0,0,'Opengut Behemoth - Out of Combat - Attack the Andorhal Deathguard within 5 yd'),
(44562,0,5,0,1,0,100,0,1000,3000,5000,8000,0,49,0,0,0,0,0,0,19,44332,5,0,0,0,0,0,0,'Opengut Behemoth - Out of Combat - Attack the Andorhal Defender within 5 yd');
