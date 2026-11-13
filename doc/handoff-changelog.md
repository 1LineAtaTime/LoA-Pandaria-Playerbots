# Cumulative MoP Playerbots maintainer changelog

Prepared November12,2026. This document covers the accumulated custom overlay
against upstream playerbot-beta ef5112acc3331f7a38bc090edae887320696cac3.
It records implemented source/data changes and historical repair work; it is
not a certification that every quest, item, spell or dungeon works. Earlier
experiments with alternate databases/cores are documented history, not extra
servers to launch. Current operation uses one authserver and one worldserver.

## Read this before merging

- The source overlay contains complete changed files, not snippets alone.
- The review patch accompanies complete files; selected cleaned base SQL/config replacements are intentionally excluded from the patch to avoid exposing historical deleted private rows.
- Keep upstream author credits and licenses when reviewing the overlay.
- Clean migrations carry schemas/static content; they do not carry player saves, passwords, mail, wallet balances or purchase receipts.
- The optional static BattlePay catalogue is separate from account balances and is not a replacement for a maintainer's customized catalogue.
- The owner's explicitly requested private config/*.conf files contain working database credentials. REMOVE those three files before any public upload. Sanitized .conf.dist templates remain in the source overlay.
- Match native code, schema migrations and content bindings; SQL filename order alone is not a safe dependency plan.
- Back up related auth/character/world/playerbot data consistently before deployment.
- Do not replay old rollback scripts as forward fixes or import historical DP-reset SQL over accounts.
- See database/migrations.csv for exact target schemas, purposes, policies and hashes.
- See notes/04-VALIDATION-AND-OPEN-ISSUES.md for measured validation limits.
- Whole-file inventory and hashes are in changes/changed-files.csv and MANIFEST.sha256.
- No public upstream PR has been created by this handoff preparation.

## Native runtime, build and database support

- Restored and preserved the original LoA history rather than discarding attribution for a source ZIP.
- Documented the pinned upstream baseline and later beta reconciliation decisions.
- Kept local development and authoritative private origin separate from the GitHub push mirror.
- Added Windows Visual Studio/CMake build guidance and runtime connector packaging notes.
- Retained EXE/PDB pairing for postmortem debugging; symbols are private build artifacts rather than source handoff files.
- Added an active-runtime manifest with hashes for deployed artifacts and native source evidence.
- Preserved modern MySQL/MariaDB connector compatibility while retaining the legacy server capability floor.
- Detect database vendor and bounded version from the server's identity rather than from compile-time client library identity.
- Handle the MariaDB compatibility version prefix only for identified MariaDB servers.
- Fail database setup when required character-set setup fails rather than silently using a lossy encoding.
- Support MariaDB Connector/C discovery layouts and explicit CMake include/library hints.
- Document matching headers, import libraries, runtime DLLs and authentication plugins.
- Document isolated MySQL5.5 and MariaDB10.5 schema/connector validation and untested remote/TLS combinations.
- Use original schema names in private runtime configs without shipping the original account rows in clean SQL.
- Add database compatibility aliases/triggers where historical authored fields differ from native reader expectations.
- Preserve signed quest-link semantics across schema/native compatibility changes.
- Add a worldserver lease guard to reduce accidental concurrent realm processes against one schema set.
- Simplify normal operation to one auth/world pair and reuse that pair for local QA.
- Use relative data/log paths for portable packaging rather than a developer's absolute workspace path.
- Keep visible direct server consoles and launchers that do not strand a batch wrapper at a termination prompt.
- Preserve warnings/errors in the console and detailed SQL diagnostics in files.
- Add useful auth readiness output without enabling per-packet debug spam.

## BattlePay wallet, protocol and recovery

- Treat auth.account.dp as the sole account wallet, using whole stored DP units.
- Keep client wire-price scaling separate from database wallet units.
- Read wallet ownership from the authenticated session rather than a supplied account number.
- Use checked integer price/discount calculations with consistent confirmation quotes.
- Guard negative, NULL, missing, overflowed and unsupported wallet states.
- Check sufficient funds before debit and retain affected-row evidence for mutations.
- Add purchase state, item snapshot, delivery receipt and token-credit schema support.
- Add boost entitlement/recovery bookkeeping without discarding historical entitlements.
- Require the relevant accounting/storage invariants before enabling purchases.
- Preserve idempotency receipts across upgrades instead of replaying unknown outcomes blindly.
- Repair the balance tile so the wallet is displayed without a crossed-out zero price.
- Repair the native insufficient-funds response so the shop clears its connecting overlay.
- Repair cancellation of affordable purchase confirmation so another purchase is immediately possible.
- Preserve working affordable purchase deduction and item delivery behavior.
- Avoid treating a successful purchase-confirmation UI state as proof of a failed transaction's delivery.
- Keep customized static catalogue data and allow optional clean catalogue handoff separately.
- Do not reset existing account DP during runtime compatibility updates.

## Orb of Loyalty voucher

- Bind item35626 to a registered native redemption script.
- Change quality to Legendary, binding to soulbound and maximum stack size to20.
- Set vendor sell value to zero.
- Set the exact requested account-currency tooltip description in canonical locale records.
- Keep use spell483 as a client-use marker while native code performs the accounting.
- Add100 whole DP units per consumed orb, not10000-scaled wire currency.
- Reject bot redemption, wrong item ownership and unsupported transaction storage.
- Validate same-host auth/character storage and schema identifier safety.
- Flush prior pending inventory changes before the redemption transaction.
- Atomically update the account wallet and expected owned stack in one InnoDB transaction.
- Require the expected affected rows rather than assuming a debit occurred.
- Add a durable redemption receipt and remove empty inventory/item rows atomically.
- Publish in-memory consumption and shop refresh only after commit.
- Treat a legacy NULL wallet as zero only when that account actually redeems a voucher.
- Reject wallet overflow without consuming an item.
- Force reconnect on uncertain database failure rather than guessing compensating credits.
- Refresh both item and sparse hotfix timestamps to invalidate old client metadata.
- Verify native stack splitting, repeated use, final-item use and account-wide credit across two characters.
- Verify replay of a consumed item packet cannot add another credit.
- Verify a forced post-update SQL failure rolls back wallet and item together.
- Verify native cache responses carry Legendary quality, stack20, zero sell value and the exact description.
- Keep owner-requested old-item cleanup and the replacement test stack in private execution evidence, preserving all characters.

## Auction house simulation

- Support random existing playerbot characters as visible auction sellers and buyers.
- Preserve the existing auction engine's price/chance/stock behavior.
- Select faction-appropriate identities for faction auction houses.
- Keep neutral-market selection separate from faction restrictions.
- Recognize legacy bot ownership so simulated stock is not purchased as player stock.
- Filter TEST, DEPRECATED, UNUSED, DEBUG and related broken-item markers.
- Apply token-aware filtering without rejecting ordinary names containing incidental substrings.
- Respect the required-level100 exclusion across item classes and forced inclusions.
- Keep legitimate high item level separate from required-character-level exclusion.
- Gradually retire unbid legacy single-owner listings through normal market handling.
- Preserve player bids during restricted/legacy listing refresh.
- Allow repeated market activity under the same random character to avoid one-name-per-item artificiality.
- Record live buying/bidding/sale-mail evidence separately from rendered auction UI observations.
- Document that market stock is simulated by the auction engine; a name on a listing does not prove that bot walked to an auctioneer with that item.

## Playerbot identities, population and command controls

- Import a curated20000-name source pack with a reproducible candidate generator/validator.
- Add a persistent name-claim schema with provenance and controlled exhaustion behavior.
- Reserve existing names and protect non-managed player identities.
- Apply atomic claims during native character creation and relevant rename/customization/faction-change paths.
- Handle rejected creation without leaving a session pointing at a destroyed stack Player.
- Validate concurrent clients contending for a single remaining candidate.
- Rename the existing880 managed characters while preserving their GUID/account identity and dependent records.
- Provision4120 additional characters to reach5000 managed bots.
- Enforce at most11 characters per account; use455 managed accounts for the roster.
- Make account/character provisioning resumable and stable across restart.
- Exercise a deliberately limited candidate pool, exhaustion and fallback before resuming the full population.
- Set the current random online target to550 rather than confusing online population with total roster size.
- Record250/400/550 ramp observations and login heartbeat timing without calling a short run a long-term soak.
- Preserve human account/character identities during roster migration.
- Retain native addon command compatibility and permission-level controls for requesting companions.
- Keep level/security eligibility configurable instead of giving every new character unlimited bot support.
- Preserve party summon/follow/control functionality alongside autonomous random-bot behavior.

## Companions, LFG, loot and communication

- Disable routine hello/goodbye and quest narration whispers for human-controlled grouped companions.
- Preserve public ambient chat and explicit addon/command reply traffic.
- Allow a bounded reply window for queued commands whose actions execute after parsing.
- Remove quest-share notification sound/narration under the quiet policy.
- Retain the real player as leader of a human-and-bot LFG group.
- Permit removal/replacement of online and offline companion members.
- Mirror the initiating human's vote through companions where the native LFG vote path applies.
- Preserve mixed-human vote participation instead of bypassing it as a bot-only group.
- Keep companion item loot policy gold-only and force roll passing.
- Reject duplicate/late bot roll changes without breaking internal removal sentinels.
- Add the server-side .partyloot control for eligible one-human companion groups.
- Leave the retail LFG loot dropdown limitation explicit rather than claiming its client UI was changed.
- Verify four-companion joining, quiet quest sharing, human leadership and loot-control command.
- Verify real native LFG join/proposal formation and online/offline bot removal.
- Verify a real roster companion answers an explicit deferred position request while subsequent quest narration stays quiet.
- Preserve follow/teleport/party support work while tracking long-run movement and dungeon behavior separately.

## Expanded ambient chat

- Repair confirmed CP850-decoded UTF-8 punctuation in existing chat data.
- Normalize known CP1252/CP437 punctuation damage without lossy decoding entire localized strings.
- Apply bounded punctuation normalization in the native loader for future imports.
- Preserve genuine localized letters and all eight localized columns.
- Add10000 distinct English variants from contextual openers and reusable conversational codas.
- Add1000 variants each for general suggestions, gathering, selling, dungeon grouping, quest grouping, reputation, accepted quests, turned-in quests, ordinary loot and dungeon party flavor.
- Use real calling-hook item/quest/price substitutions rather than fictional links or fixed IDs.
- Keep existing public event probabilities; increasing the library does not increase timer frequency.
- Restore native SayToParty packet delivery from the prior no-op implementation.
- Permit only the generic suggestion action's human-controlled dungeon flavor path in grouped mode.
- Share the party flavor cooldown across companions under the same human master.
- Default to one occasional flavor message per300seconds with a60-second minimum.
- Keep routine quest/lifecycle narration disabled even when conversational flavor is enabled.
- Include deterministic source data, generator and offline validator.
- Use a temporary indexed SHA-256 identity set for fast idempotent import without permanent schema/index changes.
- Preserve old/customized chat entries and let the importer allocate IDs normally.
- Verify10000 unique pairs, ten valid categories, hook-specific placeholders and ASCII punctuation.
- Verify111-character maximum before substitutions and no embedded client link/color control payloads.
- Verify two imports leave11707 total entries without duplicate growth.

## Gilneas and other progression repairs

The migration ledger below identifies exact files and historical batches. The
following summarizes the native/data repair themes rather than claiming a
complete rendered playthrough of every starter zone.

- Repair Jaomin Ro's attackability transition so The Disciple's Challenge can complete.
- Repair bridge Master Shang Xi visibility rules and retain phase/aura diagnostics.
- Remove recurring inappropriate Rigor Mortis area application rather than only clearing one character once.
- Restore starter questgiver placement/bindings where matched authored data was missing.
- Repair Worgen training objective/target survivability for Practice Eviscerate.
- Reconcile Worgen scripted waves, actors, quest credit and supporting bindings.
- Repair Gilneas horse/vehicle interactions and retain normal progression prerequisites.
- Repair cinematic/animation-tier client crash data rather than only relocating one player.
- Reconcile cage/bite/terrain progression and lingering control-state behavior.
- Restore rescued-child quest interactions and vehicle retry paths.
- Repair collections query throttling that disconnected clients opening Mounts and Pets.
- Restore consumption/quest-state handling and document Two Forms progression constraints.
- Repair Battle for Gilneas start/retry event sequencing and phase isolation.
- Reconcile Gorerot spawn, respawn, combat and supporting assault actors.
- Repair post-battle handoff/questgiver visibility and bombing-flight phasing.
- Repair captured riding bat boarding, bomb and return behavior.
- Repair the Half Burnt Torch quest interaction rather than replacing the quest with automatic credit.
- Restore missing authored broadcast text and remove invalid legacy spell hook bindings through reviewed compatibility data.
- Repair Endgame and allied-vehicle bindings and constrain repeated interaction spawning.
- Restore vehicle health/service/encounter data and Keel Harbor vendor stock.
- Fix gunship gameobject type/model/animation-tier handling implicated in Error132.
- Remove overlapping duplicate gunship data without deleting unrelated transports.
- Repair rope landing position so it lands on the ship instead of outside the world.
- Repair wyvern camera/control restoration and post-flight interaction state.
- Restore Admiral Nightwind departure dialogue and final travel continuation.
- Repair Howling Oak turn-in interaction, vendor Tarien and bank teller Idriana services.
- Keep user-character recovery operations separate from reusable systemic migrations.

## Dungeons, movement, crash prevention and services

- Apply the scoped Detour null-edge crash guard with dump-based documentation and explicit remaining live-validation limits.
- Repair reviewed MMap coverage/format/data problems rather than treating every warning as evidence of bad Detour source.
- Preserve remaining movement/spline/taxi diagnostics as real follow-up items.
- Restore authored spell/AI effect bindings and compatible native ports instead of merely lowering log thresholds.
- Repair Gate of the Setting Sun's missing instance script binding.
- Restore80 missing authored dungeon objects and26 relevant templates/addons from matching LoA reference data.
- Restore five native dungeon area-trigger bindings.
- Guard bombardier logic against a missing instance controller.
- Guard an empty target pool before random selection.
- Verify the first door's native area-trigger state change and lift object/period data.
- Preserve exact crash EXE/PDB/dump evidence privately.
- Restore14 Cousin Slowhands merchant inventory entries on actual mounted Yak passengers.
- Restore Mystic Birdhat's5.4.8 reforger flag while retaining existing service handlers.
- Verify a real mounted Yak merchant response and successful native reforge reset.
- Keep rendered continuous bridge traversal and rendered service UI verification explicit as outstanding visual checks.

## QA, documentation and delivery

- Add headless protocol clients for real authentication, creation, quest, interaction, movement, spell and vehicle exchanges.
- Add fixture ownership and cleanup helpers to avoid modifying unrelated player saves.
- Keep automated reports distinct between static reference checks, native protocol evidence and rendered-client behavior.
- Add normal-character and accelerated-fixture campaign modes with explicit limits.
- Add offline/unattended QA scripts and report artifacts so a human need not keep an agent watching.
- Build content/reference inventories for quests, spells, items, bindings and dependency gaps.
- Record historical source statement dispositions instead of blindly replaying every old SQL release.
- Add focused starter-zone and Worgen progression campaigns.
- Add transaction failure/replay/overflow checks for monetary item use.
- Keep private backups, crash artifacts and live credentials outside the clean source patch.
- Include clean schema/static SQL, deterministic generators and whole changed native files in the handoff.
- Generate a cumulative SQL inventory, changed-file inventory, provenance and file hash manifest.
- Check patch application against the pinned upstream base before handing it off.
- Audit the source package for private identities, endpoint paths, secrets and executable/debug artifacts.
- Preserve original maintainer attribution and document alternatives/reverted experimental work honestly.

## Known open work and evidence boundaries

- No claim that all20000 quests or every profession/spell/raid was played successfully.
- Native automation cannot establish every visual, cinematic or human-gameplay expectation by checking database rows alone.
- Long550-bot combat/dungeon soak remains outstanding.
- Rendered uninterrupted Gate of the Setting Sun traversal remains outstanding.
- Normal AI scheduling and perceived naturalness of the new party flavor need observation over time.
- Mixed-human vote matrices and real companion loot-roll outcomes need broader runtime coverage.
- Original-world SmartAI, obsolete quest/inventory and movement diagnostics remain separate repair work.
- A temporary intentional SQL failure in transaction QA is evidence of a rollback test, not a production fix to hide.
- Historical release compatibility dispositions are not a claim of literal byte-for-byte adoption of every upstream release statement.

## Complete clean SQL delivery inventory

Each entry below is a distinct delivered SQL artifact/category. Some are fresh
base schemas, optional fixtures or compatibility references; the application
policy is part of the entry and must not be discarded. Do not concatenate and
run this list indiscriminately.

1. **clean-base — auth**

   `source-overlay/sql/base/auth.sql`

   Policy: Fresh empty schema only; never overwrite an existing auth/characters database

2. **clean-base — characters**

   `source-overlay/sql/base/characters.sql`

   Policy: Fresh empty schema only; never overwrite an existing auth/characters database

3. **compatibility-reference — auth additive candidate**

   `source-overlay/sql/compat/emucoach/auth_additive_candidate.sql`

   Policy: Optional reference candidate; not part of default migration order

4. **compatibility-reference — characters additive candidate**

   `source-overlay/sql/compat/emucoach/characters_additive_candidate.sql`

   Policy: Optional reference candidate; not part of default migration order

5. **compatibility-reference — world columns preserving**

   `source-overlay/sql/compat/emucoach/world_columns_preserving.sql`

   Policy: Optional reference candidate; not part of default migration order

6. **compatibility-reference — world quest preserving**

   `source-overlay/sql/compat/emucoach/world_quest_preserving.sql`

   Policy: Optional reference candidate; not part of default migration order

7. **compatibility-reference — auth**

   `source-overlay/sql/compat/mysql55/auth.sql`

   Policy: Optional reference candidate; not part of default migration order

8. **compatibility-reference — characters**

   `source-overlay/sql/compat/mysql55/characters.sql`

   Policy: Optional reference candidate; not part of default migration order

9. **auth — 2026 10 28 00 battlepay accounting**

   `source-overlay/sql/updates/auth/2026_10_28_00_battlepay_accounting.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

10. **auth — 2026 10 28 01 battlepay boost recovery**

   `source-overlay/sql/updates/auth/2026_10_28_01_battlepay_boost_recovery.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

11. **auth — 2026 10 28 02 playerbots account default**

   `source-overlay/sql/updates/auth/2026_10_28_02_playerbots_account_default.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

12. **auth — 2026 10 28 03 mop client build**

   `source-overlay/sql/updates/auth/2026_10_28_03_mop_client_build.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

13. **auth — 2026 11 04 00 companion command permission**

   `source-overlay/sql/updates/auth/2026_11_04_00_companion_command_permission.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

14. **characters — 2026 10 28 00 battlepay accounting**

   `source-overlay/sql/updates/characters/2026_10_28_00_battlepay_accounting.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

15. **characters — 2026 10 28 01 battlepay boost recovery**

   `source-overlay/sql/updates/characters/2026_10_28_01_battlepay_boost_recovery.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

16. **characters — 2026 11 07 02 characters dreaming glory action**

   `source-overlay/sql/updates/characters/2026_11_07_02_characters_dreaming_glory_action.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

17. **world — 2026 10 29 963w missing waypoint paths**

   `source-overlay/sql/updates/world/2026_10_29_963w_missing_waypoint_paths.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

18. **world — 2026 10 29 986 70 hb southshore purgation alterac**

   `source-overlay/sql/updates/world/2026_10_29_986_70_hb_southshore_purgation_alterac.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

19. **world — 2026 11 04 00 world restore flight movement**

   `source-overlay/sql/updates/world/2026_11_04_00_world_restore_flight_movement.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

20. **world — 2026 11 04 01 world creature text legacy columns**

   `source-overlay/sql/updates/world/2026_11_04_01_world_creature_text_legacy_columns.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

21. **world — 2026 11 04 02 world unused split pool**

   `source-overlay/sql/updates/world/2026_11_04_02_world_unused_split_pool.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

22. **world — 2026 11 04 03 world unused condition values**

   `source-overlay/sql/updates/world/2026_11_04_03_world_unused_condition_values.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

23. **world — 2026 11 04 04 world unused pool components**

   `source-overlay/sql/updates/world/2026_11_04_04_world_unused_pool_components.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

24. **world — 2026 11 04 05 world retired access keys**

   `source-overlay/sql/updates/world/2026_11_04_05_world_retired_access_keys.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

25. **world — 2026 11 04 06 world retired generic binding**

   `source-overlay/sql/updates/world/2026_11_04_06_world_retired_generic_binding.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

26. **world — 2026 11 04 07 world empty pool candidates**

   `source-overlay/sql/updates/world/2026_11_04_07_world_empty_pool_candidates.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

27. **world — 2026 11 04 08 world reference loot equal groups**

   `source-overlay/sql/updates/world/2026_11_04_08_world_reference_loot_equal_groups.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

28. **world — Restore authored legacy broadcast text and locales without overwriting existing text**

   `source-overlay/sql/updates/world/2026_11_04_09_world_legacy_script_broadcast_text.sql`

   Policy: Stop worldserver; back up affected tables (MyISAM may not roll back). Apply with paired native build; preserve account and character schemas.

29. **world — 2026 11 04 10 world registered script aliases**

   `source-overlay/sql/updates/world/2026_11_04_10_world_registered_script_aliases.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

30. **world — 2026 11 04 11 world missing classic loot references**

   `source-overlay/sql/updates/world/2026_11_04_11_world_missing_classic_loot_references.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

31. **world — 2026 11 04 12 world obsolete spell bindings**

   `source-overlay/sql/updates/world/2026_11_04_12_world_obsolete_spell_bindings.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

32. **world — 2026 11 04 13 world invalid gameobject locks**

   `source-overlay/sql/updates/world/2026_11_04_13_world_invalid_gameobject_locks.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

33. **world — 2026 11 04 14 world sourced quest talks**

   `source-overlay/sql/updates/world/2026_11_04_14_world_sourced_quest_talks.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

34. **world — 2026 11 04 15 world original loa talks**

   `source-overlay/sql/updates/world/2026_11_04_15_world_original_loa_talks.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

35. **world — 2026 11 04 16 world deathknell aura requirements**

   `source-overlay/sql/updates/world/2026_11_04_16_world_deathknell_aura_requirements.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

36. **world — 2026 11 04 17 world quest area state masks**

   `source-overlay/sql/updates/world/2026_11_04_17_world_quest_area_state_masks.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

37. **world — 2026 11 04 18 world original loa gameobject fields**

   `source-overlay/sql/updates/world/2026_11_04_18_world_original_loa_gameobject_fields.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

38. **world — 2026 11 04 19 world original loa runtime talks**

   `source-overlay/sql/updates/world/2026_11_04_19_world_original_loa_runtime_talks.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

39. **world — 2026 11 05 01 world verified spell bindings**

   `source-overlay/sql/updates/world/2026_11_05_01_world_verified_spell_bindings.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

40. **world — 2026 11 05 02 world triggered spell roles**

   `source-overlay/sql/updates/world/2026_11_05_02_world_triggered_spell_roles.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

41. **world — 2026 11 05 03 world authored smartai bindings**

   `source-overlay/sql/updates/world/2026_11_05_03_world_authored_smartai_bindings.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

42. **world — 2026 11 05 04 world unused creature flag**

   `source-overlay/sql/updates/world/2026_11_05_04_world_unused_creature_flag.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

43. **world — 2026 11 05 05 world retired criteria data**

   `source-overlay/sql/updates/world/2026_11_05_05_world_retired_criteria_data.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

44. **world — 2026 11 05 06 world customer greeting players**

   `source-overlay/sql/updates/world/2026_11_05_06_world_customer_greeting_players.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

45. **world — 2026 11 05 07 world taskmaster dialogue recipient**

   `source-overlay/sql/updates/world/2026_11_05_07_world_taskmaster_dialogue_recipient.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

46. **world — 2026 11 05 08 world class spell roles**

   `source-overlay/sql/updates/world/2026_11_05_08_world_class_spell_roles.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

47. **world — 2026 11 05 09 world encounter spell roles**

   `source-overlay/sql/updates/world/2026_11_05_09_world_encounter_spell_roles.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

48. **world — 2026 11 05 10 world marksman scorpid**

   `source-overlay/sql/updates/world/2026_11_05_10_world_marksman_scorpid.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

49. **world — 2026 11 05 11 world missing core messages**

   `source-overlay/sql/updates/world/2026_11_05_11_world_missing_core_messages.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

50. **world — 2026 11 05 12 world combat dialogue speakers**

   `source-overlay/sql/updates/world/2026_11_05_12_world_combat_dialogue_speakers.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

51. **world — 2026 11 05 13 world restore gameobject loot**

   `source-overlay/sql/updates/world/2026_11_05_13_world_restore_gameobject_loot.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

52. **world — 2026 11 05 14 world legacy smart actions**

   `source-overlay/sql/updates/world/2026_11_05_14_world_legacy_smart_actions.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

53. **world — 2026 11 05 15 world invalid script sentinels**

   `source-overlay/sql/updates/world/2026_11_05_15_world_invalid_script_sentinels.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

54. **world — 2026 11 05 16 world spawn difficulty masks**

   `source-overlay/sql/updates/world/2026_11_05_16_world_spawn_difficulty_masks.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

55. **world — 2026 11 05 17 world disabled spawn respawn links**

   `source-overlay/sql/updates/world/2026_11_05_17_world_disabled_spawn_respawn_links.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

56. **world — 2026 11 05 18 world missing quest poi points**

   `source-overlay/sql/updates/world/2026_11_05_18_world_missing_quest_poi_points.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

57. **world — 2026 11 05 19 world missing property pools**

   `source-overlay/sql/updates/world/2026_11_05_19_world_missing_property_pools.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

58. **world — 2026 11 05 20 world glyph and parachute bindings**

   `source-overlay/sql/updates/world/2026_11_05_20_world_glyph_and_parachute_bindings.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

59. **world — 2026 11 05 21 world equipment and enrage**

   `source-overlay/sql/updates/world/2026_11_05_21_world_equipment_and_enrage.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

60. **world — 2026 11 05 22 world property pool contamination**

   `source-overlay/sql/updates/world/2026_11_05_22_world_property_pool_contamination.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

61. **world — 2026 11 05 23 world duplicate class bindings**

   `source-overlay/sql/updates/world/2026_11_05_23_world_duplicate_class_bindings.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

62. **world — 2026 11 05 24 world jaomin duel**

   `source-overlay/sql/updates/world/2026_11_05_24_world_jaomin_duel.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

63. **world — 2026 11 05 25 world shang xi phase boundary**

   `source-overlay/sql/updates/world/2026_11_05_25_world_shang_xi_phase_boundary.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

64. **world — 2026 11 05 26 world stair destiny flight options**

   `source-overlay/sql/updates/world/2026_11_05_26_world_stair_destiny_flight_options.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

65. **world — 2026 11 05 27 world starting training**

   `source-overlay/sql/updates/world/2026_11_05_27_world_starting_training.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

66. **world — 2026 11 05 28 world existing quest ports**

   `source-overlay/sql/updates/world/2026_11_05_28_world_existing_quest_ports.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

67. **world — 2026 11 05 29 world training chain links**

   `source-overlay/sql/updates/world/2026_11_05_29_world_training_chain_links.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

68. **world — 2026 11 05 30 world gilneas progression**

   `source-overlay/sql/updates/world/2026_11_05_30_world_gilneas_progression.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

69. **world — 2026 11 05 31 world gilneas backend alignment**

   `source-overlay/sql/updates/world/2026_11_05_31_world_gilneas_backend_alignment.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

70. **world — 2026 11 05 32 world crowley ride**

   `source-overlay/sql/updates/world/2026_11_05_32_world_crowley_ride.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

71. **world — 2026 11 06 33 world release26 quests ai**

   `source-overlay/sql/updates/world/2026_11_06_33_world_release26_quests_ai.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

72. **world — 2026 11 06 34 world bloodfang duplicates**

   `source-overlay/sql/updates/world/2026_11_06_34_world_bloodfang_duplicates.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

73. **world — 2026 11 06 35 world release26 backend alignment**

   `source-overlay/sql/updates/world/2026_11_06_35_world_release26_backend_alignment.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

74. **world — 2026 11 06 36 world release26 loot services**

   `source-overlay/sql/updates/world/2026_11_06_36_world_release26_loot_services.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

75. **world — 2026 11 06 37 world release26 class spells**

   `source-overlay/sql/updates/world/2026_11_06_37_world_release26_class_spells.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

76. **world — 2026 11 06 38 world release26 loot references**

   `source-overlay/sql/updates/world/2026_11_06_38_world_release26_loot_references.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

77. **world — 2026 11 06 39 world release26 spare parts alignment**

   `source-overlay/sql/updates/world/2026_11_06_39_world_release26_spare_parts_alignment.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

78. **world — 2026 11 06 40 world release27 combat**

   `source-overlay/sql/updates/world/2026_11_06_40_world_release27_combat.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

79. **world — 2026 11 06 41 world release27 services**

   `source-overlay/sql/updates/world/2026_11_06_41_world_release27_services.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

80. **world — 2026 11 06 42 world release27 quests loot travel**

   `source-overlay/sql/updates/world/2026_11_06_42_world_release27_quests_loot_travel.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

81. **world — 2026 11 06 43 world dragonmaw loot ownership**

   `source-overlay/sql/updates/world/2026_11_06_43_world_dragonmaw_loot_ownership.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

82. **world — 2026 11 06 44 world sf27 native spells**

   `source-overlay/sql/updates/world/2026_11_06_44_world_sf27_native_spells.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

83. **world — 2026 11 06 45 world sf27 interactions**

   `source-overlay/sql/updates/world/2026_11_06_45_world_sf27_interactions.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

84. **world — 2026 11 06 46 world sf27 typed loot credit spawns**

   `source-overlay/sql/updates/world/2026_11_06_46_world_sf27_typed_loot_credit_spawns.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

85. **world — 2026 11 06 47 world sf27 detection range**

   `source-overlay/sql/updates/world/2026_11_06_47_world_sf27_detection_range.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

86. **world — 2026 11 06 48 world sf27 remaining content**

   `source-overlay/sql/updates/world/2026_11_06_48_world_sf27_remaining_content.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

87. **world — 2026 11 06 49 world sf27 authored damage**

   `source-overlay/sql/updates/world/2026_11_06_49_world_sf27_authored_damage.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

88. **world — 2026 11 06 50 world sf27 vehicle pool dependencies**

   `source-overlay/sql/updates/world/2026_11_06_50_world_sf27_vehicle_pool_dependencies.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

89. **world — 2026 11 07 51 world dreaming glory starter action**

   `source-overlay/sql/updates/world/2026_11_07_51_world_dreaming_glory_starter_action.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

90. **world — 2026 11 07 52 world loa beta starters**

   `source-overlay/sql/updates/world/2026_11_07_52_world_loa_beta_starters.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

91. **world — 2026 11 07 53 world loa beta schema**

   `source-overlay/sql/updates/world/2026_11_07_53_world_loa_beta_schema.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

92. **world — 2026 11 07 54 world loa legacy aliases**

   `source-overlay/sql/updates/world/2026_11_07_54_world_loa_legacy_aliases.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

93. **world — 2026 11 07 55 world loa locale preservation**

   `source-overlay/sql/updates/world/2026_11_07_55_world_loa_locale_preservation.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

94. **world — 2026 11 08 00 world loa missing dialogue**

   `source-overlay/sql/updates/world/2026_11_08_00_world_loa_missing_dialogue.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

95. **world — 2026 11 08 01 world loa script dialogue**

   `source-overlay/sql/updates/world/2026_11_08_01_world_loa_script_dialogue.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

96. **world — 2026 11 08 02 world loa runtime bindings**

   `source-overlay/sql/updates/world/2026_11_08_02_world_loa_runtime_bindings.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

97. **world — 2026 11 08 03 world loa qa references**

   `source-overlay/sql/updates/world/2026_11_08_03_world_loa_qa_references.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

98. **world — 2026 11 08 04 world loa ai fallback batch**

   `source-overlay/sql/updates/world/2026_11_08_04_world_loa_ai_fallback_batch.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

99. **world — 2026 11 08 05 world loa legendary native**

   `source-overlay/sql/updates/world/2026_11_08_05_world_loa_legendary_native.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

100. **world — 2026 11 08 06 world loa invalid animation tiers**

   `source-overlay/sql/updates/world/2026_11_08_06_world_loa_invalid_animation_tiers.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

101. **world — 2026 11 08 07 world loa gilneas prison event**

   `source-overlay/sql/updates/world/2026_11_08_07_world_loa_gilneas_prison_event.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

102. **world — 2026 11 08 08 world loa anzu brood**

   `source-overlay/sql/updates/world/2026_11_08_08_world_loa_anzu_brood.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

103. **world — 2026 11 08 09 world loa sf interaction reconciliation**

   `source-overlay/sql/updates/world/2026_11_08_09_world_loa_sf_interaction_reconciliation.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

104. **world — 2026 11 09 00 world loa compat alias triggers**

   `source-overlay/sql/updates/world/2026_11_09_00_world_loa_compat_alias_triggers.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

105. **world — 2026 11 09 01 world historical stranglethorn tele**

   `source-overlay/sql/updates/world/2026_11_09_01_world_historical_stranglethorn_tele.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

106. **world — 2026 11 09 02 world historical quest fields**

   `source-overlay/sql/updates/world/2026_11_09_02_world_historical_quest_fields.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

107. **world — 2026 11 09 03 world signed next quest links**

   `source-overlay/sql/updates/world/2026_11_09_03_world_signed_next_quest_links.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

108. **world — 2026 11 09 04 world historical quest compatibility**

   `source-overlay/sql/updates/world/2026_11_09_04_world_historical_quest_compatibility.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

109. **world — 2026 11 09 05 world gilneas terrain and cage**

   `source-overlay/sql/updates/world/2026_11_09_05_world_gilneas_terrain_and_cage.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

110. **world — 2026 11 09 06 world gilneas progression wiring**

   `source-overlay/sql/updates/world/2026_11_09_06_world_gilneas_progression_wiring.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

111. **world — 2026 11 09 07 world halga dialogue groups**

   `source-overlay/sql/updates/world/2026_11_09_07_world_halga_dialogue_groups.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

112. **world — 2026 11 09 08 world verified script aliases**

   `source-overlay/sql/updates/world/2026_11_09_08_world_verified_script_aliases.sql`

   Policy: Review matched schema/native prerequisites; not a blanket replay command

113. **world — Preserve legacy zero-ID equipment variants for the current loader**

   `source-overlay/sql/updates/world/2026_11_11_00_world_original_mop_compat.sql`

   Policy: Existing legacy world only; stop worldserver and back up equipment first; re-runnable

114. **world — Gilneas quest artillery and friendly inspiration; short reusable artillery respawn range**

   `source-overlay/sql/updates/world/2026_11_11_01_world_gilneas_battle_spells.sql`

   Policy: Pair with current native spell scripts; back up bindings, conditions and affected spawn rows

115. **world — Original-world late Gilneas phase boundaries and quest-NPC handoffs**

   `source-overlay/sql/updates/world/2026_11_11_02_world_gilneas_battle_phase_isolation.sql`

   Policy: Original world with phase_definitions entries 8..13 and matching quest milestones only; back up first

116. **world — Restore missing battle dialogue while preserving existing lines**

   `source-overlay/sql/updates/world/2026_11_11_03_world_gilneas_battle_dialogue.sql`

   Policy: Review native dialogue group IDs; inserts only missing rows

117. **world — Place Gorerot in the courtyard at the published battle spawn position**

   `source-overlay/sql/updates/world/2026_11_11_04_world_gorerot_ground_spawn.sql`

   Policy: Stop worldserver; back up map 654 entry 38331 spawn positions; apply and restart; re-runnable

118. **world — Shorten persistent Gorerot respawn to 60 seconds**

   `source-overlay/sql/updates/world/2026_11_11_05_world_gorerot_respawn.sql`

   Policy: Back up spawn; restart worldserver. Existing saved respawn waits need targeted reset; see realm-gilneas-cleanup notes

119. **world — Bind explicit Gorerot combat AI instead of passive vehicle AI**

   `source-overlay/sql/updates/world/2026_11_11_06_world_gorerot_combat.sql`

   Policy: Requires paired current native script and a worldserver restart; preserve vehicle kit and character progress

120. **world — Preserve post-battle city quest chain and bridge Krennan turn-in**

   `source-overlay/sql/updates/world/2026_11_11_07_world_gilneas_post_battle_handoff.sql`

   Policy: Apply after migration 02; back up phase conditions and Krennan spawns; restart worldserver

121. **world — Enter bombing phase on acceptance and preserve adjacent quest handoffs**

   `source-overlay/sql/updates/world/2026_11_11_08_world_gilneas_bombing_phase.sql`

   Policy: Apply after migrations 02/07; back up scoped data, stop worldserver, apply, restart. No character quest changes

122. **world — Restore personal bombing bat and passenger-triggered flight/return**

   `source-overlay/sql/updates/world/2026_11_11_09_world_captured_bat_ride.sql`

   Policy: Apply after migrations 02/07; back up scoped data, stop worldserver, apply, restart. No character quest changes

123. **world — Restore torch response and correct XT002 hook; fill missing SmartAI fallback bindings**

   `source-overlay/sql/updates/world/2026_11_11_10_world_torch_and_script_compat.sql`

   Policy: Stop worldserver; back up affected tables (MyISAM may not roll back). Apply with paired native build; preserve account and character schemas.

124. **world — Restore68 complete missing matched LoA reference groups,136 rows**

   `source-overlay/sql/updates/world/2026_11_11_11_world_missing_mop_loot_references.sql`

   Policy: Stop worldserver; back up affected tables (MyISAM may not roll back). Apply with paired native build; preserve account and character schemas.

125. **world — Remove unreferenced empty pools and unsupported command/invalid emote metadata**

   `source-overlay/sql/updates/world/2026_11_11_12_world_obsolete_runtime_metadata.sql`

   Policy: Stop worldserver; back up affected tables (MyISAM may not roll back). Apply with paired native build; preserve account and character schemas.

126. **world — Restore controllable finale vehicles, bounded native click handling and missing ship scene**

   `source-overlay/sql/updates/world/2026_11_12_00_world_gilneas_finale_vehicles_scene.sql`

   Policy: Stop worldserver; back up affected tables (MyISAM may not roll back). Apply with paired native build; preserve account and character schemas.

127. **world — Additive static gunship template for finale scene; preserve existing authored template**

   `source-overlay/sql/updates/world/2026_11_12_01_world_endgame_ship_template.sql`

   Policy: Apply with November12 migration00 before starting the paired native build; back up affected tables.

128. **world — Level12 playable glaive durability, repeatable quest encounter respawns and missing Marie Allen inventories**

   `source-overlay/sql/updates/world/2026_11_12_02_world_keel_harbor_encounter_vendor.sql`

   Policy: Stop worldserver; back up creature/template/vendor tables and pending respawns. Apply with existing finale native build; restart. See follow-up notes to cap already-pending encounter waits.

129. **world — Correct WMO map-object type from client crash evidence and scripted flight health**

   `source-overlay/sql/updates/world/2026_11_12_03_world_gunship_wmo_client_safety.sql`

   Policy: Back up scoped templates/transports, stop worldserver, apply after earlier gunship migrations, deploy paired native rope repair, restart. Keep map-object type14; do not reapply the old generic ship definition afterward.

130. **world — Disable obsolete overlapping moving gunship; pair with safe detached rope/native dismounts**

   `source-overlay/sql/updates/world/2026_11_12_04_world_endgame_single_ship.sql`

   Policy: Back up scoped templates/transports, stop worldserver, apply after earlier gunship migrations, deploy paired native rope repair, restart. Keep map-object type14; do not reapply the old generic ship definition afterward.

131. **world — Restore Nightwind bind/teleport departure, valid addon emotes and linked SmartAI events**

   `source-overlay/sql/updates/world/2026_11_12_05_world_departure_emotes_smartlinks.sql`

   Policy: Back up scoped static rows, apply after SQL04, deploy paired Nightwind native script and restart worldserver. No accounts or characters included; individual requested quest reset is excluded.

132. **world — Restore absent Genn Howling Oak questgiver and Tarien jewelcrafting stock**

   `source-overlay/sql/updates/world/2026_11_12_06_world_howling_oak_and_tarien.sql`

   Policy: Back up scoped world rows; apply SQL06 then restart worldserver for the added spawn. Keeps existing map1 Genn and custom Tarien stock. No player quest edits or native rebuild required.

133. **world — Restore Idriana deposit-box gossip banker action**

   `source-overlay/sql/updates/world/2026_11_12_07_world_idriana_bank_gossip.sql`

   Policy: Back up menu2404 option0, apply SQL07, then reload gossip_menu_option. No native rebuild or restart needed; no bank items or player data changed.

134. **characters — Curated candidate pool, ownership/provenance and reversible rename journal schema**

   `source-overlay/sql/updates/characters/2026_11_12_00_characters_curated_playerbot_names.sql`

   Policy: Apply before opt-in curated naming; no live identity data

135. **characters-data — 20000 public candidate names and hint metadata only**

   `source-overlay/modules/mod_playerbots/data/sql/characters/curated_name_candidates.sql`

   Policy: Alternative CSV import; use manage_roster import for existing roster provenance

136. **world — Map962 controller, placeholder templates and80 missing authored objects**

   `source-overlay/sql/updates/world/2026_11_12_08_world_setting_sun_controller_objects.sql`

   Policy: Back up scoped tables; restart worldserver; repeat-safe positioned spawns

137. **characters — Atomic voucher redemption receipt ledger**

   `source-overlay/sql/updates/characters/2026_11_12_01_characters_orb_redemption.sql`

   Policy: Review schema/native dependencies and backup; apply to the named schema only; see paired feature documentation

138. **world — Restore missing native dungeon area triggers**

   `source-overlay/sql/updates/world/2026_11_12_09_world_setting_sun_area_triggers.sql`

   Policy: Review schema/native dependencies and backup; apply to the named schema only; see paired feature documentation

139. **world — Legendary bound stack20 voucher, native binding and cache invalidation**

   `source-overlay/sql/updates/world/2026_11_12_10_world_orb_of_loyalty.sql`

   Policy: Review schema/native dependencies and backup; apply to the named schema only; see paired feature documentation

140. **world — Yak merchant stock and 5.4.8 reforger service**

   `source-overlay/sql/updates/world/2026_11_12_11_world_expedition_yak_services.sql`

   Policy: Review schema/native dependencies and backup; apply to the named schema only; see paired feature documentation

141. **playerbots — Repair known imported punctuation encoding damage**

   `source-overlay/modules/mod_playerbots/data/sql/playerbots/2026_11_12_00_chat_punctuation.sql`

   Policy: Review schema/native dependencies and backup; apply to the named schema only; see paired feature documentation

142. **playerbots — 10000 contextual English chat variants without higher event frequency**

   `source-overlay/modules/mod_playerbots/data/sql/playerbots/2026_11_12_01_ambient_chat_pack.sql`

   Policy: Review schema/native dependencies and backup; apply to the named schema only; see paired feature documentation

143. **world — Repair69 emote command modes and NPC action links**

   `source-overlay/sql/updates/world/2026_11_12_12_world_runtime_emote_links.sql`

   Policy: Back up first; pair with the native follow-up and target the named schema; see runtime-noise-and-social-chat-2026-11-12.md

144. **world — Restore48 authored dialogue candidates across31 groups; existing rows win**

   `source-overlay/sql/updates/world/2026_11_12_13_world_missing_authored_dialogue.sql`

   Policy: Back up first; pair with the native follow-up and target the named schema; see runtime-noise-and-social-chat-2026-11-12.md

145. **world — Restore four verified Gilneas graveyard links**

   `source-overlay/sql/updates/world/2026_11_12_14_world_gilneas_graveyards.sql`

   Policy: Back up first; pair with the native follow-up and target the named schema; see runtime-noise-and-social-chat-2026-11-12.md

146. **playerbots — Standalone general/party social text for revised native hooks**

   `source-overlay/modules/mod_playerbots/data/sql/playerbots/2026_11_12_02_standalone_social_chat.sql`

   Policy: Back up first; pair with the native follow-up and target the named schema; see runtime-noise-and-social-chat-2026-11-12.md

## Whole-file source and documentation inventory

This inventory describes the delivered overlay paths. It includes generated
reference/QA material as well as production code; an entry here is not an
independent assertion that its entire subsystem is perfect. Review the file
and its feature notes for the actual implementation and test scope.

### Build integration, data packs and supporting files

- `.gitattributes` — .gitattributes.
- `.gitignore` — .gitignore.
- `CMakeLists.txt` — CMakeLists.
- `CMakePresets.json` — CMakePresets.
- `CMakeSettings.json` — CMakeSettings.
- `IMPLEMENTATION_STATUS.md` — IMPLEMENTATION STATUS.
- `MoP_Playerbots_Crash_Fix_Handoff.md` — MoP Playerbots Crash Fix Handoff.
- `README.md` — README.
- `Run-QA.cmd` — Run QA.
- `Start-Server.cmd` — Start Server.
- `cmake/compiler/msvc/settings.cmake` — settings.
- `cmake/genrev.cmake` — genrev.
- `cmake/macros/FindMySQL.cmake` — FindMySQL.
- `modules/CMakeLists.txt` — CMakeLists.
- `modules/mod_playerbots/data/chat/ambient_en_10000.json` — ambient en 10000.
- `modules/mod_playerbots/data/chat/ambient_general_548.txt` — ambient general 548.
- `modules/mod_playerbots/data/chat/party_flavor_548.txt` — party flavor 548.
- `modules/mod_playerbots/data/names/README.md` — README.
- `modules/mod_playerbots/data/names/playerbot_names.csv` — playerbot names.
- `modules/mod_playerbots/data/names/validate_and_preview.py` — validate and preview.
- `src/genrev/CMakeLists.txt` — CMakeLists.
- `src/server/scripts/EasternKingdoms/CMakeLists.txt` — CMakeLists.
- `src/server/scripts/Kalimdor/CMakeLists.txt` — CMakeLists.
- `src/server/scripts/Pet/CMakeLists.txt` — CMakeLists.
- `src/server/scripts/Spells/CMakeLists.txt` — CMakeLists.
- `src/server/scripts/Spells/SKYFIRE27-LICENSE.md` — SKYFIRE27 LICENSE.

### Build, QA, import and operational tooling

- `contrib/auction-tests/CMakeLists.txt` — CMakeLists.
- `contrib/battlepay-tests/CMakeLists.txt` — CMakeLists.
- `contrib/battlepay-tests/Test-BattlePayDatabase.ps1` — Test BattlePayDatabase.
- `contrib/bot-chat/generate_chat_pack.py` — generate chat pack.
- `contrib/bot-chat/tests/CMakeLists.txt` — CMakeLists.
- `contrib/bot-chat/validate_chat_pack.py` — validate chat pack.
- `contrib/companion-tests/CMakeLists.txt` — CMakeLists.
- `contrib/content-audit/addon_auras.py` — addon auras.
- `contrib/content-audit/catalogue.py` — catalogue.
- `contrib/content-audit/compare_quest_audits.py` — compare quest audits.
- `contrib/content-audit/quest_dependencies.py` — quest dependencies.
- `contrib/content-audit/test_addon_auras.py` — test addon auras.
- `contrib/content-audit/test_catalogue.py` — test catalogue.
- `contrib/content-audit/test_compare_quest_audits.py` — test compare quest audits.
- `contrib/content-audit/test_quest_dependencies.py` — test quest dependencies.
- `contrib/database-tests/CMakeLists.txt` — CMakeLists.
- `contrib/database-tests/Check-GameObjectModelTypes.py` — Check GameObjectModelTypes.
- `contrib/database-tests/Prepare-PlayerbotsSchemaFixture.ps1` — Prepare PlayerbotsSchemaFixture.
- `contrib/database-tests/Test-AuthHandshake.ps1` — Test AuthHandshake.
- `contrib/database-tests/Test-WorldserverLease.py` — Test WorldserverLease.
- `contrib/database-tests/mysql55-test.ini.example` — mysql55 test.ini.
- `contrib/diagnostics/script_binding_inventory.py` — script binding inventory.
- `contrib/diagnostics/server_log_report.py` — server log report.
- `contrib/diagnostics/test_server_log_report.py` — test server log report.
- `contrib/headless-client/README.md` — README.
- `contrib/headless-client/auth_srp_regression.py` — auth srp regression.
- `contrib/headless-client/character_creation_regression.py` — character creation regression.
- `contrib/headless-client/companion_lifetime_regression.py` — companion lifetime regression.
- `contrib/headless-client/companion_party_regression.py` — companion party regression.
- `contrib/headless-client/companion_reply_regression.py` — companion reply regression.
- `contrib/headless-client/gilneas_escape_regression.py` — gilneas escape regression.
- `contrib/headless-client/gilneas_torch_regression.py` — gilneas torch regression.
- `contrib/headless-client/gilneas_vehicle_regression.py` — gilneas vehicle regression.
- `contrib/headless-client/gunship_rope_regression.py` — gunship rope regression.
- `contrib/headless-client/gunship_wire_regression.py` — gunship wire regression.
- `contrib/headless-client/headless.py` — headless.
- `contrib/headless-client/howling_oak_tarien_regression.py` — howling oak tarien regression.
- `contrib/headless-client/idriana_bank_regression.py` — idriana bank regression.
- `contrib/headless-client/keel_harbor_regression.py` — keel harbor regression.
- `contrib/headless-client/nightwind_departure_regression.py` — nightwind departure regression.
- `contrib/headless-client/orb_cache_regression.py` — orb cache regression.
- `contrib/headless-client/orb_yak_regression.py` — orb yak regression.
- `contrib/headless-client/qa-invalid-quest-scenarios.json` — qa invalid quest scenarios.
- `contrib/headless-client/qa-legendary-scenarios.json` — qa legendary scenarios.
- `contrib/headless-client/qa-scenarios.json` — qa scenarios.
- `contrib/headless-client/qa-workflow-scenarios.json` — qa workflow scenarios.
- `contrib/headless-client/qa_actions.py` — qa actions.
- `contrib/headless-client/qa_cleanup.py` — qa cleanup.
- `contrib/headless-client/qa_compare.py` — qa compare.
- `contrib/headless-client/qa_inventory.py` — qa inventory.
- `contrib/headless-client/qa_report.py` — qa report.
- `contrib/headless-client/qa_run.py` — qa run.
- `contrib/headless-client/qa_spec.py` — qa spec.
- `contrib/headless-client/runtime_spell_noise_regression.py` — runtime spell noise regression.
- `contrib/headless-client/setting_sun_regression.py` — setting sun regression.
- `contrib/headless-client/starter_campaign.py` — starter campaign.
- `contrib/headless-client/test_qa.py` — test qa.
- `contrib/headless-client/test_qa_actions.py` — test qa actions.
- `contrib/headless-client/test_qa_logout.py` — test qa logout.
- `contrib/headless-client/wyvern_camera_turnin_regression.py` — wyvern camera turnin regression.
- `contrib/headless-client/yak_services_regression.py` — yak services regression.
- `contrib/navigation-tests/CMakeLists.txt` — CMakeLists.
- `contrib/navigation/repair_missing_tiles.py` — repair missing tiles.
- `contrib/playerbot-names/Export-CleanPool.py` — Export CleanPool.
- `contrib/playerbot-names/Measure-Capacity.py` — Measure Capacity.
- `contrib/playerbot-names/README.md` — README.
- `contrib/playerbot-names/Test-NativeNames.py` — Test NativeNames.
- `contrib/playerbot-names/manage_roster.py` — manage roster.
- `contrib/qa-automation/availability.json` — availability.
- `contrib/qa-automation/backup-example.json` — backup example.
- `contrib/qa-automation/checkpoint.py` — checkpoint.
- `contrib/qa-automation/diagnostics.py` — diagnostics.
- `contrib/qa-automation/evidence.py` — evidence.
- `contrib/qa-automation/known-data-findings.json` — known data findings.
- `contrib/qa-automation/pipeline.py` — pipeline.
- `contrib/qa-automation/prioritize.py` — prioritize.
- `contrib/qa-automation/recover_accounts.py` — recover accounts.
- `contrib/qa-automation/required-gameplay.json` — required gameplay.
- `contrib/qa-automation/run_lock.py` — run lock.
- `contrib/qa-automation/test_automation.py` — test automation.
- `contrib/qa-automation/verify_account_recovery.py` — verify account recovery.
- `contrib/qa-automation/verify_checkpoint_restore.py` — verify checkpoint restore.
- `contrib/quest-qa/adventure.py` — adventure.
- `contrib/quest-qa/adventure_campaign.py` — adventure campaign.
- `contrib/quest-qa/adventure_notes.py` — adventure notes.
- `contrib/quest-qa/adventure_progress.py` — adventure progress.
- `contrib/quest-qa/campaign.py` — campaign.
- `contrib/quest-qa/catalogue_matrix.py` — catalogue matrix.
- `contrib/quest-qa/contracts.json` — contracts.
- `contrib/quest-qa/diagnose_pandaren_reward.py` — diagnose pandaren reward.
- `contrib/quest-qa/diagnose_visibility.py` — diagnose visibility.
- `contrib/quest-qa/environment.py` — environment.
- `contrib/quest-qa/pool.py` — pool.
- `contrib/quest-qa/prewarm.py` — prewarm.
- `contrib/quest-qa/profiles/persistent-playerbots.json` — persistent playerbots.
- `contrib/quest-qa/protocol.py` — protocol.
- `contrib/quest-qa/published_batch.py` — published batch.
- `contrib/quest-qa/reference_schema.py` — reference schema.
- `contrib/quest-qa/regular_runtime.py` — regular runtime.
- `contrib/quest-qa/report.py` — report.
- `contrib/quest-qa/run_delivery.py` — run delivery.
- `contrib/quest-qa/run_simple.py` — run simple.
- `contrib/quest-qa/runtime_diagnostics.py` — runtime diagnostics.
- `contrib/quest-qa/test_adventure_notes.py` — test adventure notes.
- `contrib/quest-qa/test_adventure_progress.py` — test adventure progress.
- `contrib/quest-qa/test_campaign.py` — test campaign.
- `contrib/quest-qa/test_cleanup_live.py` — test cleanup live.
- `contrib/quest-qa/test_infrastructure.py` — test infrastructure.
- `contrib/quest-qa/test_migration.py` — test migration.
- `contrib/quest-qa/test_runtime_diagnostics.py` — test runtime diagnostics.
- `contrib/quest-qa/trace_sacrifices.py` — trace sacrifices.
- `contrib/server-cli-tests/CMakeLists.txt` — CMakeLists.
- `contrib/upstream-tests/README.md` — README.
- `contrib/upstream-tests/assemble_gilneas_batch.py` — assemble gilneas batch.
- `contrib/upstream-tests/assemble_historical_quest_fields.py` — assemble historical quest fields.
- `contrib/upstream-tests/assemble_release26_quest_batch.py` — assemble release26 quest batch.
- `contrib/upstream-tests/assemble_release26_services_batch.py` — assemble release26 services batch.
- `contrib/upstream-tests/assemble_release27_combat.py` — assemble release27 combat.
- `contrib/upstream-tests/assemble_release27_world_batch.py` — assemble release27 world batch.
- `contrib/upstream-tests/assemble_sf27_content.py` — assemble sf27 content.
- `contrib/upstream-tests/assemble_sf27_interactions.py` — assemble sf27 interactions.
- `contrib/upstream-tests/assemble_sf27_native_spells.py` — assemble sf27 native spells.
- `contrib/upstream-tests/assemble_sf27_typed_loot.py` — assemble sf27 typed loot.
- `contrib/upstream-tests/audit_active_rollup.py` — audit active rollup.
- `contrib/upstream-tests/audit_content_catalog.py` — audit content catalog.
- `contrib/upstream-tests/audit_remaining_quest_inputs.py` — audit remaining quest inputs.
- `contrib/upstream-tests/audit_world_patches.py` — audit world patches.
- `contrib/upstream-tests/batches/gilneas-20261105.json` — gilneas 20261105.
- `contrib/upstream-tests/deploy_historical_quest_fields.py` — deploy historical quest fields.
- `contrib/upstream-tests/extract_release26_reference_dependencies.py` — extract release26 reference dependencies.
- `contrib/upstream-tests/finalize_historical_quest_compatibility.py` — finalize historical quest compatibility.
- `contrib/upstream-tests/finalize_sf27_spell_owners.py` — finalize sf27 spell owners.
- `contrib/upstream-tests/fixtures/shiftp-18414-creature-queries.json` — shiftp 18414 creature queries.
- `contrib/upstream-tests/inventory_full_history.py` — inventory full history.
- `contrib/upstream-tests/port_skyfire_spell_module.py` — port skyfire spell module.
- `contrib/upstream-tests/reconcile_sf27_inventory.py` — reconcile sf27 inventory.
- `contrib/upstream-tests/rollback/2026_11_05_27_world_starting_training.sql` — 2026 11 05 27 world starting training.
- `contrib/upstream-tests/rollback/2026_11_05_28_29_world_ports_chains.sql` — 2026 11 05 28 29 world ports chains.
- `contrib/upstream-tests/run_console_world_event.py` — run console world event.
- `contrib/upstream-tests/run_world_contracts.py` — run world contracts.
- `contrib/upstream-tests/sf27_db.py` — sf27 db.
- `contrib/upstream-tests/skyfire_spell_api.py` — skyfire spell api.
- `contrib/upstream-tests/sql_batch.py` — sql batch.
- `contrib/upstream-tests/suites/crowley-ride.json` — crowley ride.
- `contrib/upstream-tests/suites/existing-quest-ports.json` — existing quest ports.
- `contrib/upstream-tests/suites/gilneas-progression.json` — gilneas progression.
- `contrib/upstream-tests/suites/loa-ai-fallback-batch.json` — loa ai fallback batch.
- `contrib/upstream-tests/suites/loa-ef5112acc3.json` — loa ef5112acc3.
- `contrib/upstream-tests/suites/loa-qa-reference-repairs.json` — loa qa reference repairs.
- `contrib/upstream-tests/suites/loa-runtime-bindings.json` — loa runtime bindings.
- `contrib/upstream-tests/suites/release26-quest-fields.json` — release26 quest fields.
- `contrib/upstream-tests/suites/release26-quests.json` — release26 quests.
- `contrib/upstream-tests/suites/release26-services.json` — release26 services.
- `contrib/upstream-tests/suites/release27-combat.json` — release27 combat.
- `contrib/upstream-tests/suites/release27-services.json` — release27 services.
- `contrib/upstream-tests/suites/release27-world.json` — release27 world.
- `contrib/upstream-tests/suites/starting-training.json` — starting training.
- `contrib/upstream-tests/suites/training-chain-links.json` — training chain links.
- `contrib/upstream-tests/test_anduin_decision_live.py` — test anduin decision live.
- `contrib/upstream-tests/test_collections_burst_live.py` — test collections burst live.
- `contrib/upstream-tests/test_finalize_historical_quests.py` — test finalize historical quests.
- `contrib/upstream-tests/test_gilneas_cage_live.py` — test gilneas cage live.
- `contrib/upstream-tests/test_gilneas_followup_live.py` — test gilneas followup live.
- `contrib/upstream-tests/test_gilneas_migration.py` — test gilneas migration.
- `contrib/upstream-tests/test_gilneas_ride_live.py` — test gilneas ride live.
- `contrib/upstream-tests/test_gilneas_wave_live.py` — test gilneas wave live.
- `contrib/upstream-tests/test_gossip_hotfix_live.py` — test gossip hotfix live.
- `contrib/upstream-tests/test_historical_quest_fields.py` — test historical quest fields.
- `contrib/upstream-tests/test_inventory_full_history.py` — test inventory full history.
- `contrib/upstream-tests/test_numeric_update_batch.py` — test numeric update batch.
- `contrib/upstream-tests/test_numeric_world_migration.py` — test numeric world migration.
- `contrib/upstream-tests/test_release26_quest_batch.py` — test release26 quest batch.
- `contrib/upstream-tests/test_release26_services_batch.py` — test release26 services batch.
- `contrib/upstream-tests/test_release27_combat.py` — test release27 combat.
- `contrib/upstream-tests/test_release27_world_batch.py` — test release27 world batch.
- `contrib/upstream-tests/test_sf27_final_batch.py` — test sf27 final batch.
- `contrib/upstream-tests/test_sf27_final_dependencies.py` — test sf27 final dependencies.
- `contrib/upstream-tests/test_sf27_interactions.py` — test sf27 interactions.
- `contrib/upstream-tests/test_sf27_native_spells.py` — test sf27 native spells.
- `contrib/upstream-tests/test_sf27_typed_loot.py` — test sf27 typed loot.
- `contrib/upstream-tests/test_shattrath_vendor_live.py` — test shattrath vendor live.
- `contrib/upstream-tests/test_sql_batch.py` — test sql batch.
- `contrib/upstream-tests/test_worgen_consumables_live.py` — test worgen consumables live.
- `contrib/upstream-tests/test_worgen_two_forms_live.py` — test worgen two forms live.
- `contrib/upstream-tests/verify_legendary_dependencies.py` — verify legendary dependencies.
- `contrib/vmap-tools/convert_legacy_dtree.py` — convert legacy dtree.
- `contrib/windows/Configure-DiagnosticLogs.ps1` — Configure DiagnosticLogs.
- `contrib/windows/Patch-PlayersbotsAddon.ps1` — Patch PlayersbotsAddon.
- `contrib/windows/Run-GameplayQA.ps1` — Run GameplayQA.
- `contrib/windows/Run-QualityChecks.ps1` — Run QualityChecks.
- `contrib/windows/Run-QuestDependencyChecks.ps1` — Run QuestDependencyChecks.
- `contrib/windows/Run-QuestQA.ps1` — Run QuestQA.
- `contrib/windows/Send-ServerConsoleCommand.py` — Send ServerConsoleCommand.
- `contrib/windows/Stage-Runtime.ps1` — Stage Runtime.
- `contrib/windows/Start-Authserver.cmd` — Start Authserver.
- `contrib/windows/Start-TestServer.ps1` — Start TestServer.
- `contrib/windows/Start-Worldserver.cmd` — Start Worldserver.

### Native code and headers

- `contrib/auction-tests/auction_item_filter_tests.cpp` — auction item filter tests.
- `contrib/battlepay-tests/battlepay_accounting_tests.cpp` — battlepay accounting tests.
- `contrib/bot-chat/tests/throttle_test.cpp` — throttle test.
- `contrib/companion-tests/command_resolution_tests.cpp` — command resolution tests.
- `contrib/companion-tests/companion_policy_tests.cpp` — companion policy tests.
- `contrib/companion-tests/creator_object_refs_tests.cpp` — creator object refs tests.
- `contrib/companion-tests/creature_combat_policy_tests.cpp` — creature combat policy tests.
- `contrib/companion-tests/creature_detection_tests.cpp` — creature detection tests.
- `contrib/companion-tests/game_event_schedule_tests.cpp` — game event schedule tests.
- `contrib/companion-tests/gilneas_ride_safety_tests.cpp` — gilneas ride safety tests.
- `contrib/companion-tests/gossip_broadcast_id_tests.cpp` — gossip broadcast id tests.
- `contrib/companion-tests/immediate_event_tests.cpp` — immediate event tests.
- `contrib/companion-tests/jaomin_duel_tests.cpp` — jaomin duel tests.
- `contrib/companion-tests/movement_input_tests.cpp` — movement input tests.
- `contrib/companion-tests/navigation_grid_input_tests.cpp` — navigation grid input tests.
- `contrib/companion-tests/new_character_policy_tests.cpp` — new character policy tests.
- `contrib/companion-tests/quest_area_trigger_tests.cpp` — quest area trigger tests.
- `contrib/companion-tests/quest_dependency_tests.cpp` — quest dependency tests.
- `contrib/companion-tests/random_enchantment_tests.cpp` — random enchantment tests.
- `contrib/database-tests/database_connector_probe.cpp` — database connector probe.
- `contrib/database-tests/database_transaction_probe.cpp` — database transaction probe.
- `contrib/database-tests/database_version_tests.cpp` — database version tests.
- `contrib/database-tests/field_width_tests.cpp` — field width tests.
- `contrib/database-tests/mmap_load_probe.cpp` — mmap load probe.
- `contrib/navigation-tests/detail_edge_tests.cpp` — detail edge tests.
- `dep/SFMT/SFMT.h` — SFMT.
- `dep/recastnavigation/Detour/Source/DetourNavMesh.cpp` — DetourNavMesh.
- `modules/mod_playerbots/src/AI/PlayerbotAI.cpp` — PlayerbotAI.
- `modules/mod_playerbots/src/AI/PlayerbotAI.h` — PlayerbotAI.
- `modules/mod_playerbots/src/Factory/RandomPlayerbotFactory.cpp` — RandomPlayerbotFactory.
- `modules/mod_playerbots/src/Manager/CompanionAccess.h` — CompanionAccess.
- `modules/mod_playerbots/src/Manager/CompanionPolicy.h` — CompanionPolicy.
- `modules/mod_playerbots/src/Manager/PlayerbotMgr.cpp` — PlayerbotMgr.
- `modules/mod_playerbots/src/Manager/PlayerbotSecurity.cpp` — PlayerbotSecurity.
- `modules/mod_playerbots/src/Manager/PlayersbotsAddon.cpp` — PlayersbotsAddon.
- `modules/mod_playerbots/src/Manager/PlayersbotsAddon.h` — PlayersbotsAddon.
- `modules/mod_playerbots/src/Manager/RandomItemManager.cpp` — RandomItemManager.
- `modules/mod_playerbots/src/Manager/RandomPlayerbotMgr.cpp` — RandomPlayerbotMgr.
- `modules/mod_playerbots/src/Mgr/Text/PlayerbotTextMgr.cpp` — PlayerbotTextMgr.
- `modules/mod_playerbots/src/Scenario/BotScenario.cpp` — BotScenario.
- `modules/mod_playerbots/src/Utils/BroadcastHelper.cpp` — BroadcastHelper.
- `modules/mod_playerbots/src/Utils/PublicChatThrottle.h` — PublicChatThrottle.
- `modules/mod_playerbots/src/cs_playerbots.cpp` — cs playerbots.
- `modules/mod_playerbots/src/strategy/RPG/NewRpgAction.cpp` — NewRpgAction.
- `modules/mod_playerbots/src/strategy/RPG/NewRpgBaseAction.cpp` — NewRpgBaseAction.
- `modules/mod_playerbots/src/strategy/RPG/NewRpgInfo.h` — NewRpgInfo.
- `modules/mod_playerbots/src/strategy/actions/AcceptInvitationAction.cpp` — AcceptInvitationAction.
- `modules/mod_playerbots/src/strategy/actions/AcceptQuestAction.cpp` — AcceptQuestAction.
- `modules/mod_playerbots/src/strategy/actions/InventoryAction.cpp` — InventoryAction.
- `modules/mod_playerbots/src/strategy/actions/LeaveGroupAction.cpp` — LeaveGroupAction.
- `modules/mod_playerbots/src/strategy/actions/LootAction.cpp` — LootAction.
- `modules/mod_playerbots/src/strategy/actions/LootRollAction.cpp` — LootRollAction.
- `modules/mod_playerbots/src/strategy/actions/SuggestWhatToDoAction.cpp` — SuggestWhatToDoAction.
- `modules/mod_playerbots/src/strategy/generic/NonCombatStrategy.cpp` — NonCombatStrategy.
- `modules/mod_playerbots/src/strategy/generic/NonCombatStrategy.h` — NonCombatStrategy.
- `modules/mod_playerbots/src/strategy/value/MaintenanceValues.cpp` — MaintenanceValues.
- `src/common/Collision/Management/MMapManager.cpp` — MMapManager.
- `src/common/Collision/Management/NavigationGridInput.h` — NavigationGridInput.
- `src/common/Cryptography/BigNumber.cpp` — BigNumber.
- `src/common/Utilities/TaskScheduler.h` — TaskScheduler.
- `src/server/authserver/Main.cpp` — Main.
- `src/server/authserver/Server/AuthSession.cpp` — AuthSession.
- `src/server/database/Database/DatabaseServerVersion.h` — DatabaseServerVersion.
- `src/server/database/Database/DatabaseWorkerPool.cpp` — DatabaseWorkerPool.
- `src/server/database/Database/DatabaseWorkerPool.h` — DatabaseWorkerPool.
- `src/server/database/Database/Implementation/CharacterDatabase.cpp` — CharacterDatabase.
- `src/server/database/Database/Implementation/LoginDatabase.cpp` — LoginDatabase.
- `src/server/database/Database/Implementation/WorldDatabase.cpp` — WorldDatabase.
- `src/server/database/Database/MySQLConnection.cpp` — MySQLConnection.
- `src/server/database/Database/MySQLConnection.h` — MySQLConnection.
- `src/server/database/Database/Transaction.cpp` — Transaction.
- `src/server/database/Database/Transaction.h` — Transaction.
- `src/server/game/AI/CreatureAI.cpp` — CreatureAI.
- `src/server/game/AI/ScriptedAI/ScriptedEscortAI.cpp` — ScriptedEscortAI.
- `src/server/game/AI/SmartScripts/ImmediateEventSummons.h` — ImmediateEventSummons.
- `src/server/game/AI/SmartScripts/SmartScript.cpp` — SmartScript.
- `src/server/game/AI/SmartScripts/SmartScriptMgr.cpp` — SmartScriptMgr.
- `src/server/game/AI/SmartScripts/SmartScriptMgr.h` — SmartScriptMgr.
- `src/server/game/Accounts/CharacterCreationPolicy.h` — CharacterCreationPolicy.
- `src/server/game/Achievements/AchievementMgr.cpp` — AchievementMgr.
- `src/server/game/AuctionHouse/AuctionHouseMgr.cpp` — AuctionHouseMgr.
- `src/server/game/AuctionHouseBot/AuctionHouseBot.cpp` — AuctionHouseBot.
- `src/server/game/AuctionHouseBot/AuctionHouseBot.h` — AuctionHouseBot.
- `src/server/game/AuctionHouseBot/AuctionHouseBotBuyer.cpp` — AuctionHouseBotBuyer.
- `src/server/game/AuctionHouseBot/AuctionHouseBotBuyer.h` — AuctionHouseBotBuyer.
- `src/server/game/AuctionHouseBot/AuctionHouseBotItemFilter.h` — AuctionHouseBotItemFilter.
- `src/server/game/AuctionHouseBot/AuctionHouseBotSeller.cpp` — AuctionHouseBotSeller.
- `src/server/game/BattlePay/BattlePayAccounting.h` — BattlePayAccounting.
- `src/server/game/BattlePay/BattlePayBoostQueries.h` — BattlePayBoostQueries.
- `src/server/game/BattlePay/BattlePayMgr.cpp` — BattlePayMgr.
- `src/server/game/BattlePay/BattlePayMgr.h` — BattlePayMgr.
- `src/server/game/BattlePay/BattlePayQueries.h` — BattlePayQueries.
- `src/server/game/BattlePay/ServiceBoost.cpp` — ServiceBoost.
- `src/server/game/BattlePay/ServiceBoost.h` — ServiceBoost.
- `src/server/game/BattlePet/BattlePetMgr.cpp` — BattlePetMgr.
- `src/server/game/Chat/ChatCommands/ChatCommand.cpp` — ChatCommand.
- `src/server/game/Chat/ChatCommands/VisibleCommandMatches.h` — VisibleCommandMatches.
- `src/server/game/Combat/CombatManager.cpp` — CombatManager.
- `src/server/game/Conditions/ConditionMgr.cpp` — ConditionMgr.
- `src/server/game/DungeonFinding/LFGMgr.cpp` — LFGMgr.
- `src/server/game/DungeonFinding/LFGMgr.h` — LFGMgr.
- `src/server/game/Entities/Creature/AuthoredCreatureDamage.cpp` — AuthoredCreatureDamage.
- `src/server/game/Entities/Creature/AuthoredCreatureDamage.h` — AuthoredCreatureDamage.
- `src/server/game/Entities/Creature/Creature.cpp` — Creature.
- `src/server/game/Entities/Creature/Creature.h` — Creature.
- `src/server/game/Entities/Creature/CreatureCombatPolicy.h` — CreatureCombatPolicy.
- `src/server/game/Entities/Creature/CreatureData.h` — CreatureData.
- `src/server/game/Entities/Creature/CreatureDetectionPolicy.h` — CreatureDetectionPolicy.
- `src/server/game/Entities/Creature/GossipBroadcastId.h` — GossipBroadcastId.
- `src/server/game/Entities/GameObject/GameObject.cpp` — GameObject.
- `src/server/game/Entities/GameObject/GameObject.h` — GameObject.
- `src/server/game/Entities/Item/Item.cpp` — Item.
- `src/server/game/Entities/Item/ItemEnchantmentMgr.cpp` — ItemEnchantmentMgr.
- `src/server/game/Entities/Item/RandomEnchantmentSelection.h` — RandomEnchantmentSelection.
- `src/server/game/Entities/Object/CreatorObjectRefs.h` — CreatorObjectRefs.
- `src/server/game/Entities/Player/CompanionLoot.h` — CompanionLoot.
- `src/server/game/Entities/Player/CuratedPlayerNames.h` — CuratedPlayerNames.
- `src/server/game/Entities/Player/Player.cpp` — Player.
- `src/server/game/Entities/Player/Player.h` — Player.
- `src/server/game/Entities/Player/PlayerTaxi.h` — PlayerTaxi.
- `src/server/game/Entities/Transport/Transport.cpp` — Transport.
- `src/server/game/Entities/Unit/StatSystem.cpp` — StatSystem.
- `src/server/game/Entities/Unit/Unit.cpp` — Unit.
- `src/server/game/Entities/Unit/Unit.h` — Unit.
- `src/server/game/Entities/Vehicle/Vehicle.cpp` — Vehicle.
- `src/server/game/Events/GameEventMgr.cpp` — GameEventMgr.
- `src/server/game/Events/GameEventSchedule.cpp` — GameEventSchedule.
- `src/server/game/Events/GameEventSchedule.h` — GameEventSchedule.
- `src/server/game/Globals/ObjectMgr.cpp` — ObjectMgr.
- `src/server/game/Globals/ObjectMgr.h` — ObjectMgr.
- `src/server/game/Groups/Group.cpp` — Group.
- `src/server/game/Handlers/BattlePayHandler.cpp` — BattlePayHandler.
- `src/server/game/Handlers/BattlePetHandler.cpp` — BattlePetHandler.
- `src/server/game/Handlers/CharacterHandler.cpp` — CharacterHandler.
- `src/server/game/Handlers/GroupHandler.cpp` — GroupHandler.
- `src/server/game/Handlers/LootHandler.cpp` — LootHandler.
- `src/server/game/Handlers/MiscHandler.cpp` — MiscHandler.
- `src/server/game/Handlers/MovementHandler.cpp` — MovementHandler.
- `src/server/game/Handlers/QueryHandler.cpp` — QueryHandler.
- `src/server/game/Handlers/QuestHandler.cpp` — QuestHandler.
- `src/server/game/Handlers/SpellHandler.cpp` — SpellHandler.
- `src/server/game/Loot/LootMgr.cpp` — LootMgr.
- `src/server/game/Maps/Map.cpp` — Map.
- `src/server/game/Movement/MotionMaster.cpp` — MotionMaster.
- `src/server/game/Movement/MovementGenerators/FollowMovementGenerator.cpp` — FollowMovementGenerator.
- `src/server/game/Movement/MovementGenerators/FormationMovementGenerator.cpp` — FormationMovementGenerator.
- `src/server/game/Movement/MovementGenerators/PointMovementGenerator.cpp` — PointMovementGenerator.
- `src/server/game/Movement/MovementGenerators/SplineChainMovementGenerator.cpp` — SplineChainMovementGenerator.
- `src/server/game/Movement/MovementGenerators/WaypointMovementGenerator.cpp` — WaypointMovementGenerator.
- `src/server/game/Movement/PathGenerator.cpp` — PathGenerator.
- `src/server/game/Movement/Spline/MoveSpline.cpp` — MoveSpline.
- `src/server/game/Movement/Spline/MoveSplineInit.h` — MoveSplineInit.
- `src/server/game/Movement/Spline/MoveSplineInitArgs.h` — MoveSplineInitArgs.
- `src/server/game/Movement/Spline/MovementInputPolicy.h` — MovementInputPolicy.
- `src/server/game/Movement/Spline/VerticalKnockbackPolicy.h` — VerticalKnockbackPolicy.
- `src/server/game/Pools/PoolMgr.cpp` — PoolMgr.
- `src/server/game/Pools/PoolMgr.h` — PoolMgr.
- `src/server/game/Quests/QuestAreaTriggerPolicy.h` — QuestAreaTriggerPolicy.
- `src/server/game/Quests/QuestDef.cpp` — QuestDef.
- `src/server/game/Quests/QuestDef.h` — QuestDef.
- `src/server/game/Quests/QuestDependencyPolicy.h` — QuestDependencyPolicy.
- `src/server/game/Scripting/MapScripts.cpp` — MapScripts.
- `src/server/game/Server/GameClient.cpp` — GameClient.
- `src/server/game/Server/Protocol/Opcodes.cpp` — Opcodes.
- `src/server/game/Server/WorldSession.cpp` — WorldSession.
- `src/server/game/Server/WorldSocket.cpp` — WorldSocket.
- `src/server/game/Services/ServiceMgr.cpp` — ServiceMgr.
- `src/server/game/Services/ServiceMgr.h` — ServiceMgr.
- `src/server/game/Spells/Spell.cpp` — Spell.
- `src/server/game/Spells/SpellEffects.cpp` — SpellEffects.
- `src/server/game/Spells/SpellMgr.cpp` — SpellMgr.
- `src/server/game/Spells/SpellScript.cpp` — SpellScript.
- `src/server/game/Texts/CreatureTextMgr.cpp` — CreatureTextMgr.
- `src/server/game/World/World.cpp` — World.
- `src/server/scripts/Battlegrounds/CatapultLaunchPolicy.h` — CatapultLaunchPolicy.
- `src/server/scripts/Battlegrounds/isle_of_conquest.cpp` — isle of conquest.
- `src/server/scripts/Commands/cs_debug.cpp` — cs debug.
- `src/server/scripts/Commands/cs_ticket.cpp` — cs ticket.
- `src/server/scripts/Custom/wow_token.cpp` — wow token.
- `src/server/scripts/EasternKingdoms/BastionOfTwilight/bastion_of_twilight.cpp` — bastion of twilight.
- `src/server/scripts/EasternKingdoms/BlackrockMountain/BlackwingDescent/boss_omnotron_defence_system.cpp` — boss omnotron defence system.
- `src/server/scripts/EasternKingdoms/Deadmines/boss_glubtok.cpp` — boss glubtok.
- `src/server/scripts/EasternKingdoms/Deadmines/boss_glubtok_sf27.cpp` — boss glubtok sf27.
- `src/server/scripts/EasternKingdoms/DeathknellIntroPolicy.h` — DeathknellIntroPolicy.
- `src/server/scripts/EasternKingdoms/GilneasCageRecovery.h` — GilneasCageRecovery.
- `src/server/scripts/EasternKingdoms/GilneasCrowleyRoute.h` — GilneasCrowleyRoute.
- `src/server/scripts/EasternKingdoms/GilneasPrisonEvent.h` — GilneasPrisonEvent.
- `src/server/scripts/EasternKingdoms/GilneasRidePath.h` — GilneasRidePath.
- `src/server/scripts/EasternKingdoms/zone_elwynn_forest.cpp` — zone elwynn forest.
- `src/server/scripts/EasternKingdoms/zone_gilneas.cpp` — zone gilneas.
- `src/server/scripts/EasternKingdoms/zone_tirisfal_glades.cpp` — zone tirisfal glades.
- `src/server/scripts/Kalimdor/CavernsOfTime/HourofTwilight/boss_archbishop_benedictus.cpp` — boss archbishop benedictus.
- `src/server/scripts/Kalimdor/Firelands/firelands.cpp` — firelands.
- `src/server/scripts/Kalimdor/zone_darkshore.cpp` — zone darkshore.
- `src/server/scripts/Kalimdor/zone_teldrassil.cpp` — zone teldrassil.
- `src/server/scripts/Kalimdor/zone_teldrassil_sf27.cpp` — zone teldrassil sf27.
- `src/server/scripts/Northrend/IcecrownCitadel/icecrown_citadel.cpp` — icecrown citadel.
- `src/server/scripts/Northrend/Ulduar/Ulduar/boss_xt002.cpp` — boss xt002.
- `src/server/scripts/Northrend/zone_grizzly_hills.cpp` — zone grizzly hills.
- `src/server/scripts/Outland/Auchindoun/SethekkHalls/boss_anzu.cpp` — boss anzu.
- `src/server/scripts/Outland/zone_shattrath_city.cpp` — zone shattrath city.
- `src/server/scripts/Pandaria/GateOfTheSettingSun/boss_commander_rimok.cpp` — boss commander rimok.
- `src/server/scripts/Pandaria/GateOfTheSettingSun/gate_of_the_setting_sun.cpp` — gate of the setting sun.
- `src/server/scripts/Pandaria/GateOfTheSettingSun/instance_gate_of_the_setting_sun.cpp` — instance gate of the setting sun.
- `src/server/scripts/Pandaria/HeartOfFear/boss_tayak.cpp` — boss tayak.
- `src/server/scripts/Pandaria/HeartOfFear/boss_zorlok.cpp` — boss zorlok.
- `src/server/scripts/Pandaria/MogushanPalace/boss_xin_the_weaponmaster.cpp` — boss xin the weaponmaster.
- `src/server/scripts/Pandaria/MogushanVault/boss_will_of_emperor.cpp` — boss will of emperor.
- `src/server/scripts/Pandaria/SiegeOfOrgrimmar/boss_paragons_of_the_klaxxi.cpp` — boss paragons of the klaxxi.
- `src/server/scripts/Pandaria/StormstoutBrewery/stormstout_brewery.cpp` — stormstout brewery.
- `src/server/scripts/Pandaria/ThroneOfThunder/boss_jikun.cpp` — boss jikun.
- `src/server/scripts/Pandaria/ThroneOfThunder/boss_jinrokh.cpp` — boss jinrokh.
- `src/server/scripts/Pandaria/ThroneOfThunder/throne_of_thunder.cpp` — throne of thunder.
- `src/server/scripts/Pandaria/WanderingIsland/JaominDuelPolicy.h` — JaominDuelPolicy.
- `src/server/scripts/Pandaria/WanderingIsland/zone_wandering_island_north.cpp` — zone wandering island north.
- `src/server/scripts/Pet/pet_generic.cpp` — pet generic.
- `src/server/scripts/Pet/pet_mage_sf27.cpp` — pet mage sf27.
- `src/server/scripts/Pet/pet_priest.cpp` — pet priest.
- `src/server/scripts/Pet/pet_priest_sf27.cpp` — pet priest sf27.
- `src/server/scripts/ScriptLoader/ScriptLoader.cpp` — ScriptLoader.
- `src/server/scripts/Spells/PaladinSpellCalculations.h` — PaladinSpellCalculations.
- `src/server/scripts/Spells/spell_archaeology_sf27.cpp` — spell archaeology sf27.
- `src/server/scripts/Spells/spell_common.cpp` — spell common.
- `src/server/scripts/Spells/spell_dk.cpp` — spell dk.
- `src/server/scripts/Spells/spell_dk_sf27.cpp` — spell dk sf27.
- `src/server/scripts/Spells/spell_druid.cpp` — spell druid.
- `src/server/scripts/Spells/spell_druid_sf27.cpp` — spell druid sf27.
- `src/server/scripts/Spells/spell_generic.cpp` — spell generic.
- `src/server/scripts/Spells/spell_generic_sf27.cpp` — spell generic sf27.
- `src/server/scripts/Spells/spell_holiday.cpp` — spell holiday.
- `src/server/scripts/Spells/spell_holiday_sf27.cpp` — spell holiday sf27.
- `src/server/scripts/Spells/spell_hunter.cpp` — spell hunter.
- `src/server/scripts/Spells/spell_hunter_sf27.cpp` — spell hunter sf27.
- `src/server/scripts/Spells/spell_item.cpp` — spell item.
- `src/server/scripts/Spells/spell_item_sf27.cpp` — spell item sf27.
- `src/server/scripts/Spells/spell_mage.cpp` — spell mage.
- `src/server/scripts/Spells/spell_mage_sf27.cpp` — spell mage sf27.
- `src/server/scripts/Spells/spell_mastery_sf27.cpp` — spell mastery sf27.
- `src/server/scripts/Spells/spell_monk.cpp` — spell monk.
- `src/server/scripts/Spells/spell_monk_sf27.cpp` — spell monk sf27.
- `src/server/scripts/Spells/spell_paladin.cpp` — spell paladin.
- `src/server/scripts/Spells/spell_paladin_sf27.cpp` — spell paladin sf27.
- `src/server/scripts/Spells/spell_pet.cpp` — spell pet.
- `src/server/scripts/Spells/spell_pet_sf27.cpp` — spell pet sf27.
- `src/server/scripts/Spells/spell_priest.cpp` — spell priest.
- `src/server/scripts/Spells/spell_priest_sf27.cpp` — spell priest sf27.
- `src/server/scripts/Spells/spell_quest.cpp` — spell quest.
- `src/server/scripts/Spells/spell_quest_sf27.cpp` — spell quest sf27.
- `src/server/scripts/Spells/spell_rogue.cpp` — spell rogue.
- `src/server/scripts/Spells/spell_rogue_sf27.cpp` — spell rogue sf27.
- `src/server/scripts/Spells/spell_shaman.cpp` — spell shaman.
- `src/server/scripts/Spells/spell_shaman_sf27.cpp` — spell shaman sf27.
- `src/server/scripts/Spells/spell_warlock.cpp` — spell warlock.
- `src/server/scripts/Spells/spell_warlock_sf27.cpp` — spell warlock sf27.
- `src/server/scripts/Spells/spell_warrior.cpp` — spell warrior.
- `src/server/scripts/Spells/spell_warrior_sf27.cpp` — spell warrior sf27.
- `src/server/scripts/World/BrawlersGuild/brawlers_guild.cpp` — brawlers guild.
- `src/server/scripts/World/boss_emerald_dragons.cpp` — boss emerald dragons.
- `src/server/scripts/World/darkmoon_island.cpp` — darkmoon island.
- `src/server/scripts/World/fangs_of_father_scripts.cpp` — fangs of father scripts.
- `src/server/scripts/World/tarecgosa_scripts.cpp` — tarecgosa scripts.
- `src/server/worldserver/Main.cpp` — Main.
- `src/tools/mmaps_generator/MapBuilder.cpp` — MapBuilder.
- `src/tools/mmaps_generator/TerrainBuilder.cpp` — TerrainBuilder.

### Feature documentation and evidence inventories

- `doc/active-rollup-audit-2026-11-08.json` — active rollup audit 2026 11 08.
- `doc/auction-house-bots.md` — auction house bots.
- `doc/automated-content-testing-2026-11-05.md` — automated content testing 2026 11 05.
- `doc/automated-gameplay-qa-results-2026-11-08.json` — automated gameplay qa results 2026 11 08.
- `doc/automated-gameplay-qa.md` — automated gameplay qa.
- `doc/automated-qa-unattended-2026-11-08.json` — automated qa unattended 2026 11 08.
- `doc/battlepay-accounting.md` — battlepay accounting.
- `doc/bot-chat-pack-2026-11-12.md` — bot chat pack 2026 11 12.
- `doc/branch-cleanup-2026-11-10.md` — branch cleanup 2026 11 10.
- `doc/city-companions.md` — city companions.
- `doc/companion-crashes-2026-11-04.md` — companion crashes 2026 11 04.
- `doc/companion-loot-and-status.md` — companion loot and status.
- `doc/companion-party-policy-2026-11-12.md` — companion party policy 2026 11 12.
- `doc/content-catalogue-548/README.md` — README.
- `doc/content-catalogue-548/achievement_criteria.csv` — achievement criteria.
- `doc/content-catalogue-548/achievements.csv` — achievements.
- `doc/content-catalogue-548/areas.csv` — areas.
- `doc/content-catalogue-548/battle_pet_species.csv` — battle pet species.
- `doc/content-catalogue-548/battlegrounds_arenas.csv` — battlegrounds arenas.
- `doc/content-catalogue-548/boss_candidates.csv` — boss candidates.
- `doc/content-catalogue-548/catalogue-files.json` — catalogue files.
- `doc/content-catalogue-548/classes.csv` — classes.
- `doc/content-catalogue-548/client-table-provenance.json` — client table provenance.
- `doc/content-catalogue-548/creatures.csv` — creatures.
- `doc/content-catalogue-548/currencies.csv` — currencies.
- `doc/content-catalogue-548/dungeon_encounters.csv` — dungeon encounters.
- `doc/content-catalogue-548/factions.csv` — factions.
- `doc/content-catalogue-548/gameobjects.csv` — gameobjects.
- `doc/content-catalogue-548/item_sparse_coverage.csv` — item sparse coverage.
- `doc/content-catalogue-548/items.csv` — items.
- `doc/content-catalogue-548/journal_boss_actors.csv` — journal boss actors.
- `doc/content-catalogue-548/journal_encounters.csv` — journal encounters.
- `doc/content-catalogue-548/journal_instances.csv` — journal instances.
- `doc/content-catalogue-548/lfg_activities.csv` — lfg activities.
- `doc/content-catalogue-548/maps.csv` — maps.
- `doc/content-catalogue-548/mount_item_links.csv` — mount item links.
- `doc/content-catalogue-548/profession_recipe_candidates.csv` — profession recipe candidates.
- `doc/content-catalogue-548/quests.csv` — quests.
- `doc/content-catalogue-548/races.csv` — races.
- `doc/content-catalogue-548/reference-integrity-summary.json` — reference integrity summary.
- `doc/content-catalogue-548/reference_integrity_findings.csv` — reference integrity findings.
- `doc/content-catalogue-548/research_branches.csv` — research branches.
- `doc/content-catalogue-548/research_projects.csv` — research projects.
- `doc/content-catalogue-548/scenario_steps.csv` — scenario steps.
- `doc/content-catalogue-548/scenarios.csv` — scenarios.
- `doc/content-catalogue-548/skill_ability_links.csv` — skill ability links.
- `doc/content-catalogue-548/skill_lines.csv` — skill lines.
- `doc/content-catalogue-548/specializations.csv` — specializations.
- `doc/content-catalogue-548/spells.csv` — spells.
- `doc/content-catalogue-548/summary.json` — summary.
- `doc/content-catalogue-548/talents.csv` — talents.
- `doc/content-catalogue-548/taxi_nodes.csv` — taxi nodes.
- `doc/content-catalogue-548/taxi_paths.csv` — taxi paths.
- `doc/content-catalogue-548/validation.json` — validation.
- `doc/content-catalogue-548/world_systems.csv` — world systems.
- `doc/content-master-list-548.md` — content master list 548.
- `doc/content-reference-findings-2026-11-05.csv` — content reference findings 2026 11 05.
- `doc/database-cleanup-2026-11-11.md` — database cleanup 2026 11 11.
- `doc/database-compatibility.md` — database compatibility.
- `doc/emucoach-existing-database.md` — emucoach existing database.
- `doc/emucoach-followup.md` — emucoach followup.
- `doc/exhaustive-quest-qa-2026-11-09.md` — exhaustive quest qa 2026 11 09.
- `doc/features-and-improvements.md` — features and improvements.
- `doc/full-history-reconciliation-2026-11-09.md` — full history reconciliation 2026 11 09.
- `doc/full-history-source-inventory-2026-11-09.csv` — full history source inventory 2026 11 09.
- `doc/full-history-source-summary-2026-11-09.json` — full history source summary 2026 11 09.
- `doc/gameplay-batch-2026-11-10.md` — gameplay batch 2026 11 10.
- `doc/gilneas-finale-vehicles-2026-11-12.md` — gilneas finale vehicles 2026 11 12.
- `doc/gilneas-progression-campaign-2026-11-05.md` — gilneas progression campaign 2026 11 05.
- `doc/gunship-client-crash-2026-11-12.md` — gunship client crash 2026 11 12.
- `doc/handoff-changelog.md` — handoff changelog.
- `doc/historical-quest-field-batch-2026-11-09.json` — historical quest field batch 2026 11 09.
- `doc/historical-quest-field-batch-2026-11-09.md` — historical quest field batch 2026 11 09.
- `doc/howling-oak-tarien-2026-11-12.md` — howling oak tarien 2026 11 12.
- `doc/jaomin-repair-2026-11-05.md` — jaomin repair 2026 11 05.
- `doc/keel-harbor-followup-2026-11-12.md` — keel harbor followup 2026 11 12.
- `doc/legendary-native-restoration-2026-11-08.json` — legendary native restoration 2026 11 08.
- `doc/legendary-native-restoration-2026-11-08.md` — legendary native restoration 2026 11 08.
- `doc/loa-ai-fallback-batch-2026-11-08.json` — loa ai fallback batch 2026 11 08.
- `doc/loa-ai-fallback-batch-2026-11-08.md` — loa ai fallback batch 2026 11 08.
- `doc/loa-beta-backlog-2026-11-07.json` — loa beta backlog 2026 11 07.
- `doc/loa-beta-gameplay-results-2026-11-07.json` — loa beta gameplay results 2026 11 07.
- `doc/loa-beta-integration-2026-11-07.json` — loa beta integration 2026 11 07.
- `doc/loa-beta-integration-2026-11-07.md` — loa beta integration 2026 11 07.
- `doc/loa-content-findings-2026-11-07.json` — loa content findings 2026 11 07.
- `doc/loa-fresh-restore-2026-11-07.md` — loa fresh restore 2026 11 07.
- `doc/loa-full-beta-integration-2026-11-08.json` — loa full beta integration 2026 11 08.
- `doc/loa-full-beta-integration-2026-11-08.md` — loa full beta integration 2026 11 08.
- `doc/loa-full-beta-merge-decisions-2026-11-07.json` — loa full beta merge decisions 2026 11 07.
- `doc/loa-runtime-binding-audit-2026-11-08.json` — loa runtime binding audit 2026 11 08.
- `doc/loa-runtime-repairs-2026-11-08.md` — loa runtime repairs 2026 11 08.
- `doc/loa-runtime-repairs-results-2026-11-08.json` — loa runtime repairs results 2026 11 08.
- `doc/loa-starter-repairs-2026-11-07.md` — loa starter repairs 2026 11 07.
- `doc/mail-investigation-2026-11-04.md` — mail investigation 2026 11 04.
- `doc/map571-and-published-fixes-2026-11-05.md` — map571 and published fixes 2026 11 05.
- `doc/missing-native-script-inventory-2026-11-05.csv` — missing native script inventory 2026 11 05.
- `doc/mmap-42-23-repair-2026-11-06.md` — mmap 42 23 repair 2026 11 06.
- `doc/mmap-coverage-plan-2026-11-06.csv` — mmap coverage plan 2026 11 06.
- `doc/mmap-coverage-repair-2026-11-06.md` — mmap coverage repair 2026 11 06.
- `doc/mmap-coverage-results-2026-11-06.csv` — mmap coverage results 2026 11 06.
- `doc/native-binding-review-2026-11-05.csv` — native binding review 2026 11 05.
- `doc/native-content-campaign-2026-11-05.md` — native content campaign 2026 11 05.
- `doc/navmesh-crash-2026-10-30.md` — navmesh crash 2026 10 30.
- `doc/nightwind-ai-lifetime-2026-11-12.md` — nightwind ai lifetime 2026 11 12.
- `doc/normal-gameplay-campaign-2026-11-09.md` — normal gameplay campaign 2026 11 09.
- `doc/orb-loyalty-and-yak-services-2026-11-12.md` — orb loyalty and yak services 2026 11 12.
- `doc/original-mop-runtime-2026-11-11.md` — original mop runtime 2026 11 11.
- `doc/playerbot-naming-cumulative-handoff.md` — playerbot naming cumulative handoff.
- `doc/project-checklist.md` — project checklist.
- `doc/published-loa-batch-68f8098431.json` — published loa batch 68f8098431.
- `doc/published-loa-batch-ef5112acc3.json` — published loa batch ef5112acc3.
- `doc/published-native-gap-inventory-2026-11-08.json` — published native gap inventory 2026 11 08.
- `doc/published-sql-compatibility-2026-11-05.csv` — published sql compatibility 2026 11 05.
- `doc/qa-content-repairs-2026-11-08.json` — qa content repairs 2026 11 08.
- `doc/qa-content-repairs-2026-11-08.md` — qa content repairs 2026 11 08.
- `doc/qa/autonomous-gameplay-milestones-2026-11-09.json` — autonomous gameplay milestones 2026 11 09.
- `doc/qa/autonomous-gameplay-results-2026-11-09.md` — autonomous gameplay results 2026 11 09.
- `doc/qa/remaining-script-bindings-2026-11-09.json` — remaining script bindings 2026 11 09.
- `doc/qa/unattended-gameplay-qa.md` — unattended gameplay qa.
- `doc/qa/unattended-qa-validation-2026-11-10.md` — unattended qa validation 2026 11 10.
- `doc/qa/worgen-followup-results-2026-11-09.json` — worgen followup results 2026 11 09.
- `doc/qa/worgen-progression-results-2026-11-09.json` — worgen progression results 2026 11 09.
- `doc/qa/worgen-quest-class-coverage-2026-11-09.csv` — worgen quest class coverage 2026 11 09.
- `doc/quality-automation-2026-11-08.md` — quality automation 2026 11 08.
- `doc/quality-automation-results-2026-11-08.json` — quality automation results 2026 11 08.
- `doc/quest-dependency-audit-548/README.md` — README.
- `doc/quest-dependency-audit-548/findings.csv` — findings.
- `doc/quest-dependency-audit-548/quests.csv` — quests.
- `doc/quest-dependency-audit-548/repair-queue.csv` — repair queue.
- `doc/quest-dependency-audit-548/summary.json` — summary.
- `doc/quest-runtime-regressions-2026-11-08.json` — quest runtime regressions 2026 11 08.
- `doc/quest-runtime-regressions-2026-11-08.md` — quest runtime regressions 2026 11 08.
- `doc/realm-gilneas-cleanup-2026-11-11.md` — realm gilneas cleanup 2026 11 11.
- `doc/reconciliation-repairs-2026-11-08.md` — reconciliation repairs 2026 11 08.
- `doc/release26-quest-batch-ledger-2026-11-06.json` — release26 quest batch ledger 2026 11 06.
- `doc/release26-quests-worgen-2026-11-06.md` — release26 quests worgen 2026 11 06.
- `doc/release26-services-class-repairs-2026-11-06.md` — release26 services class repairs 2026 11 06.
- `doc/release26-services-ledger-2026-11-06.json` — release26 services ledger 2026 11 06.
- `doc/release26-statement-coverage-2026-11-09.csv` — release26 statement coverage 2026 11 09.
- `doc/release27-combat-ledger-2026-11-06.json` — release27 combat ledger 2026 11 06.
- `doc/release27-services-ledger-2026-11-06.json` — release27 services ledger 2026 11 06.
- `doc/release27-world-fixture-2026-11-06.json` — release27 world fixture 2026 11 06.
- `doc/release27-world-ledger-2026-11-06.json` — release27 world ledger 2026 11 06.
- `doc/release27-world-live-contracts-2026-11-06.json` — release27 world live contracts 2026 11 06.
- `doc/release27-world-startup-2026-11-06.json` — release27 world startup 2026 11 06.
- `doc/remaining-native-script-bindings-2026-11-08.json` — remaining native script bindings 2026 11 08.
- `doc/remaining-quest-compatibility-deployment-2026-11-09.json` — remaining quest compatibility deployment 2026 11 09.
- `doc/remaining-quest-compatibility-deployment-2026-11-09.md` — remaining quest compatibility deployment 2026 11 09.
- `doc/remaining-quest-input-dispositions-2026-11-09.json` — remaining quest input dispositions 2026 11 09.
- `doc/rollup-reconciliation-2026-11-09.md` — rollup reconciliation 2026 11 09.
- `doc/runtime-diagnostics.md` — runtime diagnostics.
- `doc/runtime-log-repairs-2026-11-12.md` — runtime log repairs 2026 11 12.
- `doc/runtime-noise-and-social-chat-2026-11-12.md` — runtime noise and social chat 2026 11 12.
- `doc/setting-sun-crash-2026-11-12.md` — setting sun crash 2026 11 12.
- `doc/sf27-archaeology-native-port.json` — sf27 archaeology native port.
- `doc/sf27-authored-damage-data.json` — sf27 authored damage data.
- `doc/sf27-darkshore-decoy-port.json` — sf27 darkshore decoy port.
- `doc/sf27-dk-native-port.json` — sf27 dk native port.
- `doc/sf27-druid-native-port.json` — sf27 druid native port.
- `doc/sf27-final-fixture-results-2026-11-06.json` — sf27 final fixture results 2026 11 06.
- `doc/sf27-final-integration-2026-11-06.md` — sf27 final integration 2026 11 06.
- `doc/sf27-final-source-dispositions.json` — sf27 final source dispositions.
- `doc/sf27-generic-native-port.json` — sf27 generic native port.
- `doc/sf27-glubtok-encounter-port.json` — sf27 glubtok encounter port.
- `doc/sf27-holiday-native-port.json` — sf27 holiday native port.
- `doc/sf27-hunter-native-port.json` — sf27 hunter native port.
- `doc/sf27-interaction-data.json` — sf27 interaction data.
- `doc/sf27-item-native-port.json` — sf27 item native port.
- `doc/sf27-mage-native-port.json` — sf27 mage native port.
- `doc/sf27-mastery-native-port.json` — sf27 mastery native port.
- `doc/sf27-monk-native-port.json` — sf27 monk native port.
- `doc/sf27-native-spell-data.json` — sf27 native spell data.
- `doc/sf27-native-spell-integration-2026-11-06.md` — sf27 native spell integration 2026 11 06.
- `doc/sf27-paladin-native-port.json` — sf27 paladin native port.
- `doc/sf27-pet-native-port.json` — sf27 pet native port.
- `doc/sf27-priest-native-port.json` — sf27 priest native port.
- `doc/sf27-quest-native-port.json` — sf27 quest native port.
- `doc/sf27-remaining-content-data.json` — sf27 remaining content data.
- `doc/sf27-rogue-native-port.json` — sf27 rogue native port.
- `doc/sf27-shaman-native-port.json` — sf27 shaman native port.
- `doc/sf27-supporting-pet-ports.json` — sf27 supporting pet ports.
- `doc/sf27-teldrassil-quest-port.json` — sf27 teldrassil quest port.
- `doc/sf27-typed-loot-data.json` — sf27 typed loot data.
- `doc/sf27-warlock-native-port.json` — sf27 warlock native port.
- `doc/sf27-warrior-native-port.json` — sf27 warrior native port.
- `doc/shang-xi-visibility-2026-11-05.md` — shang xi visibility 2026 11 05.
- `doc/skyfire-rollup-acquisition-2026-11-08.json` — skyfire rollup acquisition 2026 11 08.
- `doc/skyfire-rollup-acquisition-2026-11-08.md` — skyfire rollup acquisition 2026 11 08.
- `doc/skyfire-upstream-worklist-2026-11-05.md` — skyfire upstream worklist 2026 11 05.
- `doc/skyfire27-combat-fixture-2026-11-06.json` — skyfire27 combat fixture 2026 11 06.
- `doc/skyfire27-integration-2026-11-06.md` — skyfire27 integration 2026 11 06.
- `doc/skyfire27-integration-inventory-2026-11-06.csv` — skyfire27 integration inventory 2026 11 06.
- `doc/skyfire27-live-contracts-2026-11-06.json` — skyfire27 live contracts 2026 11 06.
- `doc/skyfire27-native-divergence-2026-11-06.json` — skyfire27 native divergence 2026 11 06.
- `doc/skyfire27-services-fixture-2026-11-06.json` — skyfire27 services fixture 2026 11 06.
- `doc/skyfire27-startup-observation-2026-11-06.json` — skyfire27 startup observation 2026 11 06.
- `doc/skyfire27-upstream-manifest.json` — skyfire27 upstream manifest.
- `doc/skyfire27-world-deployment-2026-11-06.md` — skyfire27 world deployment 2026 11 06.
- `doc/stability-quest-continuation-2026-11-05.md` — stability quest continuation 2026 11 05.
- `doc/starting-training-batch-2026-11-05.md` — starting training batch 2026 11 05.
- `doc/starting-zone-upstream-audit-2026-11-05.md` — starting zone upstream audit 2026 11 05.
- `doc/startup-batch-repair-2026-11-05.md` — startup batch repair 2026 11 05.
- `doc/startup-repair-2026-11-04.md` — startup repair 2026 11 04.
- `doc/startup-repair-2026-11-05.md` — startup repair 2026 11 05.
- `doc/startup-repair-continuation-2026-11-05.md` — startup repair continuation 2026 11 05.
- `doc/startup-repair-package.md` — startup repair package.
- `doc/validation-results.md` — validation results.
- `doc/vertical-knockback-repair-2026-11-06.md` — vertical knockback repair 2026 11 06.
- `doc/windows-vs2022.md` — windows vs2022.
- `doc/worgen-14154-readonly-2026-11-08.md` — worgen 14154 readonly 2026 11 08.
- `doc/worgen-progression-repair-2026-11-09.md` — worgen progression repair 2026 11 09.
- `doc/workflow-simplification-2026-11-10.md` — workflow simplification 2026 11 10.
- `doc/wyvern-camera-control-2026-11-12.md` — wyvern camera control 2026 11 12.

### Sanitized configuration templates

- `modules/mod_playerbots/config/playerbots.conf.dist` — playerbots.conf.
- `src/server/authserver/authserver.conf.dist` — authserver.conf.
- `src/server/worldserver/worldserver.conf.dist` — worldserver.conf.

### SQL schemas, migrations and static data

- `modules/mod_playerbots/data/sql/characters/curated_name_candidates.sql` — curated name candidates.
- `modules/mod_playerbots/data/sql/playerbots/2026_11_12_00_chat_punctuation.sql` — 2026 11 12 00 chat punctuation.
- `modules/mod_playerbots/data/sql/playerbots/2026_11_12_01_ambient_chat_pack.sql` — 2026 11 12 01 ambient chat pack.
- `modules/mod_playerbots/data/sql/playerbots/2026_11_12_02_standalone_social_chat.sql` — 2026 11 12 02 standalone social chat.
- `sql/base/auth.sql` — auth.
- `sql/base/characters.sql` — characters.
- `sql/compat/emucoach/auth_additive_candidate.sql` — auth additive candidate.
- `sql/compat/emucoach/characters_additive_candidate.sql` — characters additive candidate.
- `sql/compat/emucoach/world_columns_preserving.sql` — world columns preserving.
- `sql/compat/emucoach/world_quest_preserving.sql` — world quest preserving.
- `sql/compat/mysql55/auth.sql` — auth.
- `sql/compat/mysql55/characters.sql` — characters.
- `sql/updates/auth/2026_10_28_00_battlepay_accounting.sql` — 2026 10 28 00 battlepay accounting.
- `sql/updates/auth/2026_10_28_01_battlepay_boost_recovery.sql` — 2026 10 28 01 battlepay boost recovery.
- `sql/updates/auth/2026_10_28_02_playerbots_account_default.sql` — 2026 10 28 02 playerbots account default.
- `sql/updates/auth/2026_10_28_03_mop_client_build.sql` — 2026 10 28 03 mop client build.
- `sql/updates/auth/2026_11_04_00_companion_command_permission.sql` — 2026 11 04 00 companion command permission.
- `sql/updates/characters/2026_10_28_00_battlepay_accounting.sql` — 2026 10 28 00 battlepay accounting.
- `sql/updates/characters/2026_10_28_01_battlepay_boost_recovery.sql` — 2026 10 28 01 battlepay boost recovery.
- `sql/updates/characters/2026_11_07_02_characters_dreaming_glory_action.sql` — 2026 11 07 02 characters dreaming glory action.
- `sql/updates/characters/2026_11_12_00_characters_curated_playerbot_names.sql` — 2026 11 12 00 characters curated playerbot names.
- `sql/updates/characters/2026_11_12_01_characters_orb_redemption.sql` — 2026 11 12 01 characters orb redemption.
- `sql/updates/world/2026_10_29_963w_missing_waypoint_paths.sql` — 2026 10 29 963w missing waypoint paths.
- `sql/updates/world/2026_10_29_986_70_hb_southshore_purgation_alterac.sql` — 2026 10 29 986 70 hb southshore purgation alterac.
- `sql/updates/world/2026_11_04_00_world_restore_flight_movement.sql` — 2026 11 04 00 world restore flight movement.
- `sql/updates/world/2026_11_04_01_world_creature_text_legacy_columns.sql` — 2026 11 04 01 world creature text legacy columns.
- `sql/updates/world/2026_11_04_02_world_unused_split_pool.sql` — 2026 11 04 02 world unused split pool.
- `sql/updates/world/2026_11_04_03_world_unused_condition_values.sql` — 2026 11 04 03 world unused condition values.
- `sql/updates/world/2026_11_04_04_world_unused_pool_components.sql` — 2026 11 04 04 world unused pool components.
- `sql/updates/world/2026_11_04_05_world_retired_access_keys.sql` — 2026 11 04 05 world retired access keys.
- `sql/updates/world/2026_11_04_06_world_retired_generic_binding.sql` — 2026 11 04 06 world retired generic binding.
- `sql/updates/world/2026_11_04_07_world_empty_pool_candidates.sql` — 2026 11 04 07 world empty pool candidates.
- `sql/updates/world/2026_11_04_08_world_reference_loot_equal_groups.sql` — 2026 11 04 08 world reference loot equal groups.
- `sql/updates/world/2026_11_04_09_world_legacy_script_broadcast_text.sql` — 2026 11 04 09 world legacy script broadcast text.
- `sql/updates/world/2026_11_04_10_world_registered_script_aliases.sql` — 2026 11 04 10 world registered script aliases.
- `sql/updates/world/2026_11_04_11_world_missing_classic_loot_references.sql` — 2026 11 04 11 world missing classic loot references.
- `sql/updates/world/2026_11_04_12_world_obsolete_spell_bindings.sql` — 2026 11 04 12 world obsolete spell bindings.
- `sql/updates/world/2026_11_04_13_world_invalid_gameobject_locks.sql` — 2026 11 04 13 world invalid gameobject locks.
- `sql/updates/world/2026_11_04_14_world_sourced_quest_talks.sql` — 2026 11 04 14 world sourced quest talks.
- `sql/updates/world/2026_11_04_15_world_original_loa_talks.sql` — 2026 11 04 15 world original loa talks.
- `sql/updates/world/2026_11_04_16_world_deathknell_aura_requirements.sql` — 2026 11 04 16 world deathknell aura requirements.
- `sql/updates/world/2026_11_04_17_world_quest_area_state_masks.sql` — 2026 11 04 17 world quest area state masks.
- `sql/updates/world/2026_11_04_18_world_original_loa_gameobject_fields.sql` — 2026 11 04 18 world original loa gameobject fields.
- `sql/updates/world/2026_11_04_19_world_original_loa_runtime_talks.sql` — 2026 11 04 19 world original loa runtime talks.
- `sql/updates/world/2026_11_05_01_world_verified_spell_bindings.sql` — 2026 11 05 01 world verified spell bindings.
- `sql/updates/world/2026_11_05_02_world_triggered_spell_roles.sql` — 2026 11 05 02 world triggered spell roles.
- `sql/updates/world/2026_11_05_03_world_authored_smartai_bindings.sql` — 2026 11 05 03 world authored smartai bindings.
- `sql/updates/world/2026_11_05_04_world_unused_creature_flag.sql` — 2026 11 05 04 world unused creature flag.
- `sql/updates/world/2026_11_05_05_world_retired_criteria_data.sql` — 2026 11 05 05 world retired criteria data.
- `sql/updates/world/2026_11_05_06_world_customer_greeting_players.sql` — 2026 11 05 06 world customer greeting players.
- `sql/updates/world/2026_11_05_07_world_taskmaster_dialogue_recipient.sql` — 2026 11 05 07 world taskmaster dialogue recipient.
- `sql/updates/world/2026_11_05_08_world_class_spell_roles.sql` — 2026 11 05 08 world class spell roles.
- `sql/updates/world/2026_11_05_09_world_encounter_spell_roles.sql` — 2026 11 05 09 world encounter spell roles.
- `sql/updates/world/2026_11_05_10_world_marksman_scorpid.sql` — 2026 11 05 10 world marksman scorpid.
- `sql/updates/world/2026_11_05_11_world_missing_core_messages.sql` — 2026 11 05 11 world missing core messages.
- `sql/updates/world/2026_11_05_12_world_combat_dialogue_speakers.sql` — 2026 11 05 12 world combat dialogue speakers.
- `sql/updates/world/2026_11_05_13_world_restore_gameobject_loot.sql` — 2026 11 05 13 world restore gameobject loot.
- `sql/updates/world/2026_11_05_14_world_legacy_smart_actions.sql` — 2026 11 05 14 world legacy smart actions.
- `sql/updates/world/2026_11_05_15_world_invalid_script_sentinels.sql` — 2026 11 05 15 world invalid script sentinels.
- `sql/updates/world/2026_11_05_16_world_spawn_difficulty_masks.sql` — 2026 11 05 16 world spawn difficulty masks.
- `sql/updates/world/2026_11_05_17_world_disabled_spawn_respawn_links.sql` — 2026 11 05 17 world disabled spawn respawn links.
- `sql/updates/world/2026_11_05_18_world_missing_quest_poi_points.sql` — 2026 11 05 18 world missing quest poi points.
- `sql/updates/world/2026_11_05_19_world_missing_property_pools.sql` — 2026 11 05 19 world missing property pools.
- `sql/updates/world/2026_11_05_20_world_glyph_and_parachute_bindings.sql` — 2026 11 05 20 world glyph and parachute bindings.
- `sql/updates/world/2026_11_05_21_world_equipment_and_enrage.sql` — 2026 11 05 21 world equipment and enrage.
- `sql/updates/world/2026_11_05_22_world_property_pool_contamination.sql` — 2026 11 05 22 world property pool contamination.
- `sql/updates/world/2026_11_05_23_world_duplicate_class_bindings.sql` — 2026 11 05 23 world duplicate class bindings.
- `sql/updates/world/2026_11_05_24_world_jaomin_duel.sql` — 2026 11 05 24 world jaomin duel.
- `sql/updates/world/2026_11_05_25_world_shang_xi_phase_boundary.sql` — 2026 11 05 25 world shang xi phase boundary.
- `sql/updates/world/2026_11_05_26_world_stair_destiny_flight_options.sql` — 2026 11 05 26 world stair destiny flight options.
- `sql/updates/world/2026_11_05_27_world_starting_training.sql` — 2026 11 05 27 world starting training.
- `sql/updates/world/2026_11_05_28_world_existing_quest_ports.sql` — 2026 11 05 28 world existing quest ports.
- `sql/updates/world/2026_11_05_29_world_training_chain_links.sql` — 2026 11 05 29 world training chain links.
- `sql/updates/world/2026_11_05_30_world_gilneas_progression.sql` — 2026 11 05 30 world gilneas progression.
- `sql/updates/world/2026_11_05_31_world_gilneas_backend_alignment.sql` — 2026 11 05 31 world gilneas backend alignment.
- `sql/updates/world/2026_11_05_32_world_crowley_ride.sql` — 2026 11 05 32 world crowley ride.
- `sql/updates/world/2026_11_06_33_world_release26_quests_ai.sql` — 2026 11 06 33 world release26 quests ai.
- `sql/updates/world/2026_11_06_34_world_bloodfang_duplicates.sql` — 2026 11 06 34 world bloodfang duplicates.
- `sql/updates/world/2026_11_06_35_world_release26_backend_alignment.sql` — 2026 11 06 35 world release26 backend alignment.
- `sql/updates/world/2026_11_06_36_world_release26_loot_services.sql` — 2026 11 06 36 world release26 loot services.
- `sql/updates/world/2026_11_06_37_world_release26_class_spells.sql` — 2026 11 06 37 world release26 class spells.
- `sql/updates/world/2026_11_06_38_world_release26_loot_references.sql` — 2026 11 06 38 world release26 loot references.
- `sql/updates/world/2026_11_06_39_world_release26_spare_parts_alignment.sql` — 2026 11 06 39 world release26 spare parts alignment.
- `sql/updates/world/2026_11_06_40_world_release27_combat.sql` — 2026 11 06 40 world release27 combat.
- `sql/updates/world/2026_11_06_41_world_release27_services.sql` — 2026 11 06 41 world release27 services.
- `sql/updates/world/2026_11_06_42_world_release27_quests_loot_travel.sql` — 2026 11 06 42 world release27 quests loot travel.
- `sql/updates/world/2026_11_06_43_world_dragonmaw_loot_ownership.sql` — 2026 11 06 43 world dragonmaw loot ownership.
- `sql/updates/world/2026_11_06_44_world_sf27_native_spells.sql` — 2026 11 06 44 world sf27 native spells.
- `sql/updates/world/2026_11_06_45_world_sf27_interactions.sql` — 2026 11 06 45 world sf27 interactions.
- `sql/updates/world/2026_11_06_46_world_sf27_typed_loot_credit_spawns.sql` — 2026 11 06 46 world sf27 typed loot credit spawns.
- `sql/updates/world/2026_11_06_47_world_sf27_detection_range.sql` — 2026 11 06 47 world sf27 detection range.
- `sql/updates/world/2026_11_06_48_world_sf27_remaining_content.sql` — 2026 11 06 48 world sf27 remaining content.
- `sql/updates/world/2026_11_06_49_world_sf27_authored_damage.sql` — 2026 11 06 49 world sf27 authored damage.
- `sql/updates/world/2026_11_06_50_world_sf27_vehicle_pool_dependencies.sql` — 2026 11 06 50 world sf27 vehicle pool dependencies.
- `sql/updates/world/2026_11_07_51_world_dreaming_glory_starter_action.sql` — 2026 11 07 51 world dreaming glory starter action.
- `sql/updates/world/2026_11_07_52_world_loa_beta_starters.sql` — 2026 11 07 52 world loa beta starters.
- `sql/updates/world/2026_11_07_53_world_loa_beta_schema.sql` — 2026 11 07 53 world loa beta schema.
- `sql/updates/world/2026_11_07_54_world_loa_legacy_aliases.sql` — 2026 11 07 54 world loa legacy aliases.
- `sql/updates/world/2026_11_07_55_world_loa_locale_preservation.sql` — 2026 11 07 55 world loa locale preservation.
- `sql/updates/world/2026_11_08_00_world_loa_missing_dialogue.sql` — 2026 11 08 00 world loa missing dialogue.
- `sql/updates/world/2026_11_08_01_world_loa_script_dialogue.sql` — 2026 11 08 01 world loa script dialogue.
- `sql/updates/world/2026_11_08_02_world_loa_runtime_bindings.sql` — 2026 11 08 02 world loa runtime bindings.
- `sql/updates/world/2026_11_08_03_world_loa_qa_references.sql` — 2026 11 08 03 world loa qa references.
- `sql/updates/world/2026_11_08_04_world_loa_ai_fallback_batch.sql` — 2026 11 08 04 world loa ai fallback batch.
- `sql/updates/world/2026_11_08_05_world_loa_legendary_native.sql` — 2026 11 08 05 world loa legendary native.
- `sql/updates/world/2026_11_08_06_world_loa_invalid_animation_tiers.sql` — 2026 11 08 06 world loa invalid animation tiers.
- `sql/updates/world/2026_11_08_07_world_loa_gilneas_prison_event.sql` — 2026 11 08 07 world loa gilneas prison event.
- `sql/updates/world/2026_11_08_08_world_loa_anzu_brood.sql` — 2026 11 08 08 world loa anzu brood.
- `sql/updates/world/2026_11_08_09_world_loa_sf_interaction_reconciliation.sql` — 2026 11 08 09 world loa sf interaction reconciliation.
- `sql/updates/world/2026_11_09_00_world_loa_compat_alias_triggers.sql` — 2026 11 09 00 world loa compat alias triggers.
- `sql/updates/world/2026_11_09_01_world_historical_stranglethorn_tele.sql` — 2026 11 09 01 world historical stranglethorn tele.
- `sql/updates/world/2026_11_09_02_world_historical_quest_fields.sql` — 2026 11 09 02 world historical quest fields.
- `sql/updates/world/2026_11_09_03_world_signed_next_quest_links.sql` — 2026 11 09 03 world signed next quest links.
- `sql/updates/world/2026_11_09_04_world_historical_quest_compatibility.sql` — 2026 11 09 04 world historical quest compatibility.
- `sql/updates/world/2026_11_09_05_world_gilneas_terrain_and_cage.sql` — 2026 11 09 05 world gilneas terrain and cage.
- `sql/updates/world/2026_11_09_06_world_gilneas_progression_wiring.sql` — 2026 11 09 06 world gilneas progression wiring.
- `sql/updates/world/2026_11_09_07_world_halga_dialogue_groups.sql` — 2026 11 09 07 world halga dialogue groups.
- `sql/updates/world/2026_11_09_08_world_verified_script_aliases.sql` — 2026 11 09 08 world verified script aliases.
- `sql/updates/world/2026_11_11_00_world_original_mop_compat.sql` — 2026 11 11 00 world original mop compat.
- `sql/updates/world/2026_11_11_01_world_gilneas_battle_spells.sql` — 2026 11 11 01 world gilneas battle spells.
- `sql/updates/world/2026_11_11_02_world_gilneas_battle_phase_isolation.sql` — 2026 11 11 02 world gilneas battle phase isolation.
- `sql/updates/world/2026_11_11_03_world_gilneas_battle_dialogue.sql` — 2026 11 11 03 world gilneas battle dialogue.
- `sql/updates/world/2026_11_11_04_world_gorerot_ground_spawn.sql` — 2026 11 11 04 world gorerot ground spawn.
- `sql/updates/world/2026_11_11_05_world_gorerot_respawn.sql` — 2026 11 11 05 world gorerot respawn.
- `sql/updates/world/2026_11_11_06_world_gorerot_combat.sql` — 2026 11 11 06 world gorerot combat.
- `sql/updates/world/2026_11_11_07_world_gilneas_post_battle_handoff.sql` — 2026 11 11 07 world gilneas post battle handoff.
- `sql/updates/world/2026_11_11_08_world_gilneas_bombing_phase.sql` — 2026 11 11 08 world gilneas bombing phase.
- `sql/updates/world/2026_11_11_09_world_captured_bat_ride.sql` — 2026 11 11 09 world captured bat ride.
- `sql/updates/world/2026_11_11_10_world_torch_and_script_compat.sql` — 2026 11 11 10 world torch and script compat.
- `sql/updates/world/2026_11_11_11_world_missing_mop_loot_references.sql` — 2026 11 11 11 world missing mop loot references.
- `sql/updates/world/2026_11_11_12_world_obsolete_runtime_metadata.sql` — 2026 11 11 12 world obsolete runtime metadata.
- `sql/updates/world/2026_11_12_00_world_gilneas_finale_vehicles_scene.sql` — 2026 11 12 00 world gilneas finale vehicles scene.
- `sql/updates/world/2026_11_12_01_world_endgame_ship_template.sql` — 2026 11 12 01 world endgame ship template.
- `sql/updates/world/2026_11_12_02_world_keel_harbor_encounter_vendor.sql` — 2026 11 12 02 world keel harbor encounter vendor.
- `sql/updates/world/2026_11_12_03_world_gunship_wmo_client_safety.sql` — 2026 11 12 03 world gunship wmo client safety.
- `sql/updates/world/2026_11_12_04_world_endgame_single_ship.sql` — 2026 11 12 04 world endgame single ship.
- `sql/updates/world/2026_11_12_05_world_departure_emotes_smartlinks.sql` — 2026 11 12 05 world departure emotes smartlinks.
- `sql/updates/world/2026_11_12_06_world_howling_oak_and_tarien.sql` — 2026 11 12 06 world howling oak and tarien.
- `sql/updates/world/2026_11_12_07_world_idriana_bank_gossip.sql` — 2026 11 12 07 world idriana bank gossip.
- `sql/updates/world/2026_11_12_08_world_setting_sun_controller_objects.sql` — 2026 11 12 08 world setting sun controller objects.
- `sql/updates/world/2026_11_12_09_world_setting_sun_area_triggers.sql` — 2026 11 12 09 world setting sun area triggers.
- `sql/updates/world/2026_11_12_10_world_orb_of_loyalty.sql` — 2026 11 12 10 world orb of loyalty.
- `sql/updates/world/2026_11_12_11_world_expedition_yak_services.sql` — 2026 11 12 11 world expedition yak services.
- `sql/updates/world/2026_11_12_12_world_runtime_emote_links.sql` — 2026 11 12 12 world runtime emote links.
- `sql/updates/world/2026_11_12_13_world_missing_authored_dialogue.sql` — 2026 11 12 13 world missing authored dialogue.
- `sql/updates/world/2026_11_12_14_world_gilneas_graveyards.sql` — 2026 11 12 14 world gilneas graveyards.

## Merge procedure and rollback

1. Start from the pinned upstream checkout with a clean worktree.
2. Review the feature map, this changelog and the patch exclusions.
3. Apply the patch or overlay the complete files, including explicitly excluded sanitized replacements.
4. Review native/schema/static-data dependencies and back up the target schemas consistently.
5. Apply only the applicable clean migrations, keeping a hash-based execution ledger.
6. Configure private connection strings locally; never commit the supplied working configs to a public repository.
7. Build using a consistent connector include/library/DLL installation and retain matching EXE/PDB pairs.
8. Run relevant headless regressions against disposable owned fixtures and retain their reports.
9. Perform rendered client checks for vehicles, cinematics, interaction and dungeon geometry.
10. Roll back code/artifacts and matching related schema state together if a regression appears; never replay currency mutations on an uncertain acknowledgement.

The clean delivery deliberately excludes local player saves, transaction receipts,
private crash dumps, compiled EXEs/PDBs and large client map assets. Historical
reference inventories remain available in the accompanying reference archive.


## Latest runtime-noise and chat follow-up

# Runtime spell/emote repairs and quieter social chat — November 12

This follow-up addresses the console screenshot without changing WARN/ERROR
logging. Other original-world content errors remain visible and are not claimed
fixed by this batch.

## Actual fault repairs

- Correct69 waypoint emote mode mismatches using the loaded Emotes.dbc type.
  Hilda Stoneforge25235 uses waypoint action1120/emote16: the persistent-state
  flag was wrong for this one-shot animation. Reverse mismatches also previously
  discarded authored persistent animations; their state flag is restored.
- Repair the linked melee-weapon action for Riverpaw Scout500 and Mounted
  Ironforge Mountaineer12996. The linked child had a zero-range event instead
  of LINK; retain the parent range, action and targets.
- Remove invalid cross-links between Sanitron500's independently triggered
  respawn/reset/boarding events. Preserve the separate events; do not convert
  boarding into an unconditional spawn action.
- Restore missing authored dialogue from matching-name LoA reference creatures.
  The migration carries48 candidate rows across31 groups for affected actors;
  existing/custom keys win. It includes all available authored groups for each
  affected actor, avoiding a new missing-group error after restoring group0.
- Correct legacy quest reward8616 (The Affray1719): retain valid Berserker Stance
  learning2458 while removing only its missing Intercept20252 learn effect.
  This does not invent a replacement Intercept or modify ordinary combat spells.
- Generic learning wrappers483/55884 use a supplied base-point spell payload.
  An absent zero payload is not a spellbook entry; return before LearnSpell(0).
  Other invalid nonzero spell requests still reach the existing error checks.

World12 and13 are the clean paired data migrations. Changes were backed up in
ignored out/log-noise-20261112 and applied with the realm stopped. Repeated
application preserved text counts,69 corrected modes and repaired event wiring.
An initial empty-string hex encoding and overstrict linked-row guard were
caught during offline application and corrected in the delivered SQL. Some
world tables are MyISAM; do not assume transaction rollback covers those edits.

## Social chat adjustment

The earlier10000 variants combined20 openers with50 codas per category. The
owner's screenshot demonstrated the visible repetition of those openers. The
main general and dungeon flavor hooks now use115 individually written standalone
lines (90 general,25 party), without appended codas. The earlier library remains
available to its other hooks and as optional source material; do not describe
it as10000 independently written conversations.

Public quest play-by-play and poor/normal loot narration are disabled by default;
quest recruiting and uncommon-loot chatter are reduced. Social conversation,
trade and group finding remain enabled. This is a chat preference change, not
suppression of diagnostic errors.

Shared limits apply to successful bot broadcasts, not human chat or NPC dialogue:

- AiPlayerbot.Chat.ChannelMinInterval =30 seconds
- AiPlayerbot.Chat.ChannelMaxInterval =60 seconds
- AiPlayerbot.Chat.PerBotInterval =300 seconds
- AiPlayerbot.Chat.RepeatWindow =1800 seconds

The per-channel delay is randomized within that interval. Local channels are
scoped to faction and zone; guild/world/LFG use their corresponding broader
scope. A per-bot budget also prevents the same bot switching channels to spam.
The repeat key uses the opening sentence, so a different coda cannot evade it.
Successful sends consume the budget; a missing/unjoined channel does not.
Expired state is pruned, and a mutex protects concurrent map-thread access.
Companion whispers retain the existing quiet/explicit-reply policy.

Apply playerbots2026_11_12_02_standalone_social_chat.sql alongside the new native
hooks. Restart to refresh text caches/config. No accounts, quests, wallet or
inventory state is changed by the chat migration.

## Verification and limitations

The native x64 Visual Studio build completed with zero failed projects. The
standalone C++ policy regression passed channel and per-bot limits, cross-bot
repeat rejection, failed-send accounting, expiry and32-thread contention (one
successful sender). Build/run it with CMake from contrib/bot-chat/tests.
Data migrations passed idempotence and all targeted postconditions. New dialogue
was reloaded through the normal creature_text reload command.

The native reward/empty-payload fixture and subsequent runtime observation are
recorded separately under ignored out/log-noise-20261112. Do not infer gameplay
success merely from a cast command acknowledging that it queued a spell.
Longer rendered-client observation remains needed for animation appearance and
chat tone. Existing missing script implementations, loot references, empty
pools and obsolete saved objective data remain on the backlog.
No claim of zero total server errors is made.


The final spell fixture explicitly unlearned2458, confirmed its absence through
native spellbook inspection, cast8616 and confirmed2458 became known. Casting
an empty483 wrapper produced no invalid spell entry/error;0/20252 were absent
from saved spell rows. Owned fixtures were cleaned. An earlier fixture's direct
character_spell-row assertion did not match the native spellbook result; it is
not presented as evidence of a gameplay failure or durable spell-save coverage.

Four missing Gilneas graveyard links for zone4755 were restored from authored
LoA rows1692..1695, verified against WorldSafeLocs on map654. World14 is additive
and preserves existing keys/factions. Repeated application kept four links;
the ordinary graveyard reload activated the repair without another restart.


## Administrative follow-up and queued investigations

# Next work: Dalaran activity and auction scanning

Queued at the owner's request on November12. These are open tasks, not fixes
claimed by the current emote/spell/chat batch.

## Dalaran should feel as active as the other capitals

- [ ] Measure live bot count and faction distribution in Dalaran, rather than assuming the550 global target implies a populated city.
- [ ] Compare Dalaran's capital/travel-hub/service-NPC eligibility with Stormwind, Orgrimmar and Ironforge.
- [ ] Inspect available innkeeper, vendor, trainer, bank, mailbox and auction-service stops; do not send ordinary characters to unavailable services or the wrong faction district.
- [ ] Measure the proportion of present bots actively moving versus waiting, stuck or holding expired reservations.
- [ ] Check valid floor/path heights, stairs, portals and district restrictions before increasing local population.
- [ ] Reuse the logical city errand behavior with varied destinations, pauses and spacing; prevent bank/doorway piles and tight circling.
- [ ] Add a bounded/configurable city distribution or visit policy if lack of eligible bots is the cause.
- [ ] Verify a timed sample shows useful trips between services without teleports through floors, fixed furniture or fake auction transactions.

## Auctioneer scan performance

Owner observation: about6000 auctions previously scanned in90seconds and now
about7minutes. This is a report to reproduce, not a measured database bottleneck.

- [ ] Record Auctioneer version, scan mode (paged versus get-all), auction count, filters and elapsed time on the current server.
- [ ] Timestamp actual auction requests/replies, page sizes and deliberate pacing/rate limits.
- [ ] Compare native auction query/serialization time with database query time and result sizes.
- [ ] Check anti-abuse throttles and scan scheduling alongside client-side waits.
- [ ] Check auction cache/index behavior, simultaneous auction-bot work and server update delays.
- [ ] Change only the demonstrated bottleneck; retain anti-abuse safeguards and avoid guessing at database worker counts.
- [ ] Repeat the same mode/count baseline and document the resulting duration and normal gameplay responsiveness.

## Connection settings and GM vouchers — completed administrative follow-up

The active connection fields are grouped immediately below the worldserver and
authserver section headers. Network ports/bind address, database connection
strings and database worker/synchronous thread settings are together. The
PlayerbotsDatabase fields are included rather than buried near the end.
All active key/value pairs were compared before/after and remained identical.
The grouping helper is contrib/windows/Group-ConnectionSettings.py. Backups
contain credentials and remain in ignored local output. No restart is required
for a layout-only edit with unchanged values.

### Give an Orb of Loyalty to a player

Use an online GM account with permission to run .additem. Select the intended
online player in game, check that their name is the selected target, then run:

```
.additem 35626 1
```

For one full stack:

```
.additem 35626 20
```

The item is created in the selected player's inventory; it does not need to be
traded from a GM's bags. Each orb redeems for100 account coins;20 orbs represent
2000 coins if all are redeemed. Ensure the recipient has bag room.

Do not append a player name to this command: this implementation uses the
selected player. With no suitable player selected it can default to the GM.
The core has an intentional GM-self-grant unbinding exception; targeting the
actual recipient avoids relying on that exception and preserves the ordinary
soulbound grant path. This guidance is based on the actual cs_misc.cpp handler.
No items or coins were granted to user characters for this documentation change.


## Dalaran service visits and Auctioneer seller-cache delivery

- Fixed city-service selection incorrectly excluding usable neutral NPCs.
- Added Dalaran same-floor, line-of-sight and opposite-faction district checks.
- Retained three visitors per service and previous-destination avoidance.
- Replaced a no-service idle loop with bounded local wandering.
- Added a configurable24-visitor Dalaran surface target within the global550.
- Added finite1200-second city visits with normal service walking and pauses.
- Excluded human-led parties, instances, queues, combat, flights and vehicles.
- Avoided visible arrival/departure beside real players, including invisible GMs.
- Used authored neutral service arrival points with floor-height validation.
- Kept detailed city AI active without requiring a nearby observer.
- Added privileged read-only city census/state diagnostics.
- Added an unattended natural movement/service census with owned-fixture cleanup.
- Added a native paged/get-all auction benchmark with cold owner-name checks.
- Prefilled cached seller names before auction lists, deduplicated per response.
- Preserved auction query pacing, get-all cooldown and normal anti-abuse checks.
- Added Auction.PrefetchSellerNames for independent rollback.
- Measured6642 listings/133 pages in45.263s baseline and45.188s with prefetch.
- Verified1241 of1241 seller names prefetched before listings.
- Measured get-all6641 listings in0.292s, with complete seller resolution.
- Kept rendered Auctioneer completion time as an explicit acceptance limitation.
- Required no new SQL migration; refreshed complete source and working configs.
- Preserved WARN/ERROR reporting and documented unrelated content diagnostics.

See dalaran-auction-followup-2026-11-12.md for final city evidence and rollback.

Final Dalaran360-second acceptance passed:24 surface bots;12 moved>25yd;19 reached22 distinct services;0 large floor offsets. Visual Studio build passed and matching EXE/PDB deployed to exports.
