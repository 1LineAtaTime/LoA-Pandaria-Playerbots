-- The Howling Oak28517 ends at Genn48736, not innkeeper Gwen48735.
-- The preserved original world has no Darnassus Genn spawn. Restore a stationary
-- questgiver beside Gwen on her measured floor. This is a local placement,
-- not a claim to reproduce the complete Tyrande/Genn cinematic scene.
-- Preserve any existing/custom Darnassus Genn rather than duplicating him.
INSERT INTO creature
 (id,map,zoneId,areaId,spawnMask,phaseMask,position_x,position_y,position_z,
 orientation,spawntimesecs,spawndist,MovementType)
SELECT 48736,g.map,g.zoneId,g.areaId,1,1,g.position_x+3.0,g.position_y,
 g.position_z,4.71,120,0,0
FROM creature g
WHERE g.id=48735 AND g.map=1
 AND NOT EXISTS(SELECT 1 FROM creature existing WHERE existing.id=48736 AND existing.map=1)
ORDER BY g.guid LIMIT 1;

-- Tarien52644 is a jewelcrafting supplier with an empty vendor table.
-- Laida52584 is the corresponding Alliance supplier; retain every item,
-- limited-stock timer and ExtendedCost. Add missing keys only; never replace
-- custom Tarien stock. All92 source entries resolve in this world's item data.
INSERT INTO npc_vendor(entry,slot,item,maxcount,incrtime,ExtendedCost,type)
SELECT 52644,v.slot,v.item,v.maxcount,v.incrtime,v.ExtendedCost,v.type
FROM npc_vendor v JOIN item_template i ON i.entry=v.item
WHERE v.entry=52584 AND v.type=1
 AND NOT EXISTS(SELECT 1 FROM npc_vendor t WHERE t.entry=52644
  AND t.item=v.item AND t.ExtendedCost=v.ExtendedCost AND t.type=v.type);
