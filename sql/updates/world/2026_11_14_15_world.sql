-- Eastern Plaguelands: the paladin followers of What I Do Best 27371, A Boyhood Dream 27463, The Trial of the Crypt 27464, The
-- Noxious Glade 27465 and Ix'lar the Underlord 27487, on one script (zone_eastern_plaguelands_caravan.cpp).
--
--  * Before: nothing cast their summons (no spell_area, no script), the six templates had no AI, and the summon spells' auras
--    were saved at logout (a relog left the aura without the follower, and the standing Tarenar hidden by it).
--  * Who comes when (sourced: the quests' texts and comments; the spells' AreaGroups; the pairs by their models, caravan and
--    chapel-argent second passes):
--      84902 Tarenar 45445                                    27371 in the log, Death's Step 5446
--      85218 / 85217 Gidwin 45677 / Tarenar 45678 (no tabard) 27463 or 27464 in the log, Light's Hope 2268 / Noxious Glade 2270
--      85404 / 85402 Gidwin 45793 / Tarenar 45792 (tabard)    27465 in the log, the same areas
--      85405 Tarenar 45794                                    27487 in the log, Eastwall Tower 2271 / Ix'lar's Domain 5463
--                                                             (its reward spell 85807 cancels it)
--    In the log = incomplete or complete (27463 has no objectives and is complete on accept).
--  * The script (spell_epl_follower_summon, spell_epl_follower_aura, npc_epl_caravan_follower): never two; the quest summon slot
--    of 84902 is left alone (no eviction of another companion); no aura for a dead player; the follower goes with the aura
--    (reward, abandon, leaving the areas; the chapel pairs stay through Browman Mill, Pestilent Scar and Death's Breach on the way
--    to the glade); defensive, joins
--    what the player fights, can be healed, takes the player's level and faction, is brought along when left far behind, buffs
--    the party (Might / Kings, Retribution / Devotion Aura, Seal of Truth), Tarenar of 27371 uses Divine Storm; when he dies the
--    standing copy is back and he returns at the latest 30 s later (sooner on any area or quest status change) if the quest
--    and area still fit (ours). A summon aura saved under the old data is dropped at login so that the follower comes back. See the comments in the script.
--  * Not saved at logout (ours, as 91938 / 73306 in SpellMgr): login casts the spell again and with it the follower.
--  * Templates: 45445 does not regenerate health ("He doesn't regen health", 27371 comment 2010-12-27); 45677 has unit_flags 0
--    as both TDBs (mop 33024, IMMUNE_TO_PC); the others keep their fields (the script clears the immunity flags of every follower).
--  * Texts (sourced): Tarenar 45445 in combat "Take that! Taste the Light!" (50399), "Ahhhhh ha ha haa!" (50401), on a kill
--    "Wow. Is it just me, or did that one die really fast?" (50400); Gidwin 45677 / 45793 in combat "Did you see that, Tarenar?",
--    "Ka-boom!", "Can we slow down just a bit?", "I can't believe we're actually fighting for the Crusade!" (50403-50407). How
--    often (ours): Tarenar 30 % on engaging and on a kill, Gidwin on engaging at most every 20-40 s.

UPDATE `creature_template` SET `AIName`='', `ScriptName`='npc_epl_caravan_follower' WHERE `entry` IN (45445,45677,45678,45792,45793,45794);
UPDATE `creature_template` SET `RegenHealth`=0 WHERE `entry`=45445;
UPDATE `creature_template` SET `unit_flags`=0 WHERE `entry`=45677;

DELETE FROM `spell_script_names` WHERE `spell_id` IN (84902,85217,85218,85402,85404,85405) AND `ScriptName` IN ('spell_epl_follower_summon','spell_epl_follower_aura');
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(84902,'spell_epl_follower_summon'),(84902,'spell_epl_follower_aura'),
(85217,'spell_epl_follower_summon'),(85217,'spell_epl_follower_aura'),
(85218,'spell_epl_follower_summon'),(85218,'spell_epl_follower_aura'),
(85402,'spell_epl_follower_summon'),(85402,'spell_epl_follower_aura'),
(85404,'spell_epl_follower_summon'),(85404,'spell_epl_follower_aura'),
(85405,'spell_epl_follower_summon'),(85405,'spell_epl_follower_aura');

DELETE FROM `spell_custom_attribute` WHERE `spell` IN (84902,85217,85218,85402,85404,85405);
INSERT INTO `spell_custom_attribute` (`spell`,`attribute`,`comment`) VALUES
(84902,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Summon Tarenar Sunstrike (27371): spell_area casts it again, with the follower, at login'),
(85217,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Summon Tarenar Sunstrike (27463, 27464): spell_area casts it again, with the follower, at login'),
(85218,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Summon Gidwin Goldbraids (27463, 27464): spell_area casts it again, with the follower, at login'),
(85402,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Summon Tarenar Sunstrike (27465): spell_area casts it again, with the follower, at login'),
(85404,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Summon Gidwin Goldbraids (27465): spell_area casts it again, with the follower, at login'),
(85405,'SPELL_ATTR0_CU_DONT_SAVE_AURA_TO_DB','Summon Tarenar Sunstrike (27487): spell_area casts it again, with the follower, at login');

DELETE FROM `spell_area` WHERE `spell` IN (84902,85217,85218,85402,85404,85405);
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(84902,5446,27371,0,0,0,2,1,10,0),
(85218,2268,27463,0,0,0,2,1,10,0),
(85217,2268,27463,0,0,0,2,1,10,0),
(85218,2270,27463,0,0,0,2,1,10,0),
(85217,2270,27463,0,0,0,2,1,10,0),
(85218,2268,27464,0,0,0,2,1,10,0),
(85217,2268,27464,0,0,0,2,1,10,0),
(85218,2270,27464,0,0,0,2,1,10,0),
(85217,2270,27464,0,0,0,2,1,10,0),
(85404,2268,27465,0,0,0,2,1,10,0),
(85402,2268,27465,0,0,0,2,1,10,0),
(85404,2270,27465,0,0,0,2,1,10,0),
(85402,2270,27465,0,0,0,2,1,10,0),
(85405,2271,27487,0,0,0,2,1,10,0),
(85405,5463,27487,0,0,0,2,1,10,0);

DELETE FROM `creature_text` WHERE `CreatureID` IN (45445,45677,45793);
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(45445,0,0,'Take that! Taste the Light!',12,0,100,0,0,0,0,50399,0,'Tarenar Sunstrike (27371 follower) - in combat'),
(45445,0,1,'Ahhhhh ha ha haa!',12,0,100,0,0,0,0,50401,0,'Tarenar Sunstrike (27371 follower) - in combat'),
(45445,1,0,'Wow. Is it just me, or did that one die really fast?',12,0,100,0,0,0,0,50400,0,'Tarenar Sunstrike (27371 follower) - on a kill'),
(45677,0,0,'Did you see that, Tarenar?',12,0,100,0,0,0,0,50403,0,'Gidwin Goldbraids (27463/27464 follower) - in combat'),
(45677,0,1,'Ka-boom!',12,0,100,0,0,0,0,50404,0,'Gidwin Goldbraids (27463/27464 follower) - in combat'),
(45677,0,2,'Can we slow down just a bit?',12,0,100,0,0,0,0,50406,0,'Gidwin Goldbraids (27463/27464 follower) - in combat'),
(45677,0,3,'I can''t believe we''re actually fighting for the Crusade!',12,0,100,0,0,0,0,50407,0,'Gidwin Goldbraids (27463/27464 follower) - in combat'),
(45793,0,0,'Did you see that, Tarenar?',12,0,100,0,0,0,0,50403,0,'Gidwin Goldbraids (27465 follower) - in combat'),
(45793,0,1,'Ka-boom!',12,0,100,0,0,0,0,50404,0,'Gidwin Goldbraids (27465 follower) - in combat'),
(45793,0,2,'Can we slow down just a bit?',12,0,100,0,0,0,0,50406,0,'Gidwin Goldbraids (27465 follower) - in combat'),
(45793,0,3,'I can''t believe we''re actually fighting for the Crusade!',12,0,100,0,0,0,0,50407,0,'Gidwin Goldbraids (27465 follower) - in combat');
