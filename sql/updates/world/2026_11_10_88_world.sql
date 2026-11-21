-- Ace in the Hole 27089: Sylvanas' val'kyr in Andorhal (zone_western_plaguelands.cpp, same code as Felstone Field)
-- Sourced (5.4.8 DBC): 84195 Notify Val'kyr (spell2 of Frightened Provincial Minuteman 45101); 84197 Raise Forsaken (triggers 84196
-- Kill Credit 45102 to the caster's master); 84132-84135 Summon FT Andorhal 01-04 (45058-45061); credit 45103 for "Forsaken Trooper
-- mobilized"; areatrigger 6235 (1345.8, -1282.6, radius 25) around Lurid's body 45129, the quest's ender, used by nothing else.
-- Quest text: "kill 5 Frightened Provincial Minutemen, then use your Call to Arms ability to turn them into Forsaken Troopers. Then
-- find Thassarian". Wowhead comments: the val'kyr's pet bar as in Combat Training; the troops follow; "Once you get to the end you
-- will see the mob on the ground dead".
-- Ours: the val'kyr is the Call to Arms pet of Combat Training (45080, 84149), because 84198's guardian 44492 (SummonProperties
-- 2341) has no pet bar; the Bond aura while 27089 is in the log, zone-wide; "Hunt Thassarian" (credit 44453) given at areatrigger
-- 6235, since Thassarian (level 85, immune to players) cannot be the kill.
-- The five static Forsaken Troopers 45058/45060/45061 in Andorhal (312100-312102, 312104, 312105; 312102 and 312104 on one spot)
-- are copies of 84132-84135's summons: the event now raises them, so the copies go, with their addons and two paths. No other
-- table refers to them.
DELETE FROM `spell_area` WHERE `spell`=84204 AND `area`=28 AND `quest_start`=27089;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(84204,28,27089,0,0,0,2,1,10,11);

DELETE FROM `spell_script_names` WHERE `spell_id`=84195;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(84195,'spell_wpl_notify_valkyr');

UPDATE `creature_template` SET `ScriptName`='npc_wpl_forsaken_trooper' WHERE `entry` IN (45058,45059,45060,45061);

-- Frightened Provincial Minuteman: on death, Notify Val'kyr at the killer (its spell2)
DELETE FROM `smart_scripts` WHERE `entryorguid`=45101 AND `source_type`=0 AND `id`=1;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45101,0,1,0,6,0,100,0,0,0,0,0,0,11,84195,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Frightened Provincial Minuteman - On Death - Cast ''Notify Val''kyr'' on Killer');

DELETE FROM `areatrigger_scripts` WHERE `entry`=6235;
INSERT INTO `areatrigger_scripts` (`entry`,`ScriptName`) VALUES
(6235,'at_wpl_lurids_body');

DELETE FROM `waypoint_data` WHERE `id` IN (312102,312104);
DELETE FROM `creature_addon` WHERE `guid` IN (312100,312101,312102,312104,312105);
DELETE FROM `creature` WHERE (`guid`,`id`) IN ((312100,45058),(312101,45058),(312102,45060),(312104,45058),(312105,45061));
