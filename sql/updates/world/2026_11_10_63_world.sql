-- Western Plaguelands: creatures that stood twice on one spot, and the Midsummer celebrants imported twice.
-- Sourced: the TDB 4.4.2 spawns of each entry, taken by box (map 0, x 500..3300, y -3250..-650; TDB 1210 has the same rows). A spawn goes
-- when it and another spawn map onto one TDB spawn, stand within min(8, max(5, 2 x wander distance)) yd of each other, both lie within
-- 20 yd of that TDB spawn, and mop has more spawns than the TDB within 20 yd of it. As in the Hinterlands, but the entry test runs per
-- family of entries that share spawn points in the sniffs (Diseased Wolf / Hulking Plaguebear, Venom Mist / Plague Lurker, the Gahrron's
-- Withering ghosts 1802/1804/4472): one TDB spot holds a wolf in one sniff and a plaguebear in mop. The keeper is the copy whose entry
-- matches the TDB spot, then the nearer one. 29 spawns go: 3 Diseased Wolves, 5 lurkers, 3 Whitetail Foxes, 2 Redpine Shamans, 10
-- ghosts, 4 Black Rats, a Flesh-Cobbled Ripper and a Hearthglen Drudge (pairs 0.7-5.5 yd apart). Only the rats and the drudge have an
-- addon (path 0); nothing else refers to them. Foxes and Hounds keeps 30 foxes for 4, Students of Krastinov 5 Rippers for 2.
-- The Midsummer spawns at Chillwind Camp and the Bulwark were imported twice on the exact same spots (event 1 both); the 37xxxx copy goes
-- and the 352xxx copy stays, as in Arathi and the Hinterlands.
-- Ours: the family rule and the keeper choice.
DELETE FROM `creature_addon` WHERE `guid` IN (212174,212254,214236,214238,214270);
DELETE FROM `creature` WHERE (`guid`,`id`) IN ((214234,1817),(214233,1817),(213788,1817),(212899,1824),(212858,1822),(214244,1824),(214227,1824),
(213983,1824),(213081,44474),(213077,44474),(213084,44474),(212512,44479),(212942,44479),(214135,1802),(214141,4472),(214142,1804),(212877,1802),
(214101,1804),(212832,1804),(214077,4472),(212841,4472),(212839,1804),(212988,1804),(214236,2110),(214270,2110),(214238,2110),(212174,2110),
(214146,44485),(212254,44439));
-- Midsummer: Midsummer Celebrant 16781 x8, Midsummer Bonfire 16592 x2, Fire Eater 25962, Flame Eater 25994, Bonfire Faction Bunny 26355
DELETE FROM `game_event_creature` WHERE `guid` IN (369955,370013,370248,370249,370351,370352,370353,370354,370712,370767,370768,370769,370770);
DELETE FROM `creature_addon` WHERE `guid` IN (369955,370013,370248,370249,370351,370352,370353,370354,370712,370767,370768,370769,370770);
DELETE FROM `creature` WHERE (`guid`,`id`) IN ((370013,16592),(370712,16592),(370354,16781),(370352,16781),(370351,16781),(370353,16781),
(370767,16781),(370768,16781),(370769,16781),(370770,16781),(369955,25962),(370249,25994),(370248,26355));
