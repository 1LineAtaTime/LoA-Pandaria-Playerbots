-- Application receipt and all character/inventory changes share one transaction.
CREATE TABLE IF NOT EXISTS battlepay_boost_delivery (
  use_id BIGINT UNSIGNED NOT NULL,
  account_id INT UNSIGNED NOT NULL,
  character_guid INT UNSIGNED NOT NULL,
  state TINYINT UNSIGNED NOT NULL COMMENT '0 cancellation fence, 1 boost applied',
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (use_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
