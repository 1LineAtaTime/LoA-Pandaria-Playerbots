# Published repairs and automated gameplay coverage — November 5, 2026

The user authorized continuing through all applicable published repairs and
asked whether every quest, spell and item can be tested automatically. The
answer is substantial automation with explicit coverage; no universal complete
5.4.8 gameplay test suite was identified in the sources reviewed. A bot can
play a quest without checking every intended result. Successful SQL execution,
startup or one spell cast is not equivalent to full gameplay acceptance.

## Implemented and run now

`contrib/upstream-tests/audit_world_patches.py` evaluates the 296 published
release 26 SQL files independently against disposable world-table fixtures.
It creates a random temporary SQL account scoped to those fixtures, proves
that live SELECT and zero-row UPDATE are denied, and executes source patches
only through that account. No auth/character data is copied. Before each patch
it restores an independent baseline of affected world tables. Results include
SQL failure, no observed change, or fixture changes, plus counts/checksums and
schema hashes. Fixtures/account are removed in finally. The smoke pass of 8
patches completed, proved isolation and cleanup, and found 6 changed fixtures,
1 no-observed-change and 1 SQL failure. The complete 296 run finished at 14:42:
175 changed fixtures, 70 made no observed change, and 51 failed SQL execution.
The executor proved live-schema access denied and removed both temporary schemas
and its account. No live patch was applied by the runner. Fixture snapshots
use row counts, extended checksums and schema hashes, not cryptographic full-data
comparison. Tables are copied when first referenced, not in one consistent
transactional snapshot; rerun affected candidates after administrative changes.

The per-patch SHA256/source/result inventory is
[published-sql-compatibility-2026-11-05.csv](published-sql-compatibility-2026-11-05.csv).
These statuses do not establish deployment state or behavior. Candidates run
independently, not as one cumulative upgrade. Failures include absent legacy
tables/columns, required default fields and duplicate keys. SQL success can
still write a legacy shadow column that the current loader does not read.

`contrib/upstream-tests/audit_content_catalog.py` performed a read-only census:

| Catalog | Definitions/references scanned |
| --- | ---: |
| Quest definitions |18,145|
| Spell IDs (client plus custom server definitions) |95,337|
| Item definitions |80,022|
| Quest objectives |17,679|

Checks cover objective references (types 0–5), item spell references and
questgiver/turn-in relations. Findings: 9 missing item objectives, 2 missing
creature objectives, 2 missing gameobject objectives and 519 missing item-spell
references. Of the latter, 54 belong to items already quarantined by required
level >= 100; many remaining examples are obsolete class spell books. These
need intended-use/source/runtime review, not blanket removal or invented
replacement spells/items. Both tools explicitly record zero behavioral tests
and do not label the full catalog passed. Raw reports stay in ignored out/;
review findings without private account/character data are tracked in
[content-reference-findings-2026-11-05.csv](content-reference-findings-2026-11-05.csv).

## Existing local bot harness and its limits

The 7,000-line BotScenario.cpp harness already supports quest, spell/cast,
item/equip/use, combat, travel, instance and battleground probes/scenarios.
Its quest path can use normal RPG quest behavior and report acceptance,
completion and reward; optional modes also reset quest chains, clear other
quests, grant bags and spawn synthetic creatures/objects. Those are useful
mechanism fixtures but must be reported separately from a natural playthrough.

Never target a real user's character with mutation scenarios. Existing command
handlers can select real players. The user subsequently confirmed this entire
server is the test environment and authorized repair/gameplay batches here;
another server or realm is not a prerequisite. Use identifiable bot/test actors,
back up affected data, retain logs/dumps and record setup changes. Disposable SQL
fixtures remain useful for preservation/repeatability without another server.
Do not run indiscriminate unclassified mass casts/item-use tests.
Teleport/summon/kill/instance spells can move or destroy actors and destabilize
exactly the world being tested. Do not globally disable broken content to turn
an audit green.

## Coverage to build and maintain

| Layer | Expected automated assertions | What it cannot prove |
| --- | --- | --- |
| Native unit regressions | Boundary arithmetic, null/safe lifetime rules, protocol encoding, ownership/accounting | Full world/client behavior |
| Data/patch audit | References, schema compatibility, protected custom rows, repeat stability | Correct scripts or intended mechanics |
| Quest scenarios | Prerequisites, ordinary acceptance, natural objectives, credit, turn-in, reward, next quest, visibility | Unvisited branches/races/classes |
| Spell scenarios | Valid targets, range/LOS, resources, cooldown, effects, aura removal, immunity, rank/difficulty | Every encounter interaction or client visual |
| Item scenarios | Create/equip/use/consume, loot, mail, trade, relog persistence, no duplication | Correct item intent without a reference specification |
| Endurance/negative tests | Bot load, disconnect/evade/death/retry, bad inputs, dumps and leak/performance growth | Unlimited crash freedom |
| Real-client acceptance | Shop GUI completion/failure/cancel, packets, rendering and UI synchronization | Every client/version combination |

Each case needs a known expected result and bounded timeout, and must distinguish
PASS / FAIL / UNSUPPORTED / NOT_TESTED. A bot unable to navigate or understand an
escort is not automatically a broken quest. Test setup may seed prerequisites
for a phase fixture, but cannot claim a full-chain playthrough. Never force
quest completion to make a progression test pass. Retain separate character
fresh-start, abandon/retry, logout, pet/group, phase, escort/vehicle and instance
variants as applicable. Report both catalog coverage and behavior coverage.

First progression regressions are Jaomin 29409 and bridge turn-in/29410, then
the reviewed Gilneas chain and race/class starting-zone matrix. The prior
BattlePay UI hang is a concrete reason to retain real-client checks even when
balance deduction and item delivery pass on the server.

## Primary examples reviewed

- [AzerothCore Playerbots](https://github.com/mod-playerbots/mod-playerbots):
  questing/world/raid bots; requires its supported custom WotLK core, not a
  drop-in 5.4.8 testing package.
- [CMaNGOS automation options](https://github.com/cmangos/playerbots/blob/master/playerbot/aiplayerbot.conf.dist.in):
  quest automation, quest reward/equipment and spell-learning controls. These
  demonstrate autoplay capability, not complete validation of this fork.
- [Sunstrider-derived scripted spell tests](https://github.com/bahajan95/sunstrider-core/blob/db53b556897c8fbdc24b7231d1f6bd804b9472ef/src/server/scripts/Tests/spells/spells_warlock.cpp):
  explicit TestPlayer/dummy/expected-result test cases. Different expansion/API;
  useful architecture, not a wholesale source import.

The 195 tracked open issues include reports without ready-made fixes. The 296
published SQL updates include schema/option/script assumptions that differ
locally. Apply compatible repairs through the existing backed-up migration and
native build workflow, adapt conflicts, and retain unresolved issues honestly.
