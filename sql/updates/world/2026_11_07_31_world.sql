-- Cry of the Banshee 28538: the Banshee Mirror (item 64445, Call of the Banshee 90976 -> 90871 summons Banshee 48752).
-- Sourced:
--  * Her own lines, broadcast 48962-48965 (warcraft.wiki.gg, Wowhead comment on 48752). mop had High Prophet Barim's voiced boss lines.
--  * Gaze of the Banshee 90903 (stun + pacify, 5.4.8 DBC) exists for this event; nothing cast it. Comments on 28538: credit comes when the
--    cast gets off, and the agent can still be killed and looted "as he or she is carried off".
--  * The hint, broadcast 48975 "Call in a Banshee with the Banshee Mirror!", was unused.
-- Ours:
--  * Who casts the gaze (the agent on itself), the 6 s until the agent and the Banshee go (they were never removed), and the hint's trigger at 35%.
--  * The agent stays where it is: Carried by Banshee 90873 is a ride aura and 48752 has no VehicleId in any source, so she cannot lift it.
--  * The mirror works only on the agents, and only once on each (conditions): without them a held agent could give all ten credits.
--  * The hint goes to the first player on the agent's threat list (target 24), with or without the quest: a HEALTH_PCT event has no invoker
--    here, so a SmartAI condition on the player cannot be met.
DELETE FROM `creature_text` WHERE `CreatureID`=48752;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48752,0,0,'You are mine!',12,0,25,0,0,0,0,48962,0,'Banshee'),
(48752,0,1,'The Banshee Queen has plans for you, human.',12,0,25,0,0,0,0,48963,0,'Banshee'),
(48752,0,2,'Nowhere to run!',12,0,25,0,0,0,0,48964,0,'Banshee'),
(48752,0,3,'Death is only the beginning.',12,0,25,0,0,0,0,48965,0,'Banshee');

-- She stays behind the agent, where she was summoned (mop ran her to the invoker of JUST_SUMMONED, the player), and goes with the agent
DELETE FROM `smart_scripts` WHERE `entryorguid`=48752 AND `source_type`=0 AND `id` IN (1,2,3);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(48752,0,1,3,61,0,100,0,0,0,0,0,0,59,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'Banshee - On Summon - Set Run'),
(48752,0,3,0,61,0,100,0,0,0,0,0,0,1,0,6000,0,0,0,0,0,0,0,0,0,0,0,0,0,'Banshee - On Summon - Say Line 0');

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (48741,48742) AND `source_type`=0 AND `id` IN (0,4,5,6);
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(48741,0,0,4,8,0,100,1,90976,0,0,0,0,33,48752,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'SI:7 Operative - On Banshee Mirror - Killcredit'),
(48741,0,4,5,61,0,100,0,0,0,0,0,0,11,90903,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'SI:7 Operative - Linked - Gaze of the Banshee'),
(48741,0,5,0,61,0,100,0,0,0,0,0,0,41,6000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'SI:7 Operative - Linked - Despawn in 6 s'),
(48741,0,6,0,2,0,100,1,0,35,0,0,0,1,0,0,0,0,0,0,24,0,0,0,0,0,0,0,0,'SI:7 Operative - At 35% HP - Hint to a player on the threat list'),
(48742,0,0,4,8,0,100,1,90976,0,0,0,0,33,48752,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'SI:7 Assassin - On Banshee Mirror - Killcredit'),
(48742,0,4,5,61,0,100,0,0,0,0,0,0,11,90903,2,0,0,0,0,1,0,0,0,0,0,0,0,0,'SI:7 Assassin - Linked - Gaze of the Banshee'),
(48742,0,5,0,61,0,100,0,0,0,0,0,0,41,6000,0,0,0,0,0,1,0,0,0,0,0,0,0,0,'SI:7 Assassin - Linked - Despawn in 6 s'),
(48742,0,6,0,2,0,100,1,0,35,0,0,0,1,0,0,0,0,0,0,24,0,0,0,0,0,0,0,0,'SI:7 Assassin - At 35% HP - Hint to a player on the threat list');

DELETE FROM `creature_text` WHERE `CreatureID` IN (48741,48742) AND `GroupID`=0;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(48741,0,0,'Call in a Banshee with the Banshee Mirror!',42,0,100,0,0,0,0,48975,0,'SI:7 Operative - Banshee Mirror hint'),
(48742,0,0,'Call in a Banshee with the Banshee Mirror!',42,0,100,0,0,0,0,48975,0,'SI:7 Assassin - Banshee Mirror hint');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=17 AND `SourceEntry`=90976;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`,`SourceGroup`,`SourceEntry`,`SourceId`,`ElseGroup`,`ConditionTypeOrReference`,`ConditionTarget`,`ConditionValue1`,`ConditionValue2`,`ConditionValue3`,`NegativeCondition`,`ErrorType`,`ErrorTextId`,`ScriptName`,`Comment`) VALUES
(17,0,90976,0,0,31,1,3,48741,0,0,0,0,'','Call of the Banshee - SI:7 Operative'),
(17,0,90976,0,0,1,1,90903,0,0,1,0,0,'','Call of the Banshee - not already held by the gaze'),
(17,0,90976,0,1,31,1,3,48742,0,0,0,0,'','Call of the Banshee - SI:7 Assassin'),
(17,0,90976,0,1,1,1,90903,0,0,1,0,0,'','Call of the Banshee - not already held by the gaze');
