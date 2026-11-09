-- Western Plaguelands, Hearthglen: a Hearthglen Trainee 45162 left mid-duel stays hostile (It's About Time! 27156).
-- When the challenger runs off, dies or the trainee leashes, SmartAI::EnterEvadeMode sends it home but does not restore its faction
-- (no RestoreFaction there, in Reset/OnReset or in JustReachedHome), so it kept the duel faction 14 until it respawned: its
-- gossip was gone, MoveInLineOfSight made it attack passers-by, and anyone who beat it to 1 % gave the credit to the stored
-- challenger (list 1).
-- Ours: On Evade in phase 1 (the duel; a yielded trainee is in phase 2, see 2026_11_11_06) it restores its template faction,
-- clears list 1 (a stored-target selector of the unused list 2 returns an empty, non-null list, so StoreTargetList empties it) and
-- drops the invincibility level. The evade runs these before MoveTargetedHome and Reset; phase reset is off, so it stays in phase 1
-- and can be challenged again, and Reset clears the run-once of the yield (id 8). React state and unit flags don't change in a duel.
-- A challenge during the home walk sets faction 14 and stores the player, but Unit::Attack refuses an evading attacker, so it
-- would sit at home hostile with no fight and no evade to come. On Reached Home in phase 1 (HomeMovementGenerator's finalize on
-- arrival -> SmartAI::JustReachedHome) runs the same reset and casts Full Heal 43978 (heal max health) on itself, so the next duel
-- doesn't start at the ~1 HP left by the last one. Reached Home only follows an evade's home walk, never a challenge at home.
DELETE FROM `smart_scripts` WHERE `entryorguid`=45162 AND `source_type`=0 AND `id` IN (22,23,24,25,26,27,28);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45162,0,22,23,7,1,100,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - On Evade (Phase 1) - Restore Faction'),
(45162,0,23,24,61,0,100,0,0,0,0,0,0,64,1,0,0,0,0,0,12,2,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 22 - Clear Stored Target List 1'),
(45162,0,24,0,61,0,100,0,0,0,0,0,0,42,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 23 - Reset Invincibility Hp'),
(45162,0,25,26,21,1,100,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - On Reached Home (Phase 1) - Restore Faction'),
(45162,0,26,27,61,0,100,0,0,0,0,0,0,64,1,0,0,0,0,0,12,2,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 25 - Clear Stored Target List 1'),
(45162,0,27,28,61,0,100,0,0,0,0,0,0,42,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 26 - Reset Invincibility Hp'),
(45162,0,28,0,61,0,100,0,0,0,0,0,0,11,43978,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 27 - Cast Full Heal');
