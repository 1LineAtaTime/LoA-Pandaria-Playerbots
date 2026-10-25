-- Practice Target (5674) beside Helena Atwood in the Undercity and the Archery Targets (5202) in Durotar were spawned with
-- random movement and wandered around. They are target dummies and stand still, like the other Archery Target spawns.
UPDATE `creature` SET `MovementType` = 0, `wander_distance` = 0 WHERE `guid` IN (192550, 255194, 255197, 255272, 255290);
