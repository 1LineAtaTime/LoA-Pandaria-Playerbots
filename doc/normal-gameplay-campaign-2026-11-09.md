# Normal gameplay campaign and published-fix integration

## Validated and deployed result

Native2daf99f5f0 contains50 newly fetched LoA beta commits (merge1500aed86b)
and the local reward-replay repair. Both QA and played worlds have the paired38
published SQL files and14 versioned alias triggers. Matching EXE/PDB are in
exports; played world12452 listens8085,unchanged auth44944 listens3724.

All23 bounded natural cases have a latest PASS across five quest contracts:
7Orc,7Undead,7Worgen,1neutral Pandaren monk,and1Human warrior. That covers three
faction states and nine classes. Paladin/Death Knight gameplay and most of the
122 provisioned bodies remain unverified beyond infrastructure checks.

Batch20261109-103538-dc20ce passed22 and earned six Human kill credits,but an
immediate asynchronous-save assertion failed. The completed state was verified;
resume20261109-104920-954433 completed the normal return walk,turn-in,500XP,
100copper,275Human-adjusted reputation,item57255x1,reconnect and follow-up. No
credit was granted or replayed to replace that checkpoint. Original failures
remain linked; the return-route bound was expanded to400yards after inspection.

Sacrifices20261109-105025 passed separately on this exact binary with disclosed
controlled entry; it does not certify naturally played prerequisites. Final
Pandaren20261109-105942-f37d83 paid exactly4copper despite duplicate requests.
The unchanged rerun20261109-110212-d03cad scheduled0/cached1.

All23 native CTests,13 QA infrastructure tests and32 headless tests pass. The
SQL replay is checksum-stable across24 tables. Corrected QA startup has zero
new warning/error signatures against the recorded baseline. Existing baseline
content errors remain visible. **This does not certify the whole game.**

## Repeatable entry point and evidence

```powershell
./contrib/windows/Run-QuestQA.ps1 -Workers 4
./contrib/windows/Run-QuestQA.ps1 -Select pandaren-introduction
```

The wrapper runs45 Python checks,campaign and report export,and fails for
blocked/failed gameplay. The campaign waits for owned QA ports,rejects native
source differing from the staged build,and records native identity separately
from framework source. New batches archive content-addressed inputs with database
credentials redacted. Queued workers reject changed framework files. Earlier
runs retain their original hashes/journals but not all newer source archives.

Explicit resume supports verified completed-combat/save checkpoints and their
return-route continuation. Objective/reward/eligibility semantics,native/world
dependencies and ownership must match. This is not universal crash recovery;
unknown state or live leases remain blockers. Conservative reference checksums
cover incompletely mapped dependencies. Overpowered diagnostics are not natural
acceptance passes.

The report retains66 bounded attempt records,including3 launch blockers
recovered from original batch journals with batch-time bounds disclosed.
QUEST_QA_LATEST_CASES.csv separates latest outcomes from historical failures.
There are24 latest bounded passes:23natural plus1controlled vehicle case.
The6,183,814 static matrix rows are planning,not executed gameplay. Exhaustive
quest eligibility certification remains0.

## Policy

Normal power is the default acceptance profile. This runner performs no level,
gear,quest-credit,completion or reward grants. It restores owned fresh-character
baselines,requires level1 and rejects elevated account ranks or explicit RBAC
grants. It walks and uses normal game packets. Diagnostic fixtures and controlled
entry scenarios remain separate evidence; they cannot certify normal progression.
Whole-quest certification stays false even when one bounded contract passes.

## Reusable execution

`contrib/quest-qa/campaign.py` expands five reviewed contracts to23 valid
race/class cases,covering Horde,Alliance and neutral Pandaren. It uses the
existing11-character-cap pool. Contracts cover four introduction deliveries and
the Human warrior's six-wolf fight,quest reward item and follow-up. The batch
also checks exact XP (including level rollover),configured money,reputation,
item counts,duplicate-reward rejection and fresh authenticated reconnect.
Human reputation includes Diplomacy's10% bonus:250base becomes275.
The Undead reward spell91768 remains an explicit undecoded effect,not a pass.

Each account is a serialized lane; different accounts run concurrently. Every
stage writes durable JSON and SQLite evidence under ignored out/quest-qa. A
successful cached result is reused only when contract,framework,server EXE,
configuration,reference DBC and selected world dependencies match. Failed or
interrupted attempts are not cached as passes. This is a bounded dependency set,
not yet a complete dependency graph:global script tables,spawn groups and shared
world state need broader invalidation. Worker timeout recovery remains limited
by the account lease/offline safeguards; do not automatically steal live leases.

```powershell
python -m unittest discover -s contrib/quest-qa -p test_*.py
python contrib/quest-qa/campaign.py --workers 4
python contrib/quest-qa/campaign.py --select human-wolf-combat --force
python contrib/quest-qa/report.py
```

The combat driver no longer permanently excludes an attempted creature GUID.
Observed live health and a bounded retry delay allow another attempt or respawn.
The client also faces the target before attacking. No damage/stat increase or
quest credit injection substitutes for actual combat.

## Public-source refresh

Read-only upstream fetch on November9 found SkyFire main unchanged at
6f000f1878c547f04680cd7c269388a245132ff2 and LoA beta advanced from8a26cd41fe
to68f8098431. The new LoA range contains50 non-merge commits and50 changed files:
38 world SQL updates plus native Hillsbrad/Stormpike scenes and scoped core fixes.
One fixes a Gilneas gunship's incompatible gameobject display type,which can
crash the client. The range is not yet deployed at this checkpoint. It must be
merged preserving local changes,built,and paired with backed-up SQL before use.

Public references:

- https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/compare/8a26cd41fe...68f8098431
- https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/commit/073ed82d78709c2a389f2227ba961e5e938daa8b
- https://github.com/ProjectSkyfire/SkyFire_548

## Current execution checkpoint

The23-case baseline completed against92757b7b77:18PASS,4BLOCKED,1FAIL. Three
BLOCKED results were nonblocking pool-metadata lock collisions; bounded lock
waiting and avoiding unnecessary metadata writes repair the harness. The other
was a Pandaren giver reported absent:53566 has VehicleId1416,so its valid F15
GUID must be accepted alongside ordinary F13 creatures. Subsequent probing also
identified legitimate first-login auto-acceptance; normal quest-log observation
now handles it without fabricating a separate manual acceptance.

The Human warrior earned actual credits but died before six. Subsequent natural
attempts reached5/6. The driver now faces targets,uses only observed learned
abilities,allows longer legitimate fights,waits for the normal combat timeout
and uses ordinary regeneration. No stat boost,GM heal or credit grant replaces
the fight. This is still unverified until a complete replay passes.

## Real economy defect found by the negative test

Pandaren30039 paid4copper with one choose-reward request and8 with two,while XP
was paid once. The diagnostic one-request run is explicitly ineligible for
acceptance because it omits the duplicate-negative case. The reward-choice
overload of Player::CanRewardQuest only checked storage and never called the
common completion/objective/repeatability gate. Turn-in quests could reach it
again after being rewarded. The repair invokes the shared gate and silently
ignores duplicate nonrepeatable acknowledgements in the handler. It does not
change the payout or mark the observed8copper as expected. Post-repair evidence
must prove exactly one payout and the correct follow-up.

Source merge1500aed86b preserves the50 upstream non-merge commits and local
BattlePay/AH/bot code; no merge conflict occurred. Both world schemas have full
world-table-read-locked backups (including routines,events and MyISAM),with
SHA manifests under out/quest-qa/published-68f8098431. Earlier transaction-only
backups and the failed limited-user routine dump are preserved as such. The
38 SQL files are pinned/hash-checked and applied only to a stopped selected
realm by published_batch.py. No auth/character data is overwritten. QA native
staging and post-patch results remain pending at this checkpoint.

## Clone fidelity and reproducible schema

The initial clone omitted all triggers to avoid cross-schema writes. Modern
SQL then filled creature_text.CreatureID while the native reader used entry,
causing a false missing-group diagnostic for48922. The played schema already
had the correct triggers; the published text was present. reference_schema.py
now exports/installs only14 audited NEW/OLD row assignments on seven allowlisted
tables. It rejects outside statements/procedures and omits definers. No arbitrary
source routines/events or cross-schema triggers were cloned. Versioned SQL is
2026_11_09_00_world_loa_compat_alias_triggers.sql. Replaying38 updates restored
the aliases; the subsequent replay left all24 table checksums unchanged.

QA also received1707 public bot-text templates; no personal accounts/characters
were copied. Future provisioning includes this reference and safe alias metadata.
The per-commit inventory is published-loa-batch-68f8098431.json. Published
integration does not establish gameplay for all Hillsbrad scenes or other content.

## Artifacts and rollback

World EXE SHA256:bad4dad484e6b9475bf10e925075c2926db6c6aa6f018aba05a75a1f01f59617.
PDB SHA256:d8da70fa58d9f19b06f2b9488b09221d157dc781cc6ed51bceb8db74dbc9b232.
Authserver and database connector DLLs were retained. No BattlePay,AH or bot
service source was overwritten by this upstream range; shared-core changes
receive the named regressions. Full new in-game service certification is pending.

Full read-locked world backups,old92757b7b77 matched EXE/PDB,and source hashes
are under out/quest-qa/published-68f8098431. Rollback requires a graceful world
stop,restoring that matching pair and the selected world's world-before.sql.
Do not restore auth/characters or copy QA data over played schemas. The batch
does not alter personal accounts,passwords or character quest progress.

Remaining work includes other races/classes/chains,repeatable/currency/choice
negatives,professions,all item/spell effects,class rotations,instances,bosses,
shared reset,universal recovery,independent oracles and visual client checks.
