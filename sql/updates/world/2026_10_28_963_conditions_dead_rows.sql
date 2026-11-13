-- Issue #963: conditions load errors, part 2: rows pointing at things that exist in neither this DB nor the reference DB.
--  * "ObjectEntryGuid condition has non existing creature template entry" (14 lines): CONDITION_OBJECT_ENTRY_GUID rows
--    for creatures 27754 (Trollgore), 31317, 33906, 33909 (Ulduar), 40684: none exist in creature_template of
--    world or the reference 5.4.8 world DB; the core already skips the rows. Deleted (14 rows).
--  * "SourceEntry 35431/35433 in condition table, does not exist in creature_template" (2 lines): spellclick conditions
--    (SourceType 18) for absent ToC vehicles. Deleted (2 rows).
--  * "SourceType 1, SourceEntry 4294966519/4294966558 ... item_template" (2 lines): loot conditions with negative
--    SourceEntry -777/-738 (read as unsigned by the core, never match). Deleted (2 rows).
-- EXPECTED AFTER: 0 lines for these types. No behaviour change (core already ignores every row).
-- LEFT: "Aura condition has non existing spell (23770)" x3 (Sayge's Carnie Buff, spell absent from the spell store; gossip
--   conditions 14/15 group 6186 skip that condition today). Needs the spell, not a row edit.
DELETE FROM `conditions` WHERE `ConditionTypeOrReference` = 31 AND `ConditionValue1` = 3 AND `ConditionValue2` IN (27754, 31317, 33906, 33909, 40684);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` IN (35431, 35433) AND `SourceEntry` = 66245;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 1 AND `SourceGroup` = 73666 AND `SourceEntry` IN (-777, -738);
