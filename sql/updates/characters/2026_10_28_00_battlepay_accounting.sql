-- Apply to each isolated realm character database before enabling its store.
-- A receipt/tombstone and all delivery rows are committed in one transaction.
CREATE TABLE IF NOT EXISTS battlepay_schema (
  id TINYINT UNSIGNED NOT NULL,
  version INT UNSIGNED NOT NULL,
  PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS battlepay_delivery (
  purchase_id BIGINT UNSIGNED NOT NULL,
  account_id INT UNSIGNED NOT NULL,
  character_guid INT UNSIGNED NOT NULL,
  attempt CHAR(32) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  state TINYINT UNSIGNED NOT NULL COMMENT '0 cancellation fence; 1 items delivered',
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (purchase_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO battlepay_schema (id, version) VALUES (1, 1)
  ON DUPLICATE KEY UPDATE version = VALUES(version);
