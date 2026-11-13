-- 24741 "Trading Up": the five Raptor Egg chests (GO 201974, quest item 50239) were spawned in phase 1 only. A goblin working on this
-- quest is in the Lost Isles phases 2048/4096/8192 (spell_area 68750 / 68481 ...), the Spiny Raptors that guard the nests are spawned
-- with phase mask 14336 (2048 + 4096 + 8192), so the player met the raptors but never saw an egg: goguid found no chest within 400 yd and
-- the objective (5 eggs) could not be finished. Give the chests the same phase mask as the raptors. Idempotent.
UPDATE `gameobject` SET `phaseMask` = 14336 WHERE `id` = 201974 AND `phaseMask` <> 14336;
