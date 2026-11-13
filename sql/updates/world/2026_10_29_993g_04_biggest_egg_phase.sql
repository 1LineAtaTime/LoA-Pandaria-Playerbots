-- 24744 "The Biggest Egg Ever": the chest (GO 201977, quest item 50261) is spawned in phase 1 only. A goblin on this quest is in the
-- Lost Isles phase 2048/4096/8192 group (the raptors / Raptor Egg chests use mask 14336), so the chest is invisible to the player
-- (goguid finds nothing, standing on its position) and the quest cannot be finished. Same family as the Raptor Egg chests
-- (2026_10_29_993g_03). Idempotent.
UPDATE `gameobject` SET `phaseMask` = 14336 WHERE `id` = 201977 AND `phaseMask` <> 14336;
