-- Client18414 crash: generic(type5)800100/display8253 is a WMO, not an M2.
-- CGGameObject's model getter excludes map objects(type14) from its M2 path.
-- Keep the static geometry and server collision; no transport animation needed.
UPDATE gameobject_template SET type=14
WHERE entry=800100 AND displayId=8253 AND type=5;

-- These are scripted passenger rides, not combat vehicles. Keep the empty
-- combat action bar and existing flight protections; correct level1/42-HP data.
UPDATE creature_template SET minlevel=12,maxlevel=12,Health_mod=3
WHERE entry IN (43751,43713) AND minlevel=1 AND maxlevel=1;
