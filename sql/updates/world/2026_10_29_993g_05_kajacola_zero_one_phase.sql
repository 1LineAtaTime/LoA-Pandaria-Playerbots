-- Kaja'Cola Zero-One chests (GO 202554, quest item 52483, 4 spawns near 1752,1916): spawned in phase 1 while every creature around
-- them (the late Lost Isles scene: 39193/39194/39195, 38515-38518, 38122) uses mask 16384. Same family as 2026_10_29_993g_03/04: a
-- player in the late Lost Isles phase cannot see a phase-1 chest. Given the neighbours' mask; Verified in the goblin walk (visible and lootable). Idempotent.
UPDATE `gameobject` SET `phaseMask` = 16384 WHERE `id` = 202554 AND `phaseMask` = 1;
