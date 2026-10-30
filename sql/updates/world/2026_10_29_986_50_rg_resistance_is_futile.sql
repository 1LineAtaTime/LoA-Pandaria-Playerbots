-- ported from PR 15 2026_10_27_66_world.sql (#986)
-- Resistance is Futile (27423) is given automatically on the way out of Emberstone Village (warcraft.wiki.gg). The Cataclysm
-- Classic world database lists area triggers 6256-6259 (around Emberstone's Belmont and the road south) as the quest's
-- objectives; starting the quest from them is ours. Here only Forward Commander Onslaught offered it, who stays behind at the
-- overrun Forward Command. Start it from those area triggers: it comes over the F.C.D. ("<The F.C.D. buzzes.> This is
-- Commander Lorna Crowley..."), shown in its window as the player leaves Emberstone with Fall Back! handed in. Belmont does
-- not offer it.
DELETE FROM `areatrigger_queststarter` WHERE `id` IN (6256, 6257, 6258, 6259);
INSERT INTO `areatrigger_queststarter` (`id`, `quest`) VALUES
(6256, 27423),
(6257, 27423),
(6258, 27423),
(6259, 27423);
DELETE FROM `creature_queststarter` WHERE `id` = 45312 AND `quest` = 27423;
