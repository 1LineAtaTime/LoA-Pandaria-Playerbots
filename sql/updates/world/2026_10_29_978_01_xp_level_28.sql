-- ported from PR 15 2026_10_27_04_world.sql (#978)
-- Level 28 needed 21,120 XP, a typo for 27,120: Patch 5.0.4 cut levels 10-39 to 80% of their Cataclysm values (33,900 at 28)
UPDATE `player_xp_for_level` SET `xp_for_next_level` = 27120 WHERE `lvl` = 28;
