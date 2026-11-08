-- Into the Scarlet Monastery (26986): retired. Sourced: warcraft.wiki.gg "removed from retail World of Warcraft in patch 5.0.4"; Wowhead MoP Classic marks it obsolete ("cannot be obtained or completed").
-- Doctor Gregory Victor 12920 no longer offers it. Ours: Velonara 44637 stays its ender, so a character who already holds it keeps an ender.
DELETE FROM `creature_queststarter` WHERE `id` = 12920 AND `quest` = 26986;
