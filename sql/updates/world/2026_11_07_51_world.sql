-- Dun Garok: Spirit pairs under 3 yards apart, inside their 3 yard wander, are one mob twice. Sourced: the TDBs have one mob at each of these places; for each pair the copy farther from
-- the TDB spot goes. Leaves 27 spirits (the TDBs have 20; the rest are not close doubles and stay). No guid here is in a pool, event, formation, addon or per-spawn script.
DELETE FROM `creature` WHERE `id`=49269 AND `guid` IN (217758,177090,177149,177152);
