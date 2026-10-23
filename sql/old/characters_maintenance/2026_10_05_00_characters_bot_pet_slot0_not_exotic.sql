-- #202: 2026_10_04_00 put each owner's lowest-id pet into slot 0. When that pet is exotic
-- (world.creature_template.type_flags & 0x10000 = CREATURE_TYPEFLAGS_EXOTIC), only a Beast Mastery hunter can
-- call it (Pet::LoadPetFromDB -> CanTameExoticPets), so Call Pet 1 fails silently for the other specs.
-- Swap such a slot-0 pet with the owner's lowest-id NON-exotic pet in slots 1-4 when one exists; the current
-- pet is unchanged. Owners whose active pets are all exotic are left alone (same as a real non-BM hunter).
-- Run with the worldserver STOPPED (pet lists load at login). Re-runnable: a second run matches nothing.
-- Needs read access to the world DB (creature_template).
-- ROLLBACK: UPDATE character_pet SET slot = <old slot> WHERE id IN (<ids>) - take them from a backup.

DROP TEMPORARY TABLE IF EXISTS tmp_pet_swap;
CREATE TEMPORARY TABLE tmp_pet_swap AS
SELECT e.owner, e.id AS exotic_id,
       (SELECT n.id FROM character_pet n JOIN world.creature_template nt ON nt.entry = n.entry
         WHERE n.owner = e.owner AND n.slot BETWEEN 1 AND 4 AND (nt.type_flags & 0x10000) = 0
         ORDER BY n.id LIMIT 1) AS plain_id
FROM character_pet e JOIN world.creature_template et ON et.entry = e.entry
WHERE e.slot = 0 AND (et.type_flags & 0x10000) <> 0;
DELETE FROM tmp_pet_swap WHERE plain_id IS NULL;
ALTER TABLE tmp_pet_swap ADD COLUMN plain_slot TINYINT UNSIGNED NULL;
UPDATE tmp_pet_swap s JOIN character_pet n ON n.id = s.plain_id SET s.plain_slot = n.slot;

UPDATE character_pet p JOIN tmp_pet_swap s ON s.exotic_id = p.id SET p.slot = s.plain_slot;
UPDATE character_pet p JOIN tmp_pet_swap s ON s.plain_id = p.id SET p.slot = 0;
DROP TEMPORARY TABLE tmp_pet_swap;
