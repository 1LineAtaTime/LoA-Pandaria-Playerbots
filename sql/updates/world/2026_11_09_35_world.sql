-- The Hinterlands: the town and quest NPCs are PvP flagged, and Rualeth sheathes his weapon.
-- Sourced: TDB 4.4.2 and 1210 creature_template_addon PvPFlags 1 on every one of them (mop 0), and SheathState 0 for Rualeth 17223.
-- Revantusk Village: Gorkas 4314, Oran Snakewrithe 7825, Revantusk Watcher 14730, Lard 14731, Revantusk Drummer 14734, Smith Slagtree
-- 14737, Otho Moji'ko 14738, Mystic Yayo'jin 14739, Katoom the Angler 14740, Huntsman Markhor 14741, Renn'az 17598, Grognard 42464,
-- Malcom Fendelson 42612, Elder Torntusk 42613. Hiri'watha and Shadra'Alor: Apothecary Surlis 42896, Darkcleric Marnal 42898, Deathstalker
-- Lookout 43299. Alliance: Gryphon Master Talonaxe 5636, Rualeth 17223, Kerr Ironsight 43108, Dron Blastbrew 43109, Fraggar Thundermantle
-- 43156, Doran Steelwing 43157, Gilda Cloudcaller 43200, Wildhammer Lookout 43298, Anchorite Traska 46475, Tracker Yoro 46476.
-- The two patrolling watchers 191405/191406 have their own addon rows (PvPFlags 0), which win over the template's.
UPDATE `creature_template_addon` SET `PvPFlags`=1 WHERE `entry` IN (4314,7825,14730,14731,14734,14737,14738,14739,14740,14741,17598,42464,42612,42613,42896,42898,43299,5636,17223,43108,43109,43156,43157,43200,43298,46475,46476);
UPDATE `creature_addon` SET `PvPFlags`=1 WHERE `guid` IN (191405,191406);
UPDATE `creature_template_addon` SET `SheathState`=0 WHERE `entry`=17223;
