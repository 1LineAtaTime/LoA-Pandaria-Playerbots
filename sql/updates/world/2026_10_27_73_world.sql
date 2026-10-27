-- Ruins of Gilneas: the abominations (45345) at Dreadwatch Outpost and nearby lie in Permanent Feign Death (template aura, as on
-- retail) but were set to wander (random movement, 3 yards), so they got up and walked about. Keep them still.
UPDATE `creature` SET `MovementType` = 0, `wander_distance` = 0 WHERE `id` = 45345;
