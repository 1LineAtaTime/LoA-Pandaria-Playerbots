-- Reserves an existing boost entitlement. Never changes account.dp.
-- Apply after 2026_10_28_00_battlepay_accounting.sql, first in isolated databases.
CREATE TABLE IF NOT EXISTS battlepay_boost_use (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  account_id INT UNSIGNED NOT NULL,
  realm_id INT UNSIGNED NOT NULL,
  entitlement_realm INT NOT NULL,
  character_guid INT UNSIGNED NOT NULL,
  specialization INT UNSIGNED NOT NULL,
  alliance TINYINT UNSIGNED NOT NULL,
  state TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0 requested, 1 reserved, 2 applied, 3 restored',
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (id),
  UNIQUE KEY character_boost (account_id, realm_id, character_guid),
  KEY recovery (account_id, realm_id, state)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
