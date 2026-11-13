-- Repair legacy spawn difficulty bits using installed MapDifficulty.dbc.
-- Normal/heroic Slave Pens use bits1/2; Ulduar10/25 use bits3/4.
-- Fall of Shan Bu has the native DBCStores.cpp normal-scenario override(bit12).
-- Drop only unsupported arena/flex bits; preserve all supported authored modes.
UPDATE creature SET spawnMask=1 WHERE guid=450066 AND id=65209 AND map=998 AND spawnMask=3;
UPDATE creature SET spawnMask=1 WHERE guid=450065 AND id=65209 AND map=998 AND spawnMask=3;
UPDATE creature SET spawnMask=1 WHERE guid=450064 AND id=65209 AND map=998 AND spawnMask=3;
UPDATE creature SET spawnMask=1 WHERE guid=450063 AND id=65209 AND map=998 AND spawnMask=3;
UPDATE creature SET spawnMask=1 WHERE guid=450062 AND id=65209 AND map=998 AND spawnMask=3;
UPDATE creature SET spawnMask=1 WHERE guid=450061 AND id=65209 AND map=998 AND spawnMask=3;
UPDATE creature SET spawnMask=1 WHERE guid=450060 AND id=65209 AND map=998 AND spawnMask=3;
UPDATE creature SET spawnMask=2 WHERE guid=914916 AND id=61780 AND map=389 AND spawnMask=3;
UPDATE creature SET spawnMask=4096 WHERE guid=549710 AND id=65753 AND map=1126 AND spawnMask=1;
UPDATE creature SET spawnMask=120 WHERE guid=582637 AND id=64526 AND map=1008 AND spawnMask=16504;
UPDATE creature SET spawnMask=6 WHERE guid=582465 AND id=26190 AND map=547 AND spawnMask=3;
UPDATE creature SET spawnMask=6 WHERE guid=582464 AND id=26190 AND map=547 AND spawnMask=3;
UPDATE creature SET spawnMask=6 WHERE guid=582463 AND id=26190 AND map=547 AND spawnMask=3;
UPDATE creature SET spawnMask=6 WHERE guid=582462 AND id=26190 AND map=547 AND spawnMask=3;
UPDATE creature SET spawnMask=6 WHERE guid=582461 AND id=25740 AND map=547 AND spawnMask=3;
UPDATE creature SET spawnMask=6 WHERE guid=582468 AND id=25697 AND map=547 AND spawnMask=3;
UPDATE gameobject SET spawnMask=24 WHERE guid=547515 AND id=194635 AND map=603 AND spawnMask=3;
