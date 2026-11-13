-- Eastern Plaguelands, Buried Blades 27467: one Slain Scourge Trooper 45695 gave all ten "Blades buried", and the corpses
-- wandered about.
--  * Sourced: "If you're quick, you can actually click multiple times on the slain undead and get more than one tick"
--    (comment 2011-01-28), so a trooper went away a moment after its blade. The Light-Touched Blades 61302 cast Bury Blade 85252
--    (a 15 s dummy aura on the trooper and KILL_CREDIT2 45695 on the player); nothing reacted to it.
--  * Sourced: both TDBs spawn the 30 troopers without movement (MovementType 0, wander 0) and give the template unit_flags2 2048;
--    mop let them roam 3 yd and had FEIGN_DEATH and DISABLE_TURN in unit_flags2. The template aura 29266 Permanent Feign Death
--    (spell_gen_creature_permanent_feign_death) sets FEIGN_DEATH, the dead look and the passive state itself.
--  * Ours: the trooper despawns 2.5 s after the blade (time for the 15 s aura to show it was used); it respawns after its 60 s
--    spawn time (both TDBs have 120; kept at 60, the less grindy value).
UPDATE `creature_template` SET `AIName`='SmartAI', `unit_flags2`=2048 WHERE `entry`=45695;
DELETE FROM `smart_scripts` WHERE `entryorguid`=45695 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45695,0,0,0,8,0,100,0,85252,0,0,0,0,41,2500,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Slain Scourge Trooper - On Spellhit Bury Blade - Despawn after 2.5 s');
UPDATE `creature` SET `MovementType`=0, `wander_distance`=0 WHERE `id`=45695;
