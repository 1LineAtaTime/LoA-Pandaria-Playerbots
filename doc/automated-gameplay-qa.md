# Automated gameplay QA

Current architecture: [November9 exhaustive QA audit](exhaustive-quest-qa-2026-11-09.md).
It adds an isolated reusable pool,11-character account cap,durable stages and
explicit full-catalog UNVERIFIED accounting. Historical creation/login assertions
below do not establish complete quest gameplay.

The normal-power batch entry point is now
`contrib/windows/Run-QuestQA.ps1`; see
[normal gameplay contracts](normal-gameplay-campaign-2026-11-09.md).
It uses the isolated reusable pool,not disposable players on the played realm.

The [quality automation entry point](quality-automation-2026-11-08.md) now combines
checkpoints, audit comparisons, required-scenario protection, log triage and live
suites. Its new workflow mode composes normal client actions and observes live
positions/health/visibility; implemented actions and positive gameplay coverage
are reported separately. Historical runs below remain scoped to their own tools.

The [quest dependency baseline](quest-dependency-audit-548/README.md) now maps
98,115 links across all18,145 quests and attributes reference/native-binding
findings to affected quests. It is a separate read-only structural layer, not a
replacement for this normal-client campaign or independent expectations.

## What exists now

`contrib/headless-client/qa_run.py` combines read-only content audits, a normal
SRP/world-protocol client campaign, and explicit coverage accounting. It runs
against the existing test server. No second server or account password is needed.
Only uniquely named disposable accounts/characters are created. Native console
commands remove those exact test identities afterward. Personal characters and
accounts are never chosen as test subjects.

The version 1 scenario file contains 15 supported cases: fourteen race/class creation
and login cases, expected Death Knight prerequisite rejection, five starter quest
acceptances, and three delivery turn-ins. The delivery contracts specify exact
XP/money deltas (Orc 40/0, Undead 10/4, Worgen 80/30). Quest progress is checked after
logout, and quest cases re-enter the character on the same connection. Route discovery reads the world
DB; rewards are compared to tracked expectations rather than copied from it.
These expectations are reviewed local fixtures, not independent proof of retail
accuracy. The legacy --combat static-spawn route remains experimental and is not
counted as completed combat coverage.

Eight scenario entries explicitly record missing kill/collect/escort/phase/spell/
boss/shop/companion/auction capabilities, including fresh network reconnect. They are untested, never passing. The
entire suite cannot be reported ready while gaps, blockers or failures remain.

## Running it

The Windows wrapper checks that the target worldserver belongs to this repository:

```powershell
./contrib/windows/Run-GameplayQA.ps1
./contrib/windows/Run-GameplayQA.ps1 -AuditOnly -ReportDirectory out/qa/audit
```

Equivalent commands from the repository root, with both export servers running:

```powershell
python contrib/headless-client/test_qa.py
python contrib/headless-client/qa_run.py --world-pid (Get-Process worldserver).Id --report-dir out/qa/latest
```

Read-only audit without creating characters:

```powershell
python contrib/headless-client/qa_run.py --audit-only --report-dir out/qa/audit
```

Use --config and --client to override the config/client paths, and --suite for a
reviewed JSON scenario file. Credentials are read privately from the runtime
configuration and passed to MariaDB in the environment. No auth/session packet
trace is captured. Raw reports belong in ignored out/, not public source control.
Do not use shell redirection/captured subprocess pipes for the campaign: its
Windows native-console cleanup requires inherited console handles.

## Results and interpretation

- qa-report.json: machine-readable data and gameplay results, counts and scope.
- qa-report.md: a readable scenario/status/last-step summary.
- gameplay.json: per-character assertions, expected/actual reward evidence and
  verified account/character cleanup. Temporary character names contain no secrets.
- trace-N.json: each supported scenario's latest step journal, flushed as it runs;
  useful if interrupted before the final report is written.
- quest-coverage.json: every quest and its objective descriptors, initially
  untested. Acceptance-only is partial. Only a verified rewarded journey counts
  as passed. Database-derived descriptors are candidates, not a correctness oracle.

Passed means the specified assertions passed. Failed means an assertion or content
contract was violated. Blocked means infrastructure or a harness limitation
prevented a verdict. Untested means there is no implemented scenario/run.
Missing tables/columns fail the audit as blocked rather than disappearing.
Empty suites never count as success. A failed check keeps the last reached step.
Exit 0 means no failed/blocked checks in this run; it does not mean full coverage.
Consumers must also inspect ready and complete, which stay false for untested gaps.

Data audits check quest actor/objective references, creature/item/gameobject
objective targets, addon ownership, waypoint paths, SmartAI ownership, and shop
item references. A failed data audit is an investigation target: mixed native/
SmartAI ownership may require review rather than blindly enabling or deleting AI.
These audits do not infer all spell behavior from database references.

Twelve framework tests guard false success: untested/blocked/empty results cannot
be ready; failures remain failures; invalid statuses, duplicate IDs, SQL-shaped
quest identifiers and unspecified delivery rewards are rejected before mutations.

## Extending coverage

Add independent expectations and a reusable executor together. A mode with no
executor must retain an explicit reason and stays untested. Never implement a
kill/collect test by granting credit, teleporting past the route or completing a
quest with GM commands. Setup shortcuts, if later needed for high-level fixtures,
must be recorded separately from the player actions being validated.

Next executors, in priority order:

1. Live mover/health/death tracking for genuine kill objectives, then inventory
   collection/drop checks and item-use/target checks.
2. Gossip choices, phase visibility, escorts and vehicles for starting chains.
3. Class spell fixtures: cost, cooldown, target eligibility, aura lifecycle,
   damage/heal ranges and party targeting, including negative cases.
4. Boss state transitions, adds, wipe/reset and rewards.
5. BattlePay success/insufficient funds/cancel/retry with charge and mail checks;
   companion party/travel/loot and auction bids/buyouts/settlement.
6. Optional differential adapter against a separate reference executable. None is
   connected yet. Normalize GUIDs, randomness and time; differences require review.

## Limitations

This first release makes existing starter checks reusable and adds coverage/audit
reports. It does not automate all 18,145 quests, all spells or all encounters.
The client does not render the UI; a limited real-client check is still required
for UI, animation and visual defects. A clean console is never acceptance evidence
for every gameplay feature. Interrupted campaigns can leave disposable accounts;
consult the per-run report/trace and remove only recorded identities through the
normal console. Automatic global deletion of QZ-prefixed accounts is not used.

## First execution findings

The first full run passed ten creation/gating cases and reached expected quest
persistence/reward assertions in five quest cases, but those five cases failed
when opening a fresh authenticated world connection. That failure remains in the
historical report. Its cause is unresolved; it is not evidence that quest rewards
failed. The supported re-entry step was changed to enter the character again on
the existing connection after logout. Fresh network reauthentication remains an
explicit coverage gap, not a claimed repair.

The expanded audit covers 18,145 quests, 57,634 creature templates and 80,022 item
records structurally. It detected two missing creature objective references,
nine missing item references, two missing gameobject references, three orphaned
addons, two missing paths and 62 SmartAI ownership conflicts. These are findings,
not automatic deletion instructions. No content rows were changed by this QA run.

## Comparing behavior across revisions

For two reports produced with the same scenario suite:

```powershell
python contrib/headless-client/qa_compare.py out/qa/baseline/qa-report.json out/qa/latest/qa-report.json --report out/qa/comparison.json
```

The comparator flags a previously passing scenario becoming failed, blocked,
untested or missing, and increased structural violation counts. Different suite
names are rejected. A previously passing build is a regression baseline; it is
not automatically correct retail behavior. The independent expected values in
scenario fixtures remain the authority for the specific assertions they cover.

Final scoped quest retest: 5/5 supported quest cases passed, including exact delivery
rewards and character re-entry. All five retest accounts and all fifteen original
run accounts/characters were verified removed. The first run's five network
reauthentication failures remain in the public results summary. Structural audits
finish with four passing and six failing checks; those findings are on the repair
backlog. A nonzero exit from these runs is expected while content audits fail.

See automated-gameplay-qa-results-2026-11-08.json for compact evidence. Full private
reports and traces are under out/qa-20261108. Current runner versions record suite
and harness hashes so a result can be tied to the expectations and executor used.

The final one-case smoke test also passed on the final executor, including step
journaling, persisted acceptance, character re-entry and verified cleanup. The
Windows wrapper audit completed and propagated the expected nonzero exit for
existing content violations. No server rebuild or restart was required for these
Python/PowerShell QA tools. Both existing export servers remain running.

## Unattended full run — November 8

Ran the Windows wrapper on source d2a80b89 with the final 23 scenario suite.
All 15 supported gameplay scenarios passed;8 capability gaps stayed untested.
All 15 disposable accounts and characters were verified removed, with an additional
read-only account-residue check returning 0. Both servers stayed running. No
production gameplay assertions were bypassed with GM completion or item grants.
The command returned exit 1 because six structural audits failed, not because a
supported gameplay case failed. Four structural audits passed.

The complete quest coverage queue reports 3 rewarded journeys, 2 partial acceptance
checks and 18,140 untested quests. This is the initial regression baseline, not a
claim of complete game coverage. Store this raw report for comparisons with runs
using identical expectations: out/qa-20261108/unattended-full/qa-report.json.
Public counts, audit samples and coverage gaps are in
[unattended results](automated-qa-unattended-2026-11-08.json).

## Targeted gossip-item fixtures

The gossip_item executor creates a disposable character through the normal client
protocol, then sets its offline level/location and optional incomplete quest as
explicit fixture preconditions before first world entry. Those setup steps are
recorded and do not count as quest acceptance or completion. Actual gossip,
item delivery, duplicate selection and logout use the normal client path. The
persisted item count is asserted after logout, and the test account is removed.

```powershell
./contrib/windows/Run-GameplayQA.ps1 -Suite contrib/headless-client/qa-legendary-scenarios.json -ReportDirectory out/qa/legendary
```

The two initial contracts check item 69238 delivery once for active quest 29193 and
no delivery/option without that quest. They do not establish the complete legendary
quest chain or visual ceremony. Fresh network reauthentication remains a separate
unresolved capability; these fixtures do not depend on it.

## Content inventory and scenario planning

The [5.4.8 master catalogue](content-master-list-548.md) supplies version-labelled
ID/name/reference lists for scenario planning. Treat reference-only rows and name
differences as review queues, not automatic failures or verified retail expected
behavior. Catalogue output and gameplay evidence remain separate.
