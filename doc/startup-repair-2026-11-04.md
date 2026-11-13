# Startup repair pass — 2026-11-04

The objective remains a genuinely clean startup. Do not turn off validation or
mark this complete merely because the console is quieter.

## Applied and investigated

The current database had never received the existing four per-map pool migrations
`2026_10_15_40` through `_43`. In temporary copies of the real pool tables, they
reduced 2,334 mixed-map pool hierarchies to zero, with no cycles and all 59,896
creature/gameobject memberships retained. Parent relationships, quest pools and
event associations were checked. No new pieces in this database needed an
additional parent/event relationship. The test also exposed one unused generated
template (1001681), because guid 53202 is absent here; the guarded November 4
companion migration removes that template only if it has no members or links.

All four affected tables were dumped before application (5,265,246 bytes, ignored
`out/startup-pools-before-20261104-165103.sql`). A fresh boot at 16:51 completed
initialization in 20 seconds and reported zero cross-map creature, gameobject or
child-pool errors. The old boot had 3,857 gameobject, 1,021 child-pool and three
creature map errors. Individual per-map population caps are retained: independent
maps no longer lose their gathering-node candidates during validation.

Do **not** blindly apply the neighboring old cleanup migrations: `_44` calls pool
9868 empty, but it now contains six real creatures; `_46` assumes two event spawns
are missing, but both now exist. These assumptions were tested and those files
were deliberately left unapplied. No spawn records were removed.

The existing sourced SmartAI parameter fixes (`2026_10_15_48`) and obsolete
character-creation skill cleanup (`2026_10_15_51`) were tested on temporary copies,
including a repeat run with identical resulting rows, and applied after a full
backup of the three affected tables. The new condition migration clears only
arguments that this core does not read. It preserves condition rows, effective
arguments, targets, negation and script binding. Its real-schema temporary test
also checked that normalization did not collide with the composite primary key.

Two condition diagnostics were themselves wrong: NearCreature.value3 selects dead
versus alive, and QuestRewarded.value3 bypasses the race filter. Their validators
now respect those supported arguments instead of calling them useless. The
NearCreature boolean still rejects values outside 0/1.

Repeated runtime corpse INSERT duplicate-key failures have a separate code cause.
New corpses used per-map counters, while corpse.corpseGuid is a realm-wide primary
key. The repair seeds a shared atomic generator above the persisted maximum,
allocates new corpses from it, and retains saved IDs when loading corpses. It
does not delete corpses or replace failed INSERTs with data-overwriting REPLACE.
Pool template lookup was also widened to uint32 so the existing million-range
per-map IDs are not truncated when used by events.

## Remaining content work

The post-pool boot still contains real errors: approximately 994 missing script
names, 789 empty pools, 326 zero-chance ungrouped reference loot rows, 314 unknown
criteria-data rows, 133 missing loot references, 122 SmartAI missing talk groups,
92 invalid quest area-trigger objectives, and additional content mismatches.
Counts include runtime activity after startup and must be regenerated per boot.

The missing script names include genuinely absent implementations and legacy
bindings, not a disabled scripts build. For example Herod has an old binding;
the Protectors encounter source registers its three individual bosses, whereas
an unrelated template carries the obsolete umbrella name. Removing all bindings
would conceal unfinished behavior. Likewise, changing every zero-chance loot row
to guaranteed loot would change rewards. Restore sourced definitions or document
incompatible legacy content rather than manufacture spells, dialogue or drop rates.

The diagnostic report now groups varying missing script names and creature names
under their common error class, retaining a concrete example. This changes only
the report; every raw diagnostic remains available.

### Additional validated data cleanup

The guarded November 4 unused-pool migration removed 372 templates and 205 links
only in components containing no members, quest associations or events. A temporary
fixture confirmed all 59,896 memberships unchanged, stable results on a second run,
and refusal of the entire cleanup after inserting a member into one candidate.
Populated pool 9868 is preserved. The remaining empty pools connect to live
content and are left for a content/odds review. The full template/link backup is
`out/unused-pools-before-20261104-165827.sql`.

Fourteen access-requirement rows carried absent item keys that the core already
cleared in memory. The new retired-key migration persists that effective state.
Its temporary fixture verified identical repeat results, unchanged row count,
and preservation of all other gate fields. The backup is
`out/access-keys-before-20261104-170307.sql`.

The complete report combines Errors.txt with WARN-only records from Server.log;
SQL does not necessarily reach Server.log. Its regression fixture checks missing
script grouping, inclusion of SQL and warnings, and no duplicated core errors.
This broader audit also exposed missing MMap tiles, spell-hook/DBC mismatches,
and shutdown-only GameObject owner/map-grid cleanup errors. These remain open,
not hidden. The old boot-count examples above precede these later repairs.

### Script bindings and optional hooks

The sourced `2026_10_04_70` migration restores renamed city-guard bindings to
`npc_guard_generic` and `npc_guard_shattrath_faction`. It was tested twice on a
temporary copy of affected templates before applying with a scoped dump. The
`generic_creature` binding is deliberately not registered by the source loader;
the new migration clears only that retired name, preserving the AIName/NPC/permit
selection that CreatureAISelector already reached through its null-script fallback.
It does not enable that obsolete AI over thousands of SmartAI creatures.

Shared smart-heal scripts bind alternative area-target kinds across different
spells. Deterrence similarly binds two alternative aura kinds. Registration now
checks the bound spell's actual effects and binds only applicable alternatives.
Validate still fails if neither supported alternative exists. The same applicable
handlers run; no global hook validation or real failure reporting is disabled.

The gameobject lock diagnostic previously printed the door union member even for
other object types. It now reports the actual tested lock ID; a printed zero was
not proof that the lock check itself was false. Two debug-waypoint spawn lambdas
also now return an empty GUID on every failure path, removing C4715 compiler
warnings without changing Detour pathfinding or any MMap files.

### Repeated navigation tile requests

`MMapManager::loadMap` returned false when its requested tile was already loaded,
although the map was ready to use and no I/O failed. `Map::LoadMMap` interpreted
that as failure and printed a warning, including for an existing local tile
0000_32_31.mmtile. The loader now returns success for that idempotent case; it
still returns false for missing/invalid data. Callers were reviewed: Map consumes
the success result and Transport ignores it. No caller depends on false meaning
"already loaded." Tile allocation and loaded counters are unchanged.

`mmap_load_probe` links the actual common library and reads one operator-supplied
tile. It verifies load, repeated load with unchanged mesh/count, unload/reload,
missing-map failure and final unload. It is an integration probe rather than a
CTest dependency on untracked game assets. It does not modify those assets or
connect to a database. Separate genuinely absent tiles remain a coverage issue.

### Generic quest exploration/event objectives

All 92 area-trigger objective warnings in the current database have ObjectID=0,
Amount=0, representing legacy generic exploration/event objectives. This is not
a physical trigger numbered zero. Accept the legacy zero sentinel alongside the
already-supported -1 sentinel, while continuing to validate positive IDs against
the client store and reject other negative IDs.

The runtime had related defects: LoadQuestAreaTriggers indexed specific objectives
by objective ID instead of trigger ObjectID, the event path set Explored without
updating generic objective counters, and the physical trigger handler credited
the first area objective without checking its target. The repair shares a small
policy for sentinel and target matching, indexes specific triggers correctly,
and credits matching objectives once. Explicit scripted events still select the
quest, and every other completion requirement remains checked. No quest records
or objective requirements are deleted. Isolated policy tests cover zero/-1,
specific and unrelated IDs, and invalid negative IDs. Live quest playthroughs
remain a separate acceptance check.

Yellow “Created.” plus sound while walking in Dalaran, and sound alone on `.tel`,
are still unresolved. Neither bot spawning nor a specific packet is established
as the cause. Do not suppress legitimate effects on that suspicion.

## Second content batch and fresh boot

The remaining 417 empty pools were zero-weight/equal-chance candidates under live
parents. They contained no direct or descendant creature/object/quest/event. A
parent could select one and consume a population slot without spawning anything.
The guarded cleanup removes those choices, keeps all 59,896 memberships and live
parent limits, and refuses the entire operation if a candidate acquires content
or an explicit probability. Temporary tests verified both refusal cases and
repeat stability. The next boot reported zero empty-pool errors.

All 326 malformed equal-chance reference-loot rows belonged to complete templates
with chance=0 and group=0. Declaring group 1 restores one uniform choice rather
than converting every row to a guaranteed item. A real-schema fixture verified
all item/chance/count/mode fields and row count unchanged. Missing reference
messages fell from 133 to 11; those 11 genuinely missing definitions remain open.

The two-billion-range missing broadcast IDs were existing authored dialogue in
db_script_string. The current TALK dispatcher expects broadcast_text. An
insert-only compatibility migration restores 219 default texts and 186 locales
from that existing table, preserving existing overrides and leaving the legacy
table intact. Temporary tests checked repeat stability and override preservation.
The fresh boot/runtime interval had zero missing legacy broadcast lookup errors.

The backup for those three migrations is the ignored 86,727,148-byte
out/startup-batch2-before-20261104-210338.sql. Four additional constructor-name
aliases were matched directly to registered classes: the two Greenstone NPCs,
Polyformic Acid Potion, and the duplicate Unseen Strike aura alias. Exact affected
records were backed up; temporary repeat and binding-count tests passed.

The 52 remaining QuestState value2 warnings are another validator bug: masks 2
and 64 are actively used by Meets(), not unused arguments. The common quest check
now excludes QuestState from that warning while retaining its mask validator.

## Creator and transport lifetime repair candidate

EffectTransmitted sets creator GUIDs on timed objects without binding their
lifetimes to the caster. Unit cleanup only knew about lifetime-bound objects,
so the timed objects retained dead creator references and later produced fatal
ownership diagnostics. A separate reference registry now clears those GUIDs when
the creator leaves while preserving object lifetimes/timers. Clearing an object's
creator also removes its registry entry, preventing stale pointers. A shared
registry test covers deduplication, object-first expiry, creator-first detach and
callbacks that mutate tracking. The fatal ownership diagnostic remains enabled.

Transport deletion now performs passenger cleanup while the map is still valid.
Map shutdown removes transports and drains passenger removals before its final
grid sweep. This addresses the observed ordering that could leave a grid after
unload. Live shutdown validation of these source repairs is still pending.

## Missing tile generation and corrected validation

Five physically missing tiles were generated from existing terrain/vmaps. The
initial single-tile load test was insufficient: the generator recomputed the
Eastern Kingdoms origin from a larger input terrain set. Three new tiles loaded
alone but used shifted tile indexes and collided with existing data at runtime.
Those three newly created files were moved to ignored quarantine and worldserver
was restarted to clear them from memory. Original tiles and parent files were
preserved. This mistake is not evidence that the original MMaps are corrupt.

Single-tile generation now reuses existing parent mesh parameters. The probe now
checks tile bounds against that origin and loads a 3x3 neighborhood together.
It rejects the quarantined mismatch with exit 11; an original tile and all eight
neighbors pass. Regenerated replacements must pass that stronger check before
installation. The Kalimdor/Pandaria parent origins match, but those new tiles also
need the stronger neighborhood check. Some other requests have no input terrain
or produce no geometry; keep those cases separate from malformed navigation.

## Criteria compatibility investigation

The supplied client has both Achievement_Criteria.dbc and MoP Criteria.dbc.
Some missing auxiliary criteria IDs were renumbered: for example old 100 maps to
17411 within the same achievement and metric. An audit found 114 candidate rows,
but multiple old achievement criteria can collapse onto one shared MoP criterion.
Do not apply a blind numeric remap: it could attach one achievement's conditions
to another. No criteria data or saved player/account progress was deleted or
remapped in this pass. The ignored audit records unmatched/ambiguous mappings.

## Character creation, Deathknell and fresh logs (21:41 local)

The PvP character-creation check ignored CONFIG_ALLOW_TWO_SIDE_ACCOUNTS even
though World loaded it. Both the initial and asynchronous paths now honor that
configuration, while retaining the existing non-PvP and RBAC bypass rules.
Eight configuration/realm/permission combinations pass the policy test.

ExampleCharacter (guid 308521, account 522) had already rewarded Fresh out of the Grave
(24959), but retained permanent Rigor Mortis 73523. After an offline identity and
quest check, her aura/effect rows were backed up under ignored
out/ExampleCharacter-intro-before-20261104-213555 and only that stale aura was cleared.
Spell 11516 was not involved. Starting spell definitions remain intact. A
Deathknell PlayerScript removes 73523 after accepting/completing/rewarding the
intro quest and recovers stale post-quest auras at login. The initial pre-quest
state remains intact. Intro policy cases pass; live quest validation is pending.

The active configuration used BattleGround instead of the case-sensitive
Battleground prefix for PrematureFinishTimer and PremadeGroupWaitForMatch.
The two keys were corrected without changing their values. Both servers were
stopped, and all exports/logs and exports/Crashes contents were moved to ignored
out/runtime-diagnostics-archive-20261104-214116. Empty live folders are ready for
a fresh capture. The normal auth console close helper failed with an invalid
window handle, so its process was stopped before archiving.

## Classic loot references and spell/history follow-up (21:49 local)

Restored ten absent classic random-item reference groups (650 choices) from
pinned AzerothCore source b704af465112832706b2c389b5cc9b9f5aad61cb, preserving
parent drop chances and existing complete-group overrides. All item IDs exist,
are below the level-100 quarantine and lack broken-item name markers. Temporary
real-schema fixtures passed repeat and custom-override preservation checks.
Backup: ignored out/classic-reference-before-20261104-214345.sql. Ordos 72057
remains unresolved; do not substitute unrelated Celestial drops.

A scoped subset of the old September binding migration removes six verified
obsolete bindings, retaining MoP Incanter's Ward, Ice Barrier, Marked for Death
137619, and Fluidity. Missing 44394/44413/108507 spells and actual effect types
were checked against the installed DBC. Cauterize 147997 already has native
25%-heal/1%-periodic-damage effects; its old unexecuted handler incorrectly
converted a health percentage to absolute maximum HP. No spell effects were
removed. Fixture tests require exactly those six deletions, preserve all other
bindings and pass repeat application.

Fluidity keeps its existing damage-bonus aura hook, which maintains Conductive
Waters; the nonexistent periodic-damage duplicate hook was removed. Focused
Lightning trash now filters the actual enemy-area target instead of an absent
entry-area selector. Live encounter validation remains pending.

Normal empty completion histories and optional visibility overrides are INFO,
and successful challenge reward/history load counts are INFO. Actual missing
challenge rewards remain ERROR. The history loader also proceeds to guild
records when group history is empty, rather than returning prematurely. This
changes incorrect classification and loading behavior; other diagnostics stay
active.

## Difficulty-aware spell hook validation

The startup validator only checked the regular-difficulty SpellInfo, although
SpellMgr loads distinct heroic effect sets and runtime dispatch uses the actual
spell's effects. For example, Malkorok 142913 effect 1 exists in difficulties 5/6,
and Shared Torment 145655 is an area aura only in those heroic variants.
Validation now requires an exact hook match in at least one loaded difficulty.
Runtime effect masks/dispatch are unchanged. A hook absent from every variant
still reports the original error, and script Validate failures still reject the
binding. This is a validation compatibility repair, not a raid mechanic rewrite.
Fresh native startup comparison remains pending.

## Sourced gameobject locks and shutdown follow-up (21:57 local)

Verified the downloaded ProjectSkyfire sf_db_26 world reference asset against its
published SHA256 d12ba9ecad381f3b5460612af2a172cd373afdd796c289e2ccc11315db21dd5f.
Only 23 invalid lock fields with the same entry/type/display/name were restored
from that source; all are explicitly no-lock definitions. Other objects with
different identities/types or still-invalid source values were rejected. SQL13
also guards the old value and full identity. Real-schema fixtures passed exact
23-cell-only changes, unchanged 34,365 rows, idempotence, and preservation of a
renamed custom replacement. Full private backup: out/gameobject-locks-before-
20261104-215602.sql. The full external dump was never imported.

The first e24aa1d0 startup completed in 22 seconds. No creator-owner fatals were
seen on shutdown, but maps 1 and 648 still reported one residual grid each.
The initial progress update missed the differently worded destructor diagnostic;
it was corrected immediately. The remaining forced-unload loop retained a next
iterator across callbacks that can link grids before that iterator or invalidate
it. It now drains the live head until empty. Force-unload always unlinks/deletes
the selected grid. Repeated native startup/shutdown validation is pending.

An extra auth launch collided with the already-running local listener. This was
a duplicate launch, not a database or authentication defect; one running auth
process owns 3724 and the world process owns 8085. Preserve the failed-launch
log as diagnostic evidence; avoid starting a second listener.

## Final native comparison and sourced dialogue

Native bac2ff22 (including all preceding source repairs) built successfully in
Visual Studio; all 13 CTests passed. Export executable and PDB hashes match the
build outputs. The executable SHA256 is
b06723dd27a4e95726dbd3a8bfc72b1139699e5e10117a59f428451b51a5a961;
PDB SHA256 is
54d5c32b200fa635fe237d070b0b3613ebd223d0ef12b303a01d9c126465ed1e.

The fresh 21:59 boot completed in 20 seconds. The 52 false QuestState warnings,
old pool-map/empty-choice errors, six obsolete binding errors and ten absent
classic references are gone. Heroic-only Malkorok/Shared Torment hooks pass;
genuinely unmatched hooks (e.g. Water Strider area-aura check and Explosive Ring
unknown destination selector) still report errors. Lock errors decreased by 23.
Four missing quest talk groups (3584/0,3584/1,3692/1,7804/0) were restored from
verified SkyFire data with matching creature identity and unchanged chat enums.
Real-schema fixtures preserved every original text row, whole-group overrides
and repeat stability. Backup: out/quest-talks-before-20261104-215856.sql.
SmartAI missing positive talk groups decreased from 122 to 118.

The bac2ff22 shutdown at 22:00 completed with zero lost-owner fatals and zero
residual-grid destructor diagnostics. This is one observed cycle, not a long
stability soak. A second cycle and final visible restart are pending.

Overall clean startup is NOT achieved. Missing script implementations (986
names), criteria compatibility (314 rows), other SmartAI/data faults, navigation
requests without available geometry and remaining spell hooks are still open.
The external SkyFire reference contains Ordos item IDs but independent 0.2%
creature drops, not this core's guaranteed personal-reference semantics; do not
substitute them blindly. Source mismatches were rejected, not imported wholesale.

The second bac2ff22 shutdown (22:03) also had zero owner fatals and residual-grid
diagnostics. Keep broader summon/combat soak open despite those two clean cycles.

The original LoA v1.0.4 database asset was downloaded only as a read-only source,
verified against release SHA256 db8dd845e3a5da4bc3a6f28afa68ed839aceb222101db949ae74840a5a2365a0.
It contained 32 authored lines in 17 currently absent SmartAI talk groups.
Their NPC identities, emotes, language/chat enums and referenced broadcasts were
verified. SQL15 maps the release CreatureID/GroupID/ID key to the live legacy key
and preserves complete existing groups. It rejects groups missing referenced
broadcasts. Temporary real-schema fixtures verify all 32 additions, unchanged
original rows, idempotence and an existing custom-group override. No complete
world dump was imported and no dialogue was invented.

## Rigor Mortis recurrence: root area rule repaired (22:10)

The user reported that ExampleCharacter became immobilized again several minutes after
login. The prior saved-aura/login cleanup was therefore incomplete, and the
in-game acceptance item remains open. Investigation found a concrete recurring
trigger: spell_area automatically cast 73523 in area 5692 with quest_end_status
66, which includes COMPLETE and REWARDED for quest 24959. Entering/updating that
area could reapply the stun to a character who had finished the intro.

The original LoA v1.0.4 database has no spell_area row for 73523. SQL16 removes
only that exact erroneous autocast rule, retaining the starting cast spell and
Agatha's resurrection/aura-removal actions. The same source gives 49416 in zone
6454 quest_end_status 11 (NONE/COMPLETE/INCOMPLETE); the live value 116 wrongly
included REWARDED. That one exact mask was restored, preserving other zones'
uses of generic invisibility detection.

Real-schema fixtures verified exactly one deleted row and one changed mask,
all other fields/rows retained, and stable repeat application. Full spell_area
backup: out/deathknell-area-before-20261104-221024.sql. After graceful shutdown,
ExampleCharacter's aura/effect records were backed up again and only saved 73523 cleared.
Starting row (raceMask16,classMask925,spell73523) remains present. No spell11516
or globally learned/starting spells were deleted. Restart and sustained
Deathknell movement validation follow; no new executable is needed for SQL16.

## Wider quest-area state compatibility

A read-only full comparison found 328 additional end-quest masks with undefined
status bits. Every one is the exact seven-bit complement of the matching LoA
v1.0.4 allowed-state mask (273:116->11; 39:118->9; 15:126->1; 1:63->64).
The native evaluator checks allowed states. SQL17 restores only those source-
verified records, guarding all eight identity fields and the old mask. It does
not invert all rows or alter 130 other differing rules / 180 unmatched ones.

Real-schema fixtures verified exactly 328 mask-cell changes, no other fields or
row counts changed, stable repeat application, and preservation of a modified
custom mask. A new full spell_area backup precedes application. Runtime reload
uses the existing supported reload spell_area console command; no player quest
completion, inventory, account or character progress is modified. Wider quest
playthrough acceptance remains pending. This repairs compatibility semantics,
not a proven explanation for every earlier missing-mob observation.
