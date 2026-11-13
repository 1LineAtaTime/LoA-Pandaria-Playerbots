# Next work: Dalaran activity and auction scanning

Implemented in the Dalaran/auction follow-up. See
dalaran-auction-followup-2026-11-12.md for measured acceptance and remaining
rendered-client checks. Earlier chat/emote repairs remain separately documented.

## Dalaran should feel as active as the other capitals

- [x] Measure live bot count and faction distribution in Dalaran, rather than assuming the550 global target implies a populated city.
- [x] Compare Dalaran's capital/travel-hub/service-NPC eligibility with Stormwind, Orgrimmar and Ironforge.
- [x] Inspect available innkeeper, vendor, trainer, bank, mailbox and auction-service stops; do not send ordinary characters to unavailable services or the wrong faction district.
- [x] Measure the proportion of present bots actively moving versus waiting, stuck or holding expired reservations.
- [x] Check valid floor/path heights, stairs, portals and district restrictions before increasing local population.
- [x] Reuse the logical city errand behavior with varied destinations, pauses and spacing; prevent bank/doorway piles and tight circling.
- [x] Add a bounded/configurable city distribution or visit policy if lack of eligible bots is the cause.
- [x] Verify a timed sample shows useful trips between services without teleports through floors, fixed furniture or fake auction transactions.

## Auctioneer scan performance

Owner observation: about6000 auctions previously scanned in90seconds and now
about7minutes. This is a report to reproduce, not a measured database bottleneck.

- [x] Record Auctioneer5.19.5445, both native scan modes, counts, pacing and cold seller-name behavior.
- [ ] Time the rendered Auctioneer addon after seller-name prefetch; native delivery timing alone is not addon completion timing.
- [x] Timestamp actual auction requests/replies, page sizes and deliberate pacing/rate limits.
- [x] Compare native auction query/serialization time with database query time and result sizes.
- [x] Check anti-abuse throttles and scan scheduling alongside client-side waits.
- [x] Check auction cache/index behavior, simultaneous auction-bot work and server update delays.
- [x] Change only the demonstrated bottleneck; retain anti-abuse safeguards and avoid guessing at database worker counts.
- [x] Repeat the same mode/count baseline and document the resulting duration and normal gameplay responsiveness.

## Connection settings and GM vouchers — completed administrative follow-up

The active connection fields are grouped immediately below the worldserver and
authserver section headers. Network ports/bind address, database connection
strings and database worker/synchronous thread settings are together. The
PlayerbotsDatabase fields are included rather than buried near the end.
All active key/value pairs were compared before/after and remained identical.
The grouping helper is contrib/windows/Group-ConnectionSettings.py. Backups
contain credentials and remain in ignored local output. No restart is required
for a layout-only edit with unchanged values.

### Give an Orb of Loyalty to a player

Use an online GM account with permission to run .additem. Select the intended
online player in game, check that their name is the selected target, then run:

```
.additem 35626 1
```

For one full stack:

```
.additem 35626 20
```

The item is created in the selected player's inventory; it does not need to be
traded from a GM's bags. Each orb redeems for100 account coins;20 orbs represent
2000 coins if all are redeemed. Ensure the recipient has bag room.

Do not append a player name to this command: this implementation uses the
selected player. With no suitable player selected it can default to the GM.
The core has an intentional GM-self-grant unbinding exception; targeting the
actual recipient avoids relying on that exception and preserves the ordinary
soulbound grant path. This guidance is based on the actual cs_misc.cpp handler.
No items or coins were granted to user characters for this documentation change.

Final360-second census passed:24 surface visitors,12 moved>25yd,19 reached services across22 destinations; no floor offsets>5yd. Rendered addon timing remains open.
