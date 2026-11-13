# Quest dependency audit baseline — November 8, 2026

This is a read-only dependency census, not gameplay certification. No database
records, account data, runtime binaries, logging thresholds or server processes
were changed. Native deployment remains b7a409b3.

## Results and artifacts

- 18,145 quests, 17,682 objective rows and 98,115 dependency links examined.
- 1,037 finding links affect 587 quests, grouped into 246 repair queue entries.
- Ten missing objective definitions: two creatures, seven items and one gameobject.
- 1,027 links involve actor bindings reported unregistered in the startup log.
  Several links can concern one quest; these are not 1,027 unique broken quests.
- All 68 initially suspicious reward-spell links are supplied by spell_dbc.
  They are valid server-store references, not missing spells.

Files:

- [quests.csv](quests.csv): quest restrictions and dependency counts.
- [findings.csv](findings.csv): issues attributed to quests, targets and sources.
- [repair-queue.csv](repair-queue.csv): shared actor/definition repair groups.
- [summary.json](summary.json): source revision, SELECT-input fingerprints,
  client/diagnostic hashes, counts and output SHA-256 values.
- Full graph: ignored local `out/quest-dependency-audit-548/edges.csv`.
  Regenerate it below; its hash is recorded in summary.json. Input fingerprints
  identify a baseline; they cannot replace database backups.

## Reproduction and validation

From the repository root:

```powershell
python contrib/content-audit/test_catalogue.py
python contrib/content-audit/test_quest_dependencies.py
python contrib/headless-client/test_qa.py
python contrib/content-audit/quest_dependencies.py
```

Validation: 11 catalogue parser tests, 12 graph tests and 13 gameplay framework
tests pass (36 total). Live read-only generation completed. Tests cover addon
precedence, signed prerequisite IDs, exclusive groups, missing objective
attribution, unspawned actors, orphan relations, unsupported pet semantics,
database-only spells and positive native-registration evidence. The DB reader
was repaired to preserve an empty final column instead of stripping its trailing
tab; its regression test reproduces an empty ScriptName. No native rebuild or
gameplay rerun was needed for these audit-only changes.

## Interpretation and limitations

Quest::LoadQuestTemplateAddon determines prerequisite/source-spell, class and
exclusive-group fields. Legacy quest_template aliases are not a second truth.
Negative PrevQuestID retains its sign and references the absolute quest ID;
exclusive-group numbers are not quest IDs.

The ten missing targets need version/availability review, not fabricated items or
similarly named replacement actors. Availability remains unknown, including
disabled, obsolete, seasonal and custom content. Native findings derive from a
hashed startup log, not an in-process registry snapshot; logs can include history.
Reconfirm against the running build before restoring a handler. A missing special
handler can leave generic gossip functional; linked quests are not all proven failed.

6,162 quests lack an explicit NPC/GO/item starter and 5,341 lack an NPC/GO turn-in.
Those are review counts, not failures: auto-accept, scripted starts, area triggers
and auto-completion exist. Zero fixed spawns does not prove a summoned actor is
absent. Conditions, phase/visibility, summons, item-acquisition paths, auxiliary
reward-display tables, pet/money/playerkill/area objectives and actual script
execution need dedicated checks. SELECTs are sequential; quest definitions are
rechecked at the end, but not every table is transactionally snapshotted.

Repair grouping allows one shared-dependency fix to address several quests.
Chen Stormstout56133 links20 quests, Sassy38387 links13, and unspawned custom
actor190002 links116. Raw counts must not prioritize custom unspawned content
above active racial progression. No playable-completeness percentage is claimed.

## Public references and repair workflow

The [master list](../content-master-list-548.md) records the pinned LoA peer
database and client provenance. Public candidates include
[Wowhead MoP Classic quest lists](https://www.wowhead.com/mop-classic/quests/kalimdor)
and [Warcraft DB](https://mop.warcraftdb.com/). These are not yet imported as
independent expectations. Record each contract's quest ID, URL, version, expected
steps/rewards, confidence and independence from our own database. Classic and
original5.4.8 must not be assumed identical; no unreviewed bulk overwrite.

1. Prioritize active starter/progression blockers and shared dependencies.
2. Establish intended behavior from matching source and external evidence.
3. Before SQL changes, back up affected rows/tables, record hashes, and prepare
   scoped migration/restore instructions. Keep private dumps out of Git.
4. Apply repairs and run structural checks plus normal-account gameplay contracts.
   Unsupported behavior stays untested.
5. Record before/after results, source/migration commit, deployed EXE/PDB hashes
   when rebuilt, remaining gaps and rollback instructions. Push Forgejo and
   verify the GitHub mirror.

These tools require only a scoped Git revert to roll back; no SQL restore.
Use separate --output and --artifacts directories for future baselines. Git
preserves tracked findings; fewer findings alone do not prove better gameplay.

Next: phase/condition dependencies, item-acquisition paths, independent racial
starter expectations and actual objective gameplay coverage. BattlePay, companions
and auction house keep separate regression requirements; this audit certifies none.
