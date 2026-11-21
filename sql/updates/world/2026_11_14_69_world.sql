-- Eastern Plaguelands, The Battle of Darrowshire (27390): the data of the event built in zone_eastern_plaguelands_darrowshire.cpp.
-- Ships with the script: before it, placing the Relic Bundle did nothing and nothing gave "Accept Redpath's Forgiveness", so 27390 and
-- Hidden Treasures 27391 behind it could not be done.
--
-- Sourced:
--  * Factions as both TDBs: the heroes and the defenders (Captain Redpath 10937, Davil Lightfire 10944, Silver Hand Disciple 10949,
--    Redpath Militia 10950: 250; Darrowshire Defender 10948: 495) are friendly to players and fight the Scourge (974: Redpath the
--    Corrupted 10938, Horgus 10946, Betrayer 10947, Marauding Corpse 10951, Skeleton 10952, Servant of Horgus 10953, Bloodletter
--    10954). mop had the heroes on 21 (hostile to players) and both sides on 1080 / 14, which are neutral to each other, so nobody
--    would have fought (hostile.py: 250/495 against 974 WILL FIGHT both ways; 974 hostile to both player factions). Marduk 10939: 14;
--    Davil Crokford 10945: 35.
--  * Joseph Redpath 10936 and Marduk 10939 unit_flags 768 (IMMUNE_TO_PC | IMMUNE_TO_NPC), both TDBs.
--  * DamageModifier 7.5 on the heroes Captain Redpath 10937, Davil Lightfire 10944 and the Silver Hand Disciples 10949 (both TDBs).
--  * No XP for any actor but the Marauding Skeleton 10952 (both TDBs flags_extra 64 entry by entry; 10953/10954 retail; comments:
--    Bloodletters give no XP, OLD 2008-05-16 [+9]).
--  * Horgus 10946 drops nothing (both TDBs LootID 0; comment 2010-11-24 reply [+6]: the battle's undead give no loot). mop had a
--    classic loot table on him.
--  * The static Joseph Redpath (guid 373694, a local addition, in neither TDB) goes: Joseph's spirit appears at the well after the
--    victory (wiki; Joseph's single MoP Classic pin; comment 2010-11-24 [+70]).
--  * Texts by broadcast id (wiki quotes them): 45680 start (raid emote), 45681 Davil, 45682 Horgus, 45683 Captain Redpath (raid
--    emotes), 7366 Davil defeated (raid emote), 7345 Horgus' second line, 7347-7354 the Militia (classic speakers), 7398 Joseph
--    "Hahah!". mop filed 45681/45683/7366 on Davil Crokford 10945, who is not there during the battle: copies go on the speakers.
-- Ours, balance (both TDBs: 7.5 on all six elites, Horgus HealthModifier 5): Horgus 10946 DamageModifier 1.5 and Health_mod 2.5,
-- Redpath the Corrupted 10938 DamageModifier 4, Bloodletter 10954 2.5. This core multiplies classlevelstats damage per second by
-- the modifier (Creature::SelectLevel, Creature::UpdateDamagePhysical). Counting armor, a 0.87 hit rate, Horgus' Thrash, his
-- Ravenous Claw leech (about 29 per second off Davil and onto Horgus) and Davil's Hammer of Justice, at the TDB values Davil died to
-- Horgus in about 20 s and needed 63 s alone; now Horgus (3962 HP) takes Davil 69 s to kill him and dies in about 31 s to Davil
-- alone, 14-19 s with a Disciple or a level-41 player. Horgus' Unholy Aura (50 shadow per second to enemies within 30 yd) lands only
-- after an evade (the script puts him in combat on arrival; SmartAI out-of-combat timers stop in combat): Davil then lasts 36 s.
-- A Bloodletter costs Captain Redpath about 62 per second for 22 s; over his stage (one every 60 s, a trash mob between) he loses
-- about 4950 of 7925 health with no heals; a lone player survives a Bloodletter 48 s. The sources: players who stood on the hill
-- and "did very little" won (2012-07-07 [+4], 2012-11-23 [+2], 5.1), the heroes "killed things pretty fast", Davil "is not going
-- to die in one hit, let them duke it out". These entries exist only for this event (no spawn, no other summoner).
-- Ours also: the speakers of the raid emotes; TextRange 1 (area) on every raid emote so the whole town square sees them; groups 70-71.
DELETE FROM `creature_addon` WHERE `guid`=373694;
DELETE FROM `creature` WHERE `guid`=373694 AND `id`=10936;

UPDATE `creature_template` SET `faction`=250 WHERE `entry` IN (10937,10944,10949,10950);
UPDATE `creature_template` SET `faction`=495 WHERE `entry`=10948;
UPDATE `creature_template` SET `faction`=974 WHERE `entry` IN (10938,10946,10947,10951,10952,10953,10954);
UPDATE `creature_template` SET `faction`=14 WHERE `entry`=10939;
UPDATE `creature_template` SET `faction`=35 WHERE `entry` IN (10936,10945);
UPDATE `creature_template` SET `unit_flags`=`unit_flags`|768 WHERE `entry` IN (10936,10939);
UPDATE `creature_template` SET `DamageModifier`=7.5 WHERE `entry` IN (10937,10944,10949);
UPDATE `creature_template` SET `DamageModifier`=4 WHERE `entry`=10938;
UPDATE `creature_template` SET `DamageModifier`=1.5, `Health_mod`=2.5 WHERE `entry`=10946;
UPDATE `creature_template` SET `DamageModifier`=2.5 WHERE `entry`=10954;
UPDATE `creature_template` SET `flags_extra`=`flags_extra`|64 WHERE `entry` IN (10936,10937,10938,10939,10944,10945,10946,10947,10948,10949,10950,10951,10953,10954);
UPDATE `creature_template` SET `lootid`=0 WHERE `entry`=10946;
DELETE FROM `creature_loot_template` WHERE `entry`=10946;

UPDATE `creature_template` SET `ScriptName`='npc_darrowshire_joseph_redpath' WHERE `entry`=10936;
UPDATE `gameobject_template` SET `ScriptName`='go_darrowshire_relic_bundle' WHERE `entry`=177526;

UPDATE `creature_text` SET `TextRange`=1 WHERE `CreatureID`=10938 AND `GroupID`=1 AND `BroadcastTextId`=45684;
UPDATE `creature_text` SET `TextRange`=1 WHERE `CreatureID`=10948 AND `GroupID`=4 AND `BroadcastTextId`=45685;
DELETE FROM `creature_text` WHERE (`CreatureID` IN (10936,10937,10944,10946,10948) AND `GroupID` IN (70,71)) OR `CreatureID`=10950;
INSERT INTO `creature_text` (`CreatureID`,`GroupID`,`ID`,`Text`,`Type`,`Language`,`Probability`,`Emote`,`Duration`,`Sound`,`BroadcastTextId`,`TextRange`,`comment`) VALUES
(10936,70,0,'Hahah!',12,0,100,0,0,0,7398,0,'Joseph Redpath - reunion with Pamela'),
(10937,70,0,'Captain Redpath has appeared on the battlefield!  Protect him!',41,0,100,0,0,0,45683,1,'Captain Redpath - raid emote on arrival'),
(10944,70,0,'Protect Davil Lightfire!',41,0,100,0,0,0,45681,1,'Davil Lightfire - raid emote on arrival'),
(10944,71,0,'Davil Lightfire is defeated!  Darrowshire is lost!',41,0,100,0,0,0,7366,1,'Davil Lightfire - raid emote on death before Horgus (battle failed)'),
(10946,70,0,'Horgus the Ravager has appeared!  Kill him quickly!',41,0,100,0,0,0,45682,1,'Horgus the Ravager - raid emote on arrival'),
(10946,71,0,'Now is the time, Davil.  Now is the time of your death!',14,0,100,0,0,0,7345,0,'Horgus the Ravager - second line (timing ours)'),
(10948,70,0,'Help Joseph Carlin''s troops defend Darrowshire!',41,0,100,0,0,0,45680,1,'Darrowshire Defender - raid emote at the start'),
(10950,0,0,'For Darrowshire!',12,0,100,0,0,0,7347,0,'Redpath Militia - on arrival'),
(10950,0,1,'Our captain will save us!',12,0,100,0,0,0,7348,0,'Redpath Militia - on arrival'),
(10950,0,2,'Fight!  Fight for your families!',12,0,100,0,0,0,7349,0,'Redpath Militia - on arrival'),
(10950,0,3,'The Scourge will not prevail!',12,0,100,0,0,0,7350,0,'Redpath Militia - on arrival'),
(10950,0,4,'Do not give up!  Shed your blood for Darrowshire!',12,0,100,0,0,0,7351,0,'Redpath Militia - on arrival'),
(10950,0,5,'Fight, and we will win this day!',12,0,100,0,0,0,7352,0,'Redpath Militia - on arrival'),
(10950,0,6,'Fight to your last breath, brothers and sisters!',12,0,100,0,0,0,7353,0,'Redpath Militia - on arrival'),
(10950,0,7,'Go back to your Lich King, foul creatures!',12,0,100,0,0,0,7354,0,'Redpath Militia - on arrival');
