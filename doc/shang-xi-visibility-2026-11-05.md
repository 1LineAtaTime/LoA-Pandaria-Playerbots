# Master Shang Xi bridge visibility — November5,2026

After Jaomin's29409 fight was repaired, ExampleCharacter could complete the objective but
Master Shang Xi disappeared at the bridge before turn-in. This is a separate
progression blocker; do not treat Jaomin combat acceptance as full-chain success.

## Live evidence and root cause

A new explicit read-only console command, `debug questvisibility ExampleCharacter 53566`,
captured the failing state. It does not mutate auras, phases, quest status or
positions, and logs only when an administrator requests a snapshot.

At13:59:01, ExampleCharacter was map860, zone5736, area5843, phase mask1, invisibility0 and
detection0. Bridge Master53566, spawn563502, was3.53 yards away, also phase1,
but invisible under type12 (mask4096, aura82343). Both visibility checks failed.
This was missing invisibility detection, not a map/phase-mask mismatch.

Detection aura105064 (SPELL_AURA_MOD_INVISIBILITY_DETECT, type12) is intended
between completion of29409 and the later29419 stage. Four subarea rules exist.
The live5843 rule incorrectly required29419 active/complete/rewarded (mask74),
and5846 required complete/rewarded (mask66). ExampleCharacter had not reached that quest,
so neither rule applied. The original LoA v1.0.4 source uses end mask9
(NONE/INCOMPLETE) in all four areas. Areas5825 and5834 already had9, explaining
why visibility changed at an area boundary.

## Repair and validation

SQL25 restores only those two mismatched end masks to9, guarded by exact keys,
start mask66, old values and other requirement fields. All other rows, NPCs,
spawns, factions, quest chain and personal character state are preserved.
Disposable fixtures verified exactly two changed rows, stable repeats and all
other data unchanged. The entire spell_area table was backed up first.

Applied to the static spell_area table and reloaded through its supported
console reload command; a player relog/area transition refreshes live auras.
No manual aura grant, GM visibility bypass or forced quest completion is used.
User confirmation, persisted quest progress and the post-fix snapshot follow.

The read-only command was compiled in Visual Studio (native e8799a9d), staged
with matching EXE/PDB, and exercised against the live failed case. Worldserver
PID49564 started via visible export launcher; authserver61228 unchanged.

Private spell_area backup: `<local-workspace-path>` (52,888bytes).
Prior runtime/log backup: `<local-workspace-path>`.

| Artifact | SHA-256 |
| --- | --- |
|worldserver.exe|`1784f270025196555461c1de9aaea40c4b2b24a81be121f2298e071b5b5e7cb5`|
|worldserver.pdb|`b9b856947171c13ada63252d14101e6515339b66146dcf8f82aef1f2cec3439d`|

## Separate Northrend warnings remain under investigation

Map571 is Northrend, while this visibility failure occurs on map860. The
reported (-2908.362549,4147.828125,41.296257) location indexes tile37/24.
Its terrain file0571_37_24.map is888 bytes (empty terrain); no matching
0571_37_24.mmtile exists. The swapped-coordinate tile0571_24_37.mmtile exists
but represents another location and must not be copied/renamed as a fix.

Current/saved character, corpse, homebind, spawn and graveyard-source checks
have not yet identified the caller/location lifecycle responsible for the
zone0 lookup. Neither these warnings nor their root cause are claimed fixed.
No Detour source, navigation configuration, map/MMap files or log severities
were changed. The existing navigation-crash/soak requirements remain open.


## Acceptance passed —14:01 local

User confirmed that stepping away and back made Master Shang Xi stay visible.
The post-fix read-only snapshot confirms ExampleCharacter in the same area5843/phase1 now
has detection mask4096 and aura105064. Bridge Master563502 remains invisibility
mask4096; both playerSees and npcSees changed from0 to1 at2.42 yards.
The NPC's authored invisibility was preserved rather than disabled.

Normal saveall persistence confirms29409 is rewarded and29410 is complete.
Thus the bridge turn-in and progression to the next quest are verified, with
no manual quest completion, faction/phase override or aura grant. The user's
initial “still disappears” report was withdrawn after the area transition.
The separate Northrend empty-tile/zone0 investigation remains open.
