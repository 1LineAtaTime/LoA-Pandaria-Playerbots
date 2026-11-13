# Batched warning/error repairs — 2026-11-05

Later repairs and current limitations: [November 5 continuation](startup-repair-continuation-2026-11-05.md).

Batched repairs applied; an error-free release is not yet achieved. The user asked for
batched verified fixes, especially MMaps and the remaining console spell errors.
Do not lower validation/logging levels or invent empty navigation assets.

## First applied batch

SQL08 removes seven exact wrong-role spell bindings, preserving existing valid
passive/glyph/trigger chains. Sword and Board46953, Mirror Image55342/63093/
110621, Levitate111758 (cast by1706), Healing Elixirs122280 remain. The obsolete
Druid23218 area callback only queried an area and returned true; its native
self speed aura remains. Shaman specialization passives are86108/86099/86100
in SpecializationSpells.dbc and retain native eight-piece armor checks;86529
is a dummy learned/tooltip spell, not an aura. Real-schema fixtures passed exact
seven-row removal, preservation and repeat application. Private backup:
out/class-spell-roles-before-20261105-064733.sql (209000 bytes).

Native changes fix Command Demon force-cast suppression, per-spell gateway
destination selection, the copied Divine Shield variant's optional glyph-heal
effect, Levitate removal callback type, and ICC Geist alarm's SEND_EVENT hook.
An intermediate build crashed in startup validation because Register used the
runtime-only GetSpellInfo accessor. Commit40c43d4f corrected this to registered
spell metadata via m_scriptSpellId before the next restart. The failed image and
matching symbols are preserved in out/runtime-before-final-nov5-20261105-065108;
its dump is under out/startup-registration-failure-20261105. This was a test
deployment regression, not a preexisting server fault. No account data changed.

## MMap investigation and confirmed cause

Read the navigation incident/validation/checklist context. Detour is unchanged.
The installed loadMap already correctly returns true for an already-loaded
tile. Current remaining warnings refer to files that do not exist, not corrupt
existing tile headers. A read-only input audit initially checked VMap filenames
in the wrong axis order; StaticMapTree names VMap tiles Y_X while terrain/MMap
files use X_Y. Do not treat that preliminary collision-presence list as proof.

A separate staging directory out/mmap-warning-20261105 contains read-only
junctions to maps/vmaps/dbc plus copied original parent parameters. The first
generation attempt lacked the dbc junction and returned -5 (missing LiquidType).
After supplying it, generation for0000_20_44 returned0 without an output tile;
do not call that a successful repair or manufacture a tile. Nothing generated
in this staging area was installed in exports.

Temporary Windows stack-RVA logging in Map::LoadMMap preserved WARN and added
bounded INFO-only context. LLVM symbolization against matching PDB proved:

- Darkmoon974 central tiles: Creature::Create <- transport_merry_go_round::OnCreate.
  Six carousel passengers were created at local offsets near0,0 before world
  conversion, causing unrelated empty grids to load.
- Outland530 / Northrend571 central tiles: Creature::Create <- LoadCreatureFromDB
  <- Transport::CreateNPCPassenger. The same local/world mixup affects DB riders.

Evidence: out/mmap-request-callers-20261105.txt. Candidate repairs set transport
context before DB loading and transform coordinates before Creature/GameObject
creation. Carousel world coordinates are computed before creation, while local
offsets remain in transport movement/home data. No header/API or Detour change
is required. These source edits passed the subsequent build/live startup; central passenger
requests disappeared. Temporary tracing was then removed after resolving the
remaining empty-space callers.

## Encounter and validation repairs

SQL09 removes eleven exact wrong-role bindings and installs three real trigger
bindings. Full-schema temporary fixtures checked the exact row sets, preserved
all other bindings, and proved repeat stability. Applied while stopped with
private backup out/encounter-spell-roles-before-20261105-070821.sql (208736 bytes).
This moves Mark of Nature checking to periodic25041, Rain Dance positioning to
missile124864 and Dark of Night selection to123742. Native aura/trigger chains
remain intact. Recursive Titan Gas aura callbacks and callbacks that attempted
AddAura on non-aura dummy spells were removed; their actual native damage,
vehicle, summon and aura mechanisms remain. This is not encounter acceptance.

Native repairs align Anraphet's random destination, Benedictus's single-target
wall selector, Xin's cone/area variants, Ta'yak's dummy strike, Water Strider's
self aura, Blackfuse's source-area target and Spicy Explosion's actual hostile
area effect. Geist Alarm70740 has its event at effect2, unlike70739 at effect1.
Explosive Ring registration respects effective SpellMgr destination corrections,
including144194's unmodified DBC destination. Validation reads registered spell
metadata safely, never the runtime-only spell pointer.

The unused Rimok SpellScript was removed from the periodic producer. No new
120202 binding was added: its old filter removes players and was not verified
as appropriate for the triggered spell. Existing periodic handling stays intact.
Mark of Nature's missing server-only25042 behavior was checked against the
[pinned AzerothCore definition](https://github.com/azerothcore/azerothcore-wotlk/blob/b704af465112832706b2c389b5cc9b9f5aad61cb/data/sql/base/db_world/spell_dbc.sql):
radius12=100 yards, enemy marked players, sleep25043. The periodic checker now
performs that behavior directly; death marker25040 stays a marker.

SmartAI text validation now checks an explicitly selected creature's text rather
than the script owner's text. Dynamic speakers remain checked at runtime. This
restores valid conversations without inventing or duplicating dialogue. Real
missing groups remain errors; reported event IDs now identify the actual row.
The first comparison changed101 rejected text-action diagnostics to51 actual
speaker diagnostics. This is not proof every newly admitted quest scene works.
Some remaining GUID targets demonstrably point to the wrong NPC: Yorik27857's
old targets133302+ resolve to Wild Turkeys32820. Restoring 'Turkey Hunter!' from
an unrelated source would be incorrect; no such text was imported.

SQL10 changes only two guarded SmartAI fields: marksman8338 uses ordinary NPC
Shoot6660 (same SCHOOL_DAMAGE/base23/enemy target6 as88445's valid damage effect)
without the absent force-cast companion88444; scorpid3125 speaks its Envenom
warning to its victim rather than making the victim speak it. Exact-field,
all-other-row preservation and repeat fixtures passed. Private full-table backup:
out/combat-actions-before-20261105-072043.sql (5950617 bytes).

## Navigation availability and extraction compatibility

The transport-coordinate changes are committed042b847a. The next startup no
longer requested the Darkmoon/carrier central grids. Remaining traces came from
player visibility, loading saved positions and flight movement over grids with
no ADT/collision data or only default-height dark ocean. The generator cannot
produce useful ground navigation there. No fake or synthetic tile was installed.

NavigationGridInput checks actual inputs before an eager grid navigation load.
It only recognizes empty space when a valid tiled collision root exists, both
the collision and navigation tiles are absent, and terrain is absent or contains
only default -500 height and no navigable liquid. Existing files, corrupt or
unknown headers, global collision models, ordinary water, real terrain and
uncertain per-cell liquids still reach the ordinary loader and diagnostics.
This matches existing terrain/collision empty-grid semantics; it does not
validate an entire extraction against the client's WDT coverage. Asset coverage
remains a release check. Diagnostic levels and real file validation are unchanged.
The temporary Windows stack logging was removed after identifying callers.

A separate generator defect used an incompatible liquid-header layout: format10
stores uint16 flags followed by uint16 uniform liquid flags, not two byte fields.
The generator now agrees with the extractor/server. A standalone ordinary-water
fixture produced a100732-byte tile and passed the real MMap load/duplicate/
unload/reload, missing-map rejection and parent-origin probe. This fixture tile
stays under out/navigation-liquid-fixture and was NOT installed in exports.
Existing navigation assets and Detour are unchanged in this pass.

## Saved-state repair and remaining content work

With worldserver stopped, a real-schema fixture verified the exact invalid
RNDBOT taxi-route resets. Six obsolete saved routes were cleared, preserving25
valid routes and all positions/progress. A private complete-row backup is
out/invalid-bot-taxi-before-20261105-072352.sql. A subsequent DBC route audit found
zero unusable saved routes. The existing native invalid-resume recovery and
unrelated-teleport cleanup remain enabled; invalid future routes still report.
No player/account/password/wallet/catalog rows were changed by this repair.

The all-category logs still contain969 missing native script names and other
real content defects. A read-only source audit found only38 of those names even
as source tokens; several are commented out or belong to unregistered script
packs. Do not activate incomplete packs or delete their bindings just to make
the count zero. Remaining work includes actual missing dialogue/wrong GUID
references, unsupported SmartAI actions/source types, quest objectives, loot,
conditions, spellclick data and starting-zone acceptance. Jaomin/Worgen and the
other gameplay checklist items remain open. This is a private incremental build,
not the sanitized public release.

## Native checks and deployment

Visual Studio MCP built worldserver and the corrected generator. All15 native
CTest cases passed, including the new navigation-input fixture. The diagnostics
Python test and SQL08/09/10 preservation/idempotence fixtures passed. Earlier
EXE/PDB pairs and logs are archived under ignored out/runtime-before-final-nov5-*.
Authserver remained running with its concise existing loading/ready profile.
Final runtime hashes, fresh-log counts and mirror status are appended below.

The subsequent vehicle repair handles template NPC accessories separately when
all player click actions are non-boarding spells. A Frigid Proto-Drake's54513
legitimately summons another mount, so it must not be used to seat its29458 NPC
rider. The accessory uses native46598 boarding in that specific class of case;
configured control-vehicle click spells retain their existing conditions, and
player click behavior is unchanged. Empty or invalid click data do not gain a
fallback. Successful seat occupancy is checked immediately; failed passengers
are still removed. Final live verification is recorded below.

## Final deployed observation —07:33:39 local

Source through ebe4bd54, built through Visual Studio MCP. The banner may retain
its dirty suffix because documentation/generator work was present during build;
the EXE/PDB hashes below identify the exact pair. Stage backup:
out/runtime-before-final-nov5-20261105-073101. Both deployed files match native
build output byte-for-byte:

- worldserver.exe: 4ed4aed2cf43bb609a2c1905d7afd53e03b8c059aeff4d8ba2792c337b8cbc40
- worldserver.pdb: ce8b4d2dd8583df4a818828befe6bc263127a6a28457a38b22e9f982a0e58150
- mmaps_generator.exe: 9043e717f2215dc25e6957d7dbafe57e69b5fff9b310d8e1492943a4219d3419
- mmaps_generator.pdb: edbad70855b588b7d75c377a97a56d3b86ee991cca13e02f902c993c41f00ca6

Worldserver PID41616 was launched through the visible export launcher07:31:11.
It initialized in18 seconds;198/198 bots finished login07:31:53. Authserver
PID61228 remains listening on3724; world listens on8085. Two Frigid Proto-Drake
accessories were confirmed seated in their actual seat0, with zero54513 errors.
No new crash file appeared. All15 CTests passed against the final native pair;
the one Python diagnostics test also passed. This is a short startup observation,
not a multi-hour soak or a complete quest/vehicle/instance acceptance test.

Observed Server.log warning/error count:0. This includes zero current spell
validation, LFG, navigation, taxi-resume, spline/speed and engagement diagnostics.
All-category file logs still contain1594 sql.sql errors (same messages duplicated
across Errors.txt/DBErrors.log; do not add the counts). The grouped report is
exports/logs/diagnostics/error-summary-all-20261105-073344.txt. Most are startup
content diagnostics, including969 missing native names,51 actual-speaker text
references, unsupported SmartAI source/action types, quests, conditions and loot.
The filtered-event outdoor SmartAI diagnostics also remain. Neither diagnostic
levels nor these genuine errors were disabled. Zero across all logs has NOT been
achieved, and the package is not ready for public release.

Repository synchronization: d3f691db was pushed to authoritative Forgejo origin;
the read-only checker confirmed both repositories private and all9 branch/tag
refs identical on GitHub. The scoped credential cannot inspect mirror settings,
so settings verification remains unavailable. The default main stays at the
previous validated milestone1a7ce568; this tested development batch is on
codex/city-companions. No history rewrite or independent GitHub development.
