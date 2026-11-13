# Clean SQL and application notes

## What is included

`contribution-migrations.csv` indexes146 contribution SQL files with category, purpose, policy and SHA-256. The current index is authoritative; it includes world, auth, characters, curated names, chat data, clean bases and optional compatibility references.

The two base files have all seeded runtime/account/character/mail rows removed:
56 INSERT/REPLACE statements were stripped. Static RBAC permission definitions,
realm class/race/build definitions, updater reference data and a generic realm
definition remain. AUTO_INCREMENT counters are reset. No account or administrator
is pre-created. These are fresh-schema references, **not** upgrades to import
over an existing populated auth or character database.

## BattlePay schema dependencies

Review and apply these paired files to the correct selected databases:

1. Auth: `2026_10_28_00_battlepay_accounting.sql`.
2. Characters, each realm: `2026_10_28_00_battlepay_accounting.sql`.
3. Auth: `2026_10_28_01_battlepay_boost_recovery.sql`.
4. Characters, each realm: `2026_10_28_01_battlepay_boost_recovery.sql`.

Read `doc/battlepay-accounting.md` and the migration comments.
The account/boost/log/token and character/mail/item/receipt tables participating
in accounting require InnoDB and the expected keys. Duplicate historical boost
rows can intentionally cause unique-key installation to fail: reconcile them
before retrying, without dropping customer entitlements. Do not set all existing
DP to an example value or rescale it. One raw DP remains one displayed point.

The companion RBAC migration defines permission 1346 and its normal-role linkage.
It does not assign administrator privileges to an account.

## World schema and ordering

Use the matching **LoA** world baseline, not a bare or unrelated SkyFire world.
This handoff is a merge/upgrade contribution, not a complete world database dump.
Unchanged upstream SQL and playerbot base/name/travel data remain in the public
repository at the pinned base; do not replace those with a private live dump.

There is deliberately no blind `apply-all.sql`. Review the target's existing
columns/data and migration ledger before choosing the needed files. Some repairs
were authored against earlier schemas and have later superseding corrections.
Some files use temporary tables or MySQL-client DELIMITER commands; preserve
session/file boundaries and use a client that understands them.

Important paired prerequisites include:

- `2026_11_07_53_world_loa_beta_schema.sql`, the legacy alias/locale-preservation
  work and `2026_11_09_00_world_loa_compat_alias_triggers.sql` reconcile modern
  source columns with legacy native readers. Inspect existing equivalents first.
- The signed next-quest schema/native interpretation must be present before
  historical quest-field repairs that use signed links. Read
  `2026_11_09_03_world_signed_next_quest_links.sql` alongside the `_02` historical
  field batch and `_04` compatibility reconciliation; lexical filename order
  alone is not a safe dependency plan.
- A quest/AI binding migration must be paired with its registered native
  implementation. The Jaomin, Crowley, cage/terrain and rescued-child changes
  should not be split into data-only versus code-only deliveries.
- GUID/path edits assume a matching spawn identity. Where possible the SQL
  guards entry/old values; inspect identity conflicts before applying to a fork.

Back up affected databases/tables, apply a reviewed batch offline when its native
pair requires it, retain a ledger and compare postconditions. Repeating all old
release SQL is not the same thing as installing all current compatible fixes.

## Optional clean BattlePay catalogue

`optional-battlepay-catalog.sql` contains **7 allowlisted static world tables /
1,904 rows**: products, item mappings, groups, entries and their localization.
See `catalogue-manifest.json` for counts. No auth or character database was
dumped for it. It contains no accounts, balances, purchase logs, wallets, token
credits or delivery/boost receipts.

It uses `CREATE TABLE IF NOT EXISTS` and `INSERT IGNORE` to avoid automatically
replacing colliding rows. That is not a semantic catalogue merge: an existing
product/group ID may mean something different on the recipient's server.
Import into a scratch catalogue first, inspect collisions/relationships and
choose whether to remap or import selected rows. It is optional and is not part
of the default migration list. Private branding/links were neutralized.

The compatibility candidate files are references requiring explicit schema review;
they are not a universal Emucoach database conversion or production upgrade.
Do not replay rollback reference SQL as a forward migration.

## November12 new feature migrations

Characters00 supplies curated name claims; import the clean candidate seed or
use the CSV manager, not both blindly. Characters01 creates the voucher receipt
ledger. World08/09 restore dungeon controller/objects/triggers, world10 pairs
with the native Orb script and creates client hotfix timestamps, and world11
restores Yak services. The two chat SQL files under modules/mod_playerbots/data/
sql/playerbots must run in the playerbots schema. They repair punctuation and
add10000 variants idempotently; they do not belong in the world schema.

Use the migration index for paths/hashes and the corresponding feature notes
for dependencies. The owner-specific orb removal/grant, account data, bot
passwords and name assignment ledger are deliberately absent from clean SQL.

## Complete checkout inventory

`all-sql-files.csv` also inventories every SQL file in this public checkout, including unchanged upstream and historical/archive files. Schema hints are informational. It is an inventory, not an apply-all plan. Newer upstream SQL from the reconciliation is preserved.
