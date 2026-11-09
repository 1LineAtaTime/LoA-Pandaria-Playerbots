-- The Reckoning 27144: Sylvanas' scene with Koltira in Andorhal (zone_western_plaguelands.cpp)
-- Sourced: 5.4.8 DBC 84252 The Reckoning: Force Phase (aura 261, phase 269 "Andorhal 05 - Andorhal Event"), 84208/84209/84218 Summon
-- Sylvanas 45111 / Koltira 45112 / Portal to Undercity 45114 (SummonProperties 492, personal) at their spell_target_position spots,
-- 84257 Cancel Phase and Teleport; objective "Witness the Reckoning" (credit 45111); broadcast_text 45245-45256 (the dialogue, as the
-- 2010-08-20 Wowhead comment on 27144 quotes it). Sylvanas' lines are the female texts, Koltira's the male ones.
-- Ours: who says each line is read from the text (male/female), the timing, Koltira kneeling and walking into the portal, the
-- personal text range (the actors are personal summons), no camera (84211 Reckoning Event Camera 45113 and 84258 the player's
-- doppelganger are not used), the scene starting on accept and again from Sylvanas' menu, and the actors' levels (the camp
-- Sylvanas' 88 and Koltira's 85; the templates had 1).
UPDATE `creature_template` SET `ScriptName`='npc_wpl_sylvanas_andorhal' WHERE `entry`=44451;
UPDATE `creature_template` SET `ScriptName`='npc_wpl_reckoning_sylvanas', `minlevel`=88, `maxlevel`=88 WHERE `entry`=45111;
UPDATE `creature_template` SET `ScriptName`='npc_wpl_reckoning_actor', `minlevel`=85, `maxlevel`=85 WHERE `entry`=45112;
UPDATE `creature_template` SET `ScriptName`='npc_wpl_reckoning_actor' WHERE `entry`=45114;

DELETE FROM `spell_script_names` WHERE `spell_id`=84252;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(84252,'spell_wpl_reckoning_force_phase');

DELETE FROM `creature_text` WHERE `CreatureID` IN (45111,45112);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(45111,0,0,'Koltira!  Your queen summons you!',14,0,100,0,0,0,0,45245,5,'Lady Sylvanas Windrunner - The Reckoning'),
(45111,1,0,'Enough of this insolence.  Do you think that I''m blind to everything that goes on outside of the Undercity?',12,0,100,1,0,0,0,45247,5,'Lady Sylvanas Windrunner - The Reckoning'),
(45111,2,0,'No thanks to you.  Andorhal should have been ours days ago, when we defeated the Scourge.',12,0,100,1,0,0,0,45249,5,'Lady Sylvanas Windrunner - The Reckoning'),
(45111,3,0,'I know your secret, Koltira.  I''m aware of your friendship with the Alliance death knight, Thassarian.',12,0,100,1,0,0,0,45250,5,'Lady Sylvanas Windrunner - The Reckoning'),
(45111,4,0,'Arthas clearly failed when he created his death knights.  You are still weak.',12,0,100,1,0,0,0,45252,5,'Lady Sylvanas Windrunner - The Reckoning'),
(45111,5,0,'Fortunately, I have agents that can help you... erase this weakness.  Beneath the Undercity you go.',12,0,100,1,0,0,0,45253,5,'Lady Sylvanas Windrunner - The Reckoning'),
(45111,6,0,'Shh... quiet, death knight.  When I''m finished with you, your fear will be gone.',12,0,100,0,0,0,0,45255,5,'Lady Sylvanas Windrunner - The Reckoning'),
(45111,7,0,'You can thank me later.',12,0,100,1,0,0,0,45256,5,'Lady Sylvanas Windrunner - The Reckoning'),
(45112,0,0,'Your highness.  I deliver to you... Andorhal.',12,0,100,0,0,0,0,45246,5,'Koltira Deathweaver - The Reckoning'),
(45112,1,0,'I... your majesty, we were victorious!',12,0,100,1,0,0,0,45248,5,'Koltira Deathweaver - The Reckoning'),
(45112,2,0,'Y... your majesty...',12,0,100,0,0,0,0,45251,5,'Koltira Deathweaver - The Reckoning'),
(45112,3,0,'No!',14,0,100,0,0,0,0,45254,5,'Koltira Deathweaver - The Reckoning');
