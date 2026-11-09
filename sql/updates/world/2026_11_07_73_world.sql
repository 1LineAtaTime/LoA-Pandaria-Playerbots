-- Sniffed PvP flag on the Forsaken NPCs of Galen's Fall. Sourced: TDB 442 and 1210 creature_template_addon, PvPFlags 1. Galen 41944 and Alina 41949 are in the same state and are left to their own fix.
UPDATE `creature_template_addon` SET `PvPFlags` = 1 WHERE `entry` IN (4486,41966,42088,42090,44047,51819);
