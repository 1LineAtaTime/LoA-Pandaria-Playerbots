# BattlePay accounting and recovery

## Wallet and pricing

`auth.account.dp` is the sole wallet. **One stored `account.dp` unit is one
displayed donation point.** Catalog prices use the same whole-DP units. The
MoP client packet format alone multiplies a price by 10000. Existing balances
are not reset or rescaled; websites and administration tools should use raw DP.

Balances are read using the authenticated session account ID, freshly from auth.
SQL failures, missing accounts, NULL, negative balances and values outside the
existing signed INT range fail closed. Supported balances are 0–2147483647 DP.
Price and discount calculations use exact integers, rounding discounted prices
up to whole DP. The catalog wire price, confirmation wire quote and raw DP debit
derive from the same result. Client price and token echoes must match a pending
server quote and cannot set either value.

`LOGIN_UPD_BATTLEPAY_INCREMENT_COINS` now adds, with an overflow guard; decrement
has a sufficient-funds guard. Mutation amounts bind as 64-bit values. Player
helpers report checked execution/affected-row results rather than treating an
UPDATE as a SELECT. A lost acknowledgement is not permission to blindly replay a
non-idempotent wallet update.

In-world balance messages retain the existing localized chat transport, now
displaying whole DP. The existing catalog's non-purchasable product 1 (`Your
balance`, service type, zero price) carries the account balance in its current
wire price field. Product metadata is checked so a different product 1 is not
repurposed. There is no invented balance packet for character selection.

## Required migrations

Apply and validate these in isolated databases before deployment:

1. Auth: `sql/updates/auth/2026_10_28_00_battlepay_accounting.sql`.
2. Characters, each realm: `sql/updates/characters/2026_10_28_00_battlepay_accounting.sql`.
3. Auth: `sql/updates/auth/2026_10_28_01_battlepay_boost_recovery.sql`.
4. Characters, each realm: `sql/updates/characters/2026_10_28_01_battlepay_boost_recovery.sql`.

The accounting migrations create purchase, item snapshot, completion, token credit
and delivery receipt tables. They leave existing DP and purchase logs intact.
The auth migration also requires one unique `(id,realmid)` key on `account_boost`.
If historical duplicate boost rows exist, index creation fails without deleting
or merging entitlements. Review them explicitly first:

```sql
SELECT id, realmid, COUNT(*) AS rows_found, SUM(counter) AS counter_total
FROM account_boost GROUP BY id, realmid HAVING COUNT(*) > 1;
```

All wallet, purchase, boost, legacy log, character, item and mail tables involved
must use InnoDB. Store schema/engine/key checks disable purchases if accounting
prerequisites are missing. The new queries and migrations use features available
in MySQL 5.5 and MariaDB 10.5. Never apply the archived
`sql/archive/auth/2021_03_27_BattlePay_DP.sql` to an existing account database: that
historical script resets DP balances.

Stop realm processes and use the normal backed-up deployment/migration procedure
when applying schema changes. Retain these tables and receipts across upgrades;
deleting them destroys idempotency and recovery evidence. Do not restore auth and
character databases to unrelated points in time.

## Purchase states and delivery

| Purchase state | Meaning |
| --- | --- |
| 0 | Persisted server quotation, unpaid; expires after five minutes |
| 1 | Payment durably accepted; fulfillment or compensation pending |
| 2 | Delivery committed and accounting finalized |
| 3 | Unpaid quotation cancelled or expired |
| 4 | Cancelled delivery fenced and DP refunded |

Purchase IDs come from the auth database's BIGINT AUTO_INCREMENT key. A random
128-bit nonce protected by a unique key identifies the creation attempt and
resolves an ambiguous commit acknowledgement. Catalog title, description, item
IDs/counts and price are persisted before accepting confirmation.

Pending protocol state is owned per session, removed on session destruction,
and advanced only on its world-thread update. A confirmation must belong to that
session's current allowed quotation and match its nonzero random server token and
quoted price. Accepting changes the state immediately, so repeated confirmations
cannot queue another payment. The original 500 ms preparation, 8000 ms confirmation
and 1000 ms fulfillment delays remain. Expired local quotes become terminal and
do not trap the session behind a stale purchase.

The conditional InnoDB multi-table update in `BattlePayQueries::Charge` changes
`account.dp` and purchase state together. It requires the authenticated account,
realm, unexpired unpaid state and sufficient nonnegative DP. Retries read durable
state; they cannot deduct again. No transaction across auth and characters is
assumed.

Item products deliver every configured item/count through mail, including online
characters, so full bags cannot discard a paid purchase. Large counts split into
legal stacks and mails of at most 12 attachments. Malformed catalogs exceeding
120 stacks are rejected before charging. Targets must be characters owned by the
authenticated account. A checked `INSERT ... SELECT` claims a unique character
receipt and holds the ownership row while that same transaction writes all item,
mail and attachment rows. An affected-row mismatch or any SQL failure rolls the
whole delivery back. Online mail objects become visible only after the exact
delivery attempt's committed receipt has been read back.

The auth completion key, legacy purchase log and delivered state then commit
together. A crash after character delivery reuses the receipt rather than mailing
again. Paid rows retry on the account's next session update and every 30 seconds
while connected. A disconnected account's pending delivery is recovered when it
reconnects; there is no offline background fulfillment daemon.

If a target is gone or an item snapshot can no longer be fulfilled, a character
receipt with cancellation state 0 fences out competing deliveries. Only that
durable fence permits an atomic guarded refund. An unavailable database is never
interpreted as proof of character deletion. A refund that would overflow the
existing wallet range stays pending for review and later retry; it cannot wrap,
erase DP, or silently mark the purchase refunded.

Boost purchases grant an account entitlement, completion receipt, log and delivered
state in one auth transaction. Unknown service types fail before charging. Boost
application has separate reservation/character receipt recovery in the `_01`
migrations; its character changes and receipt share a character transaction.

## Physical donation tokens

For nonstackable token templates, credit and its durable per-physical-item receipt
commit together in auth. The exact source item is consumed only after applied
credit is confirmed. A retry after a lost acknowledgement consumes that source
without crediting it twice. Token credit logging shares its transaction.

**Stackable token redemption is refused safely and retains the item.** A physical
GUID alone cannot identify separate units after stack splits/merges. Supporting
stackable tokens requires a durable inventory unit redemption workflow; changing
existing token template stack limits or guessing a key from current stack count
would not safely solve that problem. This is an explicit remaining limitation.

## Validation and remaining client work

The standalone `battlepay_accounting_tests` CTest target exercises production
integer pricing, display, range guards, confirmation matching and safe physical
token constraints. It runs in optimized builds without relying on disabled
`assert` statements.

`contrib/battlepay-tests/Test-BattlePayDatabase.ps1` runs the production charge,
refund, token credit and boost reservation/restoration SQL constants against an
explicit isolated server on loopback port 33555 or 33556. It creates uniquely
named `mop_test_battlepay_*` fixture databases and removes only those fixtures.
It tests migration reruns, concurrent charges, repeated requests, external wallet
changes, negative/NULL wallets, exact-funds handling, expired quotes, receipt
uniqueness, transactional rollback, interrupted finalization, cancellation fences,
refunds, boost reservation/application and token credit bounds.

The fixture character/mail/item tables contain only the columns used by these
SQL scenarios. These are real database state/concurrency tests; they do not run
the game's `Item` serializer, world loop, network handler or a game client. The
full imported base schemas and prepared statements are checked separately by the
database connector/server startup tests.

The existing purchase-history and distribution-list wire responses remain empty
TODOs; durable history is available in `battlepay_purchase`, item snapshots,
receipts and `battlepay_log`. Filling those client packets needs independently
verified 5.4.8 packet layouts and client testing. A live 5.4.8 client purchase
of product 4401 was confirmed on example account: `account.dp` changed from 745 to
730, the purchase reached delivered state 2, and the character delivery receipt
reached state 1. A later 15 DP purchase also delivered successfully. With a
16 DP wallet, a 20 DP purchase raised the client's native insufficient-funds
error without creating a charge or delivery. This required the start-purchase
response to serialize purchase ID, client token, then result code, matching the
5.4.8 client reader. The temporary test balance was restored to 16 DP after
the later affordable purchase. Disconnect during purchase, player auction
transactions, and process-kill recovery remain to be tested in the client. The
SQL harness simulates durable interruption boundaries rather than proving an
end-to-end crash recovery session.

See `doc/validation-results.md` for dated build/runtime evidence and the exact
remaining deployment constraints.
