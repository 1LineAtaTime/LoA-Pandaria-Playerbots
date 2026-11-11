-- Summoning Shadra (26558): only Darkcleric Marnal starts it.
-- Sourced: TDB 4.4.2 and 1210 (starter 42898 only; the Deathstalker Lookout 43299 is the ender) and warcraft.wiki.gg.
DELETE FROM `creature_queststarter` WHERE `id`=43299 AND `quest`=26558;
