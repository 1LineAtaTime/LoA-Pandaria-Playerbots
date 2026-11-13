-- Dark Ranger Alina 41949's SheathState was 3, out of range (0-2). Galen 41944, Alina and Gor'mul 42130 lacked the PvP flag.
-- Sourced: TDB 442 creature_template_addon.
UPDATE `creature_template_addon` SET `SheathState`=2,`PvPFlags`=1 WHERE `entry`=41949;
UPDATE `creature_template_addon` SET `PvPFlags`=1 WHERE `entry` IN (41944,42130);
