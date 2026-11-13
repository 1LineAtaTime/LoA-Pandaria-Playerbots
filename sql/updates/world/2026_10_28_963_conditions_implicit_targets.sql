-- Issue #963: conditions load errors, part 1: SourceType 13 (implicit target) SourceGroup effect masks.
-- WHAT: "spell N does not have implicit targets of types _AREA_/_CONE_/_NEARBY_ for effect E, SourceGroup needs
--   correction, ignoring" (54 log lines, 45 condition rows, 40 spell/mask pairs). The core already clears the bad
--   effect bits at load; rows whose whole mask is invalid (23 rows) are discarded. This file makes the DB say
--   exactly what the core uses: bad effect bits removed (22 rows, same behaviour), fully-invalid rows deleted (23 rows,
--   already dead). No behaviour change; derived from the log + effect data the core checked, no external source.
-- EXPECTED AFTER: 0 lines of this type.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 77782 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 77925 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 77932 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 77937 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 80199 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 80702 AND `SourceGroup` = 2;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 85123 AND `SourceGroup` = 3;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 86911 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 87517 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 92831 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 96931 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 109440 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 114379 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 116711 AND `SourceGroup` = 3;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 119841 AND `SourceGroup` = 3;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 122551 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 126642 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 136909 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 139848 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 143487 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 147369 AND `SourceGroup` = 1;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 147370 AND `SourceGroup` = 1;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 64558 AND `SourceGroup` = 7;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 68355 AND `SourceGroup` = 7;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 74670 AND `SourceGroup` = 3;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 78499 AND `SourceGroup` = 3;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 85555 AND `SourceGroup` = 3;
UPDATE `conditions` SET `SourceGroup` = 2 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 89008 AND `SourceGroup` = 3;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 89892 AND `SourceGroup` = 7;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 100342 AND `SourceGroup` = 7;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 100344 AND `SourceGroup` = 7;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 100345 AND `SourceGroup` = 7;
UPDATE `conditions` SET `SourceGroup` = 2 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 100555 AND `SourceGroup` = 3;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 126022 AND `SourceGroup` = 3;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 134789 AND `SourceGroup` = 3;
UPDATE `conditions` SET `SourceGroup` = 2 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 137433 AND `SourceGroup` = 3;
UPDATE `conditions` SET `SourceGroup` = 2 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 137442 AND `SourceGroup` = 3;
UPDATE `conditions` SET `SourceGroup` = 2 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 137443 AND `SourceGroup` = 3;
UPDATE `conditions` SET `SourceGroup` = 2 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 137444 AND `SourceGroup` = 3;
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 137967 AND `SourceGroup` = 7;

-- 60535: 21 rows used SourceGroup 0 ("has incorrect SourceGroup 0 (spell effectMask)", 21 lines). the reference DB conditions
-- carry the same spell at SourceGroup 1 (25 rows), AC at 1 (58). Set to effect 0 mask. Expected after: 0 lines (or
-- at most one implicit-target line for effect 0, which the log will show on next boot).
UPDATE `conditions` SET `SourceGroup` = 1 WHERE `SourceTypeOrReferenceId` = 13 AND `SourceEntry` = 60535 AND `SourceGroup` = 0;
