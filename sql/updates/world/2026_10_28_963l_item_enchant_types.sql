-- #963 (2026-10-28, run v10: "Enchantment id #272/#273/#281/#283/#292/#293 used but it doesn't have records in
-- 'ItemRandomProperties.dbc'", ~35/h): suffix ids filed as type 0 (random property) - same defect 963b fixed for
-- 461/462/464. TrinityCore TDB 434.22011 item_enchantment_template (entry,type,ench,chance): 458 272/273, 460 281/283 and
-- 465 292/293 are type 1 (suffix); entry 463 has no 292/293 row at all.
-- Idempotent. Undo: type 0 for the moved rows; re-insert (463,292,?,0)/(463,293,?,0) from the 2026-10-28 backup.
UPDATE `item_enchantment_template` SET `type` = 1 WHERE `type` = 0 AND ((`entry` = 458 AND `ench` IN (272, 273)) OR (`entry` = 460 AND `ench` IN (281, 283)) OR (`entry` = 465 AND `ench` IN (292, 293)));
DELETE FROM `item_enchantment_template` WHERE `entry` = 463 AND `type` = 0 AND `ench` IN (292, 293);
-- 963l addendum (run v11: "Enchantment id #274"): a full comparison with TC434 found 5 more suffix ids filed as type 0 in
-- the same entries: 458/274, 460/280, 460/282, 465/294, 465/295 (TC434 type 1 only). Entry 413's type-0 rows (ids 14-139,
-- TC434 type 1) are left as they are: those ids exist in ItemRandomProperties.dbc and log nothing.
UPDATE `item_enchantment_template` SET `type` = 1 WHERE `type` = 0 AND ((`entry` = 458 AND `ench` = 274) OR (`entry` = 460 AND `ench` IN (280, 282)) OR (`entry` = 465 AND `ench` IN (294, 295)));
