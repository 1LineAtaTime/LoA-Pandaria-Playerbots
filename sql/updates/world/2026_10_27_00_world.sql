-- Silverpine Forest: Warlord Torok and Admiral Hatchet move to the Sepulcher with Sylvanas after Lordaeron. Hair of the Dog, Reinforcements
-- from Fenris and The Waters Run Red... are handed in to them "at the Sepulcher", but they were only spawned at the Forsaken Rear Guard.
-- Nothing sniffed them there: Torok stands at 45.8, 42.0 on the Silverpine map (a screenshot), with Hatchet on his left as at the
-- Rear Guard; heights from the terrain.
DELETE FROM `creature` WHERE `guid` IN (4046100, 4046101) AND `id` IN (44916, 44917);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`,
`equipment_id`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`) VALUES
(4046100, 44917, 0, 130, 228, 1, 1, 490.67, 1526.40, 131.08, 1.31, 0, 300, 0, 0, 0, 0, 0, 0), -- Warlord Torok
(4046101, 44916, 0, 130, 228, 1, 1, 488.74, 1526.92, 131.38, 1.62, 0, 300, 0, 0, 0, 0, 0, 0); -- Admiral Hatchet
