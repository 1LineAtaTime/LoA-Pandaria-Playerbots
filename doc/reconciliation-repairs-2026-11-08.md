# Active LoA reconciliation and client-crash repair — November 8

The active game is mop_loa_*, not the historical SF deployment. Historical
release26/27 ledgers are retained as provenance, not current acceptance results.

## Repairs in this batch

- Gilneas Soultethered Banshee38473 sent animation tier8. Build18414 crashed
  at image RVA0x62b08e indexing the animation table with tier8 (RDI=8,
  indexed pointer0xfb). Two spawn addons and three template addons contained
  tiers outside0–4. SQL06 corrects invalid split values from legacy byte3,
  and two invalid stand states. Both addon loaders and Unit::SetAnimTier now
  reject invalid tiers before sending them. ExampleCharacter's personal row, position,
  health, auras and quest14212 were not rewritten. Client rendering acceptance
  remains separate from server/data checks.
- NPC text queries hardcoded BroadcastText1 whenever legacy dialogue lacked a
  broadcast ID, and discarded options1–7. Broadcast1 is the repressed line.
  WNPC now retains all eight alternatives. Reserved IDs0x10000000–0x1fffffff
  serve original npc_text strings, locale and emotes through BroadcastText DB_REPLY;
  no NPC dialogue is overwritten with a stock greeting. Unknown menu text uses
  a neutral greeting. The active broadcast table has no reserved-ID collisions.
- Port the SF prison-wave controller and all six matching NPC bindings for
  quest14154 from release-tag f7a9880, including completion follow-updc61c594.
  Preserve current ObjectGuid/chrono APIs and use authored direct rooftop legs.
  SQL07 disables the competing old periodic-dummy spawner binding.
- Port current SF Anzu encounter607dba2179 and preceding phase/add implementation:
  serialized brood waves, kill-based banish release, banish-safe event updates,
  Flesh Rip melee-player targeting and Dive. Retain LoA Druid spirits. Published
  cooldowns remain provisional, as upstream documents. SQL08 binds Brood AI.

## Provenance and rollback

Published release: https://github.com/ProjectSkyfire/SkyFire_548/releases/tag/sf_db_27
Prison event: https://github.com/ProjectSkyfire/SkyFire_548/commit/42163b724df3528c5906e58d0d6c9c5fce8b699d
Completion follow-up: https://github.com/ProjectSkyfire/SkyFire_548/commit/dc61c594e41b8e7737e1c88d0159a9753d21c0a8
Latest source checked:6f000f1878c547f04680cd7c269388a245132ff2.
Anzu: https://github.com/ProjectSkyfire/SkyFire_548/commit/607dba2179cf7dee3bf0b1247c992330c94b9969

Private full world/character/auth backups and matched EXE/PDB/config checkpoints
are under out/reconcile-20261108/checkpoint, with SHA256/size manifests. They
contain private state and are not committed. SQL06–08 were applied twice with
stable resulting bindings and zero invalid tier/stand values. No personal
account/character/shop/wallet row was modified by these patches.

## Current-state reconciliation

The new read-only audit_active_rollup.py checks historical *adapted* contracts
against the active schema. It understands SET loot-mode masks, float precision,
SQL quoted strings and changed-field rather than whole-row update semantics.
Initial corrected audit:18,766 checks;18,738 matched and28 differ. Differences
include intentional later native binding/proc overrides, deleted old spawn GUIDs,
calendar normalization, and overlapping Teldrassil SmartAI datasets. They are
not automatically28 missing fixes. Detailed results are private in
out/reconcile-20261108/active-rollup-before-v2.json. Broad reconciliation is not
complete and this document does not claim all published source changes or all
quests are implemented. Acquisition alone never counts as integration.

Visual Studio x64 RelWithDebInfo build succeeded; all23 CTests passed. Deployment
and live regression results will be appended after staging.

## Live results and additional reconciliation

- User confirmed ExampleCharacter loads and stays in world after the animation-tier repair.
  No password, position, quest or character aura was reset.
- Native headless client received the exact authored strings for text3583,
  text4938 and default16777215 through WNPC and BroadcastText hotfix packets.
  The disposable account and character were deleted and absence verified.
- The GM-observer Gilneas fixture seeded only documented prerequisite rewards,
  accepted14154 via the normal quest packet, observed first-wave alphas35167/
  35170 with rooftop movement and all four wave entries35167/35170/35188/35456.
  After125 seconds the quest persisted as complete. This verifies encounter
  spawning/timing, not ordinary-player combat difficulty or the full quest chain.
  All fixture accounts/characters, including initial harness failures, were removed.
- SQL09 restores14 keyed interactions: visual Scout Cage flags, Bloodfeather
  talk targeting, Darkscale spell-hit event, Withered Ent movement/cleanup,
  three sprite combat definitions and monk Arcane Torrent quest credit.
  Remapped event IDs retain local combat rows; links are adjusted together.
  Scoped backups precede the repeat-stable application and SmartAI reload.
- The corrected after-audit has18,748 literal matches and18 explicit compatible
  overrides/not-applicable records. Nine active-state checks verify their key
  dispositions. See active-rollup-audit-2026-11-08.json. In particular the current
  scripted-event objective sentinel-1 is required by this core: blindly replacing
  it with the published foreign-schema0 would break scripted quest credit.

A subsequent Crowley-ride trace exposed a separate viewpoint teardown defect:
SetClientControl(false) can release a camera that was never acquired, and late
release callbacks can refer to an older vehicle. Player::SetViewpoint now handles
those transitions without retaining a departing seer, removing a newer camera,
or adding duplicate vision listeners. Replacing an active viewpoint also releases
its previous listener. This fixes lifecycle state rather than lowering log levels.

The Anzu port has compilation/data coverage; a complete live dungeon fight is
still pending. Broad unregistered native bindings and unsupported SmartAI actions
remain in the diagnostic backlog. This batch does not certify every published
C++ fix, every archived SQL statement or every game feature as implemented.


## Final camera/startup regression and registry batch

The302cec2779 build passed the owned Crowley-ride start, mid-ride logout and
character re-entry contract with zero viewpoint diagnostics. The fixture uses
GM visibility and a seeded incomplete14212; ride start is verified by the server
trace emitted after both seats validate. Vehicle create-packet decoding is not
claimed as an independent renderer check. The account/character were removed.

All15 supported starter scenarios passed on that build and all15 owned accounts/
characters were removed. Eight declared unsupported scenarios remain untested;
the standalone runner returns exit1 for that lack of coverage, not for a failed
supported scenario. The wave encounter also passed its separate125-second case.

The fresh startup inventory contains860 missing registry names. Exact string
lookup finds83 in the release-tagged SF source and777 absent from that source.
Some matches are comments, retired encounters or alternative handlers, so source
presence is not automatically a required import. The full named inventory is
published-native-gap-inventory-2026-11-08.json. This establishes why a database
rollup cannot supply every missing implementation in this fork.

Eight already compiled handlers were left unregistered despite matching active
bindings: four Alysrazor trash AIs, their three helper spell scripts, and the
Shattrath flask vendor. Restore the registrations together, keep their existing
vehicle/encounter implementations and add null-hit guards to the two affected
spell callbacks. These templates have no competing SmartAI owner. The monk
Mana Tea legacy handler, retired Silithus event and commented ancestral-wolf code
are not blindly registered: they require distinct compatibility/ownership review.
Full Firelands encounter remains pending. On sourcef5a152480b,the normal-role
Shattrath fixture was denied trade without reputation,then received inventory
after controlled exalted setup for all three required reputations. The exact
owned account/character were removed. Startup missing names fell860→852 with
zero new exact diagnostic signatures. Raw evidence is
out/reconcile-20261108/shattrath-vendor-live.json and diagnostic-diff.json.
