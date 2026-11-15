-- Companion to2026_11_12_00: make its custom static ship definition explicit
-- for maintainers applying the repair to a world without the older gunship patch.
-- Preserve an existing authored template; no spawn GUIDs or player data changed.
INSERT INTO gameobject_template
 (entry,type,displayId,name,IconName,castBarCaption,unk1,size,data0,data1,AIName,ScriptName,VerifiedBuild)
SELECT 800100,5,8253,'Orc Gunship','','','',1,0,0,'','',0
WHERE NOT EXISTS (SELECT 1 FROM gameobject_template WHERE entry=800100);
