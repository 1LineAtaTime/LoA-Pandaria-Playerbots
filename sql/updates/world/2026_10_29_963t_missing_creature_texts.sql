-- #963 (lane t): SmartAI talk rows rejected at the v14 boot with "using non-existent Text id N" (creature_text group missing).
-- the reference DB (the reference 5.4.8 world DB) lacks all of these too. Searched: acore_world, TDB 4.3.4 / 3.3.5 / 4.4.2 / 12.1 dumps, SFDB 5.4.8 dump,
-- GitHub code search (TrinityCore, SkyFire, AzerothCore, MoP/Cata cores), broadcast_text, Wowpedia MediaWiki API (2026-10-29).
-- NOT touched (AC parity): 4484 row 24, 30945 row 9 (TEXT_OVER validation rejects).
--
-- FOUND -> creature_text rows inserted:
--   30284 g0   Bonegrinder enrage emote            mynew/PandaDB Database/pandabase/creature_text.sql: (30284,0,0,'%s becomes enraged!',..,'combat Enrage')
--   46134 g0   High Commander Kamses flee emote     same file, 'combat Flee' row
--   46402 g0   Ramkahen Citizen flee emote          same file
--   48012 g0   Sergeant Mehat flee emote            same file
--       (PandaDB stores these with type 2; this DB / TDB / AC use 16 = monster text emote, so 16 here.)
--       BroadcastTextId: 1150 '%s attempts to run away in fear!' and 10677 '%s becomes enraged!' are the broadcast_text rows
--       the other flee / enrage groups of this DB already use.
--   54944 g1   Tian Pupil post-fight lines (7)     davidwow1024/Karmalygos-Master sql/updates/world/2022_07_17_00_Fix_more_creature_text.sql
--       (54944,1,0..6,...,'Tian Pupil'); BroadcastTextIds matched by exact string in this DB's broadcast_text.
--   57760 g0   Wugou (INFERRED): 57760 carries a clone of the 55539 Wugou script ("Data Set 0 1 - Say Line 0", 5 s, then set data).
--       55539 g0 is '%s snorts loudly, and continues sleeping.' (type 16, BC 54991) in this DB and in
--       MttAI-dev/MTT-WoW-BfA sql/old/world/2020_05_18_02_pandaren_terminado.sql line 1467 ('Wugou to Player'). Same NPC, same script shape.
--
-- NOT FOUND ANYWHERE -> only the Talk action is removed (row turned into SMART_ACTION_NONE, link chain / event kept):
--   -84714 row 0  (creature guid 84714 is entry 23696 Gordok Brew Chief; the script is a stray copy of the 23698 Drunken Brewfest
--                  Reveler "Say Line 1 (random)". No creature_text exists for 23696 in TDB 335/434/442/1210, SFDB, AC, the reference DB;
--                  creature_template 23696 has no AIName so the script is dead anyway. Row 1 (cast Drunken Vomit) left as it is.)
--   30474 row 8   (North Wind, "On data set 0 2 - Say text 2"; Wowpedia lists only 2 yells, TDB/AC/the reference DB only groups 0-1.
--                  Link 9 (Die) kept.)
--   46276 row 1   (Caimas on-aggro "Say Line 4"; Wowpedia lists only 3 yells, all already groups 0/1/3. Link 2 (Attack Start) kept.)
--   46425 row 11  (Ramkahen Prisoner "Say Line 3" after text-over 2; Wowpedia "Neferset Prison" shows the prisoner saying only the two
--                  lines already in groups 1 and 2, then the quest auto-completes. Row 12 (Waypoint Start) was triggered by text-over
--                  of group 3, which can no longer fire, so row 12 now triggers on text-over of group 2 and the escape still happens.)
--   54615 row 0   (Nodd Codejack "Data Set 0 1 - Say Line 2"; SFDB has only group 0 'Over here, $n! ...'. The three lines Wowpedia lists
--                  for "Nodd Codejack" belong to entry 55832, not 54615, so they are not used.)
--
-- Idempotent (DELETE the exact groups, re-INSERT; UPDATEs are keyed on entry/id and set absolute values).
-- Backup of the affected rows before this script: (not shipped),
--   backup_smart_scripts.sql (mysqldump --skip-extended-insert, test a local DB `world`).
-- UNDO:
--   DELETE FROM `creature_text` WHERE (`CreatureID`,`GroupID`) IN ((30284,0),(46134,0),(46402,0),(48012,0),(54944,1),(57760,0));
--   UPDATE `smart_scripts` SET `action_type`=1,`action_param1`=1,`action_param2`=0,`action_param3`=0 WHERE `entryorguid`=-84714 AND `source_type`=0 AND `id`=0;
--   UPDATE `smart_scripts` SET `action_type`=1,`action_param1`=2,`action_param2`=1000 WHERE `entryorguid`=30474 AND `source_type`=0 AND `id`=8;
--   UPDATE `smart_scripts` SET `action_type`=1,`action_param1`=4,`action_param2`=0,`action_param3`=1 WHERE `entryorguid`=46276 AND `source_type`=0 AND `id`=1;
--   UPDATE `smart_scripts` SET `action_type`=1,`action_param1`=3,`action_param2`=6000 WHERE `entryorguid`=46425 AND `source_type`=0 AND `id`=11;
--   UPDATE `smart_scripts` SET `event_param1`=3 WHERE `entryorguid`=46425 AND `source_type`=0 AND `id`=12;
--   UPDATE `smart_scripts` SET `action_type`=1,`action_param1`=2,`action_param2`=6000 WHERE `entryorguid`=54615 AND `source_type`=0 AND `id`=0;
--   (30284 / 54944 / 57760 / 46134 / 46402 / 48012 smart_scripts rows are not modified.)

-- ---------------------------------------------------------------------------------------------------------------------------
-- creature_text groups
DELETE FROM `creature_text` WHERE (`CreatureID`,`GroupID`) IN ((30284,0),(46134,0),(46402,0),(48012,0),(54944,1),(57760,0));
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`SoundType`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(30284,0,0,'%s becomes enraged!',16,0,100,0,0,0,0,10677,0,'Bonegrinder - Enrage'),
(46134,0,0,'%s attempts to run away in fear!',16,0,100,0,0,0,0,1150,0,'High Commander Kamses - Flee'),
(46402,0,0,'%s attempts to run away in fear!',16,0,100,0,0,0,0,1150,0,'Ramkahen Citizen - Flee'),
(48012,0,0,'%s attempts to run away in fear!',16,0,100,0,0,0,0,1150,0,'Sergeant Mehat - Flee'),
(54944,1,0,'Good fight.',12,0,100,0,0,0,0,53244,0,'Tian Pupil'),
(54944,1,1,'Good fight. I''ll get you next time.',12,0,100,0,0,0,0,53240,0,'Tian Pupil'),
(54944,1,2,'I don''t think I ever stood a chance. See you around, then!',12,0,100,0,0,0,0,53246,0,'Tian Pupil'),
(54944,1,3,'Man... I thought I had that one...',12,0,100,0,0,0,0,53239,0,'Tian Pupil'),
(54944,1,4,'Master Stone Fist was right about you. You''re good.',12,0,100,0,0,0,0,53241,0,'Tian Pupil'),
(54944,1,5,'Nice moves!',12,0,100,0,0,0,0,53238,0,'Tian Pupil'),
(54944,1,6,'You take this pretty seriously, don''t you?',12,0,100,0,0,0,0,53243,0,'Tian Pupil'),
(57760,0,0,'%s snorts loudly, and continues sleeping.',16,0,100,0,0,0,0,54991,0,'Wugou');

-- ---------------------------------------------------------------------------------------------------------------------------
-- Talk actions with no text anywhere: SMART_ACTION_NONE (0), event / link / target untouched.
UPDATE `smart_scripts` SET `action_type`=0,`action_param1`=0,`action_param2`=0,`action_param3`=0,`action_param4`=0,`action_param5`=0,`action_param6`=0,
  `comment`='Gordok Brew Chief - Out of Combat - (Say Line 1 removed, no creature_text 23696) (#963t)'
  WHERE `entryorguid`=-84714 AND `source_type`=0 AND `id`=0;
UPDATE `smart_scripts` SET `action_type`=0,`action_param1`=0,`action_param2`=0,`action_param3`=0,`action_param4`=0,`action_param5`=0,`action_param6`=0,
  `comment`='North Wind - On data set 0 2 - (Say text 2 removed, no text known) (#963t)'
  WHERE `entryorguid`=30474 AND `source_type`=0 AND `id`=8;
UPDATE `smart_scripts` SET `action_type`=0,`action_param1`=0,`action_param2`=0,`action_param3`=0,`action_param4`=0,`action_param5`=0,`action_param6`=0,
  `comment`='Caimas the Pit Master - On Aggro - (Say Line 4 removed, no text known) (#963t)'
  WHERE `entryorguid`=46276 AND `source_type`=0 AND `id`=1;
UPDATE `smart_scripts` SET `action_type`=0,`action_param1`=0,`action_param2`=0,`action_param3`=0,`action_param4`=0,`action_param5`=0,`action_param6`=0,
  `comment`='Ramkahen Prisoner - On Text Over 2 - (Say Line 3 removed, no text known) (#963t)'
  WHERE `entryorguid`=46425 AND `source_type`=0 AND `id`=11;
-- the escape (Waypoint Start) used to follow text-over of the removed group 3; follow group 2 instead
UPDATE `smart_scripts` SET `event_param1`=2,
  `comment`='Ramkahen Prisoner - On Text Over 2 - Waypoint Start (was text over 3, #963t)'
  WHERE `entryorguid`=46425 AND `source_type`=0 AND `id`=12;
UPDATE `smart_scripts` SET `action_type`=0,`action_param1`=0,`action_param2`=0,`action_param3`=0,`action_param4`=0,`action_param5`=0,`action_param6`=0,
  `comment`='Nodd Codejack - On Data Set 0 1 - (Say Line 2 removed, no text known) (#963t)'
  WHERE `entryorguid`=54615 AND `source_type`=0 AND `id`=0;
