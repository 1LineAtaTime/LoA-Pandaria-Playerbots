# The Disciple's Challenge29409 — Jaomin Ro repair

User-reproduced progression blocker: ExampleCharacter, neutral Pandaren race24, level2,
quest29409 incomplete, map860. Jaomin54611 becomes friendly/green when the
player approaches. No character faction, phase or quest state was changed to
bypass progression. Those fields remain governed by the quest chain.

## Findings

- Active template: AIName=SmartAI, empty ScriptName, faction2104 for both
  teams. Installed FactionTemplate.dbc identifies2104 with friendly player
  masks. Native sparring faction2357 has no friendly/hostile masks or mutable
  reputation list. The existing compiled boss_jaomin_ro was unused.
- Dialogue is present: one intro and six victory variants. A gossip menu is
  not required; the authored/upstream encounter starts dialogue on proximity.
- The separate old native handler also lacked complete resets and guarded
  credits. The existing SmartAI path and hawk actions were incompatible with
  a native encounter, including an upward135-unit relative hawk move.
- Spell108955 already summons57750. Explicitly summoning another hawk, as
  upstream also did, would create two competing vehicles. This repair lets
  the spell summon one and tracks its exact GUID, not a nearby stranger.
- Spell108937 already includes native knockback; extra scripted knockback
  would duplicate it. Upstream's double-jump lag fix informed the single-owner
  ascent/landing sequence without modifying global spell movement behavior.

## Repair

Native boss_jaomin_ro now owns intro, defensive sparring, nonlethal defeat,
eligible participant/group credit, five-second friendly recovery, full reset
and the spell-summoned hawk. Damage from questless/non-player attackers cannot
complete or kill him. A threshold or lethal hit is intercepted before damage
is applied; credit is guarded once per encounter. Pending-quest participants
within40 yards receive normal group-event credit, including player-owned pets'
contributions. The recovery timer updates after combat stops. Normal evade
clears events, summons, participants and temporary auras for retry after combat
loss or a challenger disconnect. No scheduled lambda captures a player pointer.

The hawk has a dedicated passive native AI; its movement and despawn belong to
its owning Jaomin. Nearby vehicles are not borrowed. SQL24 binds both actors,
sets rival faction2357 and retires35 replaced SmartAI rows plus two conditions
on their obsolete SetData event. It preserves all seven dialogue rows, spawns,
quest definitions, next-quest links, character data and world phase data.

Either customized actor binding prevents migrating both actors. Disposable
SQL fixtures verified exactly two template changes,35 old AI rows and two old
conditions replaced, all other data preserved, stable repeats and custom
binding preservation. A new native threshold regression test covers normal,
threshold, lethal, zero, invalid-health and overflow cases. All17 CTests pass.

The first compile caught an API-name mismatch for clearing threat; corrected
to this core's GetThreatManager().ClearAllThreat(). Native build then passed.
Final runtime deployment and user acceptance results are recorded below after
startup. Compilation/policy tests are not an in-game victory/credit result.

## Upstream sources and remaining campaign

Reviewed primary sources:
- [SkyFire's Jaomin implementation](https://github.com/ProjectSkyfire/SkyFire_548/commit/d50a59cb5c3a5d36a5ca3e7c19cf1474f6ff3a15)
- [Hawk/vehicle follow-up](https://github.com/ProjectSkyfire/SkyFire_548/commit/23eed67128d0032c50d6b1ea19c6107e5c24ef8b)
- [Double-jump lag correction](https://github.com/ProjectSkyfire/SkyFire_548/commit/d12963adc4528289cb29b4309ee1226e5495874b)
- [Pinned current encounter source](https://github.com/ProjectSkyfire/SkyFire_548/blob/505f6e63a20c3ea75dfebb17f58e891f4e5fb44c/src/server/scripts/Pandaria/zone_wandering_island.cpp)

The complete candidate inventory is [SkyFire upstream worklist](skyfire-upstream-worklist-2026-11-05.md):195 open issue reports and296 archived release26 SQL
fixes. These are upstream candidates, not491 confirmed local bugs or completed
repairs. The three previously identified Gilneas fixes remain next starting-zone
priorities. Preserve custom databases; do not replay the entire archive blindly.

## Deployment — November5,13:45 local

Native source through4b6b2f48, embedded revision1339, compiled through Visual
Studio RelWithDebInfo x64 with zero failed projects. Final17/17 CTests pass.
SQL24 applied with worldserver stopped; all seven dialogue rows verified
byte-for-byte unchanged and all35 replaced SmartAI rows absent. ExampleCharacter remains
quest29409 incomplete before testing; no quest completion was injected.

Private database backup: `<local-workspace-path>` (26,757,236bytes).
Prior runtime/log backup: `<local-workspace-path>`.

| Artifact | SHA-256 |
| --- | --- |
|worldserver.exe|`9a9c73760e0ed8649b92c355c551b96bec4cbca5d689400a81e93db0559ddb5a`|
|worldserver.pdb|`9aeb0c9ad653805566a4238932ec0d658cc490504ef960435e93e81596ce5941`|

Built/export hashes match. Visible export launcher used; authserver remains
unchanged. User confirmed: “Jaomin stays attackable and quest completes.” The preexisting unrelated
startup/content errors remain visible in file logs and are not claimed fixed.


## User acceptance —13:47 local

ExampleCharacter's live retry passed: Jaomin remained attackable at close range and the
quest completed on defeat. This confirms the reported progression blocker is
resolved. Turn-in/next quest, unrelated second challenger, pet/group credit,
longer hawk sequence and evade/disconnect replay remain separate acceptance
checks; do not infer all starter quests work from this single quest result.
No new Jaomin/vehicle-related errors were recorded during this test.
