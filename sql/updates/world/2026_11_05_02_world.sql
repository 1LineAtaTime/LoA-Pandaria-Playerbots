-- Hillsbrad Foothills, Purgation Isle: the Stormpike Mountaineers 48482 and Bloodfang Sentries 48484 stand on their posts, as in
-- the Cataclysm Classic sniff (TDB 442 guids 323938-323954 and 324287-324299: all MovementType 0, wander 0, on the spots of
-- these spawns to 0.1 yd; that TDB marks 68000 other spawns as random movers, so its 0 is what the sniff saw). mop had every
-- one wandering 3 yd, so the sniffed pairs (2 yd apart) walked into each other. Wowhead pins agree (7 for the 8 Mountaineers,
-- 15 for the 18 Sentries: the pairs share a pin). The Stormpike Ram Riders 48483 keep their movement: Wowhead has 13 pins for
-- the 4 of them, so they do move on retail.
-- The landed Kasha 48471 (guid 312259, LoA's spawn: neither TDB has her, one Wowhead pin 6 yd off) waits for Orkus where Kasha's
-- flight in The Road to Purgation comes down and hands over to her: she stays put (ours).
UPDATE `creature` SET `MovementType` = 0, `wander_distance` = 0 WHERE `id` IN (48482, 48484) AND `guid` IN
(219124, 219125, 219129, 219130, 219131, 219132, 219137, 219138,
 219105, 219109, 219116, 219117, 219118, 219119, 219120, 219121, 219122, 219123, 219128, 219133, 219135, 219139, 219140, 219141, 219142, 219143);
UPDATE `creature` SET `MovementType` = 0, `wander_distance` = 0 WHERE `guid` = 312259 AND `id` = 48471;
