# Restore the original mop_ database runtime

The owner selected the original existing databases as the default for both live
exports configs and the separate handoff's full working configs:
`mop_auth`, `mop_characters`, `mop_world`, `mop_playerbots`, and `mop_log`.
The alternative `mop_loa_` and `mop_sf_` sets were later archived and removed
at the owner's request; see database-cleanup-2026-11-11.md. The supplied
credentials exist only in ignored runtime files and the explicitly approved
private handoff configs. Remove the handoff's config/*.conf before public upload.
Source configuration templates remain sanitized. The account password was not reset.

## Compatibility changes and rollback

Backups and the exact applied-file journal are under the ignored directory
`out/backups/mop-schema-compat-20261110/`. Earlier config copies are under
`out/backups/mop-prefix-config-*`. Stop worldserver before restoring backed-up
tables or old configs. Do not restore clean base SQL over existing account saves.

Applied published migrations:

- `2026_10_30_1046_open_world_damage_modifier.sql`: required DamageModifier field
  and its upstream absolute values, backed up with creature difficulty data.
- Playerbots `2026_10_24_00.sql`: additive missing account-type registry table;
  no existing account assignments copied from another environment.
- `2026_10_29_989_07_dungeon_normal_gold.sql`: nullable difficulty-specific gold
  overrides required by this executable and paired normal-dungeon gold values.
- `2026_11_07_53_world_loa_beta_schema.sql`: canonical gossip keys and normalized
  locale tables, retaining legacy translation tables.
- `2026_11_07_55_world_loa_locale_preservation.sql`: preserve old translations in
  normalized tables; existing normalized translations win.
- `2026_11_11_00_world_original_mop_compat.sql`: move equipment variant ID zero to
  the next unused positive ID for that creature, preserving all 20,121 rows and
  every existing positive-ID reference. Equipment/gossip backups precede changes.

Transport phasing columns were also added with the zero defaults specified by
`2026_10_30_1056_00_transports_phase_columns.sql`; all 27 existing transport rows
were preserved, with a separate pre-change transport backup.

## Native startup crash

A matching crash report identified RandomItemManager::Init line 118: a quest
reward item lookup returned null and the bot equipment cache dereferenced it.
Both guaranteed and choice reward loops now check before reading the template,
report the invalid quest/item pair and exclude it from the cache. No quest
reward records were deleted or fabricated. Missing item IDs 5396, 11000, 12144,
13704 and 22486 also do not exist in the newer local reference world. Their
content repair remains separate from the crash fix.

The regular launcher now accepts either supported, internally consistent prefix
rather than rejecting the owner's original mop_ configuration. It still checks
auth/world agreement, expected ports, realm build and deployed executable hash.

## Scope

This restores the original data set, not a promise that its older world content
has every newer quest/script repair. Loader diagnostics remain enabled. Legacy
world script bindings, loot references and addon emotes need a separate data
reconciliation; no blanket replacement of the owner's catalogue was performed.
Final build and live smoke results are recorded below after deployment.

## Deployment and validation results

Visual Studio built worldserver RelWithDebInfo/x64 successfully with zero failed
projects. The new EXE and matching PDB are in exports; their SHA-256 hashes are
recorded in ACTIVE-RUNTIME-MANIFEST.json. Existing authserver and dependency DLLs
were retained. All 24 existing CTests passed. No full-game QA is claimed.

One visible authserver/worldserver pair reached READY on ports 3724/8085 using
only the selected original schemas. A disposable ordinary account authenticated,
created a Human Warrior, enumerated and immediately entered the world, and logged
out. Native cleanup removed that exact test character, its dependencies and then
its account. The private report is under
out/quest-qa/original-mop-runtime-20261111/report.json.

The original account set and character records remained present; the original
shop retains 615 products, 659 entries and 11 groups. Existing user passwords,
wallet balances, purchases and mail were not edited by this restoration. Bot
login messages confirm the original playerbot population is active.

This is a successful database/runtime restoration and bounded login check, not
a declaration that old world content is clean. The original world still reports
invalid script bindings, loot references and addon data; these are visible and
need a later backed-up content reconciliation. No diagnostic suppression was
added. Shop purchasing was not retested against a real user's wallet this pass.
