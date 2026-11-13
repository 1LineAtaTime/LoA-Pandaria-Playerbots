-- Align the published template30020 with this fork's six actual Spare Part chests.
-- The original release loot key is not referenced here. Keep other items/modes.
DELETE FROM `gameobject_loot_template` WHERE entry=30020 AND item=57764 AND lootmode+0=1
AND ChanceOrQuestChance=-100 AND groupid=0 AND mincountOrRef=1 AND maxcount=1
AND NOT EXISTS (SELECT 1 FROM gameobject_template WHERE type IN (3,25) AND data1=30020);
DELETE FROM `gameobject_loot_template` WHERE entry IN (203443,203964,203965,203966,203967,203968)
AND item=57764 AND lootmode+0=0 AND groupid=0 AND mincountOrRef=1 AND maxcount=1;
INSERT INTO `gameobject_loot_template` (`entry`,`item`,`ChanceOrQuestChance`,`lootmode`,`groupid`,`mincountOrRef`,`maxcount`) VALUES
(203443,57764,-100,1,0,1,1),(203964,57764,-100,1,0,1,1),(203965,57764,-100,1,0,1,1),
(203966,57764,-100,1,0,1,1),(203967,57764,-100,1,0,1,1),(203968,57764,-100,1,0,1,1)
ON DUPLICATE KEY UPDATE ChanceOrQuestChance=VALUES(ChanceOrQuestChance),groupid=0,mincountOrRef=1,maxcount=1;
