# NPC vertical knockback repair — November 6, 2026

The reported `Invalid spline velocity0` case is reproduced and repaired in source
00348248, with observation safety follow-up c2a21704. This is a real NPC movement
repair. Logging levels, spell data, navigation files and personal accounts were
not changed. Other content/runtime errors remain on the project checklist.

## Cause and behavior

The live trace identifies Iron Sentinel29984 (including82393/104702) receiving
Torn Earth55715, effect1: KNOCK_BACK with horizontal0 and upward12. The client
DBC contains that same authored vertical-only effect. The current chase generator5
was the previous movement state, not the caller supplying the speed; provenance
identified MotionMaster::MoveKnockbackFrom. The old travelling-spline initializer
required positive translational speed and could not represent the valid knock-up.
The user confirmed that no real player was logged in when the errors occurred.

- Stop the previous spline at its actual current position and keep X/Y fixed.
- Calculate the flight time from upward speed, gravity and the allowed landing
  floor. A same-height12yd/s launch lasts about1.244 seconds and rises3.732 yards.
  A lower landing floor receives the appropriate longer flight. Preserve hover
  offsets and transport-relative coordinates; unreachable/nonfinite plans fail.
- Initialize a two-point vertical parabolic spline using physical flight time,
  without dividing zero distance by zero horizontal speed. Its linear vertical
  baseline plus parabolic term permits a different landing height.
- Permit zero translational speed only for this finite, vertical-only, two-point,
  noncyclic/nonsmooth parabolic case, with no animation/falling conflict, zero
  time shift and a positive representable duration. Ordinary invalid zero-speed
  travel still fails validation and reports an error. No arbitrary speed is used.
- Keep the normal effect movement owner and restore subsequent movement through
  its existing completion path. No quest credit or player teleport is fabricated.

## Validation and a caught observer regression

All19 native CTests pass, including Torn Earth timing, lower-floor landing,
unreachable surfaces, invalid/nonfinite input, and rejection of ordinary
zero-speed travel. Five Python SQL/assembly tests pass. The prior458 live world
contracts and actual SQL32 backup recovery remain valid; this fix writes no world
migration. Natural Worgen progression and client rendering acceptance are separate.

The first physics trial observed correct lifts/landings and zero velocity errors,
but the new diagnostic sampler then crashed. The matching dump/PDB attributes
C0000005 to BotScenario.cpp:5037: telemetry called Duration() on an uninitialized
NPC spline. That function indexes an absent length array. This was an introduced
observation bug, not an accepted stable build. c2a21704 checks Initialized() before
reading duration, velocity or endpoint and reports explicit empty state instead.
The Windows runner also opens CONIN$ explicitly so redirected stdin cannot leave
it using an invalid console input handle. A failed cleanup is recorded separately.

The full guarded repeat on the current server completed:

| Observation | Result |
| --- | ---: |
| Reserved-bot snapshots |600|
| Nearby NPC snapshots |600|
| Uninitialized spline states safely sampled |391|
| Successful Torn Earth vertical launches |2|
| Active vertical samples |12|
| Maximum sampled rise |3.728394 yards|
| Return to landing floor observed |Yes|
| Invalid spline velocity errors in window |0|
| Server crash during repeat |No|
| Reserved actor back in saved area, confirmed live |Yes|

This was about121 seconds of actual NPC observation, not a natural quest
playthrough or a universal combat/soak certification. The reserved actor was not
in combat in this final sample; the world still generated Torn Earth's actual
NPC effect. No forced kill/credit was used. Initial and final positions, character
backup, failed-trial evidence and complete traces stay private under out/.
The corrected empty-spline path was exercised391 times, beyond the earlier
observer crash point. Active NPC records show constant X/Y, rising and falling
Z, followed by an inactive spline with zero remaining vertical displacement.

## Deployment, symbols and recovery

Visible worldserver43228 starts00:18:52 and remains running after the repeat.
Auth61228 remains unchanged. The exported files match the Visual Studio output:

- worldserver.exe SHA256: `98cbaa0656a3123b15dbefa0204a0ae9a2cb48043d55b712c82d5455fb21333b`
- worldserver.pdb SHA256: `3645ae51f3f82f696a4737bc13bf85da4dd22e50e23231c7b223f076f245ba9c`

The binary banner is00348248dirty because the guard was built before its commit;
source includes c2a21704. Identify the exact build by the paired hashes, not the
revision banner alone. The failed trial's8ce93b83 EXE/4dd355e5 PDB pair and logs
are preserved in out/runtime-before-final-nov5-20261106-001842, alongside the
private crash dump/report. They are not part of a sanitized release.

Current evidence: out/vertical-knockback-validation.json and
out/iron-sentinel-reproduction.json. The failed acceptance is explicitly recorded
in out/vertical-knockback-observer-crash.json. Actor recovery is confirmed by
out/iron-sentinel-actor-recovery.json and the final live return trace.

No database rollback is needed for this native-only change. To revert, stop the
server normally, preserve current logs/dumps and restore a previously matched
EXE/PDB pair. Do not mix symbols from different builds. SQL32 recovery remains
in stability-quest-continuation-2026-11-05.md.

## Remaining work

- [ ] Longer NPC/bot combat, death/relog and navigation soak; other invalid-speed
  producers if observed, and client-visible vertical animation acceptance.
- [ ] Broader hover/flying/transport variants and genuine quest progression tests.
- [ ] Remaining missing-native/SmartAI/spell/loot/vehicle/condition/navigation
  diagnostics. A clean test window is not an error-free emulator release.
