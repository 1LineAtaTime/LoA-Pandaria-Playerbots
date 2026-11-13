-- Eastern Plaguelands, Duskwing (Duskwing, Oh How I Hate Thee..., 27523) is tameable in Mists (Wowhead comments 2012-10-03
-- [+7], 2013-02-24 [+12], 2014-02-18 [+7]; TDB 12.1.0 type_flags 0x80000001). With it a tamed Duskwing can again be the
-- flare's target (85431 picks the nearest 11897), the retail quirk the same comments describe.
UPDATE `creature_template` SET `type_flags`=`type_flags`|1 WHERE `entry`=11897;
-- Landlocked Grouper 45664 (Frederick's Fish Fancy, 27450) skinned into level-85 Savage Leather and Deepsea Scale. Comments
-- give Heavy and Thick leather (Liri 2010-11-25 [+14], elvisdumbledore 2010-12-13 [+4], 618811 2011-04-29 [+3]); both TDBs
-- give it no SkinLootID, against those comments. Ours: the table of the level 41-42 beast Sawtooth Snapper 1087 (one
-- group: Heavy Leather 43 %, Heavy Hide 5 % x1-2, Thick Hide 5 %, Thick Leather the rest).
DELETE FROM `skinning_loot_template` WHERE `entry`=45664;
INSERT INTO `skinning_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(45664,4234,43.1005,1,1,1),(45664,4235,5,1,1,2),(45664,4304,0,1,1,1),(45664,8169,5,1,1,1);
