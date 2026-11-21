-- Local Worgen starting-zone repair, paired with the native Bloodfang handler.
-- The three higher GUIDs overlap a retained same-entry/same-phase spawn within
-- one yard and have no addon, event, pool, formation or linked-respawn references.
DELETE c FROM `creature` c JOIN `creature` kept ON kept.guid=220893 AND kept.id=35118 AND kept.map=654 AND kept.phaseMask=2
WHERE c.guid=221504 AND c.id=35118 AND c.map=654 AND c.phaseMask=2
AND ABS(c.position_x+1685.2)<0.02 AND ABS(c.position_y-1381.61)<0.02 AND ABS(c.position_z-52.3705)<0.02;
DELETE c FROM `creature` c JOIN `creature` kept ON kept.guid=221482 AND kept.id=35118 AND kept.map=654 AND kept.phaseMask=2
WHERE c.guid=221484 AND c.id=35118 AND c.map=654 AND c.phaseMask=2
AND ABS(c.position_x+1662.26)<0.02 AND ABS(c.position_y-1344.72)<0.02 AND ABS(c.position_z-15.2599)<0.02;
DELETE c FROM `creature` c JOIN `creature` kept ON kept.guid=911724 AND kept.id=35229 AND kept.map=654 AND kept.phaseMask=8
WHERE c.guid=911770 AND c.id=35229 AND c.map=654 AND c.phaseMask=8
AND ABS(c.position_x+1508.32)<0.02 AND ABS(c.position_y-1626.87)<0.02 AND ABS(c.position_z-20.4855)<0.02;
