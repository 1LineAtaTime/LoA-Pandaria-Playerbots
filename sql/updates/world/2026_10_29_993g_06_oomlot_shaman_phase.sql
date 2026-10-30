-- 24925 (Lost Isles, after the Town-In-A-Box defence): the 26 Oomlot Shamans (38644) are spawned in phase 1 while their camp mates (Oomlot
-- 38643 / 38574 / 38575) use 12288 (4096 + 8192) and the player is in the 4096 phase (spell_area 72157). The shamans are invisible to the
-- player (cansee: samephase=0 within 150 yd of their camp), so the 5-kill objective cannot be finished. Same family as
-- 2026_10_29_993g_03/04/05. Idempotent.
UPDATE `creature` SET `phaseMask` = 12288 WHERE `id` = 38644 AND `phaseMask` = 1;
