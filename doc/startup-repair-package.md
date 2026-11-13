# November 4–5 startup repair package

The active local databases already received the scoped repairs recorded in
startup-repair-2026-11-04.md. Do not import a full external world database or run
every historical update in this repository. Preserve accounts, characters,
BattlePay catalog/balances, custom loot and client navigation assets.

For another installation with the same starting schema/data, take backups and
review each scoped migration before applying it while worldserver is stopped.
The exports/sql/world folder includes:

1. Existing 2026_10_04_70 guard aliases, 2026_10_15_40 through _43 per-map pool
   splits, _48 SmartAI parameter repairs and _51 retired starting skill lines.
2. November 4 _01 through _17: legacy text key compatibility; guarded unused
   pool cleanup; unused condition fields; retired access keys and generic script
   binding; empty pool choices; equal-chance loot groups; legacy dialogue;
   registered aliases; absent classic references; verified obsolete spell
   bindings; sourced invalid lock fields; sourced missing quest dialogue;
   original LoA dialogue restoration; Deathknell aura rules; source-verified
   quest-area state masks.
3. November 4 _18 / _19 and November 5 _01 through _07: original LoA object
   field repairs, two missing authored dialogue groups, verified spell-role
   bindings, explicit authored SmartAI bindings, rejected unused flag bits,
   absent-client criteria extra-data, player-only customer greetings and the
   taskmaster's aggro dialogue recipient.

4. November5 _08 through _10: verified class/encounter spell-role bindings and
   guarded marksman/scorpid SmartAI action repairs. Deploy the matching native
   executable with these script-role migrations; see startup-batch-repair-2026-11-05.md.

5. November5 _11 through _23: sourced missing messages/loot, combat dialogue
   speakers, legacy action compatibility, sentinel bindings, spawn difficulty
   masks, disabled-copy links, missing quest polygons/property pools, glyph
   bindings, reserved equipment IDs, Flamekin enrage and contaminated property pools and duplicate class bindings. SQL20 requires the
   matching native glyph build. See startup-repair-continuation-2026-11-05.md.

Apply only relevant migrations in chronological order. The older _44 and _46
pool cleanups conflict with real data on this installation and are deliberately
excluded. The separate _00 flight movement update predates this startup pass;
its presence is not authorization to reapply it without checking current data.
The September broad spell-binding deletion migration must not be applied
wholesale: some affected spells need source corrections and remain functional.

Use the executable and matching PDB together. Active worldserver.conf contains
relative data/log directories and the corrected case-sensitive Battleground
keys. Keep local database credentials in the private active configurations.
Launch Start-Worldserver.cmd and Start-Authserver.cmd once each for visible
consoles; a second auth process cannot bind the same port.

Current logs are exports/logs/Errors.txt, DBErrors.log and Server.log. Run
`python server_log_report.py --logs logs --source all --scope current` from the
exports folder after a fresh boot. Old logs/crashes are archived under ignored
out/runtime-diagnostics-archive-20261104-214116, outside the distributable folder.

This package is an incremental repair build, not an error-free release. The
remaining script/content errors and gameplay acceptance checks stay open in the
project checklist and startup repair report.


November5 _24 is the Jaomin Ro29409 progression repair. Deploy native revision
1339/source4b6b2f48 or newer with this migration, which switches both boss and
hawk from the old SmartAI subsystem to coordinated native handlers. All seven
authored dialogue rows are preserved. See jaomin-repair-2026-11-05.md; this is
not a full-world replacement or proof that other starter quests are repaired.


November5 _25 restores two sourced Master Shang Xi type12 detection-area
end-state masks for the29409-to29419 transition. See shang-xi-visibility-2026-11-05.md.
After deployment, reload spell_area and refresh the player's area auras through
relog/area transition. Preserve character quest/phase/faction state.


November5 _26 adapts published Stair of Destiny flight-menu fixes to this fork's
option IDs. Introductory rides accept active incomplete/complete delivery quests;
normal taxi maps no longer require their intro quest to have been rewarded.
Native1348 additionally includes SF1408 null-target safety and actionable
failed graveyard zone context. See map571-and-published-fixes-2026-11-05.md.
Map571 remains under investigation; no repaired navigation pack is implied.

November5 _27 adapts the numeric training level/eligibility subset of seven
published starting-zone files. It changes223 guarded fields across111 quests,
correcting the source's Simple Sigil3106/3116 mix-up. The deployment passes all306
data assertions, plus full-row preservation, repeat stability and tested backup/
rollback recovery. Quest playthroughs and other portions of the seven source
files remain pending. See starting-training-batch-2026-11-05.md. No native rebuild
is required for _27. Apply with worldserver stopped after backing up both quest
tables; restart the visible launcher. Never put the manual rollback file into
the automatic updates directory or assume Git alone restores database state.

November5 _28/_29 pair with native7f75c1fb or newer: activate complete Hot Rod
friend/quest and Worgen pillory/release ports, correct authored dialogue IDs and
repair nine training-chain links. Native additionally implements immediate
SmartAI cannon event26277 with validated topology/chance, preserves complete
legacy world-event sequences, and registers Nightmare Vine/Sinestra handlers.
Back up creature_template, quest_template_addon, smart_scripts, spell_script_names
and spell_area, stop worldserver, deploy the paired native/data changes and
restart visibly. Both SQL files are repeat-safe and guarded for the inspected
baseline; divergent installations must run the tracked postconditions.

Combined357 data checks,18 native tests and an actual17-NPC event sample pass.
These are not full quest playthroughs. Full logs still contain953 missing native
names and other content defects. See native-content-campaign-2026-11-05.md for
artifact hashes, backup/restoration evidence and remaining work. Never place
manual reversal SQL in the auto-update directory or distribute private dumps.


November5 _30/_31 are a paired Gilneas migration with native source through
77777246 (ride/catapult implementation fce8533f). Apply both offline after backing
up conditions, creature, creature_addon, creature_template, creature_text,
gameobject, gameobject_template, gossip_menu_option, quest_template_addon,
smart_scripts, spell_script_names, vehicle_template_accessory and waypoints.
The only schema change makes waypoints.orientation nullable/defaultNULL; retain
original values. Text writes populate both legacy/runtime actor identities.
Reserved ship template800100 and GUID4100001..4100004 must be unused before a
new installation. Run the tracked fixture/contracts against divergent data;
this package does not authorize overwriting a custom world database.

SQL31 is required: align Lorna's actual SmartAI/dialogue backend, remove obsolete
survivor SmartAI under the active native handler, and defer Crowley's incomplete
boarding bridge. Its complete native ride remains work to do. Do not apply _30
alone or clear missing scripts merely to quiet diagnostics. The pair passes85
new plus357 retained data checks and actual-backup restore/repeat/preservation.
Native19 and Python5 tests pass, along with live gunship floor and cannon-event
mechanism checks. Natural quest acceptance is still required. Worldserver and
matching PDB are staged; authserver did not need changes. See the included
GILNEAS-PROGRESSION-2026-11-05.md for hashes and recovery. The package is not yet
a sanitized public release and does not claim zero errors.


November5 _32 requires source4f904145 or newer. It switches only mounted
Crowley35230 to the owned passive native actor, removes its one superseded
source0 nearest-horse SmartAI row, and adds retry gossip to35552. It leaves all
route, dialogue, spell and objective/credit rows unchanged. Back up all seven
tables listed in stability-quest-continuation-2026-11-05.md and apply offline
with matching binaries.16 new data contracts, preserved rows, repeat stability
and actual-backup restoration pass. Natural quest acceptance is still required.


Native00348248 with c2a21704 or newer additionally supports physical vertical-only
NPC knockbacks (Torn Earth55715) and guards empty splines in diagnostic snapshots.
No SQL/data change is required. Deploy the paired EXE/PDB and retain exact symbols;
see VERTICAL-KNOCKBACK-REPAIR-2026-11-06.md for hashes and the bounded real lift/
landing regression.19 native tests pass. Other startup/content errors remain open.
