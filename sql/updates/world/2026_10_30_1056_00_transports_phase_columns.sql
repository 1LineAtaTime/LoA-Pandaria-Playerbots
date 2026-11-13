-- Phased moving transports: `transports` gets TrinityCore master's phase columns.
--
-- phaseUseFlags: 1 = always visible, 2 = inverse (seen by everyone except the holders of the phase); phaseid: a
-- Phase.dbc id; phasegroup: a PhaseXPhaseGroup.dbc group (phaseid wins when both are set). TransportMgr reads them in
-- SpawnContinentTransports: a moving transport with a phase is sent only to the players holding it (inverse: to all
-- the others), and its crew is seen with it. All rows default to 0 = no phase, sent to every player on the map, as
-- before. The worldserver that reads these columns needs this file applied first (it selects them).
--
-- Backup (state before this file): `transports` (guid, entry, name).
-- Undo (with the previous worldserver binary):
--   ALTER TABLE `transports` DROP COLUMN `phasegroup`, DROP COLUMN `phaseid`, DROP COLUMN `phaseUseFlags`;
-- Re-runnable: the ADD COLUMN is guarded by information_schema (PREPARE).

SET @has := (SELECT COUNT(*) FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE()
    AND TABLE_NAME = 'transports' AND COLUMN_NAME = 'phaseUseFlags');
SET @ddl := IF(@has = 0, 'ALTER TABLE `transports` ADD COLUMN `phaseUseFlags` tinyint unsigned NOT NULL DEFAULT 0 AFTER `name`, ADD COLUMN `phaseid` int unsigned NOT NULL DEFAULT 0 AFTER `phaseUseFlags`, ADD COLUMN `phasegroup` int unsigned NOT NULL DEFAULT 0 AFTER `phaseid`', 'DO 0');
PREPARE stmt FROM @ddl;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
