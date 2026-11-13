-- Apply to an isolated auth database first. Does not change account.dp or old logs.
-- Requires the existing account and account_boost tables to use InnoDB.
-- Existing duplicate boost rows cause this ADD UNIQUE KEY to fail safely.
-- Review/merge those entitlements explicitly before retrying; no row is deleted.
SET @battlepay_boost_key = (SELECT COUNT(*) FROM (
  SELECT INDEX_NAME FROM information_schema.STATISTICS
  WHERE TABLE_SCHEMA=DATABASE() AND TABLE_NAME='account_boost'
  GROUP BY INDEX_NAME, NON_UNIQUE
  HAVING NON_UNIQUE=0 AND GROUP_CONCAT(COLUMN_NAME ORDER BY SEQ_IN_INDEX) IN ('id,realmid','realmid,id')
) AS boost_keys);
SET @battlepay_boost_ddl = IF(@battlepay_boost_key>0, 'SELECT 1',
  'ALTER TABLE account_boost ADD UNIQUE KEY battlepay_account_realm (id,realmid)');
PREPARE battlepay_boost_statement FROM @battlepay_boost_ddl;
EXECUTE battlepay_boost_statement;
DEALLOCATE PREPARE battlepay_boost_statement;

CREATE TABLE IF NOT EXISTS battlepay_schema (
  id TINYINT UNSIGNED NOT NULL,
  version INT UNSIGNED NOT NULL,
  PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS battlepay_purchase (
  id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  nonce CHAR(32) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  account_id INT UNSIGNED NOT NULL,
  realm_id INT UNSIGNED NOT NULL,
  character_guid INT UNSIGNED NOT NULL DEFAULT 0,
  product_id INT UNSIGNED NOT NULL,
  product_type TINYINT UNSIGNED NOT NULL,
  price BIGINT UNSIGNED NOT NULL,
  title VARCHAR(255) NOT NULL,
  description TEXT NOT NULL,
  state TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0 quoted, 1 paid, 2 delivered, 3 cancelled, 4 refunded',
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at TIMESTAMP NULL DEFAULT NULL,
  PRIMARY KEY (id),
  UNIQUE KEY nonce (nonce),
  KEY recovery (account_id, realm_id, state)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS battlepay_purchase_item (
  purchase_id BIGINT UNSIGNED NOT NULL,
  line_id INT UNSIGNED NOT NULL,
  item_id INT UNSIGNED NOT NULL,
  item_count INT UNSIGNED NOT NULL,
  PRIMARY KEY (purchase_id, line_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS battlepay_service_delivery (
  purchase_id BIGINT UNSIGNED NOT NULL,
  delivered_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (purchase_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Token redemption can be retried after an ambiguous acknowledgement without
-- crediting the same physical item twice across auth/character databases.
CREATE TABLE IF NOT EXISTS battlepay_token_credit (
  realm_id INT UNSIGNED NOT NULL,
  item_guid INT UNSIGNED NOT NULL,
  account_id INT UNSIGNED NOT NULL,
  character_guid INT UNSIGNED NOT NULL,
  amount BIGINT UNSIGNED NOT NULL,
  applied TINYINT UNSIGNED NOT NULL DEFAULT 0,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (realm_id, item_guid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO battlepay_schema (id, version) VALUES (1, 1)
  ON DUPLICATE KEY UPDATE version = VALUES(version);
