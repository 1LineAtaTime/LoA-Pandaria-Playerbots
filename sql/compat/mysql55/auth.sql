-- Apply only to a reviewed MySQL 5.5 schema after enabling Barracuda,
-- innodb_file_per_table=1 and innodb_large_prefix=1 on that isolated instance.
-- Preserves full primary-key values; no account or wallet data is changed.
ALTER TABLE `updates` ROW_FORMAT=DYNAMIC;
ALTER TABLE `updates_include` ROW_FORMAT=DYNAMIC;
