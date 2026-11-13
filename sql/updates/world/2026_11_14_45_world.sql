-- Eastern Plaguelands: four rares are no longer up twice at once. Each had one spawn outside its pool, the single spot both
-- TDBs have (a sniff import added it beside the older pooled alternatives): it joins the pool (chance 0 = equal share, as
-- the existing members). Zul'Brin Warpbranch's two spawns (95 yd apart) get a pool of their own.
-- Hed'mush the Rotting (pool 1309) is the staging commit's (his tower points go there).
-- Three rares get the faction both TDBs give them instead of 14: Death Knight Soulbearer 10818 and Nerubian Overseer 16184
-- 16, Deathspeaker Selendre 10827 21. All hostile to players either way; it decides which neighbours come to help.
DELETE FROM `pool_creature` WHERE `guid` IN (172870,174838,318950,19005,318951);
DELETE FROM `pool_template` WHERE `entry`=3129600;
INSERT INTO `pool_template` (`entry`,`max_limit`,`description`) VALUES (3129600,1,'Zul''Brin Warpbranch (10823), Eastern Plaguelands');
INSERT INTO `pool_creature` (`guid`,`pool_entry`,`chance`,`description`) VALUES
(172870,318,0,'Duggan Wildhammer'),
(174838,1310,0,'Death-Hunter Hawkspear'),
(318950,1312,0,'Lord Darkscythe'),
(19005,3129600,0,'Zul''Brin Warpbranch'),
(318951,3129600,0,'Zul''Brin Warpbranch');
UPDATE `creature_template` SET `faction`=16 WHERE `entry` IN (10818,16184);
UPDATE `creature_template` SET `faction`=21 WHERE `entry`=10827;
