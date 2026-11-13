-- Sniffed PvP flag on the Hammerfall NPCs. Sourced: TDB 442 and 1210 creature_template_addon, PvPFlags 1 on all of these templates (mop 0).
-- Two Hammerfall Guardians (160040, 160117) have their own creature_addon row, which replaces the template addon in this core; ours: they get the template's flag too.
-- Gor'mul 42130 and the Hammerfall Guardian 51899 already have it.
UPDATE `creature_template_addon` SET `PvPFlags` = 1 WHERE `entry` IN (2621,2771,2772,2787,2818,2819,2820,2821,2851,4954,6574,9501,9555,9976,12920);
UPDATE `creature_addon` SET `PvPFlags` = 1 WHERE `guid` IN (160040,160117);
