-- #969 (2026-10-28, private mirror of a public BattlePay report): every store token item (110001-110019, 110100, 110101)
-- had RequiredLevel = 32, inherited from another 5.4.8 world DB (a second 5.4.8 world DB has the same value). CanUseItem
-- refused them below level 32 (SMSG_INVENTORY_CHANGE_FAILURE, CANT_EQUIP_LEVEL_I), so the "Level up to 90" token and the
-- service tokens could not be used by the low-level characters they are for (the reporter's "level 32 items").
-- Verified with a headless client on the test server: below 32 every item is refused; at 32+ every script works.
-- Idempotent. Undo: SET `RequiredLevel` = 32 for the same entries.
UPDATE `item_template` SET `RequiredLevel` = 0 WHERE (`entry` BETWEEN 110001 AND 110019 OR `entry` IN (110100, 110101)) AND `RequiredLevel` = 32;
