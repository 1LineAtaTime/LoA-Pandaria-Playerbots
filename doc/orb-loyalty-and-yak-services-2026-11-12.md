# Orb of Loyalty and Grand Expedition Yak — November 12

## Orb of Loyalty35626

The item now has Legendary quality5, Bind on Pickup1, maximum stack20 and
SellPrice0. Its exact description is:

Adds currency to your account of 100 Battle Shop Coins.

The native item_orb_of_loyalty script uses whole account.dp units:100 per
right-click, never10000-scaled wire units. It consumes exactly one orb.
Spell 483 is only the existing client-use marker; the script handles the use.
New items are soulbound and cannot be mailed/traded/auctioned; zero sell price
prevents selling to a vendor. Bots cannot automatically redeem the voucher.
There are no shop-product entries that sell this item in the current catalogue.

Auth and characters must be on the same database host/port, with permission for
the character connection to update the auth wallet. The script validates SQL
schema identifiers and InnoDB storage for account, item_instance,
character_inventory and the receipt table. Unavailable/unsupported storage
retains the orb rather than guessing a credit. NULL legacy wallets are treated
as zero on redemption, without a blanket update to other accounts.

After flushing pending inventory changes, one cross-schema InnoDB transaction
updates the wallet and decrements the expected owned inventory stack, records a
receipt, and removes the inventory/item rows when the stack reaches zero.
The affected-row guard requires both wallet and item changes. Only after commit
is the inventory change published in memory and the shop catalogue refreshed.
An unexpected/uncertain database failure forces reconnect with the item left
unchanged in memory and already flushed; the next login reloads durable truth.
It does not guess a compensating credit or recreate a possibly consumed orb.

Migration files: characters 01 creates battlepay_orb_redemption; world 10 changes
the item/template binding and canonical translated descriptions. No account or
character identity is included in those clean public migrations.

## Owner-requested local reset

The owner clarified that only old orb items should be removed, preserving every
character. An offline scoped backup preceded removal of 865 item instances,
45,024 orbs in total, and their inventory/mail/auction/guild-bank references.
All 5,317 characters remain. Exactly one stack of 20 soulbound orbs was placed in
the requested existing character's backpack; no DP was granted by that reset.
Private exact GUIDs, quantities, old rows and wallet/identity evidence are in
out/backups/orb-loyalty-20261112. This destructive local cleanup is intentionally
not part of the public reusable migration.

## Yak services

Grand Expedition Yak62809 carries Mystic Birdhat62821 and Cousin Slowhands62822.
Slowhands had no merchant inventory. World11 additively restores14 authored LoA
vendor entries, retaining existing/custom entries and repair capability.
Birdhat lacked the native5.4.8 reforger flag 134217728; restore it without adding
a later-expansion transmog service or inventing an OPEN_REFORGE packet. The
existing core reforge handler validates interaction and processes requests.

Reference data: matching LoA v1.0.4 world SQL; scoped before/reference rows are in
out/backups/yak-vendors-20261112 and the orb reset's scoped world backups.
Protocol cross-check: TrinityCore/WowPacketParser V5_4_8_18291 opcode table and
this core's ItemHandler.cpp are the sources for the native reforge exchange.
https://github.com/TrinityCore/WowPacketParser/blob/master/WowPacketParser/Enums/Version/V5_4_8_18291/Opcodes.cs

## Measured validation and limitations

A disposable two-character account proved native22-item grants split20+2 with
bound flags; three uses consumed three items and credited300 DP; overflow
retained the orb; a fixture-only duplicate-receipt trigger failed after the
wallet/item update and rolled both back. Reconnect retained the original stack.
Another character on the same account redeemed the last orb for the same
wallet; replaying its old item-use packet added no further DP. Owned fixtures
and the temporary trigger/receipts were removed.

The separate riding-trained fixture summoned the actual yak passengers,
received14 merchant items from Slowhands and a successful native reforge-reset
response from Birdhat. No stationary replacement NPC was used. The aggregate
passed report is out/orb-yak-20261112/aggregate.json. These are protocol and
persistence tests; visual client tooltip/service-frame acceptance remains a
separate check. The initial combined fixture lacked riding training; its
currency leg passed, and the service leg was then verified separately.

The forced receipt failure intentionally generated SQL/transaction diagnostics
for that test fixture. They were retained. Other unrelated world-data warnings
remain on the project checklist; no error logger was suppressed for these fixes.

## Recovery

Preserve redemption receipts and committed wallet/item changes when reverting
feature code/template settings. Replaying old orb backups after redemptions
would reintroduce spendable value: reconcile receipts/wallets first, offline,
if explicitly undoing the local reset. Never restore account balances and
inventory from unrelated snapshots. All old orb/template/reference rows and
pre-change EXE/PDB/config recovery copies are under ignored out/backups.

The client-cache follow-up received both native item and sparse hotfix records.
Its payload verified quality5, SellPrice0, stack20 and the exact description.
World10 also refreshes both hotfix_data timestamps so existing client caches
can invalidate outdated metadata after the required worldserver restart.
Disposable fixture cleanup passed; visual tooltip acceptance remains separate.
