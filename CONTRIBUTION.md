# Contribution by Nicalicious

This is a full source contribution to
[LoA Playerbots playerbot-beta](https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/tree/playerbot-beta).
It includes the development work described in [FEATURES.md](FEATURES.md) and the
large [changelog.md](changelog.md), rather than just the most recent city change.
Original copyright, GPL notices and imported-source attribution remain intact.
The contribution is offered for maintainer review, not represented as entirely
newly authored code or complete retail gameplay coverage.

## Included together

- Playerbots, curated names, population controls, city errands, companion access,
  addon commands, quiet narration, group leadership, loot/vote and lifecycle work.
- Auction market identities, buying/selling filters, minimum-level exclusion,
  and seller-name prefetch for auction scanners.
- BattlePay raw-DP balance, purchase/confirmation/cancel/failure handling,
  transactional delivery/recovery, and Orb of Loyalty redemption.
- MySQL/MariaDB client/version/transaction compatibility and build/staging tools.
- Quest, spell, phase, vehicle, transport, NPC service and crash repairs, with
  paired SQL/native changes and focused regression programs.
- Unattended QA, owned temporary fixtures, cleanup, diagnostics, reports and
  explicit coverage limits. No Codex/API connection is required to run QA.
-146 indexed contribution SQL files, including clean schemas, names/chat seeds,
  feature migrations and separately classified compatibility references.
- An optional static1904-row BattlePay catalogue; no live accounts or balances.
- Three complete public configuration examples preserving the tested gameplay
  settings, with credentials replaced by `DB_USER` / `CHANGE_ME`.

See [database/README.md](database/README.md), [UPGRADE.md](UPGRADE.md), the
[migration inventory](database/contribution-migrations.csv) and the linked
feature documents. This repository is a normal complete checkout, not a patch
that depends on access to a private development repository.

## Base and reconciliation

The clean contribution was reconstructed against public beta
`ef5112acc3331f7a38bc090edae887320696cac3`, then merged with beta
`2f9aebde9f68f7c79fbde2e7c1f2250ed49b0ef7` from November12.
The merge retains both the movement-input trace and upstream movement-template
root check. Soul Reaper retains its periodic-dummy removal hook with the matching
`AuraEffectRemoveFn` wrapper. Newer upstream files/SQL remain present.

Only upstream public ancestry and sanitized contribution commits are intended
for this public branch. Private development history, runtime configuration,
account/character data, wallets, mail, dumps and debug symbols are excluded.

## Evidence and limits

The pre-reconciliation runtime was built in Visual Studio2022 x64
RelWithDebInfo, staged with matching EXE/PDB hashes, and exercised against
MariaDB with550 managed bots. Focused feature notes describe each test and its
limitations. In the latest batch:

- A360-second natural Dalaran sample ended with24 surface bots;12 moved more
  than25yd,19 reached22 distinct services, and no floor offsets above5yd occurred.
- Paged auction delivery retrieved6642 listings in45.188s; all1241 seller names
  arrived before listing processing. Get-all delivered6641 listings in0.292s.
- Rendered Auctioneer completion time remains an acceptance check.
- The final reconciled public candidate receives a separate build/check report
  in [doc/public-release-validation.md](doc/public-release-validation.md).

This is not a claim that every quest, spell, dungeon, boss or item has been
played or repaired. Original-world diagnostics, broader gameplay coverage,
long-running550-bot combat, flying NPC behavior and Linux runtime acceptance
remain documented work. Warnings/errors are not disabled to manufacture a pass.

## Maintainer review

Keep source and its SQL together. The groups in
[the briefing](doc/handoff/01-MAINTAINER-HANDOFF.md) can be reviewed separately,
but accounting, permissions, native script registration and schema dependencies
must remain paired. The changelog describes historical repairs; it is not an
instruction to replay every historical SQL file. The maintainer can merge the
whole branch or request smaller changes without needing private data.
