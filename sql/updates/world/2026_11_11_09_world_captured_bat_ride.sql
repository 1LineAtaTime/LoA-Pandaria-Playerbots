-- Restore the missing personal bombing ride using existing SmartAI actions.
-- Route coordinates from the archived LoA world (35 points), corresponding to
-- the historical Gilneas bat route in sql/old/.../2022_02_26_03_world.sql.
-- Start only after boarding (the existing #1020 vehicle timing rule).
-- No quest completion/credit action: actual bombing kills must satisfy 24920.
UPDATE creature_template SET VehicleId=641,AIName='SmartAI',ScriptName='',
    npcflag=0,InhabitType=4,spell1=72246,spell5=72849 WHERE entry=38540;
-- The parked bat summons the player's vehicle rather than seating them on itself.
UPDATE creature_template SET VehicleId=0,npcflag=(npcflag | 16777216),AIName='',ScriptName='' WHERE entry=38615;
DELETE FROM npc_spellclick_spells WHERE npc_entry=38615;
INSERT INTO npc_spellclick_spells (npc_entry,spell_id,cast_flags,user_type) VALUES (38615,72472,1,0);
DELETE FROM conditions WHERE SourceTypeOrReferenceId=18 AND SourceGroup=38615 AND SourceEntry=72472;
INSERT INTO conditions (SourceTypeOrReferenceId,SourceGroup,SourceEntry,ConditionTypeOrReference,ConditionValue1,Comment)
VALUES (18,38615,72472,9,24920,'Captured bat: Slowing the Inevitable must be active');
DELETE FROM smart_scripts WHERE entryorguid=38540 AND source_type=0;
INSERT INTO smart_scripts (entryorguid,source_type,id,link,event_type,event_phase_mask,event_chance,event_flags,event_param1,event_param2,event_param3,event_param4,action_type,action_param1,action_param2,action_param3,action_param4,action_param5,action_param6,target_type,target_param1,target_param2,target_param3,target_x,target_y,target_z,target_o,comment) VALUES
(38540,0,0,1,27,0,100,0,0,0,0,0,60,1,0,0,0,0,0,1,0,0,0,0,0,0,0,'Bat: enable flight on boarding'),
(38540,0,1,2,61,0,100,0,0,0,0,0,154,6,40,0,0,0,0,1,0,0,0,0,0,0,0,'Bat: flight speed multiplier 4'),
(38540,0,2,0,61,0,100,0,0,0,0,0,53,1,38540,0,0,1000,0,7,0,0,0,0,0,0,0,'Bat: start bombing route after passenger seats'),
(38540,0,3,0,40,0,100,0,35,38540,0,0,41,1000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Bat: despawn and release rider at landing'),
(38540,0,4,0,8,0,100,0,72849,0,0,0,53,1,3854001,0,0,1000,0,12,16777215,0,0,0,0,0,0,'Bat: return button starts landing route without quest credit'),
(38540,0,5,0,40,0,100,0,8,3854001,0,0,41,1000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Bat: release rider after return landing'),
(38540,0,6,0,28,0,100,0,0,0,0,0,41,1000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Bat: clean up after rider exits');
DELETE FROM waypoints WHERE entry IN (38540,3854001);
INSERT INTO waypoints (entry,pointid,position_x,position_y,position_z,point_comment,orientation,delay) VALUES
(38540,1,-1667.73,1662.47,22.6075,'Captured Riding Bat',0,0),
(38540,2,-1665.67,1630.52,25.3575,'Captured Riding Bat',0,0),
(38540,3,-1657.57,1619.93,25.3575,'Captured Riding Bat',0,0),
(38540,4,-1637.11,1616.46,28.7186,'Captured Riding Bat',0,0),
(38540,5,-1621.25,1625.79,29.663,'Captured Riding Bat',0,0),
(38540,6,-1578.61,1633.11,32.0797,'Captured Riding Bat',0,0),
(38540,7,-1491.44,1632.97,32.0797,'Captured Riding Bat',0,0),
(38540,8,-1410.29,1638.58,37.3853,'Captured Riding Bat',0,0),
(38540,9,-1304.24,1650.36,64.7608,'Captured Riding Bat',0,0),
(38540,10,-1241.32,1747.77,53.7625,'Captured Riding Bat',0,0),
(38540,11,-1186.81,1708.94,53.7625,'Captured Riding Bat',0,0),
(38540,12,-1107.87,1646.18,53.7625,'Captured Riding Bat',0,0),
(38540,13,-1056.33,1608.78,53.7625,'Captured Riding Bat',0,0),
(38540,14,-984.186,1623.41,64.9014,'Captured Riding Bat',0,0),
(38540,15,-984.186,1623.41,64.9014,'Captured Riding Bat',0,0),
(38540,16,-960.571,1668.05,69.8459,'Captured Riding Bat',0,0),
(38540,17,-919.264,1708.54,90.707,'Captured Riding Bat',0,0),
(38540,18,-859.622,1700.19,90.707,'Captured Riding Bat',0,0),
(38540,19,-864.622,1626.05,90.707,'Captured Riding Bat',0,0),
(38540,20,-887.007,1510.62,90.707,'Captured Riding Bat',0,0),
(38540,21,-950.325,1506.31,82.3459,'Captured Riding Bat',0,0),
(38540,22,-1025.24,1428.93,74.5125,'Captured Riding Bat',0,0),
(38540,23,-1077.82,1401.01,58.4569,'Captured Riding Bat',0,0),
(38540,24,-1143.06,1407.38,51.0402,'Captured Riding Bat',0,0),
(38540,25,-1197.04,1471.48,47.3458,'Captured Riding Bat',0,0),
(38540,26,-1269.99,1510.73,47.3458,'Captured Riding Bat',0,0),
(38540,27,-1352.62,1566.73,47.3458,'Captured Riding Bat',0,0),
(38540,28,-1410.29,1638.58,37.3853,'Captured Riding Bat',0,0),
(38540,29,-1491.44,1632.97,32.0797,'Captured Riding Bat',0,0),
(38540,30,-1578.61,1633.11,32.0797,'Captured Riding Bat',0,0),
(38540,31,-1621.25,1625.79,29.663,'Captured Riding Bat',0,0),
(38540,32,-1637.11,1616.46,28.7186,'Captured Riding Bat',0,0),
(38540,33,-1657.57,1619.93,25.3575,'Captured Riding Bat',0,0),
(38540,34,-1665.67,1630.52,25.3575,'Captured Riding Bat',0,0),
(38540,35,-1667.73,1662.47,22.6075,'Captured Riding Bat',0,0),
(3854001,1,-1410.29,1638.58,37.3853,'Captured Riding Bat return',0,0),
(3854001,2,-1491.44,1632.97,32.0797,'Captured Riding Bat return',0,0),
(3854001,3,-1578.61,1633.11,32.0797,'Captured Riding Bat return',0,0),
(3854001,4,-1621.25,1625.79,29.663,'Captured Riding Bat return',0,0),
(3854001,5,-1637.11,1616.46,28.7186,'Captured Riding Bat return',0,0),
(3854001,6,-1657.57,1619.93,25.3575,'Captured Riding Bat return',0,0),
(3854001,7,-1665.67,1630.52,25.3575,'Captured Riding Bat return',0,0),
(3854001,8,-1667.73,1662.47,22.6075,'Captured Riding Bat return',0,0);
