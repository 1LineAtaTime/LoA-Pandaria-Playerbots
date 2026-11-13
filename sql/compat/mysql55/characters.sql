-- Same prerequisites as auth.sql. Review locks/disk space before applying to
-- an existing deployment; these operations rebuild tables, preserving rows.
ALTER TABLE `banned_addons` ROW_FORMAT=DYNAMIC;
ALTER TABLE `updates` ROW_FORMAT=DYNAMIC;
ALTER TABLE `updates_include` ROW_FORMAT=DYNAMIC;
