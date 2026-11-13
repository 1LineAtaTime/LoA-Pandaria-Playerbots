-- 24925 (Lost Isles): besides the wrong phase (2026_10_29_993g_06) the 26 Oomlot Shaman spawns (38644) carry spawnMask 0, which means
-- "not spawned in any difficulty": cansee finds no shaman at all within 600 yd of their camp, so the 5-kill objective cannot be finished
-- even for a player in the right phase. Their camp mates (Goblin Captives 38643, Oomlot 38574 / 38575) use spawnMask 1. The reference
-- 5.4.8 world DB has the same 0, so this is a gap that exists upstream. Idempotent.
UPDATE `creature` SET `spawnMask` = 1 WHERE `id` = 38644 AND `spawnMask` = 0;
