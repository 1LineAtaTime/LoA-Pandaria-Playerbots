# MoP Playerbots project checklist

## Official rollup acquisition — November 8

- [x] Verify already-acquired release26.002 and latest corrected27.001 packages
  against published checksums; obtain and verify actual release-tagged source.
- [x] Confirm both published Gilneas encounter commits in source ancestry, with
  matching handler code and archived SQL present.
- [ ] Complete matching native/data reconciliation into active LoA; acquisition
  alone is not deployment or gameplay acceptance.

Provenance and explicit state: [rollup acquisition](skyfire-rollup-acquisition-2026-11-08.md).

## Quality automation — November 8

- [x] Add one-command targeted/full checks, source-sensitive build selection and
  explicit known-debt versus regression status.
- [x] Add private deduplicated runtime/source checkpoints and typed scoped SQL
  backup plans; verify rollback on a disposable table.
- [x] Protect required gameplay scenarios, audit identities and source fingerprints.
- [x] Rank shared repair causes by starter progression and fixed-spawn context;
  keep uncertain availability unknown.
- [x] Compare warning/error signatures and frequency without changing log levels.
- [x] Add bounded packet-driven workflows with live movement/health/visibility,
  correct quest-only interaction packets and persisted postconditions.
- [x] Journal verified QA ownership and test safe, repeatable native account recovery.
- [x] Record consolidated validation: 92 Python/22 native checks pass; 22 live cases
  pass and the six-kill workflow stops at 5/6. All 23 test accounts cleaned. The
  pipeline remains failed, with the combat-route limitation explicitly unresolved.
- [ ] Improve the bounded combat workflow to complete six credits reliably;
  do not weaken the expected result to obtain a passing pipeline.
- [ ] Add positive item-use/collect/escort/phase contracts and complete multi-quest
  chain coverage; implemented primitives alone do not verify those mechanics.

Usage, limits and rollback: [quality automation](quality-automation-2026-11-08.md).

- [ ] Reconcile Worgen 14154 wave/controller/actor bindings with the published
  encounter and completion-credit series; user requested investigation only.
  [Read-only evidence](worgen-14154-readonly-2026-11-08.md).

## Shared quest runtime and regression gates — November 8

- [x] Repair both addon aura loaders: ignore empty space tokens, append validated
  IDs without zero-filled holes or8-bit index wrap; retain real error logging.
- [x] Audit18,822 populated addon lists; identify seven formatting cases and one
  missing aura definition (template36805/spell58284) for review.
- [ ] Investigate observed aura-save deadlock1213 and persisted taxi-state warnings.
- [ ] Resolve addon36805/spell58284 from a matching source, without inventing data.

- [x] Correct escort death-state guard; guard stale final-waypoint notifications.
- [x] Add hash-validated dependency comparison: new findings/lost quest definitions
  fail even when aggregate counts stay constant.
- [x] Add normal-account invalid quest accept tests, including maximum uint32 ID.
- [x] Compile through Visual Studio;22 native and51 Python tests pass.
- [x] Final build:20 supported live cases pass, all20 temporary accounts cleaned;
  starter/gossip/dependency comparisons show no regressions. Eight broad mechanic
  groups and three known structural audit failures remain explicit.
- [x] Verify repeated spell-zero diagnostics disappear after the loader repair,
  without changing the configured log thresholds or discarding genuine bad IDs.
- [ ] Direct escort death/party failure and duplicate-final-waypoint gameplay acceptance.
- [ ] Selectively evaluate community quest walkers without replacing local crash,
  BattlePay, AH and companion fixes.

Evidence and rollback: [shared quest runtime batch](quest-runtime-regressions-2026-11-08.md).

## Quest dependency checks — November 8, 2026

- [x] Generate read-only graph for all18,145 quests:98,115 links; preserve per-quest
  findings, shared repair queue, input fingerprints and artifact hashes.
- [x] Follow runtime addon precedence and database spell overrides; do not count
  the68 valid database-only reward-spell links as missing client spells.
- [x] Run36 parser/graph/framework tests, including an empty-final-column reader fix.
- [ ] Triage246 grouped review entries affecting587 quests; ten missing objective
  definitions remain, alongside diagnostic evidence of unavailable actor handlers.
- [ ] Classify active/obsolete/disabled/seasonal/custom content and prioritize
  starter chains before raw counts of unspawned custom content.
- [ ] Add phase/condition and item-acquisition dependency evaluation.
- [ ] Add independent versioned public expectations and normal-client objective
  playthroughs. Keep BattlePay/companion/AH gameplay coverage separate.

Evidence, limitations, regeneration and rollback:
[quest dependency baseline](quest-dependency-audit-548/README.md).

Updated 2026-11-05. Check an item only after a live validation on the target client and database. Keep the existing production catalog and character data.

## 5.4.8 content master catalogue — November 8

- [x] Research version-specific sources and verify the LoA v1.0.4 reference archive/member hash.
- [x] Generate 36 account-free CSV lists with 414,598 rows including overlaps; compare IDs and names where supported.
- [x] Record 453 client-table file hashes and header status, including the 18273 DB2 header provenance caveat.
- [x] Validate all CSV row counts/output hashes and pass ten parser/read-only safety tests.
- [ ] Classify client/reference-only, custom, retired/test and availability-restricted content before importing/deleting definitions.
- [ ] Verify pristine 18414 extraction and hotfix provenance; local header compatibility alone is insufficient.
- [ ] Expand independent expected-behavior scenarios; catalogue presence is not gameplay acceptance.

See [master list guide](content-master-list-548.md) and [catalogue index](content-catalogue-548/README.md).

## Native legendary handler restoration — November 8

- [x] Wire both dormant world-script modules into ScriptLoader; 13 native handlers register.
- [x] Fix Timeless Eye storage ID, quest eligibility, duplicate ownership and null-item handling; add ceremony/reset guards.
- [x] Add normal-client gossip-item fixtures and pass both positive/negative cases with exact persisted counts.
- [x] Pass all 15 starter scenarios, 22 native tests, 13 framework tests and declared dependency checks; remove all 17 test identities.
- [ ] Verify complete legendary journeys, ring expiry, teleports and ceremony behavior.
- [ ] Restore native parity for the separate SmartAI fallback actors and port remaining missing handler groups.

See [native restoration report](legendary-native-restoration-2026-11-08.md).

## Large AI ownership batch — November 8

- [x] Enable existing SmartAI for 32 NPC templates whose assigned native handlers do not exist; retain all 85 event rows.
- [x] Preserve 28 valid SmartVehicleAI templates and correct the QA false-positive rule.
- [x] Back up both affected tables; repeated application changes no checksums.
- [x] Pass 33 new ownership checks, 408 existing data contracts, 12 framework tests and all 15 supported gameplay scenarios; remove all disposable test identities.
- [x] Save the missing-native inventory: 621 creature templates, 356 with spawns and 103 spawned quest-giver templates, across 876 missing native names of all types.
- [ ] Port/restore missing native behavior by area/quest chain; SmartAI fallback does not establish native encounter parity.
- [ ] Add live acceptance scenarios for the 32 newly selected AI handlers.

See [batch report](loa-ai-fallback-batch-2026-11-08.md) and [native backlog inventory](remaining-native-script-bindings-2026-11-08.json).

## QA-driven reference repairs — November 8

- [x] Restore exact original LoA templates for two missing items and Iron Belly Spirits; existing rows are preserved.
- [x] Remove three ownerless addon rows and clear two unused nonexistent paths on already stationary caster spawns.
- [x] Restore original SmartAI ownership for Crowley 35077 and Heart of Arkkoroc 37143; their assigned native scripts are absent.
- [x] Verify idempotence and pass five new reference checks plus 85 existing Gilneas data assertions.
- [ ] Test the restored templates' actual quest chains; one restored gameobject has no current spawn and the source item names are incomplete.
- [ ] Complete live scene/credit testing for the two restored SmartAI handlers.
- [ ] Investigate remaining two creature, seven item and one gameobject objective references plus 60 AI ownership conflicts.

See [repair report](qa-content-repairs-2026-11-08.md). The patrol, spell and encounter backlogs remain open.

## Unattended QA execution — November 8

- [x] Run the full final framework locally: 15 supported scenarios passed, eight mechanics remain untested.
- [x] Verify removal of all 15 disposable accounts/characters and independently check account residue is zero.
- [x] Save per-scenario traces, audit samples, exact fixture/executor hashes and an all-quest coverage inventory.
- [ ] Investigate audit findings: two missing creature objectives, nine item objectives, two gameobject objectives, three orphaned addons, two absent paths and 62 SmartAI ownership conflicts. Confirm whether affected content is active before repairing it.
- [ ] Expand beyond the three rewarded quests and two acceptance-only checks; 18,140 quests remain untested.

See [unattended QA evidence](automated-qa-unattended-2026-11-08.json).

## Automated QA framework — November 8

- [x] Versioned normal-client scenarios with independent reward expectations and persisted-state checks.
- [x] Structural quest/objective/actor/path/AI/shop audits with violation samples.
- [x] Explicit passed/failed/blocked/untested reporting; enumerate every quest in a coverage queue.
- [x] Regression comparison rejects lost passing coverage and increased violations.
- [x] Framework tests reject false success and unsafe/malformed scenario inputs.
- [ ] Investigate fresh network reauthentication failure observed by the first QA run; character re-entry is a separate test.
- [ ] Implement real kill/collect/item-use, escort/phase/vehicle, spell and encounter executors.
- [ ] Add shop, companion and auction transaction scenarios and an independent reference-server adapter.

See [automated QA guide](automated-gameplay-qa.md) for commands and coverage limits.

## Runtime repair follow-up — November 8

- [x] Repair opening/later Prince Liam speech bindings; five data assertions pass.
- [x] Activate existing SmartAI for Civilian Recruit and both Marshtide defenders.
- [x] Remove 25 inactive SmartAI duplicates owned by registered native scripts.
- [ ] Validate Gilneas guard conversation with absent citizens after the empty-list crash guard.
- [ ] Continue remaining unregistered native scripts and invalid content definitions; no blanket suppression.

See [runtime repair report](loa-runtime-repairs-2026-11-08.md).

## Full LoA beta deployment — November 8

- [x] Merge pinned beta 8a26cd41 with local BattlePay, companion, auction and safety changes preserved.
- [x] Apply 440 world/character SQL files, ten playerbot updates, and populate the split item store.
- [x] Build with Visual Studio; 22 native and 408 world-data checks pass.
- [x] Ordinary-account protocol campaign: 15/15 cases; five quest accepts, three rewarded turn-ins; disposable accounts removed.
- [x] Restore 356 missing dialogue rows (189 text groups) and legacy script speech without disabling diagnostics.
- [ ] Resolve remaining loader/content errors, including missing dialogue groups without reference rows and stale SmartAI bindings.
- [ ] Restore authored patrols for the 46 spawns made stationary by upstream SQL963m; this fallback is not a completed movement repair.
- [ ] Add full kill/collect/item-use/escort/phase/instance quest journeys and fresh-schema BattlePay/companion/AH transaction coverage.
- [ ] Produce a clean account-free distribution after the acceptance suite is complete.

See [full integration report](loa-full-beta-integration-2026-11-08.md) for exact scope, compatibility repairs, backups and limits. Earlier checked entries below retain their historical environment-specific validation.

## BattlePay

- [x] Display the account's DP balance and complete affordable purchases with a durable charge and delivery.
- [x] Show the native insufficient-funds result without charging or delivering an item.
- [x] Closing an affordable purchase's confirmation page immediately cancels its quote and allows another purchase. The 5.4.8 client retest succeeded; purchase 13 reached cancelled state in about three seconds and example account remained at 16 DP.
- [ ] Complete a cancelled-then-successful purchase sequence without waiting for a timeout.
- [ ] Restore example account from the temporary 16 DP test balance to its original 730 DP after testing ends.

## Auction house and bots

- [ ] Fix bot crowding at Stormwind bank counters and the outdoor noticeboard: screenshots show multiple bots sharing the same standing positions. Review spawn/teleport arrival, all banking/RPG paths, spacing and dwell limits; the current city service-target cap alone is insufficient. Validate walking arrivals and separate positions in game.

- [ ] Verify player bids, buyouts, and settlement against the AuctionHouseBot's auctions.
- [x] Use random playerbot character names as auction sellers, with an owner matching each faction. On the first live cycle, 40 new listings had 32 distinct RNDBOT owners.
- [x] Exclude internal and obsolete item names from new bot listings. The server expired 121 unbid legacy listings, and the live auction table then had zero marked listings.
- [x] Observe random bot bids and buyouts on ExampleCharacter's normally priced Windwool Cloth auctions. Multiple RNDBOT bidders appeared in `auctionhouse`, and successful-sale mail rows were created for ExampleCharacter.
- [ ] Confirm ExampleCharacter receives the sale proceeds in the game client after the configured delivery delay.
- [ ] Tune auction stock and pricing over several update cycles.
- [ ] Confirm the level-99 required-level ceiling rejects level-100 quarantine items from every item class after the new build starts.
- [ ] Validate the expanded 80-account, 80â€“100-online random bot population and watch startup time and memory usage.
- [ ] Confirm each restart refreshes at most 400 unbid legacy Auctionbot listings under new random sellers.
- [ ] Confirm the intended playerbot population remains active after restart, including dungeon and battleground participation.
- [ ] Observe Alliance and Horde duel bots in combat through completion and recovery.
- [ ] Diagnose the unusually fast gryphon rider and any other abnormal NPC or bot movement.
- [ ] Fix flying city patrols in Stormwind, Orgrimmar, and Ironforge: gryphon and wind riders jitter, run at excessive speed, and clip through floors and walls instead of following flight routes. Check other flying NPC patrols too.
- [x] Verify example account can use `.gm on` after a fresh login. Administrator rank 3 plus an explicit administrator RBAC role worked in the 2026-10-29 client retest.
- [x] Remove old Auctionbot-owned listings after confirming zero active bids and backing up the auction rows. The live market refilled to 5,725 listings under 879 random bot names, with zero Auctionbot-owned listings.

## Runtime and release

- [x] Clear example account mail with private backups, protect inventory-referenced items, and remove its orphan attachment links.
- [x] Repair the unsafe `mail.checked` byte-field read and restore saved account achievements in the real-player and bot login paths.
- [x] Confirm repeated player logins no longer reissue completed achievement reward mail. User logged ExampleCharacter in twice on 2026-11-04 and confirmed the spam/duplication stopped.
- [x] Display concise authserver loading/ready messages while retaining the quiet warning/error console profile.

- [x] Repair cross-map pool data. The per-map split retained 59,896 memberships, passed temporary-table hierarchy checks, and produced zero cross-map pool errors on the next boot.
- [ ] Finish the clean-startup audit: missing script implementations/bindings, remaining empty pools, missing/grouped loot references, unsupported criteria, quest objectives and SmartAI/spell hooks. See startup-repair-2026-11-04.md; do not clear diagnostics to claim completion.
- [ ] Validate the realm-wide corpse allocator under deaths on multiple maps and across a restart; repeated INSERT duplicate-key errors were traced to map-local IDs and a missing persisted maximum seed.
- [ ] Repair remaining missing navigation tile coverage and spell-hook/DBC mismatches identified by the all-category report. Missing tiles alone are not proof of corrupted MMaps.
- [x] Repair timed creator references and shutdown grid drainage; two bac2ff22 startup/shutdown cycles had zero lost-owner fatals or residual-grid diagnostics. Broader summon/combat soak remains open.
- [ ] Trace yellow "Created." plus level-up-like sound while walking around Dalaran. User suspects nearby bot spawning; that cause is unverified. `.tel` produces the sound without the "Created." text. Treat these as separate observed triggers; addons and developer-mode toggles were already tested. Identify the triggering server packet or script before suppressing legitimate effects.
- [ ] Validate bot presence in Dalaran, other capitals and quest zones at the user's 190â€“210 target. Audit live counts, activity allocation and map/level distribution before raising the target. Current relocation maps omit Pandaria (870).

- [ ] Validate the portable `exports` package on Windows Server 2019, including data files, relative paths, logs, and startup from both launchers.
- [ ] Confirm Command Prompt selection cannot pause worldserver or authserver through QuickEdit.
- [ ] Tune known database/content diagnostics after behavior is correct, then reduce logging verbosity for release.
- [x] Correct raw `mail.checked` field width; fresh logs no longer show out-of-range mail saves. Account reward replay is covered separately by the confirmed repeated-login test.
- [ ] Repair recurring world-content SmartScript errors; preserve diagnostics while suppressing per-step debug traces.
- [ ] Build and package a Linux runtime with equivalent data, configuration, and startup checks.
- [ ] Keep Forgejo `main` and `develop` at the latest validated commit and verify the private GitHub push mirror and default `main` branch.

<!-- mop-navmesh-validation-2026-10-30 -->
## Detour navigation crash follow-up

- [ ] Check Git working tree and commit the navmesh source fix and notes as a scoped change, if not already committed.
- [ ] Build Windows x64 RelWithDebInfo worldserver and matching PDB.
- [ ] Stage and deploy the newly built server using existing project workflow (not done by source patch).
- [ ] Test random creature navigation under normal NPC/playerbot load over time.
- [ ] If the crash recurs, compare new stack and dump before replacing MMap files.

See `doc/navmesh-crash-2026-10-30.md`. Do not mark runtime validation complete because the installer passed.

## Recovery milestone â€” 2026-11-04

- [x] Preserve and commit the recovered handoffs and existing Detour repair.
- [x] Resolve the November 2 dump with matching symbols and fix reconnect slot lifetime.
- [x] Verify prepared statements after forced reconnect on isolated MariaDB.
- [x] Build Windows worldserver and stage its matching executable/PDB in exports.
- [x] Restore missing valid source flight definitions with a database backup.
- [x] Add randomized repeated seller/buyer activity per auction house.
- [ ] Validate deployed production stability and visible flight routes over time.
- [ ] Audit mixed/scripted flight for Dalaran Visitor and Forsaken Bat separately.

Details: ../MoP_Playerbots_Crash_Fix_Handoff.md.

## City life and companion addon â€” 2026-11-04

- [x] Add Playersbots chat-command compatibility and configurable minimum level/rank checks.
- [x] Add nearby capital visits, service-stop selection, varied pauses and target crowd limits.
- [x] Pass 16 isolated companion access/capacity checks.
- [ ] Validate addon party/raid roles, attack, hold, passive and summon in the game client.
- [ ] Observe capital activity, spacing and path recovery with representative bot population.

See city-companions.md for configuration and acceptance checks.

## Companion crash and behavior follow-up â€” 2026-11-04

- [x] Preserve df5768fc crash symbols and inspect the repeated Unit::SetPower stack.
- [x] Repair missing command permission, RBAC graph reload and restricted-command ambiguity.
- [x] Build and stage the aura-iteration repair with matching symbols; pass 9 automated tests.
- [ ] Validate repeated summons/combat and prolonged local stability.
- [ ] Currency-only companion loot and pass every item roll.
- [ ] Quiet greetings/quest chatter; rare useful notices in party chat.
- [ ] Validate party-wide buffs, cleansing and healing.
- [ ] Preserve companions across instances/teleports; correct mounted flight following.

See companion-crashes-2026-11-04.md.

### 2026-11-04 fresh startup and new-character follow-up
- [x] Archive prior runtime logs/crashes and leave live folders empty.
- [x] Correct case-sensitive Battleground config keys without changing values.
- [x] Honor AllowTwoSide.Accounts in both PvP character-creation checks; policy tests pass.
- [x] Back up and clear ExampleCharacter's stale 73523 aura after verifying completed intro.
- [ ] Deploy and validate mixed-faction creation on an ordinary account.
- [x] Validate post-intro Deathknell recovery; user confirmed sustained movement without recurring Rigor Mortis. Fresh-character intro playthrough remains separate.

### 2026-11-04 verified data compatibility follow-up
- [x] Remove 417 empty pool choices while retaining all 59,896 real memberships.
- [x] Repair 326 equal-chance reference-loot group IDs without changing item/chance/count fields.
- [x] Restore ten absent classic reference groups; preserve existing complete-group overrides.
- [x] Restore 219 authored legacy broadcasts and 186 translations.
- [x] Repair six obsolete spell bindings and actual Fluidity/Focused Lightning hooks.
- [x] Validate heroic-only hooks against loaded difficulty variants without altering runtime dispatch.
- [x] Restore 23 sourced invalid lock cells and 36 sourced dialogue rows in 21 groups.
- [x] Correct 328 source-verified area-aura allowed-state masks; preserve unmatched/custom rules.
- [x] Confirm Rigor Mortis stays absent through several minutes of Deathknell movement after removing the recurring area autocast; user confirmed it no longer returns.
- [ ] Finish remaining script/criteria/SmartAI/object/quest/loot/navigation defects. Startup remains noisy with real errors.

### 2026-11-05 startup/runtime cleanup and release requirements
- [x] Deploy tested object/dialogue/spell/SmartAI/flag migrations with full private table backups and stable repeat application.
- [x] Pass all 14 native tests and diagnostics unit test; stage matching Visual Studio worldserver EXE/PDB in exports.
- [x] Correct incoming movement opcode registration collisions and three additional verified aura-hook indexes; fresh startup reports neither category.
- [x] Validate player-only Dalaran vendor greetings in a fresh 191-bot session: no unrelated-NPC greeting requests during six minutes; authored lines/player targeting remain intact.
- [x] Validate removal of 314 retired criteria extra-data rows while preserving all 1490 rows whose IDs exist in installed Criteria.dbc; fresh startup has zero missing-criteria diagnostics.
- [x] Recover two observed unusable taxi resumes and verify their empty routes/index0 through the normal save path. Remaining offline corrupt routes and valid cross-map/resume gameplay acceptance stay open.
- [x] Keep taskmaster17058 aggro speech on its authored speaker; fresh 192-bot run no longer requests group1 from attacker19477.
- [ ] Fix recurring zero-duration splines, invalid speeds and stale taxi-resume state; retain actionable diagnostics and verify under bot load.
- [ ] Finish actual spell hook compatibility repairs and LFG entrance-data validation.
- [x] Fix the reported The Disciple's Challenge29409 blocker, Jaomin Ro54611: user ExampleCharacter confirmed close-range attackability and quest completion with native repair/SQL24. Extended reset/group/vehicle acceptance remains below; see doc/jaomin-repair-2026-11-05.md.
- [ ] Audit the Worgen/Gilneas starting chain from a fresh character, including evacuation doors (14098), By the Skin of His Teeth (14154), The Rebel Lord's Arsenal (14159), transformation/phases and departure. Review identified SkyFire fixes against existing local SmartAI/native implementations before adapting code and SQL together.
- [ ] Build a race/class starting-zone acceptance matrix; use reviewed upstream quest/phase/gossip/credit fixes rather than importing an entire replacement world database.
- [ ] Produce a shareable Windows/Linux release using fresh auth/character/playerbot schemas plus required static data, with no personal accounts, characters, inventories, mail, wallet balances or private settings.
- [ ] Exclude active credential-bearing configs, account/database dumps, private logs, crashes and symbols containing private local artifacts from the public distribution; supply configuration templates and install/upgrade instructions.
- [ ] Validate a clean installation from the sanitized package. The current local exports folder is a private test runtime, not yet the shareable release.

### 2026-11-05 batched warning repair follow-up
- [x] Correct bound class/encounter spell roles and all currently reported hook variants; fresh startup after SQL08/09 has no hook/Validate errors. Preserve ordinary spell effects and actual trigger chains.
- [x] Stop resolving queue entrances for eight unsupported seasonal prototypes; preserve all four supported holiday mappings. No fabricated Sarsarun entrance.
- [x] Create carousel and database transport passengers in world coordinates before terrain queries; retain their local transport offsets.
- [x] Avoid eager navigation requests for confirmed empty grids; keep corrupt/missing real-terrain diagnostics. New input-availability test passes, including corruption/unknown/global-WMO cases.
- [x] Correct the generator's format10 uniform-liquid header; an isolated ordinary-water tile generates and passes the real MMap probe. No fixture tile installed in runtime.
- [x] Validate SmartAI dialogue against the selected speaker; retain runtime checks for dynamic speakers and errors for actual missing groups.
- [x] Repair marksman Shoot and scorpid Envenom warning roles with exact guarded field changes and full-table backups.
- [x] Clear six invalid RNDBOT saved taxi routes with complete-row backup and preservation of25 valid routes. The subsequent DBC route audit reports zero invalid records; future bad states still diagnose/recover normally.
- [ ] Finish actual missing native implementations:969 names remain, only38 even occur in local source tokens, and several of those are disabled/unregistered. Do not remove bindings or activate incomplete code just to lower the count.
- [ ] Repair the51 current actual-speaker text diagnostics, including stale creature GUID references (Yorik's targets resolve to Wild Turkeys). Do not import unrelated text based on the accidental current target.
- [ ] Finish SmartAI source/action support, loot/quest/condition/object/vehicle data defects. Console cleanliness does not prove that all-category file logs are clean.
- [ ] Complete longer bot/transport/flight/companion and quest acceptance; the fresh-start observation is a short smoke test, not release certification.

Details, backups, source commits and fresh-log results: startup-batch-repair-2026-11-05.md.

- [x] Fix Frigid Proto-Drake template accessory boarding while retaining player mount-summon clicks; two actual rider seats confirmed after the ebe4bd54 build, with no54513 error.
- [x] Observe a fresh198-bot startup with zero console-category warnings/errors. All-category logs still show1594 content diagnostics; this does not close the clean-startup or public-release requirements.


### November5 continuation: real diagnostic repairs
- [x] Apply backed-up, fixture-tested SQL11â€“19: messages, combat speakers, loot, legacy actions, spawn masks, respawn links, POIs and valid enchantment pools.
- [x] Fix weighted enchantment selection and no-enchantment0 handling; distribution/edge-case tests pass.
- [x] Guard active Emily escorts; use valid follow speeds and add actionable speed diagnostics.
- [x] Implement missing glyph/language behavior and owner-safe Geist binding; add validated legacy action154 speed semantics.
- [x] Deploy matching Visual Studio EXE/PDB and SQL20â€“23;16 CTests and diagnostics test pass;198 bots online. Full logs still contain1,494 diagnostics including958 missing native names; see startup-repair-continuation-2026-11-05.md and its static inventory.
- [ ] Validate completed-arrival taxi recovery under live multi-leg/logout/teleport behavior; unit boundary tests are not gameplay acceptance.
- [ ] Resolve remaining missing native scripts, event-source3 compatibility, SmartAI text/path/flags, vehicle clicks, conditions, POIs and loot diagnostics. Do not call a quiet console an error-free release.
- [ ] Repair authored path for Infested Bear47204 spawn175727 and reconcile outdoor legacy SmartAI flags1706/1538/20075.
- [ ] Resolve stale command override records and map571 graveyard/zone failure observed after bot login; preserve real diagnostics.


### November5 upstream quest repair campaign
- [x] Index all195 current SkyFire open issue reports and296 release26 world-update files, with direct upstream links, in doc/skyfire-upstream-worklist-2026-11-05.md.
- [x] Reproduce Jaomin29409's wrong active binding/faction configuration and preserve authored dialogue; implement guarded native encounter migration, nonlethal threshold/credit/reset, one spell-summoned hawk and one movement owner.
- [x] Deploy Jaomin native source4b6b2f48 and backed-up SQL24; matching EXE/PDB,17/17 CTests, world PID45872 listening8085, no Jaomin/vehicle script-load errors. See doc/jaomin-repair-2026-11-05.md.
- [x] ExampleCharacter user acceptance: Jaomin stays attackable and quest29409 completes on defeat; no encounter-related errors observed.
- [ ] Finish Jaomin follow-up acceptance: pet/group credit, longer hawk sequence, repeat challenger and evade/disconnect reset.
- [ ] Triage and adapt the remaining upstream worklist by local evidence, starting with the three pinned Gilneas fixes; do not mark upstream reports locally fixed without verification.


### November5 bridge quest turn-in follow-up
- [x] Capture live ExampleCharacter/Shang Xi visibility mismatch: same phase1, missing type12 detection in area5843; read-only diagnostic command added.
- [x] Restore two original sourced105064 area5843/5846 quest-end masks through guarded/backed-up SQL25 and supported cache reload; fixture preserves all other rows and repeat stability.
- [x] ExampleCharacter confirms bridge Master stays visible after area transition. Snapshot: detection4096/aura105064, both visibility checks pass. Persisted29409 rewarded and29410 complete; normal turn-in/progression verified.
- [ ] Trace the separate map571 zone0/empty-navigation-tile request to its caller/position lifecycle. Do not copy the transposed24/37 tile or suppress the warning.


### November5 map571 / published-fix continuation
- [x] Review all296 release26 SQL source files locally; keep per-fix applicability checks, not blind imports.
- [x] Fix SF1408 core null-target dereference after a nearby-target selection hook; compile and17-test suite pass. Rune quest gameplay remains separate.
- [x] Apply guarded/backed-up SQL26 for Stair of Destiny introductory rides and ordinary taxi menus, adapted to local option IDs and QUESTSTATE47 mask10; preservation/idempotence fixtures pass.
- [x] Verify Tarindrella92420 destination and Disengage obsolete-link absence already match expected fixes; preserve existing working data.
- [ ] Verify flight menus in a client for both factions and full Rune of Fire quest behavior.
- [ ] Capture/reproduce the map571/37/24 zone0 request. Earlier saved point not retained; two fresh boots including suspected bot Orgek did not reproduce. Diagnostic includes requester/current map/teleport/corpse context, including login/transfer states; no log severity or navigation files changed.
- [x] Repair the real missing map0/42/23 tile: generate from installed terrain/collision, verify existing-parent/eight-neighbor compatibility, install and restart visibly. Other coverage and runtime travel acceptance remain open; see mmap-42-23-repair-2026-11-06.md.

### November 5 published-patch compatibility and automation
- [x] Complete all 296 independent privilege-isolated SQL fixtures: 175 changed, 70 no observed change, 51 failed. Verify live access denied and fixture/account cleanup. No live patch deployment or gameplay pass is implied.
- [x] Implement reusable isolated SQL and read-only content-reference audit tools under contrib/upstream-tests; compile-check both Python tools and execute the catalog census.
- [x] Scan 18,145 quests, 95,337 client/custom spell IDs, 80,022 items and 17,679 quest objectives. Preserve explicit zero gameplay coverage; missing references are review findings, not automatic removal candidates.
- [ ] Adapt and validate the SQL candidates in published-sql-compatibility-2026-11-05.csv. Fixture SQL success is neither deployment approval nor gameplay acceptance; a no-change result is not proof of an existing repair.
- [ ] Investigate 13 missing quest-objective references and 519 missing item-spell references in content-reference-findings-2026-11-05.csv. Account for obsolete/quarantined items and runtime corrections before changing data.
- [ ] Extend the existing BotScenario harness on the authorized current test server: ordinary quest prerequisites/credit/rewards/next quest, spell effect/resource/aura assertions, item use/mail/trade/relog, and crash/soak scenarios. Use identifiable test actors, backups and explicit setup records; another server is not required. Separate synthetic fixtures from natural playthrough coverage; never mutate real player characters for test setup.
- [ ] Retain real-client acceptance for phase visibility, protocol/shop UI and client behavior. No universal complete 5.4.8 gameplay suite was identified; see automated-content-testing-2026-11-05.md.

### November 5 authorized current-server repair batches
- [x] Record user authorization: use this whole deployment as the test environment; sweeping documented fixes may proceed without another realm/server. Back up runtime data separately from Git.
- [x] Adapt the numeric portion of seven published starting-zone patches; correct their Night Elf Simple Sigil3106/3116 mistake. SQL27 changes223 fields across111 quests; no spawns, questgiver, text or chain-link import.
- [x] Track306 expected data values and reproduce223 baseline failures. Whole-row fixture preservation/idempotence pass; existing42 active class masks already match and are retained.
- [x] Stop worldserver, back up both quest tables, apply SQL27 and verify306/306 postconditions. Restore the backup and reverse the migration in a disposable SQL fixture, proving full-row recovery; no runtime rollback performed.
- [ ] Validate ordinary level1/2/3 eligibility, training credit, turn-in/reward and chain continuity with race/class test actors on this server. Data assertions are not quest playthroughs.
- [ ] Finish the other portions of those seven upstream files and the Gilneas batches, with source/schema/ID checks and backed-up deployment.
Details and recovery evidence: doc/starting-training-batch-2026-11-05.md.

### November 5 larger native/content campaign
- [x] Implement validated immediate SmartAI world-event summoning and real Map dispatch for 25 cannon rows; preserve complete legacy21888 with its extra quest trigger. Reject unsupported/cyclic/partial groups without duplicate execution.
- [x] Add topology/chance regression tests and a reserved-bot-only worldevent scenario/Windows trace runner. Native18/18 CTests and actual Map event sample pass (17 NPCs created); no quest completion shortcut used.
- [x] Activate complete Hot Rod world/summoned friend/quest handlers and Worgen pillory/release port through guarded SQL28. Correct two native dialogue-group IDs to existing authored text. Register complete Nightmare Vine and Sinestra handlers.
- [x] Adapt nine training-chain fields via SQL29, including Monk letter31141; correct upstream Night Elf/warlock ID mistakes and preserve four tutorial gates. Combined357/357 data assertions pass.
- [x] Verify full-field preservation, repeat stability, scoped reversal and restoration of the actual23.7MB five-table backup. Stage matching Visual Studio EXE/PDB and restart visible worldserver28648; auth61228 unchanged.
- [x] Classify all958 baseline missing bindings without deleting them: five names resolved, npc_friend_gob partially rebound,33 other exact local source tokens still unverified and919 without an exact source token. Fresh runtime missing-name count953.
- [ ] Natural Hot Rod14071 pickup/vehicle/credit and Worgen14222->14375 scene/release/relog acceptance; source hooks and data checks are not full playthroughs.
- [ ] Resolve21889's six unsupported rows and review custom homie clones934957/934958/934959; do not silently ignore unknown action parameters or alias different NPC entries.
- [ ] Continue the native-binding/SmartAI/text/loot/condition/vehicle/navigation backlog, companion/city/flight behaviors and sanitized release work in the campaign report. No error-free release claim.
Evidence/backups/artifacts: doc/native-content-campaign-2026-11-05.md.


### November 5 Gilneas progression and ride safety batch
- [x] Review nineteen pinned historical migrations; adapt text identities and optional waypoint facing to the actual schema, preserving original source attribution.
- [x] Implement owned horse-path snapshots and safe invalid-path handling; validate catapult destinations/speeds and delayed vehicle-kit access.
- [x] Add reserved-bot phased-height mechanism and nineteen native regression tests; confirm the gunship deck floor at105.347 and cannon event creation of14 NPCs.
- [x] Complete paired SQL30/31 deployment,85 new/357 retained contracts, actual-backup recovery and fresh-log comparison; four introduced conflicts resolved, no new exact startup errors,952 missing native names remain. See the campaign report.
- [x] Implement Crowley's33-point native ride, jumps, owned seats and retry/cleanup in source4f904145; data/native-load tests pass. Natural credit, concurrent riders and recovery acceptance remain open below.
- [ ] Play through new Worgen progression naturally: vehicle boarding/exit, survivor credit, ambush escape, escort retries, group/battle ownership, gunship combat/escape and relog. Structural contracts do not establish these.
- [x] Identify and fix Torn Earth55715's valid vertical-only NPC knockback from the repaired velocity diagnostic. Native timing/lift/landing regression passes; other producers and longer soak remain open.
Evidence and rollback: doc/gilneas-progression-campaign-2026-11-05.md.


### November 5 stability and quest priorities continuation
- [x] Implement the missing Crowley questgiver/owned33-point horse ride and explicit mounted actor, with two jump legs, retry gossip and safe cleanup. All16 new data contracts,19 native tests and actual-backup restore/preservation/repeat checks pass. Native load resolves two missing names;950 remain.
- [x] Guard Krennan's missing-player/control path and delayed passenger resolution, Josiah's delayed actors, and mountain-horse/caravan vehicle access.
- [ ] Validate Crowley14212 naturally in game: UI/credit, jumps/arrival, simultaneous riders and disconnect/dismount/death retries. Implementation is not client acceptance.
- [x] Trace Iron Sentinel29984 to Torn Earth55715/MoveKnockbackFrom and implement its valid zero-horizontal/upward12 trajectory.
- [ ] Recheck other movement reports, including Mud Wasp24731, and broader flight/transport variants using caller provenance; do not assume every producer shares this repair.
- [ ] Continue the remaining stability and starter-chain work; keep full diagnostic evidence visible.
Details: doc/stability-quest-continuation-2026-11-05.md.


### November 6 vertical knockback regression
- [x] Reproduce Iron Sentinel29984's explicit0 speed as Torn Earth55715's valid upward12 knockback; implement physical vertical-only timing and landing, preserving ordinary invalid-speed errors.
- [x] Fix the introduced observation crash on uninitialized NPC spline Duration() using the matching dump/PDB; keep failed-trial evidence and exact symbols.
- [x] Complete600 bot/NPC observations,391 empty-spline cases and two real vertical launches: peak3.728 yards, landing, zero velocity errors and no crash in the guarded repeat. Confirm the reserved actor's return live.
- [x] Build/stage matching Visual Studio EXE/PDB;19 native and five Python checks pass. SQL32/458 data contracts and tested recovery remain valid; no physics database migration.
- [ ] Extend movement/quest/client and long-duration load acceptance. Remaining content errors are not suppressed or declared fixed.
Evidence: doc/vertical-knockback-repair-2026-11-06.md.


### November 6 navigation coverage batch
- [x] Audit all 6,748 existing tiles and 259 parents for header/version/size/origin and duplicate-coordinate defects; no issues in those checks.
- [x] Process all 1,295 missing candidates with installed parent/VMap inputs: generate and install 495 real tiles, including all 68 Northrend candidates and the actual map571/37/24 tile. Record 800 runs without output separately; no fabricated floor or substituted coordinate.
- [x] Probe every addition against the original parent and available neighbors. Final 7,243-tile header audit passes; staged/runtime parents are identical and existing parent capacities are sufficient. No logger, executable, database or Detour change.
- [x] Track coordinate plan, per-tile results/hashes and reusable Windows repair tool; update export package manifest. Stop worldserver normally and relaunch visibly.
- [ ] Resolve any future concrete runtime requests in the 800 no-output cases or excluded inputs (256 map169 grids without parent, 66 grids without VMap root). These are input/geometry limitations, not proof of broken playable paths; do not fabricate tiles or hide warnings.
- [ ] Retain broader live navigation/soak acceptance and separate map571 zone0 investigation. This asset batch does not certify all movement or the original Detour crash.
Evidence: doc/mmap-coverage-repair-2026-11-06.md and its plan/result catalogs.

### November 6 release-26 quest and Bloodfang batch
- [x] Adapt 458 selected statements from 79 pinned published files (44 complete data files, 35 subsets); translate active quest tables/text/chain fields and preserve prior training gates. Track all296 per-source dispositions without claiming every fix deployed.
- [x] Deploy SQL33/34/35 with private whole-table backups. Final fixture passes377 field checks, unrelated-field preservation and repeat stability; live prior/new contracts835/835. Repair the Frostmane self-link and engine-specific relative movement action instead of suppressing diagnostics.
- [x] Compile/register missing Bloodfang35118 quest14272 handler; protect rogue practice targets before finisher credit, preserve ordinary kills and prevent ambient NPC chasing. Remove three guarded duplicate spawns only, retaining their counterparts.
- [x] Stage matching Visual Studio EXE/PDB, all19 CTests pass, visible normal restart. Keep accounts/mail/BattlePay and warning levels unchanged.
- [x] User confirmed the rogue Eviscerate practice quest completes in game (November 6).
- [ ] Check Bloodfang ambient spacing, ordinary kills after completion and longer combat/evade behavior.
- [ ] Review later-phase Bloodfang35229 density and67 one-health saved spawns in their actual quest/cinematic context. Do not blanket-delete262 later-phase actors based on the first-phase report.
- [ ] Continue source portions outside this batch, including native training credit/objective handling, full Gilneas progression and remaining published content/stability fixes. Ledger exclusions are not a declaration that everything is broken or already repaired.
Evidence: doc/release26-quests-worgen-2026-11-06.md and release26-quest-batch-ledger-2026-11-06.json.

### November 6 release-26 services and class batch
- [x] Implement reviewed portions of 77 archived sources: 21 loot/vendor/trainer and 56 class/proc files. Preserve unmentioned custom keys and non-client spell variants; pin the pre-update binding snapshot for reproducible cleanup.
- [x] Generate 15 transitive reference groups/540 rows from the digest-verified public26.002 package into a checked unused namespace; preserve existing groups and normalize its invalid110% group proportionally.
- [x] Port published Last Stand, Runic Corruption, Skull Bash and Shattering Throw behavior with this fork's API/registration adaptations; successful Visual Studio build and19 native tests.
- [x] Back up14 mutated world tables, deploy SQL38/36/37/39, and stage matching EXE/PDB. Final2410/2410 prior/new contracts pass with preservation/repeat checks; fresh boot has zero new exact error signatures and no MMap warnings.
- [x] Repair Scrounging for Parts26222 against six actual chest templates: active quest drops, disabled-mask repair, real owner checks and guarded removal of the introduced orphan. Keep all70 object spawns.
- [ ] Exercise natural quest collection, skinning/loot samples, vendor costs/stock, profession learning and the four native class repairs in gameplay; data/build/load checks do not establish complete behavior.
- [ ] Continue native handlers excluded by the registry check and complex quest/event/phase work. The two release-26 batches cover portions of147 distinct files; remaining files are not automatically defective or unapplied by older campaigns.
- [ ] Add portable installer checks for reference-ID collisions, required assets/schema/backups and sanitized clean Windows/Linux installation.
Evidence: doc/release26-services-class-repairs-2026-11-06.md and its source/value ledger.


### November 6 SkyFire27 integration
- [x] Pin official current source and release27 revisions; isolate integration worktree, preserve unrelated Git histories and make official remote fetch-only.
- [x] Inventory all 102 release27 archived and seven current world updates with hashes/module dispositions; preserve earlier release26 ledgers.
- [x] Adapt 380 class/level stat rows and 6,312 identity-matched NPC melee ranges/AP; retain 20 extra local rows and record nine identity differences. No table replacement.
- [x] Restore native template damage loading/selection while retaining LoA BG/AutoBalance/ranged/runtime multiplier policies; fix shifted single-template reload layout.
- [x] Adapt seven service/gate sources: stable masters, innkeepers, vendors, Lakka, Plan B and monk class eligibility. Disposable full-row/repeat fixtures pass.
- [x] Complete Visual Studio build (zero failures), 20 CTests and 9166 live data checks; take stopped-server five-table backup and stage matching EXE/PDB.
- [x] Record fresh startup/reload observations: no new exact diagnostic signatures, zero fresh MMap warnings, existing content errors remain visible. Verify all 562 package hashes.
- [x] Fast-forward validated main through Forgejo; both repositories private, default main and all 10 refs verified at bf246b71a. Documentation follow-up uses the same source tree.
- [x] Complete authored multiplier/rank policy in SQL49 and native code; preserve the nine reviewed identity exclusions instead of modifying different NPCs.
- [x] Integrate the SF27 inventory modules through SQL44–50 and matched native scripts/schema; see the111-input disposition ledger for explicit adaptations and equivalences.
- [ ] Complete natural combat/service/client and sustained bot/load acceptance, followed by sanitized portable Windows/Linux packaging.
Evidence: doc/skyfire27-integration-2026-11-06.md, upstream manifest, inventory and paired component ledgers.


### November 6 SF27 quest/loot/travel deployment
- [x] Adapt reviewed parts of 30 pinned published files: quest text/chains/race eligibility, turn-in links, ordinary creature/skinning/container loot, trainer cleanup and Scarlet LFG coordinates.
- [x] Preserve custom extra loot and quest links through keyed upserts/additions; map legacy quest fields to active split tables and LFG rows to the actual loader. Check all affected quests and turn-in actors exist locally.
- [x] Deploy SQL42/43 after stopped-server backups; all 9314 live checks pass. Preserve 420 populated Dragonmaw loot rows and repair three donor redirects found on fresh boot. Zero new diagnostic signatures and no MMap warnings after correction.
- [x] Verify all 566 package hashes; retain authoritative Forgejo/default-main promotion and native GitHub mirroring workflow for this tested milestone.
- [x] Complete the later SF27 native-dependent modules in SQL44–50; retain separate gameplay acceptance and older-source backlog.


### November 6 SF27 full spell module integration (deployed)
- [x] Import all eighteen active upstream spell modules at df19329b: eleven classes, generic, quest, item, pet, holiday, mastery and archaeology. Keep local-only registrations; assign each imported name exactly one owner.
- [x] Adapt newer local GUID, aura, cooldown, pet and SmartAI interfaces. Preserve archaeology advancement and delayed mushroom event ownership; do not replace core/bot/BattlePay/AH APIs with old upstream versions.
- [x] Generate scoped SQL44 for1,773 canonical bindings (1,793 individual rank bindings), proc/scaling/link/group/learning data and supporting NPCs from seventeen pinned files. Public fixture:468 checks, repeat-stable across fourteen tables.
- [x] Finish Visual Studio executable link and matched EXE/PDB deployment. Repair duplicate Polymorph ownership and pet-notification recursion found during startup/bot login. Final 193/193 bot login passes under Visual Studio; longer acceptance remains open.
- [ ] Continue remaining dungeon/quest/spawn/detection-range/core portions of the SF27 inventory; preserve earlier custom quest repairs and private account/shop content.

### November 6 SF27 interaction batch (deployed)
- [x] Adapt seventeen published sources covering Northshire reactions, Forlorn Spirit/incense credit, Night Elf travel objectives, quest containers, Exodar donations, guild services, spellclick, Flaming Torch LOS, racial action bars and apprentice professions.
- [x] Preserve local DBC-derived starter learning and level/race/class gates; do not recreate an unused old spell table. Mirror active gossip IDs and quest addon flags; preserve unrelated custom rows/fields.
- [x] SQL45 public fixture: 189 desired rows and repeat-stability across eighteen tables.
- [x] Back up eighteen affected live tables and deploy SQL45 offline. Native30cd0296b remains paired; existing unrelated content diagnostics remain visible.


### November 6 SF27 final integration and upstream refresh (deployed)
- [x] Reconcile the remaining content, typed loot/credit, detection range and authored damage modules while preserving BattlePay, AH and playerbot implementations.
- [x] Refresh official upstream through a69eda918: include the newly published Teldrassil dataset, creature visibility type flag correction, static Tarindrella lifetime and nightsaber vehicle ride; retain local Mist/door handlers and the existing two-point navigation API.
- [x] Deploy paired SQL46/48/47/49 plus SQL50 startup dependencies after offline backups. All16564 combined fixture/live checks and22 native CTests pass. Matching EXE/PDB are staged; fresh startup has zero new exact diagnostic signatures.
- [x] Reconcile all111 published world SQL input dispositions and publish the source/data to authoritative Forgejo. Final main/mirror verification is recorded in the deployment report.
- [ ] Gameplay acceptance: racial/class opening quest sequences, representative higher-level chains and dungeon encounters; successful compilation/data contracts are not a complete playthrough.
- [ ] Continue recurring unrelated loader/content diagnostics and city/companion behavior from the earlier checklist. Preserve real warnings/errors; no blanket log suppression.

Final runtime initialization completed in20 seconds. The five new import dependency
errors were repaired by SQL50; fresh DBErrors/Errors each still contain1388
pre-existing diagnostic records. These are real remaining content work, not
hidden or declared fixed. Server.log has no warning/error records at this check.


## Active LoA restoration — November 7, 2026

- [x] Restore customized pre-cumulative-conversion LoA source milestone `4aca50c794` on `codex/loa-fresh-restore`; retain all later source history.
- [x] Populate fresh `mop_loa_auth`, `mop_loa_characters`, `mop_loa_world`, `mop_loa_playerbots`, and `mop_loa_log` without importing old personal state.
- [x] Rebuild and stage matched auth/world EXE/PDB/DLL files in root `exports`; 22 native CTests pass.
- [x] Verify native ADMIN authentication and disposable normal-account character creation/world entry.
- [x] Verify 880 generated bot characters, roughly 190 online bots, 615-product shop and 3,000 auctions with 606 sellers.
- [ ] Client acceptance of BattlePay purchases/cancellation, companion addon commands, starting quests and sustained gameplay on this fresh game.
- [ ] Repair retained content diagnostics; restoration does not certify zero errors or a finished distributable.

Current baseline and SQL provenance: [LoA restoration report](loa-fresh-restore-2026-11-07.md). Earlier clean-SF/cumulative-conversion deployment notes are historical; the active configs now use `mop_loa_`.


## LoA starter/runtime batch — November 7, 2026

- [x] Repair character-create acknowledgement before durable commit; populate modern character cache and fail closed on missing committed ownership.
- [x] Fill the seven reported Arena.SeasonReward defaults without changing log thresholds.
- [x] Make stale internal loot releases idempotent before container/corpse teardown.
- [x] Run 408 starting/training/Gilneas/native-port data contracts and 22 native CTests.
- [x] Inventory broader missing quest/objective/item-spell references in [the census](loa-content-findings-2026-11-07.json).
- [ ] Natural Worgen curse/Last Stand14222 -> Last Chance at Humanity14375 progression acceptance; level-2 human form is expected.
- [ ] Replace invalid instance-only criteria17826/17831 with real outdoor Hyjal achievement condition handlers.
- [ ] Triage the 13 missing objective references and 519 item-spell references against active/obsolete/quarantined content and matching published data.

Details and release evidence: [starter repairs](loa-starter-repairs-2026-11-07.md).

- [x] Correct the database-worker connection mismatch caught by live creation regression; final native `f73779f9a5` passes human and Worgen create -> immediate enum -> login.
- [x] Keep auth listening while configured realms are temporarily unavailable during world startup; verify normal realm rediscovery.
- [x] Back up and remove 110 invalid Dreaming Glory starting buttons and 413 unlearned saved copies using scoped SQL51/character SQL02; repeat-safe, and no recurrence in the fresh observation.

## LoA beta repairs and real-client campaign — November 7, 2026

- [x] Fetch stable and beta upstream; inventory all 150 beta commits separately from historical SF deployment reports.
- [x] Integrate 25 scoped native crash/gameplay fixes; retain two already-present fixes and explicitly defer dependent summon lifecycle work.
- [x] Prepare five matching starter/event SQL patches and back up seven affected world tables.
- [x] Add a normal-account SRP/game-packet starter campaign, persisted reward checks and an optional actual-kill scenario.
- [x] Finish batch build/deployment and record live campaign results in `loa-beta-integration-2026-11-07.md`.
- [x] Complete the source integration dispositions in `loa-beta-backlog-2026-11-07.json`: all150 included in the November8 full merge; another50 commits and38 paired SQL files deployed November9. Gameplay coverage remains a separate open task.
- [ ] Expand gameplay coverage to objective combinations, escorts, phases, instances, BattlePay, companions and AH transactions.

- [ ] Improve real-client combat with live target tracking; initial route earned 2/6 credits and correctly failed.
- [ ] Investigate intermittent first human quest acceptance/persistence; isolated retry passed.


## Active reconciliation and crash repair — November 8, 2026

- [x] Trace ExampleCharacter's client crash to animation fallback tier8 and repair all5 out-of-range addon tiers plus2 invalid stand states; add native guards.
- [x] User acceptance: ExampleCharacter loads and stays in world after the repair.
- [x] Fix the BroadcastText1/repressed fallback; verify exact authored greetings through real WNPC/hotfix packets.
- [x] Pair SF prison-wave controller with all6 native bindings; live observer verifies moving waves and persisted14154 survival completion.
- [x] Port current SF Anzu boss/add changes while retaining LoA Druid spirits; live full fight remains pending.
- [x] Reconcile14 keyed interaction changes without overwriting local combat handlers.
- [x] Recheck18,766 adapted historical data contracts against mop_loa_world:18,748 literal matches plus18 documented dispositions.
- [ ] Complete broader missing-native/unsupported-SmartAI integration; historical deployment notes and downloaded rollups are not proof of active coverage.
- [ ] Full Anzu combat, ordinary-player prison survival and continued racial-chain acceptance.

Evidence: [active reconciliation](reconciliation-repairs-2026-11-08.md) and
[per-difference data dispositions](active-rollup-audit-2026-11-08.json).

- [x] Add a real-client owned vehicle start/logout/re-entry regression; no camera lifecycle failure on302cec2779.
- [x]15 supported starter cases pass with all disposable data cleaned;8 unsupported cases remain explicitly untested.
- [ ] Continue the named missing-native inventory:83 names are present in tagged SF source,777 not found there; classify retired/alternate ownership before importing.
- [x] Deploy8 registrations: missing names860→852,no new exact diagnostics. Normal-role Shattrath vendor denied without reputation and returned inventory after controlled exalted setup. Full Firelands combat remains pending.

## Exhaustive gameplay QA — November9,2026

- [x] Audit historical evidence; separate starter assertions from whole-quest passes.
- [x] Create isolated owned mop_loa_qa_ schemas/runtime without personal accounts/characters.
- [x] Generate all six artifacts for18,145 quests and6,183,814 static-boundary cases; dynamic axes explicitly unexpanded.
- [x] Implement reusable identities,11-character cap,extra accounts,leases,typed snapshots,durable stages and guarded resets.
- [x] Pass one normal-role9-stage delivery/reward/fresh-reconnect/negative lifecycle; preserve3 prior harness blockers.
- [x] Provision all122 bodies across14 accounts,each at most11 characters; creation is infrastructure coverage only.
- [x] Sacrifices real-client protocol lifecycle:30/30 actual casts/credit,33-point arrival,normal turn-in,150configured copper,XP increase,follow-up,fresh reconnect and protection cleanup.
- [ ] Sacrifices visual WoW actionbar acceptance and independent exact XP oracle.
- [ ] Expand all objective combinations,professions,spells,items,bosses,instances and raids.
- [ ] Expand dynamic eligibility,account/shared reset,dependency invalidation and independent oracles.
- [ ] Execute all applicable cases; catalog enumeration or direct completion grants never establish PASS.

## Normal gameplay and public-source batch — November9

- [x] Reconcile1,398 historical/current SF5.4.8 world inputs against the final SFDB27 quest data; deploy6,888 mapped field corrections across5,831 quests to both worlds. See historical-quest-field-batch-2026-11-09.md.
- [x] Include canonical prerequisite ancestors; remove6 existing positive-prerequisite cycles without introducing new ones. Back up four tables and verify every changed field plus repeat-stable checksums.
- [x] Integrate signed NextQuestId loading/reverse links/eligibility and schema;618/8554→-619 applied. Preserve valid LoA12222/12223→12225 instead of old self-links. Native05e94aabf6,24 native tests pass.
- [x] Reconcile the11 reported absent entries:7 untitled source placeholders are not playable imports;4 named content ports remain explicitly open below. Conditional/flag assignments now have source dispositions; broader native coverage is separate.
- [x] Account for all1,012 conditional inputs and380 field assignments; six build metadata assignments and retired transitions are not gameplay fixes. See remaining-quest-input-dispositions-2026-11-09.json.
- [x] Deploy294 compatible/identity corrections across260 quests with five-table backups and paired EXE/PDB. Both starts add0 new exact signatures; broader gameplay QA deferred.
- [x] Classify7 absent untitled reference placeholders; preserve local-only31664. Do not import empty metadata as playable quests.
- [ ] Port/reconcile actual progression and routes for31892/31902/32010/32011 before treating those four missing rows as playable content.
- [ ] QA the protected local class/race access and lesson identities; source-presence/policy dispositions are not retail gameplay acceptance.
- [ ] Investigate remaining4 prerequisite cycles and stored bot taxi-resume recovery (new exact GUID28/46 messages on this restart).

- [x] Inventory all10,738 SF/LoA SQL/native source files,including archived releases; index all3,478 release26 statements with non-deployment evidence labels.
- [x] Find and deploy missing historical Stranglethorn teleport height correction to both worlds; scoped backups,repeat-stable checksums and live reloads recorded.
- [ ] Reconcile ALL historical statement/base/adaptation/supersession coverage; latest-tip/selected-ledger checks do not establish this. See full-history-reconciliation-2026-11-09.md.
- [ ] Reconcile Westfall Transient ownership: existing SmartAI/timed lists already implement parts of26209; empty native ScriptName alone is not a missing feature. Preserve existing behavior when evaluating the SF native implementation and GUID-based roaming data.

- [x] Add normal-power contracts,account lanes,source/binary/data invalidation and a one-command entry point.
- [x] Record23 latest natural case passes across five contracts and three faction states.
- [x] Earn six Human kills normally,resume verified progress,and check XP/money/reputation/item/follow-up.
- [x] Reproduce/fix duplicate turn-in payout; Pandaren negative case now pays once.
- [x] Merge50 public LoA commits through68f8098431,build2daf99f5f0,and deploy38 paired SQL files with backups.
- [x] Preserve14 safe aliases and1707 reference bot texts in QA; no personal account copies.
- [x] Verify repeat-stable SQL checksums and zero new corrected QA startup signatures.
- [x] Recheck Sacrifices and demonstrate unchanged-case cache reuse.
- [ ] Expand beyond the five natural quest contracts to remaining content and eligibility states.
- [ ] Complete shared/account reset,universal recovery,visual checks and independent full-content oracles.

## Worgen transition and follow-up — November9,2026

- [x] Fix mutually exclusive client terrains and early bite/phase expiry; shared below-pillory recovery without quest grants.
- [x] Recompile/stage matched EXE/PDB;24 native tests and seven ordinary-class controlled14375 transitions pass.
- [x] Inventory all Gilneas quest/class pairs, clearly distinguishing unexecuted gameplay.
- [ ] Complete natural all-Worgen-quest/class gameplay QA; current controlled14375 checks do not satisfy it.
- [ ] Diagnose and fix actual Shift+P disconnect; empty journal wire tests pass, full client acceptance pending.
- [ ] Repair Cynthia/children click wiring and Greymane Manor ride/gates.
- [ ] Reconcile later Gilneas aura masks and quest prerequisites to prevent premature destruction.
- [ ] Validate underwater liquid/breathing state on ordinary accounts.
See worgen-progression-repair-2026-11-09.md for evidence and rollback.

### Worgen follow-up results — November9

- [x] Fix Shift+P AntiDOS kick: captured326 requests/200 replies; bounded2048 creature-query limit; replay326/326 plus live user acceptance.
- [x] Register pet-journal lock acknowledgement; guard species/model serialization; preserve mutation rate limits.
- [x] Repair all three child rescue bindings; Cynthia's spellclick persists objective credit (representative overlevel fixture).
- [x] Restore missing quest prerequisites; positive14369/negative14386 live gate check passes.
- [x] Restore15-point manor route, implement Gwen accept/retry ride, correct gate phases; user confirms progression.
- [x] Fix flood-phase liquid lookup;180-second breath timer, surfacing and clean logout verified on a normal-security fixture.
- [x] Fix headless PING field order and bound server latency to prevent character-save rollback.
- [x] Preserve Drowning Watchman lines in expected text groups0..2.
- [ ] Fix duplicate manor gate artwork (cosmetic; user says progression is sufficient for now).
- [ ] Trace three NULL_OPCODE sends during later ExampleCharacter progression; guarded sends remain visible in logs.
- [ ] Full natural Worgen chain/class QA remains open;122-quest/854-row catalogue is not gameplay proof.
- [x] Live-test Ashley and James: real spell-click credit persists on normal-security controlled fixtures. Whole-quest/multi-player acceptance remains open.

- [x] Final-build natural Lockdown! contract passes for all seven ordinary Worgen classes: exact rewards, duplicate protection, reconnect and follow-up.
- [x] Controlled real-item use: Worgen rogue/mage apply apple food433 and water430, health restores, no cast/inventory failure.
- [ ] Capture ExampleCharacter-specific food/drink failure if it recurs; generic item use currently passes, so no speculative food patch was applied.
- [x] Verify Two Forms timing/data:24593 Neither Human Nor Beast ->72829 ->72857 ->learn68996. ExampleCharacter has not reached it; natural quest gameplay acceptance remains open.

- [x] Fix stationary gameobject positions in the headless QA observer; native Y,Z,O,X layout regression passes.
- [x] Controlled rogue24593: all three wells, native quest reward, Two Forms cast and reconnect spellbook pass without spell/credit injection.
- [x] Repair Halga34258 dialogue groups used by existing caravan SmartAI; preserve original text and back up both schemas. Full caravan gameplay remains open.

- [x] Controlled24593 lifecycle/reward/cast/reconnect passes all seven ordinary Worgen classes on nativee142acb1b6. Natural prior-chain leveling remains untested.
- [ ] Validate repaired29901 Anduin gossip/summon/objective credit in QA.
- [ ] Reconcile remaining844 missing script names semantically; never bulk-clear genuinely missing gameplay bindings.


## Autonomous native gameplay milestones — November9 directive

- [x] M0: reuse existing QA runtime/pool; native self-bot questing operational; actualrealm1 verified.
- [x] Protect persistent-adventure bodies from deterministic reset; reuse untouched Orc Warrior120.
- [x] Fix redirected-console WinError6; captured command delivery now works.
- [x] Use racial language and require explicit self-bot enable acknowledgement.
- [x] Pilot120 naturally turned in25152 and2383, killed six3098 for25126, earned level3 without grants.
- [x] Normal logout/reconnect retains pilot's progression; independent observer can see its real world positions.
- [x] M1: native pilot120 rewarded25152/2383/25128/25126, earned/equipped53360, ordinary reconnect retains gear/quests;08 continues chain25172 and equips53386. See doc/qa/autonomous-gameplay-results-2026-11-09.md.
- [x] Fix native proactive grind starvation, unusable vendor-goods classification and full-bag quest reward recovery; original failures retained, same persistent character sold/returned/rewarded/equipped. Native8e4b42d17c, QA only.
- [ ] M2: naturally reach15 with verified gear/spells/inventory/death recovery.
- [ ] M3: form eligible1tank/1healer/3DPS party and verify actual dungeon completion/rewards.
- [ ] M4–7 remain NOT RUN; no level90/full-content certification claim.
- [ ] Anduin29901 fixture remains BLOCKED by visibility; binding spelling correction alone is not gameplay acceptance.
- [ ] Factory sizing side effect:264 generated bodies retained outside125-body certification pool; no autologin. Assess scoped cleanup only with backups, never delete original test history.


## Standalone unattended QA — November10,2026

- [x] Existing pipeline adventure profile and Run-Autonomous-QA.cmd run without Codex/API/client UI.
- [x] Bounded episodes, persistent pool leases, exact QA process/DB/bind/hash guards, graceful STOP and reconnect.
- [x] Readable report, structured run JSON, repair queue, archived framework sources, immutable episode evidence and launcher startup reports.
- [x] Native live25127 turn-in, earned53362 boots and reconnect; six pilot quests actually rewarded, level4.
- [x]74 Python checks pass; real invalid-character boundary produces BLOCKED report; cold QA world start succeeds with played servers coexisting.
- [x] Restore two missing QA config defaults detected by the unattended diagnostic gate; warnings remain enabled.
- [ ] Reconcile23 deferred obsolete/mismatched command-table errors identified by final run20261110T084612736299Z; exact signatures retained, no bulk deletion.
- [ ] Continue natural level15, eligible dungeon party and broader race/class/content acceptance; standalone operation does not certify the whole game.
See doc/qa/unattended-gameplay-qa.md and doc/qa/unattended-qa-validation-2026-11-10.md.


## Watching native QA and live notes — November10

- [x] Write concise notes.md alongside immutable JSONL during native play; objective/gear/level/spell/failure events and minute status snapshots.
- [x] Watch-QA-Notes.cmd follows the latest episode without Codex; real objectives265233 and earned loot1370/2649 observed and persisted.
- [x] Watch-QA-Game.cmd prepares the existing local client for the separate QA observer, with exact-byte settings backup/restoration and interrupted-session recovery.
- [x] Read-only game-launch readiness check and three focused notes/config tests pass; existing seven supervision tests pass.
- [x] Fix native adventure combat observation to use w.char_guid (full protocol GUID), rather than database counter.
- [ ] First normal graphical client login/visibility/follow acceptance. GM .appear acknowledgements pass, but new headless visibility probes remain inconclusive; preserve them and do not call graphical watching verified.


## Simplified single-pair workflow — November10 (supersedes observer workflow)

- [x] Start-Server.cmd and Run-QA.cmd replace the multi-launcher observer/autoplay flow.
- [x] One existing owned QA auth/world pair; repeated starts reuse it; other pairs block duplicate startup.
- [x] No automatic game client, observer addon, live-notes window or autoplay in default QA.
- [x] Preserve all database sets, character progress, original evidence and protected reference binaries.
- [x] Remove un-deployed native observer prototype from active source; deployed8e4 native source remains matched.
- [x] Two forced protocol lifecycle cases PASS (25152OrcWarrior,24959UndeadRogue);81 Python checks pass.
- [x] Reviewed reward oracles/profile explicitly1x; include modules in native provenance guard.
- [ ] Natural bot terrain/clipping, unsupported quest-item targets and circling remain gameplay defects; not repaired by launcher cleanup.
- [ ] Broader reviewed case/race/class/instance coverage remains unfinished.
See doc/workflow-simplification-2026-11-10.md and doc/branch-cleanup-2026-11-10.md.

## Regular login restoration — November 10 (supersedes QA-default selection)

- [x] Restore Start-Server to exports with regular mop_loa_* accounts on3724/8085; validate schema, realm build/port and recorded worldserver hash.
- [x] Restore original client endpoint; user confirms 'Login works'. No passwords changed and no client binary patched.
- [x] Run-QA reuses the same regular pair; quest-25152 lifecycle/re-entry passes and temporary test-account cleanup is verified.
- [x] Retain separate structural debt in report20261110T192721726573Z; overall NEEDS_REVIEW, not a false full pass.
- [ ] Repair missing objective references (2 creature,7 item,1 gameobject) and1 active SmartAI ownership finding after reviewing the recorded rows.
- [ ] Monitor for recurrence of pre-login client Error134; exact cause was not established by the routing repair.

## November10 integrated gameplay batch

- [x] Merge45 published LoA beta commits through ef5112acc3; preserve custom systems.
- [x] Deploy41 paired SQL files to regular mop_loa_world with full backup and stable replay checksums.
- [x] VS rebuild and stage matching EXE/PDB/DLL set in exports; keep one regular server pair.
- [x] Port SF cast-failure packet layout and Northshire worg engagement correction; integer-field parsing already equivalent.
- [x] Expand unattended Run-QA with authored workflows and invalid-quest regressions, concise repair queue and runtime diagnostics.
- [ ] Adapt SF Darkshore53e1b75bc1 and generic idle-creature engagement b53c84a8; not completed by this batch.
- [ ] Resolve missing native bindings/content diagnostics and broaden authored gameplay coverage; no zero-error/full-game claim.
See doc/gameplay-batch-2026-11-10.md for evidence and rollback.
- [x] Focused ordinary Human earns6/6 kill credits and persists complete quest28766; original logout failure retained.
- [x] Recover exact owned QA identity after native disconnect grace; no account/character/dependency left behind.
- [x] Retain logout refusal codes, bounded combat-only logout retry and90-second cleanup grace;37 headless tests pass.
- [ ] Re-run full combat workflow with repaired logout handling; persisted combat objective alone is not full workflow success.
- [x] Final helper live checks pass: ordinary Orc lifecycle and Human one-kill/persistence/logout/re-entry/automatic cleanup.

## November 12: original-world logs and Gilneas finale

- [x] Restore authored legacy broadcast text and locales; keep missing-text diagnostics enabled.
- [x] Correct XT002 periodic script binding and guard detached-heart teardown.
- [x] Restore68 complete missing loot-reference groups; correctly load zero-item reference rows.
- [x] Remove388 unreferenced empty pools and obsolete command/emote metadata with private backups.
- [x] Supply root city-errand configuration keys, preserving custom values.
- [x] Verify actual Half-Burnt Torch use removes the isolated rat; clean up test identities.
- [x] Repair QA item GUID transmission and destroy-object visibility tracking.
- [x] Restore glaive/hippogryph/wyvern vehicle kits and block duplicate click-summons.
- [x] Verify glaive boarding,40 repeated requests yielding one vehicle, exit and despawn.
- [x] Restore53 ship actors, six objects, authored speech and finale phase sequencing.
- [x] Verify timed hippogryph boarding, arrival/ejection at the ship and visible Lorna.
- [ ] Verify glaive projectile damage and natural24681 objectives/reward; boarding is not combat proof.
- [ ] Verify complete26706 deck/furnace/escape combat and reward flow on the restored original world.
- [x] Verify furnace recovery, explicit escape boarding, harbor arrival and completed26706 status with a seeded enlarged-health fixture; normal combat and turn-in remain separate checks.
- [ ] Resolve missing final ship-fixture voluntary logout acknowledgement; native owned cleanup succeeds.
- [x] Implement missing bot taxi map-boundary handoff; keep invalid-route errors enabled.
- [ ] Run deterministic cross-map taxi regression and longer crash soak.
- [ ] Repair remaining related empty pools,28 unresolved loot-reference groups, missing reward items and stale saved bot objectives.

See gilneas-finale-vehicles-2026-11-12.md and runtime-log-repairs-2026-11-12.md
for evidence, rollback material and the remaining diagnostic inventory.

- [x] Match the playable glaive to its level12 chassis; verify1525 HP and survival under normal infantry attacks.
- [x] Replace two-hour encounter respawns with60 seconds plus corpse decay; cap existing scoped pending waits.
- [ ] Measure a complete normal Outrider kill/corpse/respawn cycle; retain three authored spawns.
- [x] Restore missing Marie Allen inventories and verify the actual merchant response.
- [x] Verify Endgame acceptance after rewarded24681; explain the prerequisite following abandonment.
- [ ] Verify the intended status of the decorative Keel Harbor mailbox if stronger source evidence becomes available.
See keel-harbor-followup-2026-11-12.md for scope and rollback notes.

- [x] Identify the exact crashing gunship object from the build18414 client dump; correct generic/M2 versus WMO map-object type.
- [x] Recover the affected character to Keel Harbor without resetting quests; correct level1 passenger-ride health data.
- [x] Add DBC asset/model-type audit; scoped ship check passes.
- [x] User confirms real-client login and flight/render arrival without Error132 after the type14 repair.
- [x] Disable the obsolete overlapping moving gunship; verify one static ship and no old transport spawn.
- [x] Validate and detach rope descent; real rope packet lands on measured floor128.997 with transguid0.
- [x] User confirms no overlapping ship and safe rope descent; character is aboard fighting after migration04.
- [ ] Review346 other WMO/type audit candidates individually; do not bulk-convert functional door/goober behavior.
See gunship-client-crash-2026-11-12.md; protocol arrival alone is not render acceptance.

- [x] Preserve newer mover ownership when old vehicle control is revoked.
- [x] Anchor Endgame wyvern camera during AI flight; release camera and restore rider control at dismount/abort.
- [x] Verify return, accepted walking to Lorna, real offer-reward packets and actual quest turn-in on a disposable fixture.
- [x] Preserve the live character's already-rewarded Endgame; remove owned temporary recovery bookmarks.
- [ ] Confirm the wyvern's visual camera framing in a real-client replay; packet ownership is verified separately.
See wyvern-camera-control-2026-11-12.md for evidence and rollback.

- [x] Restore Nightwind departure destination for bind and teleport effects; add reward-gated stranded-player retry.
- [x] Repair ten invalid persistent addon emotes and two broken SmartAI event definitions; retain diagnostics.
- [x] Unregister AI before DisablePlayerBot deletes it; reject Player/AI session identity mismatches.
- [x] Verify departure/retry/Howling Oak acceptance and five companion lifecycles, former-bot player login and normal logout.
- [x] Ignore duplicate/mismatched time-sync responses without corrupting the queue; replay four replies and verify clean logout.
- [x] Fix authserver variable-width SRP assertion and temporary-buffer leaks; pass short/generated/malformed credential regressions.
- [ ] Monitor the invalid-spline-pointer crash path; the small overnight dump cannot prove the entire corruption origin.
See nightwind-ai-lifetime-2026-11-12.md for source, scope and rollback.

- [x] Restore the missing Howling Oak Genn48736 and verify normal28517 turn-in; Gwen remains the innkeeper.
- [x] Restore Tarien52644 jewelcrafting inventory with preserved costs and verify a92-item merchant response.
- [ ] Reconstruct/verify the optional Tyrande/Genn welcome roleplay scene separately from functional quest turn-in.
See howling-oak-tarien-2026-11-12.md for scope, evidence and rollback.

- [x] Restore Idriana’s missing bank gossip action and verify SMSG_SHOW_BANK through normal menu selection; hot-reload without restart.


## November12: curated names and5,000-character roster

- [x] Import the supplied20,000-name CSV with provenance; exclude invalid/reserved/style candidates.
- [x] Positively identify880 bot characters; rename, roll back and reapply with protected-state fingerprints unchanged.
- [x] Provision only the4,120-character shortfall through native creation; reach5,000 on455 accounts, max11/account.
- [x] Share atomic name claims between bot creation and player naming paths; no generated fallback while enabled.
- [x] Prove native preferred-bucket fallback, global exhaustion, resumed provisioning and final restart idempotence.
- [x] Pass11-character race/class/gender matrix, competing clients, relogin and fixture cleanup; repair rejected-Player session pointer.
- [x] Ramp250/400/550 and hold550 for90seconds; retain measured CPU/RAM/DB/error reports.
- [ ] Perform a longer550-bot soak and multi-bot combat/dungeon load, separately from logged-in capacity.
- [ ] Reconcile preexisting obsolete quest objectives and inventory-load repairs exposed by dormant bots logging in.
- [ ] Repair the invalid emote16 state command for creature25235; do not hide its diagnostic.
See playerbot-naming-cumulative-handoff.md and contrib/playerbot-names/README.md.

## November12: Gate of the Setting Sun

- [x] Identify13:14 crash: null instance controller in bombardier UpdateAI.
- [x] Back up scoped world data and binary/symbol/dump pair; restore map962 binding.
- [x] Restore80 missing authored objects and placeholder template mechanics; repeat is stable at82 objects.
- [x] Compile null-controller and empty-random-target guards through Visual Studio.
- [ ] Confirm the rendered first-section gap and lift traversal in the actual client.
- [ ] Audit other original-world instance bindings and object coverage against matching source; no broad overwrite yet.
See setting-sun-crash-2026-11-12.md for scope and rollback.

- [x] Restore five missing dungeon progression area-trigger bindings, including the first entrance door.

## November12: companion party control

- [x] Add central opt-in routine narration policy and suppress quest-accept sound; retain direct command/addon replies.
- [x] Prefer human LFG leader; allow leader removal of bot-only-party members and recovery of offline bots.
- [x] Mirror the real leader's LFG vote on pending bot votes, without voting for humans; handle counts above threshold.
- [x] Preserve auto-passes and reject late/duplicate loot votes; add leader-only .partyloot control.
- [x] Finish Visual Studio build and real-protocol companion party regression: silence, leadership, loot control and online/offline removal.
- [x] Implement Orb of Loyalty35626: +100 account DP per right-click, consume exactly one, Legendary/BOP/unsellable/stack20, requested tooltip.
See companion-party-policy-2026-11-12.md; currency redemption follows the current dungeon/party repairs.

- [x] Complete native entrance-door/lift/bombardier regression and fixture cleanup.
- [x] Verify orb stack20, bound flags, exact100-DP uses, overflow refusal, transactional rollback, account-wide credit and replay rejection.
- [x] Restore14 yak vendor entries and Birdhat reforger flag; verify both services on actual mounted passengers.
- [ ] Obtain visual client acceptance for the repaired dungeon route and yak service frames.
- [ ] Review duplicate realmcharacters inserts during rapid multi-character creation and general shop handling of legacy NULL wallets.
See orb-loyalty-and-yak-services-2026-11-12.md for transactional design, clean SQL and recovery.

## November 12: chat, companion controls and handoff

- [x] Preserve public ambient bot chatter; restrict quiet routine whispers to human-controlled grouped companions.
- [x] Repair confirmed stored punctuation mojibake and normalize future imports at load.
- [x] Add reproducible10,000-variant English context pack and idempotent playerbots-only SQL.
- [x] Keep existing public chat probabilities; add shared300-second dungeon party flavor throttle.
- [x] Verify real-roster explicit command reply and quiet quest-share behavior.
- [x] Verify native Orb of Loyalty cache responses: Legendary, unsellable, stack20, exact description.
- [ ] Longer550-bot dungeon/combat soak and rendered continuous Gate of Setting Sun route.
- [ ] Observe dungeon flavor through normal AI scheduling and review naturalness of expanded chat over time.
- [ ] Resolve unrelated original-world SmartAI/quest/inventory diagnostics; no blanket zero-error claim.
- [ ] Runtime mixed-human LFG vote matrix and actual companion loot roll outcomes.

See bot-chat-pack-2026-11-12.md, companion-party-policy-2026-11-12.md,
orb-loyalty-and-yak-services-2026-11-12.md and setting-sun-crash-2026-11-12.md.

## November12: runtime emotes, learning and social-chat follow-up

- [x] Repair69 authored waypoint emote modes, including Hilda25235's one-shot16.
- [x] Repair linked weapon actions500/12996 and invalid Sanitron46185 cross-links.
- [x] Restore matched authored dialogue:48 reference candidates/31 groups; existing rows retained.
- [x] Correct legacy8616 reward's missing20252 effect while retaining2458.
- [x] Keep zero generic-learning payload out of the spellbook API.
- [x] Slow public bot conversation with shared channel/per-bot/repeat budgets.
- [x] Replace general/party coda combinations with115 standalone lines.
- [x] Disable public quest play-by-play and low-quality loot narration.
- [x] Native32-thread chat-budget regression and repeated SQL postconditions pass.
- [ ] Continue unresolved original-world content errors; do not hide missing implementations or fabricate loot/dialogue.

See runtime-noise-and-social-chat-2026-11-12.md for exact scope and evidence.

- [x] Native reward/empty-payload spellbook checks passed; owned fixtures cleaned.
- [x] Restore four verified authored Gilneas graveyard links and reload normally.

## Queued: Dalaran presence and Auctioneer scan speed

- [x] Populate Dalaran logically: audit eligible city hubs/service stops, local bot counts, faction districts and path heights; reuse varied errands with spacing and recovery.
- [x] Audit Auctioneer cache waits; implement seller-name prefetch and repeat native paged/get-all benchmarks.
- [ ] Confirm rendered Auctioneer addon scan duration after the cache repair.
- [x] Group world/auth connection and thread settings near the config section headers, including the formerly buried playerbots database fields; preserve all values.
- [x] Document selected-player .additem35626 grants for soulbound Orb of Loyalty.

See next-city-auction-and-admin-followup.md for acceptance checks and commands.

Final city sample:24 surface bots,12 moved>25yd,19 reached services,22 distinct services,0 floor offsets>5yd. Native auction6642/45.188s and get-all6641/0.292s; all seller names prefetched. See dalaran-auction-followup-2026-11-12.md.
