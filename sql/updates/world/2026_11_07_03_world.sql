-- Gnome starting area: Irradiated (80653) is never removed.
-- New gnomes get the aura from playercreateinfo_cast_spell. It is infinite, can't be cancelled and persists through death, so
-- only the spell_area check can take it away (Player::UpdateAreaDependentAuras removes an aura whose spell has spell_area rows
-- when none of them fits). The row for New Tinkertown (area 5495) was deleted, so nothing removed the aura any more.
-- Restore the row without autocast: it only removes the aura once the player leaves the area or rewards Decontamination
-- (27635); it never puts the aura back on a player who skipped the chain. quest_end_status 9 = NONE | INCOMPLETE.
DELETE FROM `spell_area` WHERE `spell`=80653 AND `area`=5495;
INSERT INTO `spell_area` (`spell`,`area`,`quest_start`,`quest_end`,`aura_spell`,`racemask`,`gender`,`autocast`,`quest_start_status`,`quest_end_status`) VALUES
(80653,5495,0,27635,0,64,2,0,0,9);

-- Gnome Hunters were the only gnome class without the starting aura.
DELETE FROM `playercreateinfo_cast_spell` WHERE `raceMask`=64 AND `classMask`=4 AND `spell`=80653;
INSERT INTO `playercreateinfo_cast_spell` (`raceMask`,`classMask`,`spell`,`note`) VALUES
(64,4,80653,'Hunter - Gnome - Irradiated Aura');
