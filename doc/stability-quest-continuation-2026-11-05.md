# Stability and Worgen progression continuation — November 5, 2026

The user selected runtime stability/errors and quest progression as the current
priorities. Source4f904145 implements the previously missing Sacrifices14212
questgiver and horse handlers and hardens unsafe actor/vehicle access in other
Gilneas scenes. This is an implementation/data-validation milestone; no complete
natural Worgen quest playthrough is claimed.

## Concrete stability defects repaired

- Krennan rescue horse: after logout/removal, GetPlayerForEscort can return null.
  Check the player and current vehicle before restoring/restricting control.
  Dismount no longer marks the player as newly seated.
- The delayed rescue announcement retained a passenger pointer across two seconds.
  Capture its GUID and resolve a current player still riding this horse instead.
- Josiah's two delayed callbacks retained player/summoner pointers across scene
  changes. Resolve the existing actor GUID at execution; a disconnected player
  is not dereferenced or replaced by another nearby actor.
- Guard vehicle-kit access in the rescue, mountain-horse and nested caravan scenes.
  Missing/removed vehicle components must not turn passenger queries into crashes.

## Sacrifices quest implementation

The older [MoPCore source](https://github.com/alexkulya/MoPCore/blob/4fd6a16a053dd4f35c63e3969bf3761401b3885f/src/server/scripts/EasternKingdoms/Gilneas/gilneas.cpp)
was reviewed as a reference. Its disabled questgiver,21-point assumptions and
unchecked nearby-actor/player accesses were not copied. This installation has an
existing33-point35231 route and different authored jump/dialogue positions.

- Questgiver35552 starts the ride on quest acceptance and offers retry gossip for
  an incomplete quest. Preserve ordinary quest menus; reject combat/dead/on-vehicle
  starts and duplicate living rides owned by the same player. A retry is available
  at the original questgiver, not a forced quest completion or teleport shortcut.
- Summon a player-owned horse and its own mounted Crowley in seat1. Replace only
  that actor's legacy nearest-horse SmartAI, preventing simultaneous rides from
  stealing another horse's companion. Horse35231 remains a native movement owner.
- Copy the authored route into owned memory, validate all33 contiguous points,
  and follow two jump legs1→2 and18→19. Accept only the current point and correct
  motion type; duplicate/out-of-order callbacks cannot skip points. Schedule the
  next leg after jump finalization so MotionMaster cannot overwrite it.
- Restore control, remove passengers and despawn owned actors on arrival,
  dismount, death, disconnect, quest removal or a45-second leg stall. Missing
  route/seat failures release the rider for retry and retain actionable errors.
- Preserve firebomb67063, existing Bloodfang Stalker SpellHit/timed-action credit,
  quest objective35582×30, normal turn-in and authored dialogue. Arrival grants
  no credit, reward or quest completion. Mounted Crowley is passive/nonattackable.

SQL32 changes two creature-template rows (mounted backend and retry-gossip flag)
and removes one superseded source0 mounted-Crowley SmartAI row. It preserves
its old unbound timed list and every other row. Path/text/quest data is unchanged.
Deploy only with the matching native build, with worldserver stopped.

## Verification and recovery

Visual Studio final native build succeeded. All19 CTests pass, including new
33-point/two-jump, wrong-type, duplicate and stale-callback route assertions.
Five Python SQL/assembly tests pass. All458 live data contracts pass:442 retained
plus16 Crowley. Both the pre-deployment fixture and the actual-backup recovery
fixture pass full-row preservation, repeat stability and full baseline restoration;
all temporary schemas were removed. These use reviewed SQL as administrator,
not an untrusted-SQL sandbox and not gameplay certification.

Seven-table pre-SQL32 backup: out/crowley-ride-before-20261105-204200.sql (26,732,472 bytes), SHA256 `c23ac4a105db8e228e883fb4db7c6500769db569ec8ac7b9c3b18c08b1b325cc`.
The actual dump was imported into disposable schemas and tested through upgrade,
repeat and restoration. Keep it private; stop worldserver and preserve newer
work before restoring all seven tables. A Git revert cannot undo database writes.
Restore the old matching EXE/PDB if reversing the native source. Do not place a
restoration dump in automatic updates or overwrite custom data with a foreign DB.

Private reports: out/crowley-fixture.json,
out/crowley-actual-backup-recovery.json, out/crowley-final-*.json and
out/crowley-log-comparison.json. Initial native deployment: visible world49008
started20:42:30, auth61228 unchanged. All three new handlers load; missing native
names952→950, with no new exact startup error lines. Existing errors remain.

## Newly reported zero spline velocity

Later runtime messages identify ordinary Iron Sentinel29984, including spawn
104702, with explicit velocity0 while configured run/walk/swim/flight speeds
remain positive. Chase generator5 is the current generator, not proof that chase
itself supplied the speed: spell jumps/knockbacks launch before switching the
active generator. A playerbot may trigger combat, but that is not evidence of a
playerbot movement defect. The follow-up trace identifies Torn Earth55715
requesting horizontal0/upward12 through MoveKnockbackFrom. Native00348248
implements a physical vertical arc; c2a21704 fixes a diagnostic-sampler crash
found during validation. The guarded600-snapshot repeat passes with a3.728-yard
lift, landing and zero velocity errors. Ordinary invalid travel still errors.
See vertical-knockback-repair-2026-11-06.md for symbols, evidence and limits.

## Remaining acceptance

- [ ] Natural14212 acceptance, seat/UI/firebomb credit, all33 points/two jumps,
  normal turn-in, two simultaneous players, disconnect/dismount/death retry.
- [ ] Krennan rescue and Josiah scene disconnect/relog under the new guards.
- [x] Reproduce and repair the Torn Earth vertical-only NPC knockback, then pass a guarded live observation. Other producers and longer soak remain open.
- [ ] Continue missing-native/SmartAI/spell/loot/vehicle/condition and navigation
  work; retain real diagnostics. No Detour or navigation files were changed.


## Outdoor SmartAI flag triage retained for the next repair batch

Read-only inspection confirms that1706 has only dungeon-normal-gated events on
two map0 spawns;1538 has raid40 flags512 on ten map0 spawns;20075 has flags6
and occurs both on map1 and in dungeon269. These flags explain the runtime
"no events added" diagnostics, but blindly removing them is unsafe:1538's
current rows include vehicle/passenger/self-kill behavior despite its Scarlet
Friar template, and20075's dungeon behavior must be preserved. Establish authored
quest/combat roles and correct source bindings before activating those rows.
Infested Bear47204 spawn175727 has MovementType2 with no spawn addon/path; the
correct authored path or movement mode still needs evidence. A fabricated path,
random-wander replacement, or disabled diagnostic is not a verified repair.


Current final native artifacts, paired symbols and guarded runtime43228 are
recorded in vertical-knockback-repair-2026-11-06.md. The Crowley implementation
also uses fully triggered server boarding, five-second cleanup for failed entry,
and proximity checks on delayed Josiah movement. Natural client acceptance remains
open; prior SQL32 backup/contracts are unchanged. Initial deployment PID49008 is
historical, superseded by the documented later builds.
