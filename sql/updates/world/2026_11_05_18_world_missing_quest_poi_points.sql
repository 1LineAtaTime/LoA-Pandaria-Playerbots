-- Restore40 absent polygon points in five groups from SkyFire world26.002.
-- Match current map/area/objective metadata and preserve existing whole groups.
-- Item legacy index4 maps to item storage index0; objective identity is guarded.
CREATE TEMPORARY TABLE codex_missing_poi_seed LIKE quest_poi_points;
INSERT INTO codex_missing_poi_seed (questId,id,idx,x,y,Idx1,Idx2) VALUES
(3379,1,0,-7215,-1237,1,0),
(11078,5,0,2073,7330,5,0),
(11078,5,1,2099,7330,5,1),
(11078,5,2,2112,7336,5,2),
(11078,5,3,2125,7356,5,3),
(11078,5,4,2125,7382,5,4),
(11078,5,5,2112,7408,5,5),
(11078,5,6,2086,7421,5,6),
(11078,5,7,2060,7421,5,7),
(11078,5,8,2047,7421,5,8),
(11078,5,9,2034,7401,5,9),
(11078,5,10,2027,7369,5,10),
(11078,5,11,2034,7349,5,11),
(13205,6,0,406,-60,6,0),
(13809,1,0,8085,2595,1,0),
(13809,1,1,8026,2687,1,1),
(13836,1,0,5727,469,1,0),
(13836,1,1,5736,471,1,1),
(13836,1,2,5748,475,1,2),
(13836,1,3,5758,485,1,3),
(13836,1,4,5771,505,1,4),
(13836,1,5,5696,568,1,5),
(13836,1,6,5679,574,1,6),
(13836,1,7,5668,558,1,7),
(13836,1,8,5664,551,1,8),
(13836,1,9,5655,533,1,9),
(13836,1,10,5656,530,1,10),
(13836,1,11,5712,478,1,11),
(13836,1,12,5727,473,1,12),
(13836,1,13,5745,473,1,13),
(13836,1,14,5765,493,1,14),
(13836,1,15,5762,513,1,15),
(13836,1,16,5744,534,1,16),
(13836,1,17,5722,551,1,17),
(13836,1,18,5700,566,1,18),
(13836,1,19,5683,575,1,19),
(13836,1,20,5660,546,1,20),
(13836,1,21,5660,525,1,21),
(13836,1,22,5665,513,1,22),
(13836,1,23,5716,475,1,23);
CREATE TEMPORARY TABLE codex_existing_poi_groups AS SELECT DISTINCT questId,Idx1 FROM quest_poi_points;
INSERT IGNORE INTO quest_poi_points (questId,id,idx,x,y,Idx1,Idx2)
 SELECT s.questId,s.id,s.idx,s.x,s.y,s.Idx1,s.Idx2 FROM codex_missing_poi_seed s
 JOIN quest_poi p ON p.questId=s.questId AND p.Idx1=s.Idx1
 LEFT JOIN codex_existing_poi_groups e ON e.questId=s.questId AND e.Idx1=s.Idx1
 WHERE e.questId IS NULL AND ((p.`questId`=3379 AND p.`id`=1 AND p.`objIndex`=-1 AND p.`mapid`=0 AND p.`WorldMapAreaId`=28 AND p.`FloorId`=0 AND p.`unk3`=0 AND p.`unk4`=3 AND p.`Idx1`=1 AND p.`ObjectiveIndex`=-1 AND p.`QuestObjectiveId`=0 AND p.`Floor`=0 AND p.`Priority`=0 AND p.`Flags`=3)
 OR (p.`questId`=11078 AND p.`id`=5 AND p.`objIndex`=0 AND p.`mapid`=530 AND p.`WorldMapAreaId`=475 AND p.`FloorId`=0 AND p.`unk3`=0 AND p.`unk4`=3 AND p.`Idx1`=5 AND p.`ObjectiveIndex`=0 AND p.`QuestObjectiveId`=0 AND p.`Floor`=0 AND p.`Priority`=0 AND p.`Flags`=3 AND EXISTS(SELECT 1 FROM quest_objective o WHERE o.questId=p.questId AND o.id=259919 AND o.`index`=0 AND o.type=1 AND o.objectId=32732 AND o.amount=1))
 OR (p.`questId`=13205 AND p.`id`=6 AND p.`objIndex`=0 AND p.`mapid`=574 AND p.`WorldMapAreaId`=523 AND p.`FloorId`=1 AND p.`unk3`=0 AND p.`unk4`=3 AND p.`Idx1`=6 AND p.`ObjectiveIndex`=0 AND p.`QuestObjectiveId`=0 AND p.`Floor`=1 AND p.`Priority`=0 AND p.`Flags`=3 AND EXISTS(SELECT 1 FROM quest_objective o WHERE o.questId=p.questId AND o.id=264498 AND o.`index`=0 AND o.type=1 AND o.objectId=43513 AND o.amount=5))
 OR (p.`questId`=13809 AND p.`id`=1 AND p.`objIndex`=0 AND p.`mapid`=571 AND p.`WorldMapAreaId`=492 AND p.`FloorId`=0 AND p.`unk3`=0 AND p.`unk4`=7 AND p.`Idx1`=1 AND p.`ObjectiveIndex`=0 AND p.`QuestObjectiveId`=0 AND p.`Floor`=0 AND p.`Priority`=0 AND p.`Flags`=7)
 OR (p.`questId`=13836 AND p.`id`=1 AND p.`objIndex`=0 AND p.`mapid`=571 AND p.`WorldMapAreaId`=504 AND p.`FloorId`=1 AND p.`unk3`=0 AND p.`unk4`=7 AND p.`Idx1`=1 AND p.`ObjectiveIndex`=0 AND p.`QuestObjectiveId`=0 AND p.`Floor`=1 AND p.`Priority`=0 AND p.`Flags`=7 AND EXISTS(SELECT 1 FROM quest_objective o WHERE o.questId=p.questId AND o.id=258991 AND o.`index`=0 AND o.type=1 AND o.objectId=45323 AND o.amount=1)));
DROP TEMPORARY TABLE codex_missing_poi_seed;
DROP TEMPORARY TABLE codex_existing_poi_groups;
