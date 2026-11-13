# Worgen progression repair — November9,2026

## First deployed checkpoint

Shared cause: map654 enabled sibling terrains638,655,656 without conditions.
Gilneas phase definitions selected one terrain, but the modern terrain builder
reapplied every unconditional default. The client displayed conflicting terrain,
while the pillory actors used authored z42 coordinates. Existing ExampleCharacter was saved
near z0.52 below those actors. The repaired conditions select638 before14386,
655 between14386 and14466, and656 after14466.

The first migration fixes early bite/phase spell_area end-status masks. In this
core that field is an ALLOWED quest-state mask:64 means rewarded, whereas11 keeps
the spell only before its end quest is rewarded. Zone-level cage triggers cover
the base-map area IDs, and the shared player script recovers an alive, pre-release
Worgen saved within15yd below the pillory. It grants no quest, credit, reward or
racial spell. Later flooded phases, dead players, vehicles and active teleports
are excluded. Quest14375 still releases the linked stun through reward68639.

Battle pet serialization now validates species and uses the first valid creature
model rather than dereferencing model index0. This removes two unsafe null
accesses; it is NOT proof that the reported Shift+P disconnect is solved.

## Executed evidence

Visual Studio RelWithDebInfo/x64 succeeded. All24 native CTests passed, including
new positive/negative cage-recovery policy cases. The14 historical SQL logic
checks passed. The migration is repeat-stable: checksums of conditions,
spell_area and terrain_swap_defaults matched before/after the second application.

Real18414 QA clients for warrior, hunter, rogue, priest, mage, warlock and druid
all passed the controlled14375 transition: below-ground login recovery, only638
sent, Genn visible, quest accepted/rewarded through client packets, curse/stun/old
bite auras absent after release, two fully decoded empty pet journals, logout and
re-entry. These are normal-security clients with seeded prerequisite checkpoints,
not natural full-chain playthroughs. Death knights start in Acherus separately.

Private execution artifacts: out/quest-qa/worgen-cage-20261109-all/. The generated
quest/class CSV inventories the Gilneas catalogue and labels all unexecuted
objectives NOT_GAMEPLAY_TESTED. Inventory coverage is not gameplay certification.
The broader all-Worgen-quest/all-class request remains open.

## Deployment and rollback

Played and QA worldserver.exe/PDB pairs were staged from Visual Studio. Existing
MariaDB/MySQL-compatible DLLs and the working authserver were retained. Both
worldservers were shut down normally and relaunched in visible consoles.
Private backups are in out/backups/gilneas-cage-20261109/: old EXE/PDB pairs,
played/QA conditions+spell_area+terrain_swap_defaults SQL dumps, and ExampleCharacter's
pre-repair state. Production/private credentials and backups are not tracked.
Restore the matching database tables and EXE/PDB together with servers stopped;
no character/account schema replacement is required.

EXE SHA256:67e50f626a7860d4f8140eb1ff7ce69941d4cc055682b6af13f15e6f96d3a950
PDB SHA256:ac781765888d743bca1be88edea5cbcffff9b3530a953445d45c3833f80d2502

## User observations after deploy / remaining blockers

User confirms ExampleCharacter returned in the stockades on land. Shift+P still disconnects.
Newest available Error132 was independently located in the client function that
references BattlenetAPI_GetRecruitAFriendData (RVA0xA8458e); no claim that it is
the same event as the Shift+P disconnect. Recruit-a-Friend is already disabled in
the server feature packet. Preserve separate crash versus network hypotheses.

Follow-up investigation found later Gilneas aura end masks still64, active addon
quest prerequisites disagreeing with legacy fields, children wired to SmartAI
gossip despite spellclick flags, and missing native npc_gwen_armstead. These are
being repaired as the next checkpoint. User also reports underwater breathing;
inspect actual liquid state and ordinary-account timers rather than assuming a
Worgen racial. Normal breathing lasts180seconds in this core.

Two Forms is expected after24593 Neither Human Nor Beast, not immediately after
the cage. References: https://www.wowhead.com/mop-classic/quest=24593/neither-human-nor-beast
and https://warcraft.wiki.gg/wiki/Two_Forms . No early racial grant was applied.

## Follow-up deployment: collections, children, manor and flooded water

The live Shift+P capture settled the disconnect:326 distinct CMSG_CREATURE_QUERY
packets arrived in one burst, and exactly200 responses went out before the
per-opcode AntiDOS default kicked the player. The native limit is now2048 for
that read-only query only. Purchase, economy and other mutation limits were not
changed. Actual AntiDOS kicks now retain opcode/rate details at warning severity.
The captured public identifier list is a replay fixture; private session packets
remain under ignored out/. Packet logging was disabled again before final launch.

The journal-lock opcode is now registered and replies with the known empty lock
acknowledgement. That was a missing protocol step, but was not the actual observed
Shift+P kick. User confirms Shift+P now opens without disconnecting. The normal-
security replay receives all326 responses, answers a ping, and logs out normally.

A separate robustness check exposed a headless-client PING field-order bug:
latency precedes serial in this18414 core. It is corrected in the test client.
Server latency storage is bounded to characters.latency's unsigned MEDIUMINT
range, preventing an untrusted client value from rolling back a character-save
transaction. An oversized-latency replay saves16777215 and the session cleanly exits.
No schema widening or account/password change was needed.

The remaining Gilneas phase spell_area end masks are now11 rather than64. Only
missing active-addon prerequisite links are filled from existing nonzero legacy
links; explicit addon prerequisites are preserved. A positive/negative live test
accepts14369 from Godfrey but rejects14386 without14368. This prevents the known
premature destructive transition; it is not certification of every side-quest gate.
No rewarded quest was removed from ExampleCharacter and no missed quest was completed for her.

The three children were bound to SmartAI gossip events while their client clicks
cast68596/68597/68598. They now use the existing native rescue scripts, validate
14368, retain the rescuing player's GUID for dialogue, and finish their escape even
if the basement door is missing/already open. Cynthia's real spellclick persisted
objective264863=1. The representative fixture is level20, normal security, to avoid
nearby combat during persistence checks; it is not a normal-power balance test.
Sibling rescue bindings are repaired, but their individual live objectives remain
unexecuted in this batch. Already flooded characters are not rolled backwards.

Gwen's npc_gwen_armstead binding had no implementation. Accepting14465 now starts
the existing vehicle AI, and gossip provides a guarded retry after dismount or
reconnect. Its38765 path had also been overwritten with the76-point Hewell route.
The original15-point manor route from upstream commit2d4b1928 is restored, while
387650 retains the separate76-point Hewell route. Gates196863/196864 are placed in
phase16384. A live rider reached(-1669.61,2519.56,97.89), and the user confirms
ExampleCharacter rides up and can progress. Duplicate closed gate artwork remains a cosmetic
follow-up; it is not hidden or marked fixed.

Flooded Gilneas has real server/client liquid disagreement: at Cynthia's site,
base654 has dry ground around z1.39 with no liquid, terrain655 has ground around
z-17.88 and water at z0, and656 has ground around z-70.96. The liquid lookup now
uses the655/656 overlay for its authored legacy phase when an overlay tile exists.
It falls back outside those tiles and on other maps/universal phases. Overlay base
maps are created during startup, avoiding map-registry mutation on worker threads.
Detour, VMAP/MMap files and collision meshes were not modified. This is a scoped
liquid correction, not a claim of complete terrain/collision parity.
A level20 normal-security fixture at(-1987.47,2508.71,-5) receives an enabled180000ms
breath timer, clears it when surfacing, and logs out with reason0. Earlier level8
fixtures entered nearby combat; their refused logouts were retained as failed
artifacts rather than accepted as lifecycle passes.

Drowning Watchman text was also preserved and normalized: the native script selects
groups0..2 but the imported base stored three IDs in group0, producing missing-group
errors. The authored three lines now occupy the expected groups. No log severity
was reduced to hide these errors.

## Evidence, coverage and remaining work

- Final Visual Studio build:24 native CTests and32 headless framework tests pass.
- SQL06 repeat-stable checksums cover all seven modified world tables. Backups of
  the affected tables, routes/gates and creature text are in
  out/backups/gilneas-followup-20261109/.
- Live reports: out/quest-qa/worgen-followup-20261109/. Their binary hashes distinguish
  the earlier manor route run from the final collections/liquid build.
- Public summary: doc/qa/worgen-progression-results-2026-11-09.json.
- Exhaustive catalogue/mask matrix: doc/qa/worgen-quest-class-coverage-2026-11-09.csv.
  It lists122 quests,854 class rows and728 mask-allowed pairs. Prerequisites, phase,
  level and mechanics still determine actual availability. All unexecuted natural
  gameplay remains explicitly NOT_GAMEPLAY_TESTED; component checks are separate.
- Seven ordinary classes passed the controlled cage checkpoint on the first repair
  build. This is NOT an all-quest/all-class natural playthrough certification.
- Existing unrelated startup/content errors remain. Newer late-Gilneas progress also
  logged three guarded NULL_OPCODE sends; the invalid sends were blocked, the user
  progressed through14465/14466/14467/24438, and that producer still needs tracing.

Final staged EXE SHA256: 695731e6b9a4f3fa3a91b861440a6dbd5223fb0b3cb0411dd634ba977c6b8748
Final matching PDB SHA256: 5d75a52c5ea36b246c858af0db460dbd38b032c0ced1a34d708d5c44697049a7

## Natural introduction, food and Two Forms follow-up

All seven ordinary Worgen classes passed the natural level-one Lockdown! contract
on the final build (batch20261109-230717-a00174): actual accept/turn-in, exact XP,
money/reputation, duplicate reward rejection, authenticated reconnect and available
follow-up. No level, equipment, quest credit or reward was granted by that natural
campaign. These seven passes cover one introductory quest, not all122 quests.
The public coverage matrix now marks those seven rows NATURAL_GAMEPLAY_PASS.

User reports being unable to use an apple and Refreshing Spring Water, but closed
the client without retaining the red error. ExampleCharacter's actual4536/159 item records
have valid charges(-1), flags0, no duration, and the correct433/430 on-use spells.
Controlled normal-security Worgen-form rogue and mage clients used those exact
item types through CMSG_USE_ITEM with real item GUIDs. Both received food433 and
drink430, their health increased, and no cast/inventory error was observed.
The fixture adjusts health/mana and applies97709 for coverage; it is not a natural
quest-chain acceptance. No food behavior was changed without a reproduced fault.
The ExampleCharacter-specific report stays open for an exact error/state or targeted trace.

ExampleCharacter is currently on24483 Stormglen and has not rewarded24593 Neither Human Nor
Beast. Two Forms is correctly later:24593 rewards72829, which force-casts72857;
its LEARN_SPELL effect teaches68996. The data chain is present. No racial was
manually granted early and no quest reward was overwritten. This static chain
check is not a natural full24593 gameplay acceptance.


## Follow-up: child objectives, static-object QA and Two Forms

Ashley (36288) and James (36289) now pass the same normal-security,
controlled-prerequisite spell-click/persisted-objective checks as Cynthia.
These are individual objective checks, not a natural playthrough of the entire
Save the Children quest or a multi-player concurrency certification.

The QA movement decoder discarded stationary Y,Z,O,X positions. This made every
static gameobject absent from ObservedWorld despite valid create packets.
The decoder now preserves stationary positions, with a native-layout regression
check. No server visibility/phase data was changed to work around this harness bug.

Neither Human Nor Beast (24593) requires both Preparations (24628) and Take Back
What's Ours (24646). A controlled prior-checkpoint Worgen rogue accepted it,
walked to all three visible wells, used real gameobject packets, turned in the
quest, learned68996 via its normal reward, cast Two Forms (97709 observed), and
reconnected with68996 in the spellbook. No objective or reward spell was granted.
The test checkpoints prior quests and level12; it does not certify the preceding
quest chain or natural leveling/balance. The fixture stays within owned QA schemas.

Halga Bloodeye34258 had three sequential caravan lines stored as random
alternatives in group0. Existing SmartAI34258/3425800 requests groups0,1,2.
SQL07 moves only the original ID1/2 rows to groups1/2, retaining all text,
broadcast IDs and locale fields. Both world schemas were backed up and updated;
full caravan gameplay remains untested. Rollback: stop/reload affected scripts,
remove only these three current creature_text rows and restore the matching
private out/backups/worgen-qa-followup-20261109/*-halga-before.sql backup.

The read-only debug questvisibility command now reports matching gameobjects as
well as creatures. This supports investigating real object visibility failures.
No logging threshold or error guard was weakened. Existing startup content errors
and the three historical NULL_OPCODE sends remain unresolved and visible.

Validation: Visual Studio build succeeded;24 native CTests and33 headless QA
framework tests passed. Additional class runs and final artifact hashes are
recorded in the machine-readable follow-up report when completed.


Final controlled24593 class batch: Warrior,Hunter,Rogue,Priest,Mage,Warlock,Druid
all PASS on Visual Studio nativee142acb1b6, EXE76c20f85e3cb05f4139a6b3bd7acfbaa08c1dd66f2f18c254b1679df42653d97,
PDBa3b6f53044a2097725c2a82ae031e51242da75f584d98421fbf9da820735e848.
Every body used three observed wells, normal quest reward, cast68996 with97709
observed, and a reconnect containing68996 in the spellbook. Evidence:
qa/worgen-followup-results-2026-11-09.json. Prerequisites and level were fixtures;
no natural-chain or natural-leveling claim is made.

SQL08 corrects creature56434's Anduin29901 gossip registration typo. It also
removes nine obsolete spell alias rows only where the canonical same-spell-ID
binding already exists. Six unique obsolete spell names plus the Anduin typo
account for851->844 missing-name messages on fresh startup. This does not mean
seven missing gameplay implementations were written. No blanket deletion or
approximate-name mapping was used. The Blink Strike130392/frost-tomb lookalikes
were explicitly left unchanged because similar names do not establish semantics.
Anduin's actual gossip/summon/credit acceptance is pending the targeted live case.
The remaining844 names are recorded without claiming source definitions prove
registration in qa/remaining-script-bindings-2026-11-09.json; the reproducible
read-only generator is contrib/diagnostics/script_binding_inventory.py.

Anduin29901 targeted controlled case did execute and remains BLOCKED at upper
NPC visibility, before gossip/summon. Evidence is retained in ignored
out/quest-qa/worgen-followup-20261109/anduin-final.json. Do not treat the binding
correction as full quest acceptance or adjust content solely for an insufficient
fixture.
