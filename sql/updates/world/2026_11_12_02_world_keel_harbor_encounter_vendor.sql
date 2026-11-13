-- Quest24681: the playable chassis must match its level12 parked counterpart.
-- Previously41599 was level1 with126 HP, despite the level12 launcher38150.
UPDATE creature_template v JOIN creature_template parked ON parked.entry=38150
SET v.minlevel=parked.minlevel,v.maxlevel=parked.maxlevel,
    v.Health_mod=parked.Health_mod,v.Armor_mod=parked.Armor_mod
WHERE v.entry=41599;

-- Three Wolfmaw Outriders cannot supply eight kills when all encounter spawns
-- wait two hours. Keep the authored spawn count; allow repeat waves instead.
-- Core corpse decay is added to this60-second delay.
UPDATE creature SET spawntimesecs=60,spawntimesecs_max=60
WHERE map=654 AND id IN (37916,37921,37938) AND (phaseMask & 1048576)<>0;

-- Marie Allen's stage-specific copies have the merchant flag and menu but no
-- inventory. Copy her existing general-goods catalogue, preserving custom rows.
INSERT INTO npc_vendor (entry,slot,item,maxcount,incrtime,ExtendedCost,type)
SELECT target.entry,stock.slot,stock.item,stock.maxcount,stock.incrtime,stock.ExtendedCost,stock.type
FROM creature_template target JOIN npc_vendor stock ON stock.entry=36779
WHERE target.entry IN (38783,43558) AND NOT EXISTS
 (SELECT 1 FROM npc_vendor existing WHERE existing.entry=target.entry
  AND existing.item=stock.item AND existing.ExtendedCost=stock.ExtendedCost AND existing.type=stock.type);
