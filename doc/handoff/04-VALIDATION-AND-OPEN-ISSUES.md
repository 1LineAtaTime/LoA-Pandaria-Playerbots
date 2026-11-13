# Evidence and remaining work

## Evidence carried forward from development

| Check | Recorded result | Limit |
|---|---|---|
| Visual Studio final native build | x64 RelWithDebInfo, zero failed projects | Three Windows revision-macro warnings remain |
| Native CTest suite | 24/24 pass | Focused policy/accounting/CLI tests, not whole-world play |
| Final headless/action tests | 37 pass | Protocol/harness unit checks |
| Diagnostic collector tests | 3 pass | Parsing, deduplication and missing-log behavior |
| Last LoA deployment reference checks | 7/7 pass | Patrols, bindings, cave height, duplicate removal, linked POIs |
| Expanded gameplay batch | 20 passed, one combat-budget failure, eight untested | Original failure retained; not every quest |
| Focused ordinary Human combat | Six real kill credits and saved complete quest | Original end-to-end workflow still failed at logout |
| Final helper acceptance | Ordinary Orc lifecycle and one-kill Human logout/re-entry/cleanup pass | Not a replacement full six-kill workflow pass |
| BattlePay graphical acceptance | Balance, affordable purchase, native insufficient-funds UI and cancelled-confirmation reuse confirmed | Not an exhaustive delivery/service/load certification |
| Export check | Pinned upstream patch apply check and exported 37+3 Python tests pass | Privacy/template changes are not a new native rebuild |

Original detailed technical reports are retained in sanitized form under the
overlay's `doc/`; bulky reference ledgers are in `reference-data.zip`. Raw private
logs, character snapshots, packet captures and dump files are not included.
Reported acceptance is summarized, not recreated from private test accounts.

## High-priority open work

1. **Content/native bindings:** Many startup/runtime script, condition and content
   mismatches remain. One snapshot contained 1,181 unique exact diagnostic
   messages; this is not 1,181 independent bugs, and later activity can add more.
   Do not claim a warning-free startup or hide the failures to obtain one.
2. **Structural references:** The latest broad audit retained two missing
   creature objective references, seven items, one gameobject and one Crowley
   SmartAI/native overlap. The overlap needs semantic ownership review, not a
   blind switch back to SmartAI. Some referenced IDs are from newer content and
   require version reconciliation, not fabricated templates.
3. **Movement/bots:** City crowding, empty-city distribution, terrain clipping,
   circling, flight following and all flying NPC routes still need gameplay work.
   Existing guards and CityLife changes do not certify those systems.
4. **Natural progression:** Full Worgen/Undead/Pandaren and other race/class
   chains are not certified. Controlled cage/vehicle/quest fixtures do not prove
   naturally arriving at those checkpoints or interacting with every phase.
5. **Gameplay coverage:** Professions, broad spell mechanics, dungeon/raid bosses,
   all quest-item targets, collection UI and auction settlement lack comprehensive
   independent end-to-end oracles. The inventory of content is not execution.
6. **Newer SkyFire work:** Large Darkshore commit `53e1b75bc1` and the generic
   standing-hostile engagement portion of `b53c84a8e4` need LoA API/data adaptation.
   The Northshire worg part was ported. The text-field parsing fix is already
   equivalent locally. This is not every SkyFire release transplanted wholesale.
7. **Harness:** Repeat the full six-kill workflow with final logout handling.
   Preserve earlier failures, environmental assumptions and per-run cleanup
   results; do not relabel a partial objective pass as full workflow success.
8. **Store:** Stackable physical donation-token redemption is intentionally
   unsupported. Invalid/refund-overflow cases can remain pending for review.
   Client UI/service variations and recovery under real load need more coverage.
9. **Release work:** Parameterize local QA wrappers, validate Linux, resolve
   compiler warnings and produce a fresh-install deployment test on the
   maintainer's matching world baseline. This handoff is not that installer.

The remaining checklist is also in sanitized `doc/project-checklist.md`.

## November12 original-world update

See [runtime diagnostic repairs](../doc/runtime-log-repairs-2026-11-12.md) and [Gilneas finale vehicles](../doc/gilneas-finale-vehicles-2026-11-12.md). Glaive boarding/spam/exit, actual weapon damage and one infantry objective credit, timed hippogryph boarding and ship arrival, and actual torch use were observed. Full natural quest combat/reward and ship-fixture logout acknowledgement remain distinct open checks. Remaining loot/pool data errors are listed explicitly; no zero-error claim is made. Native escape boarding now replaces the credit-without-ride spell path. A seeded enlarged-health fixture returned to harbor and persisted completed26706 status through the native timed scene; normal combat and full turn-in remain separate checks.

## Keel Harbor durability and stock follow-up

See [Keel Harbor follow-up](../doc/keel-harbor-followup-2026-11-12.md): vehicle health, encounter respawns, missing merchant inventories and Endgame prerequisite eligibility. The nearby mailbox remains unconfirmed and unchanged.

## Gunship client-crash and rope follow-up

See [client-crash evidence and repairs](../doc/gunship-client-crash-2026-11-12.md). The real client now renders the corrected map-object ship without Error132; the user confirmed one ship and a working rope, then continued fighting aboard. Apply migrations03 and04 with the paired native rope repair. Full normal combat and turn-in remain separate checks.

## November 12: wyvern control and turn-in

The rebuilt native server passed the vehicle-camera lifecycle, post-landing walking, normal Lorna reward dialogue and actual quest reward regression. See doc/wyvern-camera-control-2026-11-12.md. This measures packet and server state, not visual camera framing. The existing player had already rewarded Endgame and was not reset.

## November 12: departure and lifetime repairs

SQL05 restores the Rutâ€™theran bind/teleport and valid addon emotes; deploy with the matching Nightwind script. The final build passed ordinary departure, retry, Krennan dialogue/next quest acceptance, five companion add/remove cycles, normal former-bot login, four duplicate time-sync responses and acknowledged logout, plus short/generated/malformed SRP cases. All owned fixtures removed. See doc/nightwind-ai-lifetime-2026-11-12.md for the two reproduced crash/hang repairs and limits of the overnight spline diagnosis.

## November 12: Howling Oak and Tarien

SQL06 restores one missing Genn beside Gwen and fills Tarienâ€™s missing jewelcrafting stock, preserving costs and existing custom keys. Normal quest acceptance/reward and a92-item merchant response passed. No new binary required. See doc/howling-oak-tarien-2026-11-12.md; the optional welcome roleplay scene and retail-exact stock comparison remain separate work.

SQL07 follow-up: Idrianaâ€™s bank menu action restored and hot-reloaded. Actual menu selection returned SMSG_SHOW_BANK; fixture/account removed. No native rebuild needed.

## November12 naming and dungeon follow-up

Curated naming:20,000 candidates imported; existing880 bots renamed with
successful offline rollback/reapply and protected fingerprints;4,120 additional
native characters create a5,000-bot stored roster on455 accounts, max11/account.
All317 original nonmanaged identities remain unchanged. Native creation/racing
claims, hint fallback/global exhaustion, resumable provisioning and a final
5,000-row restart identity comparison passed.550 bots held for90seconds;
peak working set4.40GB and average16.56% of host CPU. This is a short capacity
observation, not a full combat/dungeon soak. The race test exposed a rejected
Player session pointer; it was fixed, rebuilt and the test/cleanup rerun.

The later Gate of the Setting Sun crash is separate: missing instance binding
caused a null controller dereference in bombardier UpdateAI. Migration08
restores the binding and80 missing authored objects; native null/empty-target
guards compile. See doc/setting-sun-crash-2026-11-12.md for the
final live regression and limitations. A rendered bridge traversal remains open.

Read contrib/playerbot-names/README.md and the cumulative naming
handoff for import/rollback/provision controls. Clean schema and candidate SQL
are included. Private backup/rename/account evidence stays outside this package.
The previously approved config/*.conf credential exception still applies;
remove those full working configs before publishing upstream.


## Final November12 native/data follow-up

- Four real roster companions: invite/share silence, human leader, .partyloot,
  native LFG formation and online/offline removal passed.
- Explicit deferred position command reply passed on a real roster companion;
  quest sharing immediately afterward remained quiet. Owned fixtures cleaned.
- Orb native item-use tests passed stack splitting, whole-DP credit, overflow,
  last-item consumption, consumed-item replay and forced transaction rollback.
- Native Orb item/sparse cache packets passed quality5, SellPrice0, stack20 and
  exact description; both hotfix invalidation rows installed. Fixture cleaned.
- Actual mounted Yak passenger queries returned14 merchant entries and a
  successful reforger result. Stationary replacement NPCs were not used.
- Chat data imported twice with11707 total rows and10000 new unique variants;
  known English punctuation corruption repaired. Offline hook/category/length
  validation passed. The new library does not raise public event frequencies.
- Visual Studio x64 RelWithDebInfo builds completed with zero failed projects;
  deployed worldserver EXE/PDB are hash-matched to those build outputs.

Remaining: rendered dungeon route and service/tooltip acceptance, longer550-bot
combat/dungeon soak, mixed-human vote matrix, actual loot-roll outcome coverage
and normal scheduling/naturalness of dungeon flavor. Other original-world
SmartAI, graveyard, missing creature text and inventory/quest diagnostics remain
visible; this batch does not claim that all server content errors are gone.


## Runtime-noise/social-chat follow-up

World12 repairs69 emote command modes and three NPC event chains. World13
restores48 matched-reference dialogue candidates across31 groups, preserving
existing keys. World14 restores four authored Gilneas graveyard links, with
matching map654 WorldSafeLocs verified. Reloads activated added dialogue and
graveyards. Other original-world missing implementations/data remain open.

The native reward fixture unlearned2458, confirmed native absence, cast8616,
confirmed2458 became known and exercised an empty483 learning payload. No
invalid0/20252 spell row or new missing-spell error appeared. Fixture cleanup
passed. An earlier direct-row assertion was not a reliable proxy for native
spellbook state; durable reward persistence is not claimed by this test.

The chat throttle's native regression passed failed-send accounting, channel,
per-bot and repeat budgets, expiry and32 concurrent senders. General and party
social hooks now use115 standalone lines. Public quest play-by-play is off;
public broadcasts share30–60second channel/300second bot gaps and1800second
opening-sentence repeat protection. Human/NPC chat is outside that limiter.
