-- Eastern Plaguelands, Tyr's Hand: nine of the eleven Scarlet Footlockers (179498, guids 40295-40303) came back 10 seconds
-- after being looted; the other two (71198, 71199) take 180 seconds.
-- Ours: 180 seconds for all eleven. No source gives the respawn or the number of footlockers (both TDBs sniffed one; the eleven
-- spots are kept). They stay unphased: lootable in every state of the town.
UPDATE `gameobject` SET `spawntimesecs` = 180 WHERE `id` = 179498 AND `guid` BETWEEN 40295 AND 40303 AND `areaId` = 2266;
