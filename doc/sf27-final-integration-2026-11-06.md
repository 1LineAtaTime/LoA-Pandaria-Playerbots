# SF27 final integration and upstream refresh — November 6

This batch completes the pending SQL46–49 integration and incorporates official
SkyFire main through a69eda918b5bd416078fd8d832560d0fd4fb9023. The preceding spell
modules remain pinned to df19329b; the three newer upstream commits change only
Teldrassil, its route API and a creature visibility flag. Local navigation already
provides the required two-point route API. No Detour replacement is required.

## Compatibility scope

- SQL46: typed random suffix pools, six quest credit handlers, guarded duplicate
  cleanup, actual Thazz’ril loot owner and both Stonebull Crayfish click spells.
- SQL48: public quest, spawn, loot, path, pool, event, dungeon and Night Elf data.
  Spawn GUIDs are translated by identity/location; pool/path namespaces are
  allocated above existing values. Unmentioned local fields are retained.
- SQL47: native template detection ranges and matching live-reload query layout.
- SQL50: restore the three canonical heroic Defias vehicle click46598 rows and
  remove only two empty, unreferenced pool roots exposed by remapping.
- SQL49: authored factors for6312 identity-matched creatures; nine differing
  identities are excluded. Existing event multipliers and explicit local
  difficulty policy retain their roles.

Apply in order46,48,47,49,50 so detection ranges are initialized after final levels.
This requires the matched executable; the older binary does not use the new
columns or new encounter/vehicle scripts. Back up affected world tables offline.
Accounts, characters, mail, balances, custom store and auction/bot records are
not migration targets. Custom BattlePay/AH/playerbot implementation files are
not replaced by upstream files.

Native additions include published Glubtok firewall/blossom helpers, Darkshore
vehicle ownership/return, Night Elf follower/combat/corruption and nightsaber
ride, and monthly local-calendar Darkmoon scheduling. Original local Mist and
Barrow Den door registrations retain ownership. The unconditional upstream
Blade Flurry damage print is a DEBUG trace because a normal proc is not an error;
actual validation failures and loader diagnostics remain enabled.

The heroic Deadmines source references undefined loot groups50021–50048. Neither
the pinned canonical database nor this fork defines them. Those rows explicitly
name real items with maxcount1; adaptations retain the item/chance/maxcount and
set minimum count1 instead of creating fictional reference groups. Every such
adaptation is recorded in sf27-remaining-content-data.json.

## Acceptance boundary

Source/value/repeat/build checks and fresh startup are focused integration checks.
They do not certify every race/class quest sequence, all spells/items, long bot
load or a sanitized redistributable. Remaining actual content diagnostics and
user-visible gameplay defects remain open in project-checklist.md.

Deployment evidence will be appended after the paired migration and restart.


The first fresh start exposed five new diagnostics: three heroic vehicle click
rows missing from the published dungeon payload, and empty roots173/14176 after
members moved into remapped pools. Canonical reference data supplies the correct
vehicle rows; flags are not disabled. SQL50 guards pool deletion against creature,
gameobject, parent and child membership. First-start logs are retained privately.
All210/210 selected random bots logged in before the dependency restart.


## Paired deployment evidence

Visual Studio linked the final native source97d710dd9 with zero failed projects.
All22 CTests passed. The combined public-content fixture passes16564 checks,
including imported spawn/item dependencies; the migrations are repeat-stable.
The same16564 checks pass against the live world after SQL50. The overall world
backup is out/sf27-final-before-20261106-233945.sql,193989507 bytes,
SHA256cc8a52bbd472c29384bd03a02f8d2969d646db5049c83ea1a62846b344dc5167.
It contains49 affected public world tables. The SQL50 two-table supplement is
out/sf27-dependencies-before-20261106-234400.sql,1191052 bytes,
SHA256faacf0049e3209ccdf1491f6b99883c823a1e62fbd1cacc5234058199321f631.
The actual SQL backups and account data are not committed or packaged.

Original healthy EXE/PDB and logs are retained in
out/runtime-before-sf27-final-20261106-233945. The first updated startup logs,
including the five diagnosed dependency failures, are retained separately.
The exported native pair is:

- worldserver.exe: 71593472 bytes; SHA256 `02137e4a414eceb6d3f20e675721f5e2a1f808a44303a5e782d53cb588bbc4f9`.
- worldserver.pdb: 692408320 bytes; SHA256 `cf66a4d83d43c4da80b1348e1c74412fefbca6e810f2fc11d48ae537216ce245`.

No auth/character/mail/DP schema or record was migrated. The modules directory
and custom BattlePay, AuctionHouse and AuctionHouseBot implementation trees have
no diff from the previously deployed30cd0296b source. This does not mean shared
spell/core changes have zero indirect effects; gameplay acceptance remains open.


Final normal startup completed in20 seconds with world29544 listening on8085;
auth62868 stayed on3724. Reload creature_template6 exercises the new
detection/authored-factor query layout. Fresh startup contains1388 pre-existing
DB/content diagnostic records and zero new exact warning/error signatures versus
the previous healthy runtime. Server.log has zero warning/error records at this
observation. These1388 existing records remain real backlog; the server is not
claimed error-free. Log levels and filters were not changed by this batch.


Packaging verification covers580 manifest members with matching hashes and
untouched prior members verified. Forgejo main was fast-forwarded to the deployed
milestone; the read-only mirror check verified both repositories private, default
main, and all10 branch/tag refs identical. The scoped token cannot inspect mirror
administration settings, so MirrorSettingsVerified remains false; matching refs
are verified independently. Documentation follow-ups retain the same native tree.
