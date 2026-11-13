-- Original mop_world compatibility with the current LoA executable.
-- Preserve legacy equipment variants: zero is reserved by the loader, so move
-- each zero variant above that creature's existing positive IDs. No rows deleted.
-- Run offline after backing up creature_equip_template. Re-running is a no-op.
UPDATE creature_equip_template a
JOIN (SELECT entry,MAX(id)+1 AS next_id FROM creature_equip_template GROUP BY entry HAVING MAX(id)<255) b ON a.entry=b.entry
SET a.id=b.next_id WHERE a.id=0;

