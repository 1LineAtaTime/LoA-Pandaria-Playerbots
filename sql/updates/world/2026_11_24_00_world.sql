-- Kaja'mite Deposit decoration (195489), Kezan: the decor crystal is visible on top of the bombable deposit (195488) during
-- Liberate the Kaja'mite (14124), so the deposit looks as if it never goes away after it is blown up.
-- The decoration spawns use phaseMask 223 (1+2+4+8+16+64+128). Kezan phase definition 5 (zone 4737) is mask 96 (32+64) while
-- quest 14120 is rewarded and 14125 is not taken; the deposit 195488 is in phase 32. Bit 64 is shared, so a player on 14124 sees
-- both objects. The decoration is now 159 (223 without bit 64): that player sees only the bombable deposit, and the decoration is
-- shown as before in phases 1, 2, 4, 8, 16 and 128.
-- Rollback:
--   UPDATE `gameobject` SET `phaseMask` = 223 WHERE `id` = 195489 AND `phaseMask` = 159;

UPDATE `gameobject` SET `phaseMask` = 159 WHERE `id` = 195489 AND `phaseMask` = 223;
