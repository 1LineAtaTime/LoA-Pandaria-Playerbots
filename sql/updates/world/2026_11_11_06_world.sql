-- Western Plaguelands, Hearthglen: a yielded Hearthglen Trainee 45162 fights on (It's About Time! 27156).
-- At 1 % health the trainee's SmartAI stops combat, adds UNIT_FLAG_IMMUNE_TO_PC, gives the credit, says "Well fought." and despawns
-- after 5 s, but it stays on faction 14 and REACT_DEFENSIVE. SmartAI::MoveInLineOfSight ignores REACT_DEFENSIVE and
-- Creature::CanStartAttack ignores IMMUNE_TO_PC alone, so the hostile trainee attacks the player still standing next to it; a DoT or
-- pet hit re-engages it through JustStartedThreateningMe; and Unit::SetInCombatState then drops IMMUNE_TO_PC. Its gossip also
-- stays, so a second "duel" during the fade set faction 14 and attacked again with the credit event already spent.
-- Ours: after the yield the trainee turns passive (every re-engage path checks REACT_PASSIVE), takes back its own faction, loses
-- its gossip and drops the player's auras on it (ids 14-17, linked from id 12).
-- It also turns UNIT_FLAG_NON_ATTACKABLE (id 20): Spell::DoSpellHitOnUnit makes a delayed spell still in flight miss (evade)
-- instead of starting combat again, which would drop IMMUNE_TO_PC.
-- Event phases: 1 = ready, 2 = yielded (id 21). The gossip select (id 2) and the yield (id 8) only run in phase 1, so a gossip
-- window left open from before the yield (the select handler doesn't check npcflags) can't restart the fight, and an evade (which
-- clears id 8's run-once) can't give a second credit. Phase reset is turned off, so a reset never puts phase 0 back.
-- Respawn undoes all of it: SmartAI::JustAppeared restores the faction and fires On Respawn, where id 0 sets REACT_DEFENSIVE and
-- id 13 removes IMMUNE_TO_PC | NON_ATTACKABLE (258; unit flags survive a respawn) and sets phase 1 (ids 18-19); JUST_RESPAWNED
-- restores the npcflags. The credit itself (ids 9-12) is unchanged.
DELETE FROM `smart_scripts` WHERE `entryorguid`=45162 AND `source_type`=0 AND `id` IN (2,8,12,13,14,15,16,17,18,19,20,21);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(45162,0,2,3,62,1,100,0,11943,0,0,0,0,64,1,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Hearthglen Trainee - On Gossip Select (Phase 1) - Store Target List'),
(45162,0,8,9,2,1,100,1,0,0,0,0,0,203,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - On Between 0-1% (Phase 1) - Combat Stop'),
(45162,0,12,14,61,0,100,0,0,0,0,0,0,41,5000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 11 - Despawn after 5 sec'),
(45162,0,13,18,11,0,100,0,0,0,0,0,0,19,258,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - On Respawn - Remove Unit Flags Immune To PC, Non Attackable'),
(45162,0,14,15,61,0,100,0,0,0,0,0,0,8,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 12 - Set React State Passive'),
(45162,0,15,16,61,0,100,0,0,0,0,0,0,2,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 14 - Restore Faction'),
(45162,0,16,17,61,0,100,0,0,0,0,0,0,83,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 15 - Remove Npc Flag Gossip'),
(45162,0,17,20,61,0,100,0,0,0,0,0,0,28,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 16 - Remove All Auras'),
(45162,0,18,19,61,0,100,0,0,0,0,0,0,211,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 13 - Keep Event Phase Through Reset'),
(45162,0,19,0,61,0,100,0,0,0,0,0,0,22,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 18 - Set Event Phase 1'),
(45162,0,20,21,61,0,100,0,0,0,0,0,0,18,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 17 - Set Unit Flag Non Attackable'),
(45162,0,21,0,61,0,100,0,0,0,0,0,0,22,2,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Hearthglen Trainee - Linked To Id 20 - Set Event Phase 2');
