-- Eastern Plaguelands, Guardians of Stratholme (27525): Omasum Blighthoof 45867 and Karthis Darkrune 45868, the death knights in
-- the two necropoli of the Plaguewood, use all four abilities of their sniffed spell lists.
--
--  * Sourced: the abilities. creature_template spell1-4 (sniffed): Omasum Bloodworm 79893, Death and Decay 79887, Death Grip
--    79894, Heart Strike 79885; Karthis Chains of Ice 79899, Frost Strike 79895, Howling Blast 79896, Icy Touch 79897. The
--    existing rows (ids 0-2, the same as TDB 4.4.2) never cast Heart Strike or Howling Blast.
--  * Ours: the timers (no comment describes this fight), in the range of the existing rows.
--  * Death Grip 79894 is a bare dummy on the target (no spell_script_names row): spell_epl_lieutenant_death_grip pulls the
--    target in front of Omasum with the death knights' own jump 49575, as a player death knight's grip does. What the player
--    sees: being yanked to Omasum when standing 10-30 yd away (his "Cast Death Grip on Close" row). The script acts only for
--    Omasum 45867 (Karthis does not have the spell): Mogu Infiltrator 66206 (every 2-4 s in combat) and Tharil'zun 486 cast 79894
--    too and keep the plain dummy. Not for a target already in melee range or deflecting all spells (Deterrence).

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (45867,45868) AND `source_type` = 0 AND `id` = 3;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`target_type`,`comment`) VALUES
(45867,0,3,0,0,0,100,0,3000,6000,7000,10000,11,79885,0,2,'Omasum Blighthoof - In Combat - Cast Heart Strike'),
(45868,0,3,0,0,0,100,0,8000,12000,16000,22000,11,79896,0,2,'Karthis Darkrune - In Combat - Cast Howling Blast');

DELETE FROM `spell_script_names` WHERE `spell_id` = 79894;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(79894,'spell_epl_lieutenant_death_grip');
