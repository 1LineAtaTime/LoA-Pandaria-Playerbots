-- A Different Approach 26954: the Diseased Hawks circle the hill until Zen'Kiki pulls one down, and Zen'Kiki leaves with the aura.
-- Sourced: both TDBs (4.4.2 and 1210) give Diseased Hawk 44481 unit_flags 0x100 (IMMUNE_TO_PC), and TCPP's sniff import has 33024
-- (0x8100); mop dropped the 0x100, so the 22 hawks, flying 10-30 yd above the hilltop (z ~120), attacked every player on it. Both TDBs
-- keep 22 hawks there, so they stay; nothing in the 5.4.8 DBC summons a Diseased Hawk. Wowhead (quest 26954 and its comments): "the
-- diseased hawks that circle above that hill", "you can't tame or attack them unless he pulls them down", right-click Zen'Kiki and
-- he shoots one, "Hey, that tickles, mon!" / "Quit yer pokin', mon!" / "Alright, here I go!" (broadcast_text 44953, 44952, 44954).
-- Ours: the spell scripts (Wrath 83541 makes the hit hawk drop 0x100 and attack the clicker; Escorting Zen'Kiki 83456 dismisses
-- Zen'Kiki 44269 when it goes, as 83819 does for 44904), the hawk's evade/respawn rows that give 0x100 back (a respawn keeps the
-- flags it died with), and Zen'Kiki's three lines as creature_text. The old spell-hit row (attack the closest player) goes.
UPDATE `creature_template` SET `unit_flags`=`unit_flags`|256 WHERE `entry`=44481;

DELETE FROM `smart_scripts` WHERE `entryorguid`=44481 AND `source_type`=0 AND `id` IN (1,2);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(44481,0,1,0,7,0,100,0,0,0,0,0,18,256,0,0,0,0,0,1,0,0,0,0,0,0,0,'Diseased Hawk - On Evade - Set Unit Flag Immune To PC'),
(44481,0,2,0,11,0,100,0,0,0,0,0,18,256,0,0,0,0,0,1,0,0,0,0,0,0,0,'Diseased Hawk - On Respawn - Set Unit Flag Immune To PC');

DELETE FROM `creature_text` WHERE `CreatureID`=44269;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(44269,0,0,'Hey, that tickles, mon!',12,0,100,0,0,0,0,44953,0,'Zen''Kiki - Poked'),
(44269,0,1,'Quit yer pokin'', mon!',12,0,100,0,0,0,0,44952,0,'Zen''Kiki - Poked'),
(44269,0,2,'Alright, here I go!',12,0,100,0,0,0,0,44954,0,'Zen''Kiki - Poked');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_wpl_escorting_zenkiki','spell_wpl_zenkiki_wrath') AND `spell_id` IN (83456,83541);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(83456,'spell_wpl_escorting_zenkiki'),
(83541,'spell_wpl_zenkiki_wrath');
