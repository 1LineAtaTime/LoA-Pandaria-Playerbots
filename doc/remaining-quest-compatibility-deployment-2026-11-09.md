# Remaining quest SQL and signed-link integration — November 9, 2026

## Deployed result

Native source `05e94aabf6` was compiled in Visual Studio, then its matched
worldserver EXE/PDB was staged into exports and the existing QA runtime.
Both worlds received the signed-schema migration and **294 field changes across
260 quests**. Every target field was verified and a second application left all
five affected table checksums unchanged. Authserver and connector DLLs were
retained. No account, character, BattlePay, AH or playerbot table was modified.

Played world21468 listens8085; QA world32968 listens8086. Unchanged auth44944 and
61780 listen3724/3725. Visible launchers were used after graceful shutdown;
verify PIDs before future commands. Both fresh starts introduced **zero new
exact warning/error signatures**. Existing diagnostics remain enabled and are
not declared resolved by that comparison.

The [deployment ledger](remaining-quest-compatibility-deployment-2026-11-09.json)
contains all changed fields, source paths, dispositions, artifact hashes and
backup receipts. The earlier6,888-field batch remains a historical deployment;
ten of those fields are explicitly corrected here, rather than hidden in a
misleading cumulative total.

## What this closes

All **1,012 previously conditional SQL inputs** and **380 previously unmapped
field assignments** now have per-source dispositions in
[the input ledger](remaining-quest-input-dispositions-2026-11-09.json).
This is review coverage, not1,392 newly deployed fixes. Historical master files
duplicate individual migrations. Many inputs already matched, only filled blank
text, were superseded, or belonged to a retired schema.

The extended assembler handles quoted IDs, guarded IDs, tuple IDs, bounded ID
ranges and file-local quest variables. Global predicates are evaluated using
both the final reference snapshot and a read-only view of the active modern
fields. Historical DDL/DML is never blindly executed. Two old
RewardOrRequiredMoney cleanups reference a field absent from the final schema;
they are recorded as retired transitions. Six WDBVerified assignments are build
metadata, not missing gameplay repairs.

The compatible changes include class/race eligibility, profession requirements,
quest types, quest-item drop references, reward-choice cleanup, offer/request
emotes, flags and69 obsolete reward casts. Installed DBC/item/spell references
are checked before assembly. Existing authored nonempty text and newer SF/LoA
overrides are preserved. Supported SpecialFlags bit values were compared with
the pinned SF enum before mapping them.

## Matching code and data

The LoA unsigned NextQuestID column/getter could not represent SF's signed
links. The new code preserves signs through loading and reverse dependency
construction. Positive links still require rewarded predecessors; negative
links require taken predecessors, retaining LoA's existing taken-state policy.
Exclusive-group member checks use the same polarity. Integer conversion avoids
the signed-minimum absolute-value overflow case.

Quest618 and8554 now link to619 using -619. Two other old source values point
quests12222/12223 at themselves. Their current LoA successors12225 and shared
completion group are valid, so those local successors were preserved. They are
not mislabeled as missing negative-link repairs anymore.

Migration order:

1. `2026_11_09_03_world_signed_next_quest_links.sql`, SHA256
   `116a90a8e9d19459943e658109d869c659919cd81907e3f9025b013912b9ab71`.
2. `2026_11_09_04_world_historical_quest_compatibility.sql`, SHA256
   `5d0a4f0b47513f64909f0219fea6f774914a2b2eb91f5955d130e8106a9f854d`.

The new EXE SHA256 is
`87d47d881d448314f482a2a91c8234f53cf83e3127a842d794cb26661248bbc1`;
PDB SHA256 is
`328b9864eefee5d85e49388869bc701ba6441a625c9f7d63e28883a34022cd23`.

## Compatibility decisions

The source catalog is not interchangeable with every local definition. The
reference Tiger Palm class value would exclude the locally supported Monk;
other older masks would remove existing playable class/race access.58 field
contracts retain existing access rather than importing that regression. These
decisions are recorded for later eligibility QA, not declared retail-certified.

Seventeen catalog names differ. Seven lesson/profession identities differ
materially, such as Corruption versus Immolate. Ten earlier imported chain fields
for14009,14012,14274,14279,14283,24784,29067 were restored from the prior ledger's
original bytes, with exact current-value guards. Cosmetic Nedar/Ned and the Horde
suffix difference were not treated as different gameplay identities. No new
positive prerequisite cycle was introduced; four pre-existing cycles remain.

Seven absent IDs have empty titles in the final reference and are not imported
as playable quests:9754,9755,10215,12445,12731,12923,13541. Four named content rows
remain outside the target:31892,31902,32010,32011. The first pair needs pet-tamer
progression/route integration; the second pair has reputation objectives but no
source starter/ender route in the inspected package. Importing metadata alone
would not produce working quests. Local-only quest31664 is retained. These are
content-port decisions, not unapplied ready-to-run SQL patches.

## Validation and rollback

All24 native CTests and14 Python logic tests passed. Visual Studio compiled the
server successfully. Its terminal helper failed for the newly generated test
project; the generated CMake/MSBuild target built that small check successfully.
No full gameplay campaign was run on this new native build, as requested. QA
acceptance remains separate from migration/startup checks.

Private rollback files:

- `out/full-history-20261109/remaining/runtime-before`: old2daf EXE/PDB pair.
- `remaining/runtime-before.json` and `runtime-build.json`: artifact hashes.
- `remaining/extended/{qa,played}-compatible/quest-tables-before.sql`: five
  public quest tables, including the original unsigned addon schema.
- Adjacent deployment manifests and `remaining/startup-comparison.json`.

Stop the target world, restore its matching five-table dump through a UTF8 byte
stream, restore the old matched EXE/PDB, then launch visibly. Do not restore
auth/characters or run the old unsigned executable against the new signed links.
The played dump is22,271,578 bytes, SHA256
`1162f6c7d8624dab0e79c89fb621d9db583bc456efca971b6f5e966a428ee914`.

Assembly uses a hash-keyed public reference cache and retries only incomplete
read-only SELECT responses. The deployer never retries incomplete mutations.
Exact SQL hashes, field preconditions and positive-prerequisite cycle checks are
enforced before applying. Reproduction of the exact changed-field plan requires
the pre-change public table snapshot; assembling against the already updated
world correctly reports fewer changes.

This completes review of the counted quest SQL backlog. It does **not** establish
that all historical SF C++ implementations, absent native bindings or unsupported
content are integrated. Keep the wider native/content inventory open for the
next integration/QA work; do not advertise this as an all-content certification.

The exact executed assembler87d35bba0c2ed0face247c3cb5856d4943570a682e7fd0d81e1d136ea4e8a759
is archived under `remaining/assembler-sources` in the private evidence directory.
The current generator also enforces quest-identity protection in its default
mode, preventing an older invocation from reintroducing the incompatible links.

Public sources: [SF27 database release](https://github.com/ProjectSkyfire/SkyFire_548/releases/tag/sf_db_27),
[pinned signed reverse-link implementation](https://github.com/ProjectSkyfire/SkyFire_548/blob/6f000f1878c547f04680cd7c269388a245132ff2/src/server/game/Globals/ObjectMgr.cpp).

The finalizer binds the raw assembly SQL to its matching report before adding
identity corrections. Three additional guard tests cover fresh assemblies,
repeat finalization and mismatched cached plans. The guarded follow-up reproduces
the same deployed SQL hash; it changes no live data or native artifact.
