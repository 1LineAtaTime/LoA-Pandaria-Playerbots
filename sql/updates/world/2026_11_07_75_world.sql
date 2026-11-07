-- Boulderfist Hall: Enforcer pairs within 2.1 yd, inside their 3 yard wander, are one mob twice from two sniff batches. For each pair the copy farther from the TDB spot goes, as for the Dun Garok Spirits.
-- Leaves 18 enforcers (TDB 15; the rest are not close doubles and stay). No guid here is in a pool, event, formation, addon, linked respawn or per-spawn script.
-- The Boulderfist Ogres 2562 (34 here, TDB 17) are left alone: their pairs are up to 13 yd apart around one TDB spot, which is not clear enough.
DELETE FROM `creature` WHERE `id` = 2564 AND `guid` IN (160664,160665,160724,217913,218733,218735,218766,218768,218771,218776);
