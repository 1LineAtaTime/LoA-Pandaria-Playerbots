-- Felstone Field: the Enthralled Val'kyr of When Death is Not Enough 26937 and Combat Training 26938 (zone_western_plaguelands.cpp)
-- Sourced (5.4.8 DBC): 84204 Bond of the Val'kyr (dummy aura); 84016 Summon Enthralled Val'kyr 44491 (guardian, slot 6) and 84149
-- Summon Enthralled Val'kyr 45080 (pet, slot 6); 84145 Notify Val'kyr (spell2 of Alliance Laborer 45047); 84143 Raise Forsaken
-- (triggers 84187 Kill Credit 45089 to the caster's master); 84129-84131 Summon FT Felstone 01-03 (45048-45050); 84155 Call to Arms;
-- 84190 See Quest Invis 4 (detection 1000, type 12) against the Forsaken Troopers' 82343 Generic Quest Invisibility 4 (type 12,
-- strength 100; 84184 Detect: Quest Invis Zone 4 detects only 1, too weak, WorldObject::CanDetectInvisibilityOf). Quest texts: "Take my
-- enthralled val'kyr with you ... Once they are dead, she will bring them back"; "Use your Enthralled Val'kyr's Call to Arms ability
-- to mobilize 5 Forsaken Troopers". Wowhead comments on 26938: "Pet bar has the undead face button for Call to Arms. Target a
-- converted undead, hit that, and they'll team up with you."
-- Ours: the bounds of both auras (Bond while either quest is in the log, zone-wide; the detection in Felstone Field, area 199, from
-- 26937 rewarded until 26938 is rewarded) and 84190 as that detection; Call to Arms as the pet's first spell; the val'kyr
-- hovering (as Agatha 44951).
-- The eight static Forsaken Troopers 45048-45050 on Felstone are copies of 84129-84131's summons (risen laborers): the event now
-- summons them, so the copies go. No other table refers to them.

DELETE FROM `spell_area` WHERE `spell`=84204 AND `area`=28 AND `quest_start` IN (26937,26938);
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(84204,28,26937,0,0,0,2,1,10,11),
(84204,28,26938,0,0,0,2,1,10,11);

DELETE FROM `spell_area` WHERE `spell` IN (84184,84190) AND `area`=199;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(84190,199,26937,26938,0,0,2,1,64,11);

DELETE FROM `spell_script_names` WHERE `spell_id` IN (84204,84145,84155);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(84204,'spell_wpl_bond_of_the_valkyr'),
(84145,'spell_wpl_notify_valkyr'),
(84155,'spell_wpl_call_to_arms');

UPDATE `creature_template` SET `ScriptName`='npc_wpl_enthralled_valkyr' WHERE `entry` IN (44491,45080);
UPDATE `creature_template` SET `spell1`=84155 WHERE `entry`=45080;
UPDATE `creature_template` SET `ScriptName`='npc_wpl_forsaken_trooper' WHERE `entry` IN (45085,45048,45049,45050);

DELETE FROM `creature_template_movement` WHERE `CreatureId` IN (44491,45080);
INSERT INTO `creature_template_movement` (`CreatureId`,`Ground`,`Swim`,`Flight`,`Rooted`,`Chase`,`Random`,`InteractionPauseTimer`) VALUES
(44491,2,1,1,0,0,0,NULL),
(45080,2,1,1,0,0,0,NULL);

-- Alliance Laborer: on death, Notify Val'kyr at the killer (its spell2)
DELETE FROM `smart_scripts` WHERE `entryorguid`=45047 AND `source_type`=0 AND `id`=1;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45047,0,1,0,6,0,100,0,0,0,0,0,0,11,84145,2,0,0,0,0,7,0,0,0,0,0,0,0,0,'Alliance Laborer - On Death - Cast ''Notify Val''kyr'' on Killer');

DELETE FROM `creature` WHERE `guid` IN (312031,312032,312033,312034,312035,312036,312037,312038) AND `id` IN (45048,45049,45050);
