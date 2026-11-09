-- Revantusk Village: stop offering The Right Way (26987) and Moving Things Along (26988).
-- Sourced: warcraft.wiki.gg (removed in patch 5.0.4); the retail TDB 1210 flags both QUEST_FLAGS_DEPRECATED (16384), and their ender
-- Velonara (44660/44690) stands in the old Scarlet Monastery map that 5.4.8 no longer enters.
-- Ours: the ender rows stay, so a character who already holds one still has an ender (as Into the Scarlet Monastery in Hammerfall).
DELETE FROM `creature_queststarter` WHERE `id`=14741 AND `quest`=26987;
DELETE FROM `creature_queststarter` WHERE `id`=14739 AND `quest`=26988;
