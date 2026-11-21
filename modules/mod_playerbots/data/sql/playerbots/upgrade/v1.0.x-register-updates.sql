-- Run once on the playerbots database, and only when it was installed from a v1.0.0 to v1.0.4
-- database zip, before the first start with Updates.EnableDatabases including 8 (playerbots).
--
-- Those databases already contain these thirteen updates, but their `updates` table does not list them,
-- so the updater would apply them again and stop on the first duplicate row
-- (2026_10_15_00.sql: Duplicate entry for key 'playerbots_travelnode_path.PRIMARY').
-- An empty hash tells the updater the file is applied; it only records the file's hash
-- (Updates.AllowRehash = 1, the default).
--
-- Installs from v1.1.0-beta or newer already list them: there, this script changes nothing.

INSERT IGNORE INTO `updates` (`name`, `hash`, `state`) VALUES
('2024_08_07_00.sql', '', 'RELEASED'),
('2024_11_25_00.sql', '', 'RELEASED'),
('2026_09_21_00.sql', '', 'RELEASED'),
('2026_09_22_00.sql', '', 'RELEASED'),
('2026_10_02_00.sql', '', 'RELEASED'),
('2026_10_07_00.sql', '', 'RELEASED'),
('2026_10_15_00.sql', '', 'RELEASED'),
('2026_10_21_laneF_787.sql', '', 'RELEASED'),
('2026_10_21_laneF_846.sql', '', 'RELEASED'),
('2026_10_21_laneX_847_phase1.sql', '', 'RELEASED'),
('2026_10_21_laneX_847_phase2.sql', '', 'RELEASED'),
('2026_10_21_laneX_847_phase3.sql', '', 'RELEASED'),
('2026_10_21_laneZ_847.sql', '', 'RELEASED');
