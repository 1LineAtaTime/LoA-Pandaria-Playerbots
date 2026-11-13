# LoA beta integration and automated starter coverage — November 7, 2026

## Source and scope

Active game remains the customized LoA restoration with `mop_loa_` schemas.
The refreshed stable upstream `efe7f29107` adds CI/documentation since our original
`e135742e18` baseline, not gameplay repairs. The beta branch at
`8a26cd41fece10e18335ebc078f6ff7e99412c2d` has 150 commits beyond stable.
Those commits are inventoried in `loa-beta-backlog-2026-11-07.json`; **this is not
an assertion that all beta changes are installed**. Historical SF deployment
reports do not describe the restored runtime.

Sources are the actual published LoA commits:
https://github.com/1LineAtaTime/LoA-Pandaria-Playerbots/tree/8a26cd41fece10e18335ebc078f6ff7e99412c2d
Individual native changes and their disposition are in
`loa-beta-integration-2026-11-07.json`. Original authors retain attribution in
upstream Git history. This batch imports their scoped diffs, preserving local
BattlePay, companion, AH actor-selection and navigation implementations.

## Native batch

25 published native fixes address chain-spell and pending-auction lifetime bugs,
invalid combo-target pointers, phase-change iterator invalidation, raid/script
null dereferences, Assembly of Iron recursion, Azuremyst captive player lifetime,
guild recipe persistence, battle-pet slots, far-teleport position state, neutral
ranged retaliation, stationary totems, creature guardians, Hogger surrender,
creature animation kits, scripted event objective credit, auto-accepted quest
notification, health-reduction auras, alternate power and pet aura persistence.
The Displaced Energy fix was adapted to this branch's aura container.

Two more fixes are already present: single AHBot insertion and creature-template
reload selecting `ct.entry`. The summon/escort lifecycle patch needs its preceding
SmartAI changes and matching vehicle data; it is explicitly pending, not silently
applied as an isolated hunk. No Detour change or logging suppression is included.

## SQL batch and rollback

`2026_11_07_52_world_loa_beta_starters.sql` combines five published LoA patches:
quest event/POI metadata, Tirisfal camp heights, Speak with Sevren prerequisites,
scoped quest-target respawn delays, and stationary training targets. It does not
set arbitrary quest mobs stationary. The upstream broad missing-waypoint patch
was deliberately not included: removing patrol movement is not an adequate repair
for the user's missing-roaming report.

Before mutation, seven world tables were backed up to private
`out/loa-beta-pass-20261107/world-before-starter-batch.sql` (58,534,543 bytes;
SHA256 `ba50849e6cbd584aae776a1199c93761eea6864206d456ed6b46dd38452e8ac2`).
Rollback requires stopping worldserver, restoring that table backup, and restoring
the paired previous native binaries if rolling back native behavior. No auth,
character, wallet or shop data is changed by this world migration.

## Automated gameplay harness

`contrib/headless-client/starter_campaign.py` creates uniquely named temporary
normal accounts, authenticates using SRP, creates characters, immediately enumerates
and enters the world. It walks short routes and sends ordinary gossip/quest packets.
It verifies persisted quest acceptance, rewarded status and XP after normal logout.
No GM quest completion, teleport, level grant or item grant substitutes for gameplay.
`--combat --only-human` additionally attempts six real Blackrock Worg kills and
requires actual server objective-credit packets before turning in the quest.

The starter matrix covers 13 starting races with 14 race/class login combinations;
Death Knight creation on an empty account separately checks the expected level
restriction. This is not a Death Knight starting-zone playthrough. Character
creation coverage is distinct from five quest acceptance checks and three delivery
quest reward checks. It does not certify all quests, spells, items or classes.
Accounts/characters are cleaned up through native console commands and checked
absent; user characters are never selected by the cleanup.

The initial harness run incorrectly selected Mage/Panda Warrior quest IDs for
other classes and treated the normal Death Knight prerequisite rejection as a
failure. Those are test-setup defects, not evidence to change class eligibility.
MariaDB responses now require a completion marker so an incomplete empty response
cannot be mistaken for missing content. Authentication/session secrets are not logged.

## Remaining work

- Reconcile the remaining LoA beta native/SQL dependencies in the inventory,
  especially complete Forsaken/Gilneas/Goblin scripts, escorts and movement.
- Add collect, item-use, spell practice, gossip choice, escort, phase/vehicle and
  instance scenarios with explicit credit and reward assertions.
- Recheck BattlePay success, insufficient funds, cancellation and delivery on the
  fresh schemas; add protocol automation for those flows.
- Exercise companion summon, party healing/buffs, travel, loot passing and AH
  selling/buying end-to-end; startup counts alone are not acceptance.
- Continue real loader/content errors, map data and outdoor achievement conditions.
- Package a clean account-free install only after the applicable acceptance suite.

Deployment/build/results are recorded below when completed.

## Deployment evidence

Visual Studio RelWithDebInfo x64 completed with 13 successful projects, zero
failed projects, and 33 up-to-date projects. Changing Player.h required a full
consumer rebuild (18m28s). All 22 registered CTests passed. The 408 existing
starting/training/Gilneas/port data contracts also passed after the SQL deployment.
The five-source SQL batch changed all six affected table checksums and a second
application changed none. Backup includes quest POI points as supporting context.

World shutdown was requested normally; SQL ran during the shutdown interval,
then binaries were copied only after process exit. Both EXE/PDB pairs match the
Visual Studio outputs. Authserver and worldserver were relaunched through the
visible export launchers (PIDs 36836 and 17464 respectively). Existing database
client DLLs remain in exports; the rebuilt servers connected successfully to
MariaDB 10.11.19 with the restricted mop_loa_server account.

Fresh SQL diagnostics dropped from 1,389 unique signatures to 1,320: 69 resolved,
zero new SQL signatures. No warning/error threshold changed. Other real content
errors remain. No new crash dump was observed during this bounded check.
At a runtime sample there were 199 online characters, 6,501 auctions with 818
owners, and 615 BattlePay products. These counts are loading/activity evidence,
not transaction or gameplay acceptance.

The optional pre-deployment human combat route received two genuine objective
credits, but failed to reach six within its bounded static-spawn route. It is
recorded as incomplete; the harness needs live mover/health/death tracking before
it can reliably follow roaming targets. It must not manufacture kills or declare
that quest passed. The Worgen matrix route was corrected to begin with Lockdown!
14078; All Hell Breaks Loose requires Lockdown! and Something's Amiss first.

Post-deployment campaign: 14/15 cases passed initially (14 successful creations/
world entries and expected Death Knight level-gate rejection; one human quest
acceptance assertion was intermittent). The isolated human retry passed after
adding a bounded persistence wait. This does not prove the intermittent cause
is fully repaired. Orc25152, Undead24959 and Worgen14078 turn-ins all persisted
with XP rewards. Panda30039 acceptance passed. No forced objective completion
was used. All disposable accounts and characters were verified removed after
waiting for asynchronous deletion; three delayed soft-deletions needed a second
native purge command. The harness now waits for each deletion stage.
Public per-case results, including failures, are in
`loa-beta-gameplay-results-2026-11-07.json`. Complete raw reports remain private
under `out/loa-beta-pass-20261107`.
