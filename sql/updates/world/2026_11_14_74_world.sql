-- Eastern Plaguelands, Defenders of Darrowshire (27529): talking to a Darrowshire Spirit 11064 credits the quest; a spirit rises
-- from every Cannibal Ghoul 8530, Gibbering Ghoul 8531 and Diseased Flayer 8532, whoever kills it; the static spirit goes.
--
--  * Before: the quest could not be completed. The spirit's C++ npc_darrowshire_spirit called Player::TalkedToCreature, which
--    credits only QUEST_OBJECTIVE_TALKTO (type 3) objectives; 27529's objective 266636 is type 0. The 8530 summon was gated on
--    "invoker has 27529 taken", which fails when a pet, guardian or totem lands the killing blow (Unit::Kill hands the killing
--    unit to JustDied; CONDITION_QUESTTAKEN needs a player). 8531 / 8532 summoned through the C++ npc_ghoul_flayer, also for a
--    player's killing blow only.
--  * Sourced: both TDBs (4.4.2 and 12.1): the three ghouls summon the spirit on death with no condition; the spirit casts Spirit
--    Spawn-in 17321 and credits its gossip invoker (target 7, not the group: the group credit is a 2008 comment on the old
--    quest). "you need to talk to the resultant spirit to get credit" (2010-11-28 [+30]); the text closes almost at once
--    (2019 comment); spirits also rose from Gibbering Ghouls and Diseased Flayers (old comments). Neither TDB spawns 11064.
--  * Decided, the summon's quest gate: dropped, as both TDBs: the spirit rises for everyone and leaves after 60 s. Keying it on
--    the quest would need the killer's owner, and the sources describe spirits rising from the ghouls, not a quest-only event.
--  * Ours: the clicked spirit turns unselectable and goes after 4 s (TDB: 10 s, still selectable). The menu (3141 -> npc_text
--    3873, five lines) opens as before: SmartAI's OnGossipHello leaves the default menu.
--  * C++: npc_ghoul_flayer and npc_darrowshire_spirit are removed from zone_eastern_plaguelands.cpp in the same commit.
--    8531's Curse of Tongues (TDB id 1) and 8532's Birth (TDB id 2) are other scopes' and are left to them; ids 0 are taken here.

-- The spirit: SmartAI only (its AIName is already SmartAI)
UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` = 11064;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 11064 AND `source_type` = 0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`target_type`,`comment`) VALUES
(11064,0,0,0,54,0,100,0,0,0,0,0,11,17321,0,1,'Darrowshire Spirit - On Just Summoned - Cast Spirit Spawn-in'),
(11064,0,1,2,64,0,100,1,0,0,0,0,33,11064,0,7,'Darrowshire Spirit - On Gossip Hello - Kill Credit to the invoker'),
(11064,0,2,3,61,0,100,0,0,0,0,0,18,33554432,0,1,'Darrowshire Spirit - On Gossip Hello - Set Not Selectable'),
(11064,0,3,0,61,0,100,0,0,0,0,0,41,4000,0,1,'Darrowshire Spirit - On Gossip Hello - Despawn after 4 s');

-- Its orphan condition (a SmartAI row 3 that does not exist) and the static copy of the summon (Terrordale; neither TDB has it)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceEntry` = 11064;
DELETE FROM `creature_addon` WHERE `guid` = 175430;
DELETE FROM `creature` WHERE `guid` = 175430 AND `id` = 11064;

-- The Cannibal Ghoul's summon for any killer
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 22 AND `SourceGroup` = 1 AND `SourceEntry` = 8530;

-- Gibbering Ghoul and Diseased Flayer: the TDBs' death summon (17310, the TDBs' spell, is not in the 5.4.8 client data: a direct
-- summon at the corpse for 60 s, as the Cannibal Ghoul's row)
UPDATE `creature_template` SET `AIName` = 'SmartAI', `ScriptName` = '' WHERE `entry` IN (8531,8532);
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (8531,8532) AND `source_type` = 0 AND `id` = 0;
INSERT INTO `smart_scripts` (`entryorguid`,`source_type`,`id`,`link`,`event_type`,`event_phase_mask`,`event_chance`,`event_flags`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`action_type`,`action_param1`,`action_param2`,`action_param3`,`target_type`,`comment`) VALUES
(8531,0,0,0,6,0,100,0,0,0,0,0,12,11064,3,60000,1,'Gibbering Ghoul - On Death - Summon Darrowshire Spirit'),
(8532,0,0,0,6,0,100,0,0,0,0,0,12,11064,3,60000,1,'Diseased Flayer - On Death - Summon Darrowshire Spirit');
