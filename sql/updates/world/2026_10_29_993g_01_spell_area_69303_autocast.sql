-- #993 walker lane: 14238 "Infrared = Infradead". The SI:7 Assassins (36092, 73 spawns) are all in phase mask 4. Aura 69303
-- (Infrared Heat Focals) is the phase-4 aura, tied to quest 14238 incomplete in area 4782 by spell_area, but autocast was 0, so
-- Player::UpdateAreaDependentAuras never applied it: a player with the quest saw no assassins and the kill objective (10) could not
-- be finished. Idempotent.
UPDATE `spell_area` SET `autocast` = 1 WHERE `spell` = 69303 AND `area` = 4782 AND `quest_start` = 14238 AND `autocast` <> 1;
