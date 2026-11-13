-- Candidate pool only; import user-approved CSV with contrib/playerbot-names/manage_roster.py.
-- Existing human names and the existing characters-name index are not modified.
CREATE TABLE IF NOT EXISTS playerbot_curated_names (
 name varchar(12) COLLATE utf8_general_ci NOT NULL,
 category varchar(40) NOT NULL, tier char(1) NOT NULL,
 gender_hint tinyint unsigned NOT NULL DEFAULT 0,
 class_hint tinyint unsigned NOT NULL DEFAULT 0,
 race_hint tinyint unsigned NOT NULL DEFAULT 0,
 weight int unsigned NOT NULL DEFAULT 1, enabled tinyint unsigned NOT NULL DEFAULT 1,
 import_sha char(64) NOT NULL,
 owner_guid int unsigned NOT NULL DEFAULT 0,
 owner_account int unsigned NOT NULL DEFAULT 0,
 reserved_at timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
 PRIMARY KEY(name), KEY available(enabled,owner_guid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE IF NOT EXISTS playerbot_name_renames (
 guid int unsigned NOT NULL, account_id int unsigned NOT NULL,
 old_name varchar(12) NOT NULL, new_name varchar(12) NOT NULL,
 import_sha char(64) NOT NULL, state varchar(16) NOT NULL DEFAULT 'planned',
 applied_at timestamp NULL DEFAULT NULL,
 PRIMARY KEY(guid), UNIQUE KEY renamed_name(new_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
CREATE TABLE IF NOT EXISTS playerbot_roster_accounts (
 account_id int unsigned NOT NULL, username varchar(32) NOT NULL,
 provenance varchar(32) NOT NULL, created_at timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
 PRIMARY KEY(account_id), UNIQUE KEY username(username)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
