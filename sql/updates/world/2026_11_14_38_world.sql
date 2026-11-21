-- Eastern Plaguelands, Out of the Ziggurat (27481) and Into the Flames (27482).
-- Sourced:
--   27481 needs Honor and Strength (27449) (wiki; decisions R3, also on the user's question list).
--   Both are handed in from the quest log, and 27482 pops up on the ziggurat once 27481 is handed in (wiki "End:
--   Automatic"; both TDBs Flags 65544 / 589824; Player::GetNextQuest offers RewardNextQuest on a log turn-in). The summoned
--   Vex'tul 45741 is no quest giver or ender (both TDBs: npcflag 0, no quest rows); 27482's ender is Vex'tul 45574 (both TDBs).
--   27482's completion window text (both TDBs).
--   Warlord Thresh'jin respawns in 3 minutes (comment 2013-07-22) and says "I'll rip out your eyeballs, weakling..."
--   (broadcast 50355; comments 2010-12-20 [+11], OriKami).
-- Ours: that Thresh'jin says it on the pull, and as a say.
-- Not here: Vex'tul's companion AI, his farewell at the bonfire and 27482's credit (the follower script, section 2, with
-- its b1.sql); his greeting menu 12041 (lights-shield's commit sets the same).
UPDATE `quest_template_addon` SET `PrevQuestID`=27449 WHERE `ID`=27481;
UPDATE `quest_template` SET `Flags`=65544 WHERE `ID`=27481;
UPDATE `quest_template` SET `Flags`=589824 WHERE `ID`=27482;
DELETE FROM `creature_questender` WHERE `id`=45741 AND `quest` IN (27481,27482);
DELETE FROM `creature_queststarter` WHERE `id`=45741 AND `quest`=27482;
DELETE FROM `creature_questender` WHERE `id`=45574 AND `quest`=27482;
INSERT INTO `creature_questender` (`id`,`quest`) VALUES (45574,27482);
UPDATE `creature_template` SET `npcflag`=0 WHERE `entry`=45741;
DELETE FROM `quest_offer_reward` WHERE `ID`=27482;
INSERT INTO `quest_offer_reward` (`ID`,`Emote1`,`Emote2`,`Emote3`,`Emote4`,`EmoteDelay1`,`EmoteDelay2`,`EmoteDelay3`,`EmoteDelay4`,`RewardText`,`VerifiedBuild`) VALUES
(27482,0,0,0,0,0,0,0,0,'Vex''tul has passed on into the next realm.  Your job here in Zul''Mashar is done.$b$bWait, what''s this?  Vex''tul must have dropped it on his way into the fire.',0);

UPDATE `creature` SET `spawntimesecs`=180 WHERE `guid`=175267 AND `id`=10822;
DELETE FROM `creature_text` WHERE `CreatureID`=10822;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(10822,0,0,'I''ll rip out your eyeballs, weakling...',12,0,100,0,0,0,0,50355,0,'Warlord Thresh''jin - aggro');
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=10822;
DELETE FROM `smart_scripts` WHERE `entryorguid`=10822 AND `source_type`=0 AND `id`=3;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_type`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`target_x`,`target_y`,`target_z`,`target_o`,`comment`) VALUES
(10822,0,3,0,4,0,100,0,0,0,0,0,0,1,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,'Warlord Thresh''jin - On Aggro - Say Line 0');
