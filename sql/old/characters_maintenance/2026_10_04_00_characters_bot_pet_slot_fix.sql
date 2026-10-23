-- #202: BotFactory::InitPet saved hunter pets with slot 255 (never registered in an active slot), so Call Pet 1 failed
-- and "pet dead" looped Revive Pet. Re-slot such pets: for each owner that has NO pet in an active slot (0-4) and
-- NO current pet, move their lowest-id slot-255 hunter pet to slot 0 and make it current.
-- Run with the worldserver STOPPED (pet lists are loaded at login). Idempotent: a second run matches nothing.
-- ROLLBACK (only for the rows this touched, identify them first from a backup): UPDATE character_pet SET slot = 255 WHERE id IN (<ids>);
--   DELETE FROM character_pet_current WHERE owner IN (<owners>);

REPLACE INTO character_pet_current (owner, pet_id)
SELECT m.owner, m.id
FROM (SELECT owner, MIN(id) AS id FROM character_pet WHERE slot = 255 AND PetType = 1 GROUP BY owner) m
LEFT JOIN (SELECT DISTINCT owner FROM character_pet WHERE slot < 5) a ON a.owner = m.owner
LEFT JOIN character_pet_current c ON c.owner = m.owner
WHERE a.owner IS NULL AND c.owner IS NULL;

UPDATE character_pet cp
JOIN character_pet_current c ON c.owner = cp.owner AND c.pet_id = cp.id
SET cp.slot = 0
WHERE cp.slot = 255 AND cp.PetType = 1;
