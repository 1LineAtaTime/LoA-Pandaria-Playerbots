# Full LoA beta integration — 2026-11-08

## Scope and provenance

Merged the complete `upstream/playerbot-beta` history at
`8a26cd41fece10e18335ebc078f6ff7e99412c2d` into the customized LoA branch,
from local milestone `00312addff5b0066e29f0ee6e650481192634eca`.
This is the pinned upstream beta integration, not a claim that every quest is
complete or that no newer upstream changes can exist.

All 438 world and two character SQL files between the original LoA baseline
`e135742e18` and this beta pin were applied. Ten additional playerbot update
files were applied, and the empty item store was populated with 50,241 rows.
The playerbot city/farming base files split existing data out of the old combined
dump; existing populated tables were retained. The zone-path base difference
only removes a comment. Updater history was not replaced with upstream's ledger.
Per-file hashes and completed results are in the adjacent JSON inventory.

## Preserved local functionality

The customized BattlePay manager, handlers, boost and token implementation remain
intact, including DP accounting, cancellation and native insufficient-funds flow.
Local auction owner/filtering changes, companion policy, administrator permissions,
character creation repairs, and Detour null-edge guard are retained. No Detour
source changed in this merge. Conflict decisions are recorded separately in
`loa-full-beta-merge-decisions-2026-11-07.json`.

The source and database now use the paired upstream `DamageModifier`/attack-time
creature damage model. Retained SF compatibility columns are not multiplied a
second time. Compiler integration repairs reconciled the transaction retry variable,
Gilneas escort-player lifetime, and Rimok's player-target filter.

## Database compatibility and rollback

Both servers were stopped before backups and full migration. All five `mop_loa_`
schemas have offline dumps and SHA-256 records under the ignored directory
`out/loa-full-beta-20261107`; EXE/PDB/DLL/config copies are in `runtime-before`.
The backup branch is `codex/pre-full-loa-beta-20261107`. Old `mop_` and `mop_sf_`
schemas were not replaced. Personal accounts, characters and shop data remain private.

The restored world contained mixed legacy and modern aliases. SQL53 restores the
normalized locale schemas from the original LoA reference, missing metadata fields,
canonical gossip primary keys and nullable waypoint orientation. SQL54 normalizes
other primary keys and synchronizes old/new aliases for mixed historical SQL.
SQL55 preserves legacy translations without overwriting normalized translations.
These adapters must precede beta updates on this mixed schema; locale preservation
runs after the updates. They are not intended as unguarded migrations for unrelated
cores. Automatic historical database updating stays disabled.

Two upstream SQL files required local repairs: a malformed multiline waypoint
comment and a correlated waypoint DELETE rewritten as an equivalent indexed join
for MariaDB. Failed imports stopped and were resumed by matching per-file hashes;
no SQL error was ignored. All original partial attempts remain in private tool logs.

First startup detected a missing playerbots account-type table. The ten module
updates were then applied before restarting; this startup failure was not suppressed.

## Validation

Visual Studio RelWithDebInfo x64 build completed with zero failed projects.
All 22 native tests and 408 existing starter/training/Gilneas/quest data assertions
passed. Export executable and PDB hashes match the Visual Studio output.
Database connections use the retained MySQL client DLLs against MariaDB 10.11.19.
Runtime/gameplay results are recorded below after the deployment check.

## Known limits and next work

The published `2026_10_28_963m_patrol_spawns_without_path.sql` makes 46 spawns
stationary because no route data exists, and restores one available route. It is
an invalid-data fallback, not proof that those patrols behave correctly; authoring
those missing paths remains on the checklist. Warnings/errors remain enabled.
Automated coverage is bounded and does not establish that all quests, spells,
instances, companion travel or shop transactions pass.

## Deployed runtime result

The final worldserver and authserver listen on 8085 and 3724 in visible export
consoles. The ordinary-account campaign passed all 15 cases on the merged build:
14 creations/immediate enumerations/world entries and expected Death Knight
level gating; five starter accepts and three delivery turn-ins with saved XP.
All 15 disposable accounts and associated characters were removed. This did
not test complete combat/collection journeys or shop transactions.

Post-merge validation restored 356 reference dialogue rows covering 189 missing
SmartAI text groups and copied referenced legacy script speech/translations into
broadcast_text, resolving the repeating Donna/William speech errors. Existing
modern text was not overwritten. A restart loaded these repairs.

At the startup sample, 1,325 distinct SQL diagnostic messages remain, compared
with 1,346 in the archived pre-merge run (110 absent, 89 newly observed). These
are bounded log comparisons, not a complete behavior audit. Remaining content
issues and incomplete upstream patrol fallbacks are explicitly open. No blanket
warning/error suppression was applied; the server is not claimed error-free.
