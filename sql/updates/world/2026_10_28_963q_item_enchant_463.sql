-- #963 (2026-10-28, run v12: "Enchantment id #294 used but it doesn't have records in 'ItemRandomProperties.dbc'"): entry 463 still
-- had suffix ids 294/295 as type 0 (random property). TrinityCore TDB 434.22011 has neither row for entry 463 (its 463 suffixes
-- are other ids), the same case as 463/292-293 in 963l. A full check of every type-0 row against the 5.4.8 ItemRandomProperties.dbc
-- (2012 ids) leaves no other row outside the DBC, so this error type cannot recur from this table.
-- Idempotent. Undo: INSERT (463,294,21.6,0),(463,295,26,0).
DELETE FROM `item_enchantment_template` WHERE `entry` = 463 AND `type` = 0 AND `ench` IN (294, 295);
