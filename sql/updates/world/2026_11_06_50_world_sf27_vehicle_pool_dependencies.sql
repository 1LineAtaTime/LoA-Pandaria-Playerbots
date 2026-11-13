-- Canonical release27 vehicle dependencies absent from the published dungeon SQL.
-- The three heroic templates use vehicle seats1389/1390 and Ride Vehicle46598.
INSERT INTO npc_spellclick_spells (npc_entry,spell_id,cast_flags,user_type)
SELECT entry,46598,1,0 FROM creature_template
WHERE entry IN (48803,48804,48913) AND VehicleId IN (1389,1390)
ON DUPLICATE KEY UPDATE cast_flags=VALUES(cast_flags),user_type=VALUES(user_type);

-- Importing mapped replacement pools moved the last members out of these roots.
-- Remove only empty, unreferenced definitions; never remove members or parents.
DELETE p FROM pool_template p
WHERE p.entry IN (173,14176)
AND NOT EXISTS (SELECT 1 FROM pool_creature c WHERE c.pool_entry=p.entry)
AND NOT EXISTS (SELECT 1 FROM pool_gameobject g WHERE g.pool_entry=p.entry)
AND NOT EXISTS (SELECT 1 FROM pool_pool c WHERE c.mother_pool=p.entry OR c.pool_id=p.entry);
