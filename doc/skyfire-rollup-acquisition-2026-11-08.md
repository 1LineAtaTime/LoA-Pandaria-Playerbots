# SkyFire rollup acquisition and integration status

The official latest published database release checked on November8 is
[27.001](https://github.com/ProjectSkyfire/SkyFire_548/releases/tag/sf_db_27).
Its corrected November3 archive is
[SFDB_full_548_27.001_2026_011_03_Release.zip](https://github.com/ProjectSkyfire/SkyFire_548/releases/download/sf_db_27/SFDB_full_548_27.001_2026_011_03_Release.zip).

The larger packages had already been acquired during earlier work:

- Release26.002: existing root out archive, matching published SHA-256
  d12ba9ecad381f3b5460612af2a172cd373afdd796c289e2ccc11315db21dd5f.
- Release27.001: existing out/skyfire27-reference archive, plus a newly verified
  copy under out/upstream-release-20261108, matching published SHA-256
  7b2d59d25bac679b5f4e7a0e95c48ac08b4bdd4077fb4496b6cb3837f87b824f.

The failure was incomplete reconciliation of matching native gameplay code/data
after the active server returned to LoA. Acquisition/import histories do not prove
the current runtime includes every corresponding implementation. The current
Gilneas encounter gaps are documented in worgen-14154-readonly-2026-11-08.md.

This pass obtained the release-tagged source archive pinned to actual sf_db_27
target f7a9880b9452e4db900dc9ff783b7b6be3aeeb86. Its ZIP CRCs and those of the
database archive passed. The source contains the Sean Dempsey/Crowley/rooftop
Worgen encounter handlers and archived matching SQL. Both encounter commit
42163b72 and follow-up dc61c594 were verified ancestors of that source pin.

The release body's declared target d3ce4e58 differs from the actual tag target and
could not be resolved/fetched locally. Provenance therefore records the actual
verified tag/source pin instead of claiming equivalence to that unresolved value.

Files in ignored out/upstream-release-20261108:

- SFDB_full_548_27.001_2026_011_03_Release.zip:22,813,610 bytes; one126,356,763-byte
  SQL member.
- SkyFire_548-f7a9880-source.zip: complete release-tagged source; SHA-256
  528b1d7ee4548858d2eaf96f6148cd95935c8503fc93b1f0c620b33ed4fcc691.
- acquisition.json and per-archive verification manifests.

[Tracked provenance](skyfire-rollup-acquisition-2026-11-08.json) records public
download links, checksums and explicit deployment state. No database import,
native source merge, executable replacement or server restart occurred in this
acquisition pass. Current characters/accounts/shop/bot implementations remain the
active LoA versions. Matched source/data integration is still required.

Exact published quest fixes:

- [Wave encounter](https://github.com/ProjectSkyfire/SkyFire_548/commit/42163b724df3528c5906e58d0d6c9c5fce8b699d).
- [Completion-credit and bite/phase follow-up](https://github.com/ProjectSkyfire/SkyFire_548/commit/dc61c594e41b8e7737e1c88d0159a9753d21c0a8).
