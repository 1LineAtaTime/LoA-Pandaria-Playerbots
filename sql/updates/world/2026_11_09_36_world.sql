-- Fraggar Thundermantle (43156), Doran Steelwing (43157) and Anchorite Traska (46475) say their gossip texts again.
-- Sourced: TDB 4.4.2 and 1210 creature_template_gossip 11715, 11716 and 12127; the menus and their texts (16394, 16396 and 16660,
-- 17033) are already in mop. Doran's menu option for the Sharpbeak ride is not part of this; it comes with the ride.
UPDATE `creature_template` SET `gossip_menu_id`=11715 WHERE `entry`=43156;
UPDATE `creature_template` SET `gossip_menu_id`=11716 WHERE `entry`=43157;
UPDATE `creature_template` SET `gossip_menu_id`=12127 WHERE `entry`=46475;
