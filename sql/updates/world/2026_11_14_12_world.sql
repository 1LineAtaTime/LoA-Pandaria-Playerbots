-- Eastern Plaguelands, Fiona's Caravan: the party's lines outside the rides.
--
--  * Before: none of them was ever spoken (Fiona 45417, Gidwin 45431 and 45730 had no creature_text and no AI).
--  * Fiona, on accepting Onward 27373, to the player (wiki 27373, broadcast 46090): npc_epl_fiona.
--  * Gidwin paces in front of Crown Guard Tower after the first ride and grumbles (wiki 27373, broadcast 46100-46104): guid
--    401103 (the Crown Guard copy, stop 2).
--  * Gidwin by the statue of Light's Hope nags Tarenar (wiki 27455, broadcast 45814-45816): guid 373696 (the statue copy).
--  * Gidwin in the Eastwall tower, "circling the interior of the tower" (wiki 27466, broadcast 46108-46111): guid 3128014
--    (the tower copy, staged by 2026_11_14_01).
--  * Each copy is quest-invisible to everyone but the players at that stage of the story (2026_11_14_00/_01), and creature text
--    reaches every player in range whether he sees the speaker or not, so npc_epl_gidwin_idle says each line privately
--    (TextRange 5) to each player within 25 yd who sees him, and nothing with nobody to hear. Fiona's line is said only to the
--    player who took the quest (TextRange 5).
--  * Sourced: the lines, their speakers and places. Ours: one random line of its group every 40-60 s (the first 15-30 s after
--    the copy is seen), the order of the lines within a group, the 25 yd.

UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_epl_fiona' WHERE `entry`=45417;
UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_epl_gidwin_idle' WHERE `entry` IN (45431,45730);
DELETE FROM `creature_text` WHERE `CreatureID` IN (45417,45431,45730);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(45417,0,0,'By the way, you''re welcome to borrow any supplies you find inside the caravan... as long as you bring them back when you''re done!',12,0,100,0,0,0,0,46090,5,'Fiona - on accepting Onward'),
(45431,0,0,'Alright, Gidwin... deep breaths...',12,0,100,0,0,0,0,46104,5,'Gidwin Goldbraids - pacing at Crown Guard Tower'),
(45431,0,1,'What''s the holdup, Fiona?',12,0,100,0,0,0,0,46103,5,'Gidwin Goldbraids - pacing at Crown Guard Tower'),
(45431,0,2,'I can''t BELIEVE that we''re stopping!  We''ve got a duty to do here!',12,0,100,0,0,0,0,46100,5,'Gidwin Goldbraids - pacing at Crown Guard Tower'),
(45431,0,3,'Why haven''t we left yet?  I''m ready to head off on my own if we don''t get going.',12,0,100,0,0,0,0,46101,5,'Gidwin Goldbraids - pacing at Crown Guard Tower'),
(45431,1,0,'Tarenar, you dolt!  The chapel is twenty yards away!  Let''s get moving!',12,0,100,0,0,0,0,45815,5,'Gidwin Goldbraids - by the statue of Light''s Hope'),
(45431,1,1,'Come on already!  Let''s go!',12,0,100,0,0,0,0,45814,5,'Gidwin Goldbraids - by the statue of Light''s Hope'),
(45431,1,2,'Can we go yet, Tarenar?  I want to join the Crusade!',12,0,100,0,0,0,0,45816,5,'Gidwin Goldbraids - by the statue of Light''s Hope'),
(45730,0,0,'That useless blood elf!  We have a DUTY to perform, and he wastes our time!',12,0,100,0,0,0,0,46108,5,'Gidwin Goldbraids - in the Eastwall tower'),
(45730,0,1,'I don''t know why I put up with his nonsense.  He never takes anything seriously.',12,0,100,0,0,0,0,46109,5,'Gidwin Goldbraids - in the Eastwall tower'),
(45730,0,2,'Tyrosus himself sent us to Northdale!  So why are we wasting our time with upstart nerubians?!',12,0,100,0,0,0,0,46110,5,'Gidwin Goldbraids - in the Eastwall tower'),
(45730,0,3,'I''ll throw my hammer upside his head if he makes us wait any longer.',12,0,100,0,0,0,0,46111,5,'Gidwin Goldbraids - in the Eastwall tower');

-- the former per-guid SmartAI of these copies (an earlier draft of this file)
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (-401103,-373696,-3128014) AND `source_type`=0;
