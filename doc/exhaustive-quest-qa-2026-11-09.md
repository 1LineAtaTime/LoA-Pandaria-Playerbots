# Exhaustive gameplay QA — November9

The full user addendum was read before implementation. Creation/login checks
and bounded starter assertions are not whole-quest passes. Historical raw
evidence is retained; qa_inventory now separates successful assertions from
overall UNVERIFIED quest status.

## Isolation and reusable identities

The owned runtime `out/quest-qa-runtime` uses fresh `mop_loa_qa_` schemas and
loopback ports3725/8086. World references were copied; personal accounts,
characters,mail and balances were not. Credentials remain in ignored files.
Ordinary exports ports3724/8085 remain separate. Production was not restarted
merely for QA. Provisioning is table-journaled,not a cross-table atomic snapshot.

The pool enforces **11 characters per account**,allocating more normal-role
accounts as needed. All122 planned bodies are provisioned across14 accounts,
plus three disclosed DK prerequisite anchors; none exceeds11 characters.
Snapshots use typed hex values; leases and SQLite WAL preserve per-stage
evidence. Foreign/expired leases cannot reset characters. Batched snapshot reads
replace hundreds of database-client launches per body.

Reset currently covers character GUID tables and owned item instances. Shared
guild/group/mail state is rejected. Account-wide progress,item-dependent child
records and universal shared-state reset remain open. Pandaren faction bodies
and Death Knight level55 prerequisite anchors use disclosed controlled setup;
they are not verified faction-selection or creation-eligibility gameplay. The
normal one-DK-per-realm limit is retained; extra accounts receive an auxiliary
owned anchor when required. Both bodies count toward the11-character cap.

A provisioning worker was stopped for a QA-only runtime update. Two interrupted
identities were inspected (level1,zero playtime,offline,no quest rows) before
baseline recovery. The original failed records remain. Persistent identities
are retained; legacy disposable runners now erase and verify the exact owned
character/dependencies before deleting its account.

## Catalog and real evidence

Active world:18,145 quests. Client CharBaseInfo and playercreateinfo agree on122
race/class pairs. The matrix contains6,183,814 explicit static level-boundary
cases:2,719,041 allowed by static gates,3,464,773 denied by one. Dynamic phase,
condition,prerequisite,spec,party,reset and reward-choice axes are UNEXPANDED.
This is planning,not six million executed tests or an exhaustive dynamic matrix.

All requested artifacts are generated under `out/quest-qa/catalogue`:

- QUEST_QA_MATRIX.csv (868MB;ignored).
- QUEST_QA_EXECUTION_RESULTS.csv.
- QUEST_QA_FAILURES.csv.
- QUEST_QA_COVERAGE.md.
- QUEST_QA_BLOCKERS.md.
- QUEST_QA_RUNBOOK.md.

One managed normal-role Orc rogue25152 lifecycle passed9 durable stages:
visible giver,decoded offer,normal accept/client quest log,delivery,exact
40XP/0money/+75Orgrimmar reputation,persistence,fresh authenticated reconnect,
observed auto-accepted25126 follow-up,and premature/duplicate reward rejection.
The same character was reused. Three earlier attempts exposed framework
assumptions; original records remain and reports classify them BLOCKED_HARNESS.
No game patch was made to satisfy those incorrect assumptions.

**Zero quests have exhaustive eligibility certification.** This bounded case
does not verify every quest,spell,item,profession,instance,boss or raid. Those
require additional mechanic drivers and independently reviewed outcomes.

## Sacrifices14212

The older boarding/logout regression never established the action bar,roundup
credit,survival or turn-in. The new isolated normal-role test accepts14212 with
real packets after disclosed prerequisite/level/location setup. No14212 credit
or completion is granted through SQL or GM commands.

Baseline trace:control enabled,ThrowTorch67063 action bar,then immediate control
revocation in the native horse callback. The candidate keeps spell controls and
uses UNIT_FLAG_DISABLE_MOVE to prevent steering while scripted splines drive.
Visual client acceptance remains separate from packet evidence.

The candidate compiled successfully through Visual Studio. In isolated QA,
20 actual ground-target casts earned **30/30 normal roundup credits** with the
horse alive and no subsequent control-revocation packet. This establishes
casting/credit,not yet full route completion/rewards or renderer acceptance.
Evidence: `out/quest-qa/sacrifices/20261109-091723/trace.json`.

## Running and remaining work

```powershell
python -m unittest discover -s contrib/quest-qa -p test_*.py
python contrib/quest-qa/prewarm.py
python contrib/quest-qa/run_delivery.py
python contrib/quest-qa/trace_sacrifices.py
python contrib/quest-qa/report.py
```

Read environment.py before privately setting MOP_QA_ADMIN_PASSWORD to provision.
Never print pool.json/config credentials. catalogue_matrix.py regenerates large
planning artifacts; report.py merges actual durable attempts afterward.
Dependency-aware invalidation dispatch,all dynamic eligibility axes,power/gear
profiles,and universal mechanic drivers remain open. Invincible/one-hit fixtures
cannot prove boss phases,death mechanics,balance or negative eligibility.

Six infrastructure unit tests and30 existing headless QA tests pass. Tests
cover boundary/DBC decoding,offer evidence,lease ownership/expiry and typed
snapshots,including allocation of a new account rather than a twelfth character.
They are infrastructure checks,not additional quest gameplay passes. A separate
live integration erased an owned disposable character,verified all dependent
rows gone,and only then deleted the account; no pool identities were discarded.

The longer vehicle run found an additional genuine defect: melee damage killed
the horse after objective completion,before Tobias. The reference escort's
final-escape protection was absent from the native port. The second repair
protects horse/rider after the second jump/talk (local point20),clears combat,
and restores only the temporary rider flag this ride added. It does not grant
credit or make the player permanently invulnerable. The corrected route reached
point34/completed1 alive. One reward assertion then exposed a harness oracle
error:75 authored copper x configured Rate.Drop.Money2 =150 copper. The game
correctly paid150; the raw failed75-only assertion is retained and classified
BLOCKED_HARNESS rather than patched into the server.

Final isolated result: `out/quest-qa/sacrifices/20261109-093339/trace.json` is
INTEGRATION_VERIFIED. Normal casts earned30/30,all33 route points completed,
Tobias35618 accepted turn-in,level4 XP increased0→455,and money increased0→150
(75x2). By Blood And Ash14218 was offered. A fresh authenticated reconnect
showed14212 absent from the active log and rider unit flags8:temporary protection
was removed. No14212 completion/credit was granted through SQL or GM commands.
Visual actionbar acceptance and a separately reviewed exact XP oracle remain
open. This bounded case is not exhaustive quest certification.

## Deployment and rollback

Native repair source92757b7b77 is compiled and staged as the matching EXE/PDB
pair in exports. The played worldserver was gracefully shut down and relaunched
through the visible launcher; world54316 listens8085 and unchanged auth44944
listens3724. Both database connections and206 random-bot logins initialized.
QA remains separate on3725/8086. No production database migration or personal
character/password/quest edit was needed for this native repair.

World EXE SHA256:
`0052018ed442a6278fcfea71f27b470bb39798e4982a6ec049a9a4749c7fe06e`.
Matching PDB SHA256:
`8d98e98faf98aceab269b7de22df8d0ed374c14a398e94e5da377f512008b0ed`.
The prior f5a152480b pair and its hashes are retained under
`out/quest-qa/deployment-before-sacrifices`. Authserver and database-client DLLs
were retained. Restore the matching prior pair during a graceful stopped-server
window if rollback is required; no SQL rollback is needed for this batch.

Forgejo main and the private GitHub mirror matched92757b7b77 across16 refs after
the source push. The final documentation update follows the same fast-forward
workflow. The played startup is not a claim that its remaining baseline content
diagnostics or every unsupported mechanic are fixed.
