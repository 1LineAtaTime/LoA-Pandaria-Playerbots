-- #962 (2026-10-28): two corrupt spawn rows of Echo Isle Animal 40217 (an Echo Isles / Durotar crab, map 1) on map 870 with
-- garbage values: x 4.5, z -9.9e32, orientation 8.0 (a column shift). Creature::Create refused them at every boot
-- ("given coordinates ... are not valid"). The reference 5.4.8 world DB has no such rows (its 40217 spawns are all on map 1).
-- Idempotent. Undo: re-insert the rows from (not shipped)
DELETE FROM `creature_addon` WHERE `guid` IN (505704, 511897);
DELETE FROM `creature` WHERE `guid` IN (505704, 511897) AND `id` = 40217 AND `map` = 870;
