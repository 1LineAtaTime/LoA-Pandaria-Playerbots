-- Durable per-use currency voucher audit. No account/character data is seeded.
CREATE TABLE IF NOT EXISTS battlepay_orb_redemption (
 id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
 realm_id INT UNSIGNED NOT NULL,
 account_id INT UNSIGNED NOT NULL,
 character_guid INT UNSIGNED NOT NULL,
 item_guid INT UNSIGNED NOT NULL,
 count_before INT UNSIGNED NOT NULL,
 dp_added SMALLINT UNSIGNED NOT NULL DEFAULT 100,
 created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
 PRIMARY KEY(id),KEY idx_account_created(account_id,created_at),KEY idx_item(item_guid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
