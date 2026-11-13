# LoA starter and runtime repairs — November 7, 2026

## Character creation

The create handler sent CHAR_CREATE_SUCCESS immediately after queuing
SaveToDB(true). With two asynchronous character database workers, the client's
next enumeration could execute before the creation transaction committed and
return an empty list. A later login could see the character, matching the report
of successful creation returning to the creation screen.

Human first saves now wait for the database worker's commit result before the handler acknowledges them.
The handler verifies the new GUID's account ownership before publishing success,
realm counts or name caches; a failed commit returns CHAR_CREATE_ERROR instead.
The modern character cache is populated as well as the existing world name cache.
Routine saves and bulk bot creation retain their asynchronous execution.
The database error itself remains in diagnostic logs if a commit fails.

The first live regression exposed a defect in the initial implementation:
DirectCommitTransaction tried to execute async-only prepared save statements on
a synchronous connection and asserted. That binary is superseded. The corrected
implementation waits on AsyncCommitTransaction's worker future, preserving the
proper prepared-statement connection type. The failed build's crash text/dump
remain private in exports/Crashes for traceability.

`contrib/headless-client/character_creation_regression.py` checks creation,
immediate enumeration and world login on the same real 5.4.8 connection. It has
no reconnection or sleep between the success response and enumeration. Use only
an empty disposable account through MOP_ACCOUNT/MOP_PASSWORD; protocol logging
is disabled to avoid exposing authentication material.

## Screenshot warnings and loot lifecycle

Seven Arena.SeasonReward defaults were present in the matching source template
but absent from the active worldserver.conf. They are now explicitly set to
those documented defaults: cleanup enabled, keep achievement 2091, cleanup
achievements 2093/2092/2090, titles 44/43/45, and all three rating thresholds -1.
The previous config is backed up privately under
`out/loa-first-content-pass-20261107`. No logger threshold changes are included.

The shared DoLootRelease path could be called internally after a loot-view entry
had already been removed. It then tried to remove it again and continued through
container/corpse teardown. A stale release is now acknowledged without repeating
teardown. It clears the active GUID/looting flag only if that stale window is the
active window. The normal loot-view ownership check and normal release path
remain authoritative. This repairs repeated release processing rather than
downgrading RemoveLootedObject's diagnostic.

## Worgen progression

The fresh ExampleCharacter is a level-2 Worgen rogue, map 654/zone 4755, with quest 14093,
All Hell Breaks Loose. Human appearance is expected at that stage of the
[Gilneas starting story](https://www.wowhead.com/guide/story-worgen-starting-area-lore).
The racial altered-form aura is not granted by forcing every new level-1
character into wolf form.

The existing LoA scene uses area 4786 after Last Stand (14222) is rewarded to
apply Curse of the Worgen 68630, then Last Chance at Humanity (14375) rewards
68639 and releases the pillory scene. The active spell_area row, native scene
bindings and reward spell are present. All 85 Gilneas data contracts pass.
These checks do not establish a complete natural transformation playthrough;
that remains an explicit acceptance item.

## Wider quest and mob audit

- All 306 starting/training, 85 Gilneas progression and 17 existing native-port
  assertions pass: 408/408 read-only data checks.
- All 22 native CTests pass after rebuilding the changed code.
- The broader reference census covers 18,145 quests, 17,682 objectives, 95,337
  client/custom spells and 80,022 items. It identifies 9 missing item objective
  references, 2 creature references, 2 gameobject references and 519 item-spell
  references for triage. A missing reference may belong to obsolete/quarantined
  content; these are not 532 automatically proven live gameplay failures.
- The census is tracked in loa-content-findings-2026-11-07.json. It reports zero
  automated full quest playthroughs and does not claim all gameplay passed.
- Criteria 17826/17831 belong to outdoor Hyjal achievements 5868/5869,
  And the Meek Shall Inherit Kalimdor / Infernal Ambassadors, against Millagazor
  (52649, map 1). Their local type-18 requirements wrongly call a dungeon
  InstanceScript. They need actual outdoor achievement condition handlers.
  Do not delete those conditions or award credit on any kill merely to remove
  the error. This remains unresolved and visible in the logs.

## Release evidence

Source and runtime hashes, real protocol regression results and fresh startup
observations are recorded after staging. Existing BattlePay/AH/playerbot
functionality is retained; this batch does not replace world tables, reset
characters or wallets, or certify every quest/mob as fixed.

The restart also exposed an auth launch-order defect: a realm hidden by the
worldserver's temporary shutdown flags left the filtered realm list empty and
terminated authserver. Auth now distinguishes zero configured realms (a real
startup error) from configured but unavailable realms (listen and await the
existing periodic refresh). No realm is advertised online or unhidden by this
change.

### Dreaming Glory starter action repair

Installed Spell.dbc identifies 28698 as Dreaming Glory, a herbalism effect. The
world incorrectly assigned it as spell button 7 in 110 race/class starting bars.
New characters and bot initialization copied that button without knowing the
spell, generating repeated invalid action errors on later login.

World migration `2026_11_07_51_world_dreaming_glory_starter_action.sql` removes
only button 7/action 28698/type 0. Character migration
`2026_11_07_02_characters_dreaming_glory_action.sql` removes the same invalid saved
button only when the character has no learned 28698 spell. No learned spell,
other action slot, item, character or wallet is removed. Private scoped dumps
precede the changes. Deployment changed 110 world rows and 413 character rows;
repeating each migration changed zero rows. A normal world restart reloads the
starting bars. No native rebuild is needed for these two data migrations.

### Final observed release

- Native source revision: `f73779f9a5`; both EXE/PDB pairs match the Visual
  Studio x64 RelWithDebInfo output. All 22 native CTests pass.
- Final world PID 3896 listens on 8085 and initialized in 17 seconds; auth
  PID 47872 listens on 3724. Auth stayed listening with zero available realms
  during the launch-order test and subsequently discovered ExampleRealm LoA through
  its normal periodic refresh.
- The same-connection regression passed for a normal human rogue (map 0) and
  Worgen rogue (map 654), including a repeat after the action-bar data repair.
  Each received success 47, appeared immediately in a one-character list and
  entered the world. The disposable accounts and characters were removed using
  native console commands; ExampleCharacter remains unchanged.
- Fresh startup/test logs contain no missing Arena.SeasonReward settings,
  Dreaming Glory invalid-action messages or stale RemoveLootedObject messages
  during this observation. This is a bounded observation, not a long soak.
- The 408 read-only content contracts, the broad census and both real protocol
  regression logs are retained under `out/loa-first-content-pass-20261107`.
  Private runtime backups and the superseded failed native pair remain there.
- Other pre-existing content diagnostics, including the unsupported outdoor
  achievement conditions described above, remain unresolved and visible.
