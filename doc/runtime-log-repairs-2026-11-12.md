# Original-world runtime diagnostics repair

This batch repairs data and native behavior without raising logger thresholds.
Warnings/errors remain visible; detailed SQL diagnostics remain in DBErrors.log.

Applied world migrations:

* Existing `2026_11_04_09_world_legacy_script_broadcast_text.sql` fills149
  missing legacy broadcast IDs from existing authored db_script_string text,
  including locale rows. Existing broadcast text wins; no invented dialogue.
* `2026_11_11_10_world_torch_and_script_compat.sql` installs the historical
  Graveyard Rat37889 response to Half-Burnt Torch70631 and its entry filter.
  XT002's periodic dummy script is bound to62791, not its base aura62789.
  SmartAI fallback names are filled only where script rows exist and AIName is
  blank; native ScriptName bindings remain intact and retain precedence.
* `2026_11_11_11_world_missing_mop_loot_references.sql` restores68 complete
  reference groups (136 rows) from the archived matching LoA world. Existing
  groups win; all referenced items were present and below required level100.
  Original chances and row grouping are retained.
* `2026_11_11_12_world_obsolete_runtime_metadata.sql` removes388 completely
  unreferenced empty pool definitions and23 unsupported command overrides.
  It clears invalid persistent one-shot addon emotes and invalid dialogue emotes
  that the loader already rejected. Related empty pools are preserved for repair.

Native commit1673e7e621 treats item0 with negative mincountOrRef as a loot
reference, preserving negative currency entries. It also guards the XT002 heart
callback against a detached vehicle base during teardown.

The root worldserver configuration now contains the17 documented city-errand
settings that are read by sConfigMgr, preserving existing custom values. A
sidecar playerbots configuration alone did not satisfy those root lookups.
No verbosity thresholds were changed.

Private rollback material: `out/backups/log-repair-20261111/world-before.sql.gz`
(31,068,734 bytes), before-logs, config copies and the matching old EXE/PDB.
Affected tables include text/locales, templates, SmartAI/conditions, spell
bindings, loot references, pool definitions, commands, addons and quest details.
Several tables are MyISAM: use stopped-server backups for rollback, not an
assumption that a SQL transaction can undo all writes. These backups are excluded
from the clean handoff and Git.

Fresh-start inventory after this batch: SQL diagnostic entries fell from4295
to3152. There are still1464 empty pools with relationships,511 missing-reference
uses (28 unresolved IDs50021–50048),149 missing creature-loot template uses,
98 missing pickpocket-template uses, and other legacy metadata problems. The
runtime log also identifies six absent quest reward items and eight obsolete
saved objective IDs on three random bots. These remain unresolved, not suppressed.
No new legacy broadcast-text missing or62789 hook mismatch occurred in the
observed startup. This is not an all-content or zero-error certification.

Torch gameplay check: a disposable Worgen with quest24678 used the actual
item50220 against its isolated rat; health changed31 to0 and the threat was
removed. The account, character and dependent records were deleted afterward.
The first probe failed because it sent a zero item GUID: HandleUseItemOpcode
rejects that as ITEM_NOT_FOUND. The QA action now sends the persisted typed item
GUID. No rat flags, GM cast or GM kill were needed for the passing item test.

The QA observer also now consumes SMSG_DESTROY_OBJECT. Previously it retained
despawned actors in its visible cache and falsely reported that vehicle cleanup
had failed. Both fixes affect the headless QA tool, not the game client.
