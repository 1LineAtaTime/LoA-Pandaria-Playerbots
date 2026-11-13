# Startup/runtime repair continuation — 2026-11-05

Latest continuation and deployed hashes: [November5 continuation](startup-repair-continuation-2026-11-05.md).

The requested acceptance target remains zero genuine startup/runtime errors and
warnings, with validation enabled. It is not achieved yet. See the November 4
repair report for applied backups/migrations and the exact live build hashes.

The user confirmed that Rigor Mortis no longer returns after several minutes of
Deathknell movement following SQL16. The earlier login-only cleanup had been
insufficient. Mixed-faction character creation still awaits a client result.

The current bac2ff22 worldserver has stayed up for several hours without a new
crash file, but repeating spline/taxi/creature-text faults remain. This does not
prove general combat/instance/companion stability.

## Movement work in progress

MotionMaster::MoveFall used an absolute height difference, accepting a floor
above the unit as a downward fall. It now requires a finite downward drop of at
least 0.1 yard. Shared input policy cases cover downward, uphill, grounded,
near-ground, NaN/infinite positions and valid/invalid speeds.

MoveSplineInitArgs::Validate omitted the logger-category argument, so the
console showed only the failed expression, hiding the unit identity. Its ERROR
diagnostic now includes the category, GUID and creature entry as intended.
Zero-duration spline errors retain their severity and now include falling/cyclic
flags, speed, point count and endpoints. Remaining failures must be traced from
those records; this instrumentation is not a claim they are fixed.

Advancing PlayerTaxi to another leg now resets the old leg's saved node index.
This prevents a later short leg inheriting an out-of-range index. Recovery of
already-persisted invalid resume state and full cross-map/landing verification
remain open. No saved taxi records were bulk-deleted.

## Deferred gameplay and distribution

The early Pandaren starter duel/challenge quest is recorded in the checklist;
identify the exact quest/NPC when investigating after the diagnostic cleanup.
A shareable release must initialize clean databases containing required static
data but no personal accounts, characters, inventory, mail or wallet balances.
Active local configs/logs/dumps remain private. The current exports runtime is
not yet the sanitized distribution. Windows/Linux clean-install validation and
reproducible packaging remain acceptance requirements.

## Verified spell and content batch prepared

Soul Reaper's player DK variants 114866/130735/130736 use PERIODIC_DUMMY at
effect1, not PERIODIC_DAMAGE. The removal hook is corrected rather than deleting
these genuine class mechanics. The same class script was incorrectly attached
to the Lich King's 69409; the boss-specific script is preserved. Four exact
obsolete/wrong-class bindings are removed by November5 SQL01: old Last Stand,
old Divine Storm heal, Vanish aura script on its trigger, and player DK execute
on the boss. Vanish trigger/real stealth auras, native health effects, Divine
Storm glyph and player DK execute bindings remain. Real-schema fixtures require
exactly those deletions and preserve every other binding; repeat application
passes.

Havoc's shared target selector now registers only the existing effect0/effect1
enemy targets for each supported spell; it retains charge consumption and chain
target behavior. Validation still rejects bindings with neither matching effect.
This avoids registering an inert optional effect1 on spells that only have
one effect; live combat validation remains pending.

Prepared November4 SQL18 restores 54 exact source-verified cells on 41 objects,
including invalid locks/foci/traps and three incorrect object types. The source
and current identity agree, with 12 reviewed descriptive Bonfire/Brazier names
mapped to their identical-entry/type/display doodad names (local names retained).
Type repairs require all 32 raw fields and script/AI identity to match the
source. Fixtures preserve all other fields, total rows and a renamed custom
object. SQL19 restores eight authored text/voice lines for entries19477/34913;
all broadcasts/emotes/sounds are present and whole-group custom overrides win.
Both migrations are tested but not yet applied to the live database. No invented
missing text groups or unrelated templates are inserted.

Further verified spell-role corrections: Onyxia's four quadrant spells now bind
their real source-area-enemy selector; the unrelated 77827 cone is detached from
the quadrant-only script while 77836 retains its native quadrant trigger.
Omnotron's achievement check now counts the actual effect0 cone targets already
selected by this core's spell correction. Death's Advance matches its two aura
types across either slot, preserving both valid spell variants.

Apparitions 111698 is a periodic producer for 112060, not the area absorb itself.
The wrong combined binding on 111698 is removed. A core correction that erased
112060's required area target was removed; its existing filter limits the shield
to the three intended creature entries, and its absorb amount handler remains.
Tranquility's smart-heal filter moves from producer113277 to heal113278, as
explicitly named in the source. Incanter's Ward damage modifier116267 retains
native behavior without an obsolete absorb script.

An expanded temporary-table fixture passed exact changes, idempotence and
preservation of native producer, shield, glyph, Vanish and boss bindings. These
prepared changes still await the final native build and live deployment. The
full header rebuild is underway; a final incremental build is required because
additional source corrections were reviewed while it compiled.

Fall eligibility now uses the same hover-adjusted origin as spline launch, and
failed launches undo a newly added player fall flag without installing a bogus
controlled movement generator. Existing fall state is preserved on failure.

The saved-route audit found 19 already-invalid resume indexes among 84 saved
routes; several routes belonged to a different map than the saved character
position. TeleportTo only cleared taxi state when IsInFlight was true. Between
legs a route can remain pending after that flag clears, so an unrelated teleport
carried stale route data to another location. Teleport cleanup now also considers
pending routes, while preserving TELE_TO_NOT_LEAVE_TAXI transitions. It expires
a flight generator only when appropriate and preserves unrelated movement.
Eight explicit keep/flying/pending combinations cover this policy. Invalid
path IDs and undersized paths are rejected before indexing DBC storage and
unusable taxi state is cleared. Remaining invalid node-index diagnostics retain
ERROR severity and now identify GUID/map/route/position. Already-invalid node
indexes are not silently removed or described as resolved.

A full original-source alias audit found only one strict creature alias among
986 missing native script names: Training Bag -> generic training dummy. It was
NOT applied: the generic dummy suppresses lethal damage and could alter quest
credit. These missing implementations cannot be made correct by generic fallback
aliases or by clearing every ScriptName. The Pandaren quest remains separately
tracked and unverified.

The movement-clock fallback previously treated delta0 as an error. Before the
first sync response no client clock exists (including server-controlled bots),
and a measured zero offset is also valid. AdjustClientMovementTime now uses the
sample queue to distinguish initialization from a valid clock; pre-sync movement
uses the existing server-time fallback normally. True out-of-range translated
times still emit WARN and use the fallback. No clock arithmetic or anticheat
threshold was relaxed.

CombatManager::UpdateOwnerCombatState previously returned success even if
SetInCombatState rejected entering combat for a dead/evading unit. Callers then
sent an AI enter callback without an actual combat transition. It now returns
success only if the actual flag matches the requested state; exit notifications
still work after a true exit. CreatureAI also rejects an enter callback without
its combat flag. A genuine duplicate engagement still emits ERROR with entry,
GUID and map. Native compilation and live combat/evade observation are pending.

## Authored AI and template validation

SQL03 restores the original LoA v1.0.4 empty C++ ScriptName for 19 identical
entry/name templates with explicit SmartAI and existing source-type0 events.
CreatureAISelector already falls back to that explicit AI when the stale native
name cannot resolve. The guard requires the old name, SmartAI, creature name and
authored event presence. It does not remove the other missing implementations or
claim every SmartAI event is functional. SQL04 removes only unsupported bit23
from 13 reviewed custom templates, retaining CIVILIAN; ObjectMgr already masks
this same bit at runtime. Full-row temporary fixtures passed exactly 19 name
cells / 13 flag cells, unchanged other data, custom AI preservation and repeat
application.

CreatureTextMgr's missing-entry diagnostic now includes the requested group,
GUID and map. The old message omitted the requested group, preventing reliable
restoration of the actual missing line. Severity and dispatch remain unchanged.

The quest-marker audit found 31 absent point blobs. The original LoA source
also lacks points for most matching metadata; the few available point groups
have different metadata. No guessed coordinates or incomplete blob substitutes
were installed. These remain genuine content defects, separate from the
Pandaren duel quest recorded for later investigation.

The final test pass includes 14/14 native CTests and the Python diagnostics
unit test. Movement policy tests cover finite/downward fall input, hover origin
and all eight keep-taxi / flying / pending-route combinations. Native tests do
not replace live flight, combat, teleport and companion acceptance.

## Local deployment and remaining diagnostics

Six migrations (Nov4_18/_19, Nov5_01–_04) were applied with worldserver stopped.
The complete pre-change four-table backup is private:
out/nov5-content-before-20261105-022345.sql (24,328,417 bytes). Reapplication
left the full exported state of all four tables identical. No auth/character
rows, account passwords or store balances/catalogs were changed.

The b29b5bd4 native build was staged with matching symbols and launched visibly
at 02:24:14. Its prior EXE/PDB and world logs are preserved in
out/runtime-before-b29b5bd4-20261105-022405. Authserver remained running; its
locked Auth.log/AuthErrors.txt stayed in exports/logs when the other files
moved. The world Server.log/Errors.txt start at the new boot.

All 191 configured bots logged in by 02:25:04. Through 02:30:25 this run had
zero zero-duration-spline, invalid-velocity, duplicate-engagement or movement-
clock warnings. Existing invalid saved taxi node indexes are still reported,
not silently cleared. The short observation does not establish soak stability.
Missing native script-name diagnostics fell from 986 to 969; names shared by
multiple templates explain why 19 template edits remove 17 distinct names.

Follow-up source changes preserve packet direction: the legacy MSG movement
names are incoming IDs and are no longer registered as outgoing IDs, which
collided with SMSG_SPELL_GO, SMSG_PET_SPELLS_MESSAGE and SMSG_SPELL_EXECUTE_LOG.
Actual movement broadcasts use SMSG_PLAYER_MOVE. Client registrations and server
spell/pet handlers remain intact. Three verified aura hooks now bind effect1:
heroic Sonic Ring's dummy aura (effect0 is direct damage), Pheromones of Zeal
(effect0 is its native periodic producer), and Incubate (effect0 is open-lock).
The Incubate removal callback also uses AuraEffectRemoveFn.

The missing runtime dialogue diagnostic now identifies the originating SmartAI
entry/source/event/target type. This preserves ERROR severity and the action's
existing timer behavior; it replaces the otherwise context-free missing-text
message for that action. It does not ignore missing authored dialogue.

The remaining 969 unknown names include only 34 exact source-string matches;
many of those implementations are commented out or their loaders are omitted.
Unreviewed quest/raid scripts were not blindly activated to satisfy a log count.
The two Prince Sarsarun entries point to DBC map734 Ahn'Qiraj Terrace, without
an authored entrance. No made-up entrance coordinates were supplied. These and
unsupported criteria, SmartAI data, quest markers, loot references and missing
navigation coverage remain open.

## Repeating dialogue root cause and retired criteria

The new SmartAI context proved the Dalaran spam was not missing vendor lines.
Fifteen customer-greeting OOC-LOS events accepted NPC invokers. TALK target7
then selected that creature as the speaker, requesting the shopkeeper's group
from vendors/critters such as Horace Hunderland and Stormwind Rat. SQL06 sets
only those exact events' existing playerOnly parameter. Their own authored
greetings, player/bot-player recipients, cooldowns and target selection remain.
Whole-table fixtures verified 15 cells only, stable repeats and protection of
a custom target. No unrelated conversations were changed.

SQL05 removes 314 extra-data rows whose 311 unique IDs are absent from installed
Criteria.dbc. GetAchievementCriteria directly looks up that DBC. All 1490 rows
with present IDs remain byte-for-byte intact, and a fixture with modified data
was protected by exact full-row guards. This does not delete player/account
achievement progress. Both migrations were applied while stopped with backup
out/nov5-cleanup-before-20261105-023546.sql (5,986,612 bytes).

The 02:36:26 restart reports zero missing-criteria diagnostics. Through 02:39:04
no Dalaran greeting requests, spline/speed/clock/engagement warnings or opcode
collisions recurred. The remaining runtime dialogue fault was taskmaster17058
aggro event2 addressing attacker19477: the four required aggro lines already
exist on the taskmaster, but target7 without useTalkTarget makes the attacker
speak. SQL07 sets the recipient flag on that one fully guarded event; no new
lines are invented. Its full-table fixture changed exactly one cell, preserved
custom group edits and was repeat-stable. Application/live validation pending.

Invalid saved taxi resume indexes now use the same CleanupAfterTaxiFlight
recovery as unusable path IDs, after retaining the detailed ERROR. Previously
the function returned while preserving a route that could never resume. Valid
routes are unchanged. The clear is persisted by normal character save; it does
not retroactively validate the saved flight or silently bulk-edit character
records. The final native build and restart verification are pending.

## Final repair build

Visual Studio completed the cc6689cd source build with zero failed projects.
The active native image was replaced while stopped, with the prior pair/logs
preserved in out/runtime-before-final-nov5-20261105-024308. Both deployed hashes
match the Visual Studio output:

- worldserver.exe: 726b2ed69d8f55e8931d82d6296e64deb2833731ae0985ae5be2c1108e04c68b
- worldserver.pdb: 2dc1a0ddaf24857179eca52fc8421a8a27a8a55d14301e99c285d19bdb575388

The banner identifies cc6689cd with a dirty suffix from ongoing documentation;
later source-tree commits contain the separately applied SQL and reports. All
14 CTests passed against the final staged executables. Worldserver was launched
through the visible export launcher at 02:43:21. Authserver remained running.
SQL07 was applied with its complete private smart_scripts backup at
out/taskmaster-before-20261105-024247.sql (5,950,617 bytes). No entire external
database was imported, and no account/character rows were directly edited in
this pass. Taxi recovery is persisted through normal server character saves.

At 02:44:07 all 192 selected bots had logged in; world/auth listeners are owned
by PIDs4708/61228 on ports8085/3724. Two invalid resumes were diagnosed and
cleared by the recovery path. After normal `saveall`, both characters' taxi_path
were empty and taxi_path_point=0. The wider saved-route audit dropped from 15
unusable records in the earlier snapshot to 12; those remaining offline routes
will be checked/recovered when their characters next log in. No bulk character
database cleanup was performed. This proves the observed recovery/save cases,
not all valid cross-map and interrupted-flight behavior.

Through the 02:45:12 report there were no runtime missing-dialogue requests,
missing-criteria errors, spline/speed/clock/engagement warnings or opcode
collisions in the final run, and no new crash file. The startup still reports
969 missing native names, 101 missing positive SmartAI text-group references,
42 spell diagnostics and additional quest/object/loot/condition issues. Those
are unresolved; this build is not an error-free or sanitized public release.
Logs remain private under exports/logs; the detailed grouped report is
exports/logs/diagnostics/error-summary-all-20261105-024512.txt. Diagnostic levels
and validation were not reduced to obtain these results.

## Subsequent batched repairs

The next source/SQL/navigation pass is recorded in
[startup-batch-repair-2026-11-05.md](startup-batch-repair-2026-11-05.md).
Its runtime pair supersedes the hashes and counts in the earlier run above.
Use the newest report for current deployed artifacts and unresolved defects.
